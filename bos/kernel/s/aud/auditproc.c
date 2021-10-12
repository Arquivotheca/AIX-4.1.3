static char sccsid[] = "@(#)88        1.19.1.5  src/bos/kernel/s/aud/auditproc.c, syssaud, bos411, 9428A410j 3/10/94 16:09:09";

/*
 * COMPONENT_NAME: (SYSSAUD) Auditing Management
 *
 * FUNCTIONS: auditproc() system call
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#include	<sys/types.h>
#include	<sys/user.h>
#include	<sys/errno.h>
#include	<sys/var.h>
#include	<sys/malloc.h>
#include	<sys/audit.h>
#include	<sys/auditk.h>
#include	<sys/priv.h>
#include	<sys/lockl.h>
 
/*
 * Forward declarations 
 */

static int GetClasses(unsigned long *, char *, int);
int SetClasses(unsigned long *, char *, int);
static int ParseClasses(unsigned long *, char *, int);
static int FindClass(char *);
static struct ProcClasses_t *FindUser(uid_t);
static int RegisterUser(char *, int);
int AuditProcs();


/*
 * Structure for dynamically setting processes
 * procmask's through the resource handler audit_proc
 */

static struct ProcClasses_t {
	uid_t uid;
	char Classes[512];
	int  ClassLen;
	struct ProcClasses_t *next;
	struct ProcClasses_t *prev;
};
static struct ProcClasses_t *ProcClassAnchor;


/*
 * NAME: auditproc()
 *
 * FUNCTION: Gets or sets the audit state of a process.
 *
 * INPUTS :
 *	pid :	Process Id of the process to be affected. If it is 0 the
 *	     	system call affects the current process.
 *	cmd :	Specifies the action to be taken. The valid values are:
 *		AUDIT_QEVENTS : returns the list of audit events audited
 *		for the current process
 *		AUDIT_EVENTS : Sets the list of audit events to be audited
 *		for the process
 *		AUDIT_QSTATUS : returns the audit status of the current
 *		process.
 *		AUDIT_STATUS : sets the audit status of the current process.
 *	len :	specifies the size of the audit class buffer.
 *
 * INPUTS/OUTPUTS : 
 *	arg :	specifies a character pointer for the audit class buffer
 *		for an AUDIT_EVENTS or AUDIT_QEVENTS command or an integer
 *		defining the audit status to be set for an AUDIT_STATUS
 *		operation.
 *
 * ERRORS: 
 *	EPERM 	if the calling process doesn't have SET_PROC_AUDIT 
 *		privilege.
 *	EINVAL for invalid values.
 *
 * TYPE: int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.
 *	NONE
 *
 * RETURNS:
 *	-1 on error
 *	0 on success
 *
 */
int
auditproc(int pid, int cmd, char *arg, int len){

	unsigned long	*amp;
	struct	proc	*pp;
	int	rc = 0;

	/*
	 * Privilege check 
	 */

	if(privcheck(SET_PROC_AUDIT)){
		u.u_error = EPERM;
		return(-1);
	}

	/*
	 * Grab lock
	 */

	simple_lock(&audit_lock);

	/*
	 * A pointer to the auditmask field is set:
	 * if pid is specified then 
  	 * the proc table is searched to find the 
	 * correct process's auditmask.
	 */

	if((cmd == AUDIT_QEVENTS) || (cmd == AUDIT_EVENTS)){

		amp = (unsigned long *)0;

		if(!(pid)){

			amp = &(U.U_procp->p_auditmask);
		
		}
		else {

			pp = VALIDATE_PID (pid);

			if(pp != NULL){

				amp = &(pp->p_auditmask);
			}
		}

		if(amp == (unsigned long *)0){

			u.u_error = EINVAL;
			rc = -1;
			goto out;

		}
	}

	switch (cmd){

		/*
		 * Return list of classes this process
		 * is currently being audited for.
		 */

		case AUDIT_QEVENTS:

			if(GetClasses(amp, arg, len) < 0){

				rc = -1;
				goto out;

			}

			break;

		/*
	  	 * Initialize this process's bitmap of classes.
		 * A side effect of this is that auditstatus is set
		 * to AUDIT_RESUME (if we are dinking with the current
		 * process).  This is so that, in most cases, only 
		 * one call is necessary to set up a user (at login).
		 */

		case AUDIT_EVENTS:

			if(len){

				SetClasses(amp, arg, len);

				/*
				 * Set up proc mask 
				 * for user
				 */

				if(!RegisterUser(arg, len)){

					u.u_error = EINVAL;
					rc = -1;
					goto out;

				}

			}

			else {
				*amp = 0;
			}

			if (!(pid)){

				U.U_auditstatus = AUDIT_RESUME;  

			}
			break;

		case AUDIT_QSTATUS:

			if(pid){

				u.u_error = EINVAL;
				rc = -1;
				break;

			}
			rc = U.U_auditstatus;
			break;

		/*
		 * Initialize this process's bitmap of classes 
		 */

		case AUDIT_STATUS:

			if(pid){

				u.u_error = EINVAL;
				rc = -1;
				break;

			}

			rc = U.U_auditstatus;
			if((int)arg == AUDIT_SUSPEND){

				U.U_auditstatus = AUDIT_SUSPEND;

			}
			else {

				if((int)arg == AUDIT_RESUME){

					U.U_auditstatus = AUDIT_RESUME;

				}
				else {
					u.u_error = EINVAL;
					rc = -1;
				}
			}

			break;

		default:

			u.u_error = EINVAL;
			rc = -1;

	}

out:

	simple_unlock(&audit_lock);
	return(rc);

}


/*
 * NAME: GetClasses
 *
 * FUNCTION: Get all the classes specified in the audit mask into
 *	     the buffer.
 *
 * INPUT: 
 *	amp :	process's audit mask
 *	len : 	length of the buffer list
 *
 * OUTPUT:
 *	list :	buffer which holds the audit classes.
 *
 * ERRORS:
 *	EFAULT if sanity check fails
 *	ENOSPC if the total length of all class names exceeds the 'len'
 *		parameter.
 *
 * TYPE: static int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures
 *	class_names array is referenced.
 *
 * RETURNS:
 *	-1 on error
 *	0 on success
 *
 */
static
int
GetClasses(unsigned long *amp, char *list, int len){

	int	i;
	int	tlen;

	for(i = 0, tlen = 0; i < 32; i++){

		if(*amp & (1<<i))tlen += strlen(class_names[i])+1;

	}

	tlen++;

	if(tlen > len){

		if(copyout(&tlen, list, 4)){

			u.u_error = EFAULT;
			return(-1);

		}

		u.u_error = ENOSPC;
		return (-1);

	}

	for(i = 0, tlen = 0; i < 32; i++){

		if (*amp & (1<<i)){

			if(copyout(class_names[i], &(list[tlen]), 
			strlen(class_names[i]) + 1)){

				u.u_error = EFAULT;
				return(-1);

			}

			tlen += strlen(class_names[i]) + 1;

		}
	}

	if(subyte(&list[tlen], 0) < 0){

		u.u_error = EFAULT;
		return(-1);

	}

	return (0);

}


/*
 * NAME: SetClasses
 *
 * FUNCTION: set the audit mask of the process to the list of classes
 *	     given in input.
 *
 * INPUT:
 *	list :	list of audit classes
 *	len :	length of buffer which holds audit classes to be set.
 *
 * OUTPUT:
 *	amp :	audit mask to be set
 *
 * ERRORS:
 *	ENOMEM if malloc fails
 *	EFAULT if sanity check fails
 *
 * TYPE: int 
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures
 *	NONE
 *
 * RETURNS: 0
 *
 */
int
SetClasses(unsigned long *amp, char *list, int len){

	char	*klist;

	*amp = 0;

	if(len == 0)return;

	klist = malloc(len);
	if(klist == NULL){

		u.u_error = ENOMEM;
		return;

	}

	if(copyin(list, klist, len)){

		u.u_error = EFAULT;
		goto fail;

	}

	ParseClasses(amp, klist, len);

fail:

	free(klist);

}


/*
 * NAME: ParseClasses
 *
 * FUNCTION: set the auditmask by finding the index of each
 *	     class given as input from the class_names array.
 *
 * INPUTS:
 *	klist :	list of audit classes in kernel space
 *	len   : buf len
 *
 * OUTPUTS:
 *	amp : audit mask to be set
 *
 * ERRORS:
 *	EINVAL	if the number of classes existing is less than 0
 *
 * TYPE: static int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.
 *	NONE
 *
 * RETURNS: 0
 *
 */
static
int 
ParseClasses(unsigned long *amp, char *klist, int len){

	int	i;
	int	NumClasses;
	int	bitpos;
	char	*lp;

	NumClasses = CountEvents(klist, len);

	if(NumClasses < 0){

		u.u_error = EINVAL;
		return;

	}

	for(i = 0, lp = klist; i < NumClasses; i++){

		if((bitpos = FindClass(lp)) >= 0){

			*amp |= (1<<bitpos);

		}

		lp += strlen(lp) + 1;

	}

}


/*
 * NAME: FindClass
 *
 * FUNCTION: returns the array index of class name specified.
 *
 * INPUTS :
 *	s : class name string
 *
 * OUTPUTS : NONE
 *
 * ERRORS: NONE
 *
 * TYPE: int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.
 * NONE
 *
 * RETURNS:
 *	array index on success
 *	-1 on error
 *
 */
static
int
FindClass(char *s){

	int	i;

	for(i = 0; i < MAX_ANAMES; i++){

		if(class_names[i][0] == 0){

			continue;
		}

		if(strcmp(class_names[i], s) == 0){

			return(i);
		}
	}

	return(-1);
}


/*
 * NAME: FindUser
 *
 * FUNCTION: find the user correspoding to 'uid' in the list of 
 *	     ProcClasses_t. If not found allocate memory for the
 *	     new struct and make  uid field equal to 'uid'.
 *
 * INPUTS :
 *	uid : user id to be found
 *
 * OUTPUTS : returns the structure ProcClasses_t whose uid is 'uid'
 *
 * ERRORS: NONE
 *
 * TYPE: static struct ProcClasses_t *
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures.
 *	NONE
 *
 * RETURNS:
 *	pointer to the structure ProcClasses_t on success
 *	NULL if fails
 *
 */
static
struct ProcClasses_t
*FindUser(uid_t userid){

	struct ProcClasses_t *Ptr;

	/*
	 * First time
	 */

	if(!ProcClassAnchor){

		if((ProcClassAnchor = (struct ProcClasses_t *)
		malloc(sizeof(struct ProcClasses_t))) == NULL){
			
			return(NULL);

		}

		bzero(ProcClassAnchor, sizeof(struct ProcClasses_t));

		ProcClassAnchor->uid = userid;

		return(ProcClassAnchor);

	}
	else {
	
		Ptr = ProcClassAnchor;
	
		while(Ptr){
	
			if(Ptr->uid == userid){
	
				return(Ptr);
	
			}
	
			Ptr = Ptr->next;
	
		}
	
		/*
		 * Not found so add
		 */

		if((Ptr = (struct ProcClasses_t *)
		malloc(sizeof(struct ProcClasses_t))) == NULL){
			
			return(NULL);
	
		}

		bzero(Ptr, sizeof(struct ProcClasses_t));
	
		/*
		 * Set 
		 */

		Ptr->uid = userid;

		/*
		 * Insert 
		 */
	
		ProcClassAnchor->prev = Ptr;
		Ptr->next = ProcClassAnchor;
		ProcClassAnchor = Ptr;

		return(Ptr);
	}
}



/*
 * NAME: RegisterUser
 *
 * FUNCTION: Finds entry of user inb the linked list of ProcClasses_t
 *	     structures and copies classes to the structure. If not
 *	     found adds a new structure.
 *
 * INPUT:
 *	Arg :	list of classes
 *	len :	length of Arg buffer
 *
 * OUTPUT: None explicitly
 *
 * ERRORS: NONE
 *
 * TYPE:static int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures.
 *	NONE
 *
 * RETURNS:
 *	1 on success
 *	0 on failure.
 *
 */
static 
int
RegisterUser(char *Arg, int Len){

	struct ProcClasses_t *Ptr;

	/*
	 * Finds entry of user and his procmask
	 * with bits indicating current auditclasses 
	 * Adds if not found to head of linked list
	 */

	if(Ptr = FindUser(GETRUID())){
		
		/*
		 * Stuff structure with Class info
		 */
		 
        	if(copyin(Arg, Ptr->Classes, Len)){

			return(0);

		}

		Ptr->ClassLen = Len;

	}

	return(1);

}


/*
 * NAME: AuditProcs
 *
 * FUNCTION: For each process in the process table, set the audit mask of
 *	     the process according to the classes in ProcClasses_t linked
 *	     list
 *
 * INPUT: NONE
 *
 * OUTPUT: NONE
 *
 * ERRORS: NONE
 *
 * TYPE: int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.
 *	audit mask of all processes is set.
 *
 * RETURNS: NONE
 *
 */
int
AuditProcs() {
    struct ProcClasses_t *Ptr;
    struct proc *p;

    for (p = (struct proc *)&proc[2]; p < max_proc; p++) {

	    /* It is not clear if the proc_tbl_lock is needed here */
	simple_lock(&proc_tbl_lock);

	    /* Check for non-empty slot and get user */

	if ((SNONE != p->p_stat) && (Ptr = FindUser(p->p_uid)) 
	    && (Ptr->ClassLen > 0)) {

		/* First set auditmask to zero and then pass to ParseClasses
		 * since it does a |= 
		 */

	    p->p_auditmask = 0;
	    ParseClasses(&p->p_auditmask, Ptr->Classes, Ptr->ClassLen);
	}
	simple_unlock(&proc_tbl_lock);
    }
}

