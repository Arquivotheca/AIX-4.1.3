static char sccsid[] = "@(#)86        1.22.1.3  src/bos/kernel/s/aud/auditevent.c, syssaud, bos411, 9428A410j 2/24/94 17:00:03";

/*
 * COMPONENT_NAME: (SYSSAUD) Auditing Management
 *
 * FUNCTIONS: auditevents() system call
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
#include	<sys/errno.h>
#include	<sys/user.h>
#include	<sys/proc.h>
#include	<sys/var.h>
#include	<sys/systm.h>
#include	<sys/malloc.h>
#include	<sys/sleep.h>
#include	<sys/audit.h>
#include	<sys/auditk.h>
#include	<sys/pri.h>
#include	<sys/priv.h>
#include	<sys/lockl.h>

static char class_tmps[MAX_ANAMES][16];
static int CheckEveLock();
static int AuditSet(struct audit_class *, int);
static int GetClasses(struct audit_class *, int, struct audit_class **, char **, char **);
int CountEvents(char *, int);
static int RedoClasses(struct audit_class *, int);

/*                                                                              
 * NAME: auditevents()
 *                                                                             
 * FUNCTION: Gets or sets the status of system event auditing
 *	     Needs AUDIT_CONFIG privilege to execute this system call.
 *
 * INPUTS :
 *	Cmd : Specifies whether the audit classes are to be read or 
 *	      written. The valid values are AUDIT_SET, AUDIT_GET and
 *	      AUDIT_LOCK. AUDIT_SET sets the class definitions, 
 *	      AUDIT_GET and AUDIT_LOCK queries the class definitions
 *	      but the latter also blocks any other process attempting
 *	      to set or lock these definitions.
 *
 * INPUT/OUTPUT : 
 *	Classes : Specifies the buffer that contains (AUDIT_SET) or will
 *		  contain (AUDIT_GET or AUDIT_LOCK)the list of audit class
 *		  definitions. 
 *	NumClasses: For an AUDIT_SET operation, nclasses specifies the number
 *		  of audit class definitions in the classes array. For an
 *		  AUDIT_GET or AUDIT_LOCK operation, nclasses specifies the
 *		  size of the buffer pointed to by the classes parameter, in
 *		  bytes.
 * ERRORS:
 *	EPERM when the invoker doesn't have AUDIT_CONFIG privilege
 *	EFAULT if the event symbol table contains no events.
 *	EINVAL if NumClasses exceeds the maximum number of classes 
 *	       allowed or negative or if the Cmd argument is none of
 *	       AUDIT_SET AUDIT_GET or AUDIT_QUERY
 *
 * TYPE: int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.                                                    
 *                                                                             
 * DATA STRUCTURES: Effects on global data structures
 *	auditevent_block : It is set to PID of the process when Cmd
			  is equal to AUDIT_LOCK.Its value is reset
			  when Cmd=AUDIT_SET
 *                                                                             
 * RETURNS: 
 *	-1 : in case of error
 *	number of auditclasses: success and Cmd=AUDIT_GET or Cmd=AUDIT_LOCK
 *	0 : success and Cmd=AUDIT_SET
 *
 */                                                                            
int
auditevents(int Cmd, struct audit_class *Classes, int NumClasses){

	int rc = 0;

	/* 
	 * Privilege check 
	 */

	if(privcheck(AUDIT_CONFIG)){
		u.u_error = EPERM;
		return(-1);
	}

	/* 
	 * Grab lock
	 */
	simple_lock(&audit_lock);

	if(nevents == 0){
		u.u_error = EFAULT;
		goto out;
	}

	switch(Cmd){

		case AUDIT_SET:

			if(CheckEveLock()){

		 		goto out;
			}

			if((NumClasses > MAX_ANAMES - 1) || (NumClasses < 0)){

				u.u_error = EINVAL;
				goto out;

			}

			if(AuditSet(Classes, NumClasses) < 0){

				goto out;

			}

			if(auditevent_block){

				auditevent_block = 0;
				e_wakeup(&audit_anchor.lock);

			}

			break;

		case AUDIT_GET:

			rc = AuditGet(Classes, NumClasses);
			break;

		case AUDIT_LOCK:

			if(rc = CheckEveLock()){

				goto out;
	
			}

			auditevent_block = thread_self();

			rc = AuditGet(Classes, NumClasses);

			break;

		default:
			u.u_error = EINVAL;
	}

out:
	simple_unlock(&audit_lock);

	if(u.u_error){

		return(-1);

	}
	else {

		return(rc);

	}
}


/*                                                                              
 * NAME: CheckEveLock()
 *                                                                             
 * FUNCTION: Checks if the auditevent_block is set to 0 or the PID of the 
 *           (PID changed to thread ----- BULL 31/03/93 )
 *	     process.If not it checks if the process that has set 
 *	     auditevent_block is still existing. 
 *
 * PARAMETERS : None
 *                                                                             
 * ERRORS:                                                                     
 *	EBUSY if the process that has set auditevent_block is still running
 *	      and it is not the current process.
 *
 * TYPE: int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.                                                    
 *                                                                             
 * DATA STRUCTURES: Effects on global data structures
 *	auditevent_block : Refers it and resets if the process that is
 *			  holding it is no longer running.
 *                                                                             
 * RETURNS: 0 if the lock is free or -1 if busy
 *
 */                                                                            
static
int
CheckEveLock(){

        struct thread   *tp;

	if((auditevent_block == 0) || 
	(auditevent_block == thread_self())){

		return(0);

	}

 	tp = VALIDATE_TID(auditevent_block);

	if(tp == NULL){

		auditevent_block = 0;
		return(0);

	}

	if(tp->t_state != TSRUN){

		auditevent_block = 0;
		return(0);

	}

	u.u_error = EBUSY;
	return(-1);
}


/*                                                                              
 * NAME: AuditSet()
 *                                                                             
 * FUNCTION: Used for setting the class definitions. First the class_names
 *	     array and event symbol table are reset. Then the class_names
 *	     array is redone according to the class definitions given in the
 *	     input. The be_bitmap field for each event in the event symbol
 *	     table is set according to the events list present in each class
 *	     definition.
 *
 * INPUTS :
 *	uclasses : contains the list of audit_class definitions
 *	numcalsses : number of classes to be set.
 *
 * OUTPUTS : No explicit outputs but many globals are set implicitly
 *
 * ERRORS:                                                                     
 *	EINVAL if the input class definition contains no events
 *
 * TYPE: static int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.                                                    
 *                                                                             
 * DATA STRUCTURES: Effects on global data structures       
 *	This function calls many other functions which affect global data
 *	like class_names array and be_symtab symbol table. Within
 *	this function only class_names array is referenced.
 *                                                                             
 * RETURNS:
 *	-1 : in case of error
 *	0  : in case of success
 *
 */                                                                            
static int
AuditSet(struct audit_class *uclasses, int numclasses){

	struct	audit_class	*classes;
	struct	audit_class	*ep;
	char	*buf;
	char	*nbuf;
	int	i;

	/* 
	 * If there are no classes to set,  
	 * reset and return 
	 */

	if(!numclasses){
		AuditReset();	
		return (0);
	}

	/* 
	 * Copy in the user arguments  
	 */

	classes = (struct audit_class *)NULL;
	buf = nbuf = (char *)NULL;
	if(GetClasses(uclasses, numclasses, &classes, &buf, &nbuf))goto out;

	/* 
	 * Verify the validity of each list.
	 * the ae_len field in the kernel copy of each events list
 	 * is adjusted to hold the number of strings. 
	 */

	for(i = 0, ep = classes; i < numclasses; i++, ep++){
		ep->ae_len = CountEvents(ep->ae_list, ep->ae_len);
		if(ep->ae_len == -1){
			u.u_error = EINVAL;
			goto out;
		}
	}

	/* 
	 * Reset the symbol table - rehash
	 * first make a copy of the class_names array 
	 */

	bzero(class_tmps, sizeof(class_tmps));
	bcopy((char *)class_names, (char *)class_tmps, sizeof(class_names));

	AuditReset();

	/* 
	 * Remake the class_names array 
	 */

	RedoClasses(classes, numclasses);

	/* 
	 * Now copy each event list 
	 * into the event symbol table 
	 */

	for(i = 0, ep = classes; i < numclasses; i++, ep++){

		if(AuditClassAdd(ep) < 0){

			break;

		}

	}

out:
	if(classes)free((char *)classes);

	if(buf)free(buf);

	if(nbuf)free(nbuf);

	if(u.u_error)return(-1);

	else return (0);
}


/*                                                                              
 * NAME: GetClasses()
 *                                                                             
 * FUNCTION: copies the audit_class definitions from user space to 
 *	     kernel space.
 *
 * INPUTS : 
 *	uclasses : user space audit_class definitions
 *	numclasses : number of audit_class definitions
 *                                                                             
 * OUTPUTS :
 *	kclassp : kernel space audit_class definitions
 *	kbufp : buffer which holds the event lists of audit_class
 *		structures.
 *	knamep : buffer which holds the strings of class names
 *
 * ERRORS:
 *	ENOMEM if no memory cannot be allocated to hold kernel space 
 *	       definitions
 *	EFAULT if sanity check fails
 *	EINVAL if any class name length exceeds 15 characters
 *
 * TYPE: static int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.                                                    
 *                                                                             
 * DATA STRUCTURES: Effects on global data structures       
 *	NONE
 *                                                                             
 * RETURNS:                                                                    
 *	-1 in case of failure
 *	0 in success
 *
 */                                                                            
static int
GetClasses(struct audit_class *uclasses, int numclasses, 
struct audit_class **kclassp, char **kbufp, char **knamep){

	struct	audit_class *classes = NULL;
	struct	audit_class *ep;
	int	i;
	int	siz;
	int	blen = 0;
	int	c;
	int 	classlength;
	char	*p;
	char	*buf = NULL;
	char	*nbuf = NULL;

	siz = numclasses * sizeof(struct audit_class);

	/* 
	 * Allocate kernel event structures 
	 */

	classes = (struct audit_class *)malloc(siz);
	if(classes == NULL){
		u.u_error = ENOMEM;
		goto fail;
	}

	/* 
	 * Copy in the event structures 
	 */

	if(copyin((caddr_t)uclasses, classes, siz)){
		u.u_error = EFAULT;
		goto fail;
	}

	/* 
	 * Figure out how much string space we need 
	 */

	for(i = 0, ep = classes; i < numclasses; i++, ep++){
		blen += ep->ae_len;
		if(blen < 0){
			u.u_error = EFAULT;
			goto fail;
		}
	}

	/* 
	 * Allocate the space for the list of base classes 
	 */

	if((buf = malloc(blen)) == NULL){
		u.u_error = ENOMEM;
		goto fail;
	}
	
	/* 
	 * Copy in the strings of base events 
	 */

	p = buf;
	for(i = 0, ep = classes; i < numclasses; i++, ep++){
		if(copyin(ep->ae_list, p, ep->ae_len)){
			u.u_error = EFAULT;
			goto fail;
		}
		ep->ae_list = p;
		p += ep->ae_len;
	}

	/* 
	 * Allocate the event name space 
	 */

	if((nbuf = malloc(16*numclasses)) == NULL){
		u.u_error = ENOMEM;
		goto fail;
	}
	
	/* 
	 * Copy in the class names 
	 */

	bzero(nbuf, 16*numclasses);
	p = nbuf;
	for(i = 0, ep = classes; i < numclasses; i++, ep++){
		if(copyin((caddr_t)ep->ae_name, p, 16)){
			u.u_error = EFAULT;
			goto fail;
		}
		ep->ae_name = p;
		if((classlength = strlen(p)) > 15){
			u.u_error = EINVAL;
			goto fail;
		}
		p += 16;
	}

	*kclassp = classes;
	*kbufp = buf;
	*knamep = nbuf;
	return(0);

fail:
	if(classes)free((char *)classes);
	if(buf)free(buf);
	if(nbuf)free(nbuf);

	return (-1);
}


/*                                                                              
 * NAME:CountEvents()
 *                                                                             
 * FUNCTION: Counts the number of event names present in the array
 *
 * INPUTS :
 *	s : character array containing event names
 *	len: length of the array
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
 * DATA STRUCTURES: Effects on global data structures.       
 *	NONE
 *                                                                             
 * RETURNS: returns -1 in case sanity check fails else returns the number
 *	    of events present in the array.
 *
 */                                                                            
int
CountEvents(char *s, int len){

	int	n;
	int	tokenlength;
	int	slen;

	n = 0;
	while(1){
		len--;
		if(len < 0)return(-1);

		if(*s++ == '\0')break;

		/* scan the string */
		slen = 1;
		while (1){
			len--;
			slen++;
			if(len < 0)return(-1);

			if(*s++ == '\0')break;
			if(slen > 15){
				return(-1);
			}
		}

		/* count the string */
		n++;
	}
	return(n);
}


/*                                                                              
 * NAME: RedoClasses()
 *                                                                             
 * FUNCTION: Remake the class_names array from the class definitions
 *	     given as input.
 *
 * INPUTS : 
 *	Classes : Input audit_class definitions
 *	numcalsses : Number of audit_classes
 *
 * OUTPUTS : NONE explicitly but the globals are modified
 *
 * ERRORS: NONE
 *
 * TYPE: static int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.                                                    
 *                                                                             
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.       
 *	class_names array is done again from input class definitions
 *	class_tmps contains old class names
 *                                                                             
 * RETURNS: Nothing is returned explicitly but the global class_names array is 
 *	    rebuilt
 *
 */                                                                            
static 
int 
RedoClasses(struct audit_class *classes, int numclasses){

	struct audit_class *ap;
	int i, j;	

	for(i = 0, ap = classes; i < numclasses; i++, ap++){
		for(j = 0; j < MAX_ANAMES - 1; j++){
			if(strncmp(ap->ae_name, class_tmps[j], 16) == 0){
					bzero(class_names[j], 16);
					bcopy(ap->ae_name,class_names[j], 16);
					break;
			}
		}

		if(j < MAX_ANAMES-1)continue;

		/*
		 * Search for an empty slot in the tmp array 
		 */

		for(j = 0; j < MAX_ANAMES - 1; j++){
			if(class_tmps[j][0] == 0){
				class_tmps[j][0]= -1;
				bzero(class_names[j], 16);
				bcopy(ap->ae_name,class_names[j], 16);
				break;
			}
		}
	}

	for(i = 0, ap = classes; i < numclasses; i++, ap++){
		for(j = 0; j < MAX_ANAMES; j++){
			if(strncmp(class_names[j], ap->ae_name, 16) == 0)
				break;
		}
		if(j < MAX_ANAMES)continue;

		for(j = 0; j < MAX_ANAMES; j++){
			if(class_names[j] == 0){
				bzero(class_names[j], 16);
				bcopy(ap->ae_name,class_names[j], 16);
			}
		}
	}
}


/*                                                                              
 * NAME: AuditClasses()
 *                                                                             
 * FUNCTION: Searches for given names in the class_names list.
 *
 * INPUT:  list : the names to search for                  
 *                                                                             
 * ERRORS: EFAULT if for any reason there is an invalid value in list. 
 *
 * TYPE: int                
 *
 * DATA STRUCTURES: class_names is affected.       
 *                                                                             
 * RETURNS: The places in class_names of the names given             
 *
 */                                                                            
int
AuditClasses(char *list){

	char	buf[16];
	int	class;
	int	i;

	class = 0;
	while (1){

		if(copyin(list, buf, sizeof(buf))){
			u.u_error = EFAULT;
			return(-1);
		}

		/* 
		 * Done on NULL name 
		 */

		if(*buf == '\0')return(class);
		
		/* 
		 * Search table for name 
		 */

		for (i = 0; i < MAX_ANAMES; i++){
			if (class_names[i][0] == '\0')continue;

			if (strcmp (buf, class_names[i]) == 0){
				class |= 1 << i;
				break;
			}
		}
	}
}
