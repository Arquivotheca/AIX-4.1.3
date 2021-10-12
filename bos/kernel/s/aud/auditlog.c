static char sccsid[] = "@(#)87        1.33.1.7  src/bos/kernel/s/aud/auditlog.c, syssaud, bos41J, 9519A_all 5/2/95 10:00:16";

/*
 * COMPONENT_NAME: (SYSSAUD) Auditing Management
 *
 * FUNCTIONS: auditlog() system call
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
#include	<sys/proc.h>
#include	<sys/user.h>
#include	<sys/file.h>
#include	<sys/systm.h>
#include	<sys/vnode.h>
#include	<sys/uio.h>
#include	<sys/syspest.h>
#include	<sys/malloc.h>
#include	<sys/auditk.h>
#include	<sys/audit.h>
#include	<sys/pri.h>
#include	<sys/priv.h>
#include	<varargs.h>
#include	<sys/lockl.h>
#include	<sys/id.h>

/* global cred lock */

extern  Simple_lock cred_lock;

/*
 * Forward declarations 
 */

int AuditWrite(int , int , char *, int , char *, int);
int audit_svcbcopy(char *, int);

/*
 * Local defines, structs - support of nested system calls
 */

#define		AUD_NESTED	0x1
#define		AUD_MAX_ARGS	8
#define		AUD_FLAGS 	8
#define		AUD_RESERVED	9

struct aud_internal_t {

	struct aud_internal_t *next;
	struct auddata audsvc;

};

/*
 * NAME: auditlog
 *
 * FUNCTION: appends an audit record to the audit trail. 
 *
 * INPUTS :
 *	Event : the name of the audit event to be generated.
 *	Status : Describes the result status of this event. 
 *	Buffer : Specifies a pointer to a buffer containing the
 *		 event specific information.
 *	Length : Specifies the size of the buf parameter.
 *
 * OUTPUTS : NONE explicitly
 *
 * ERRORS: 
 *	EPERM if the calling process doesn't have SET_PROC_AUDIT
 *	      kernel privilege.
 *	EINVAL if the record length exceeds the limit or if auditing
 *	       is not ON.
 *	EFAULT if internal memory error occurs.
 *	ENOMEM if malloc fails
 *	
 *
 * TYPE: int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures.
 *	NONE
 *
 * RETURNS:
 *	0 on error
 *	-1 on success
 *
 */
int
auditlog(char *Event, int Status, char *Buffer, int Length){

	char	EventBuffer[16];
	char	*TmpBuf = NULL;
	int	Index;
	int	i;
	struct  uthread *ut = curthread->t_uthreadp;

	/*
	 * Privilege check 
	 */

	if(privcheck(SET_PROC_AUDIT)){

		ut->ut_error = EPERM;
		return(-1);

	}

	/*
	 * Get audit lock
	 */
	simple_lock(&audit_lock);


	/*
	 * Is auditing enabled? 
	 */
	if(!(audit_flag & AUDIT_ON)){

		ut->ut_error = EINVAL;   
		goto out;

	}

	/*
	 * Total length of record must be less 
	 * than 32K 
	 */

	if(Length >= (32 * 1024) - (sizeof(struct aud_rec))){

		ut->ut_error = EINVAL;
		goto out;

	}

	/*
	 * copyin the event name 
	 */

	bzero(EventBuffer, sizeof(EventBuffer));
	if(copyin(Event, EventBuffer, sizeof(EventBuffer))) {

		ut->ut_error = EFAULT;
		goto out;

	}

	/*
	 * Check whether this event is enabled 
	 */

	Index = AuditLookup(EventBuffer);
	if((Index >= 0) && (audit_is_on(Index))){

		int	Len;

		Len = sizeof(struct aud_rec) + Length;
		if((TmpBuf = malloc(Len)) == NULL){

			ut->ut_error = ENOMEM;
			goto out;

		}

		if(copyin(Buffer, TmpBuf + sizeof(struct aud_rec), Length)){

			ut->ut_error = EFAULT;
			goto out;

		}

		AuditWrite(Index, Status, TmpBuf, Len, TmpBuf, Length);
	}

out:

	simple_unlock(&audit_lock);

	if(TmpBuf){

		free(TmpBuf);

	}

	if(ut->ut_error){

		return (-1);

	}
	else {

		return(0);
 
	}
}


/*
 * NAME:  audit_write
 *
 * FUNCTION: This system function will write the contents of buffers into
 *	     a file. The header information is attached before writing into
 *	     a file.
 *
 * INPUTS : It takes variable number of arguments as input.
 *	I arg   : event to be logged.
 *	II arg  : status of the event that is to be logged
 *	III to Nth :  buffer and buffer length ( any number of buffers
 *		      may be specified).
 *
 * OUTPUTS : NONE explicitly
 *
 * ERRORS:
 *	EINVAL if the event obtained by popping the stack is less than 0
 *		or greater than the maximum number allowed.
 *	ENOMEM if malloc fails
 *
 * TYPE: int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures.
 *	NONE
 *
 * RETURNS: 
 *	0 on success
 *	-1 on failure.
 *
 */
int
audit_write (va_alist)
va_dcl {

	char	*av;
	char	*saveptr;
	int	event;		/* be_symtab index */
	int	status; 	/* status value */
	char	*rec = NULL;	/* (allocated) buffer */
	char	*recp;		/* pointer to next space in buffer */
	char	buf[256];	/* initial buffer */
	int	buf_len=0;
	int	numbufs=0;
	int	i, rlen;
	int 	rc = 0;

	simple_lock(&audit_lock);

	va_start(av);
	event = va_arg(av,int);
	if ((event < 0) || (event > nevents)){

		u.u_error = EINVAL;
		goto fail;

	}

	status = va_arg(av,int);
	saveptr = av; 		
	rec = buf;

	/*
	 * Make first pass to get a total 
	 * byte count for all buffers 
	 */

	while(1){

		int l;

		if(va_arg(av,char *) == NULL){

			break;
		}

		if((l = va_arg(av, int)) <= 0){

			break;

		}

		buf_len += l;
		numbufs++;

	}

	rlen = sizeof (struct aud_rec) + buf_len;
	if(rlen > sizeof (buf)){

		if((rec = malloc(buf_len))== NULL){

			u.u_error = ENOMEM; 
			goto fail;

		}
	}

	/*
	 * Make second pass to copy data into rec 
	 */

	recp = rec + sizeof (struct aud_rec);
	for(i = 1;i <= numbufs; i++){

		char 	*b;
		int	l;
		
		b = va_arg(saveptr, char *);
		l = va_arg(saveptr, int);
		bcopy (b, recp, l);
		recp += l;

	}
	
	/*
	 * Write the record out 
	 */

	rc = AuditWrite(event, status, rec, rlen, rec, buf_len);

	if(rec != buf)free(rec);

fail:

	simple_unlock(&audit_lock);

	return(rc);
}

/*
 * NAME:  auditscall
 *
 * FUNCTION: writes the SVC audit records to the kernel audit logger. This
 *	     function is called automatically by the SVC handler upon 
 *	     termination of a system call.
 *
 * INPUTS : NONE
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
 * RETURNS: NONE explicitly
 *
 */

int 
auditscall(){
	struct  uthread *ut = curthread->t_uthreadp;
	struct  auddata *audsvc_p = ut->ut_audsvc;

	if (!audsvc_p)
		return(0);

	simple_lock(&audit_lock);

	/*
	 * Must check svcnum for special case of exit(), fork()
	 * where auditscall is called explicitly to commit
	 * record.
	 * Normally svcnum is set by low.s and auditscall
	 * skipped if svcnum <= 1
	 */

	if(audsvc_p->svcnum <= 1){

		goto fail;

	}

	if(!audit_is_on(audsvc_p->svcnum)){

		goto fail;

	}

	if(ut->ut_error){

		switch(ut->ut_error){

			case EPERM:

				audsvc_p->status = AUDIT_FAIL_PRIV;
				break;

			case EACCES:

				audsvc_p->status = AUDIT_FAIL_ACCESS;
				break;

			case ESAD:

				audsvc_p->status = AUDIT_FAIL_AUTH;
				break;

			default:

				audsvc_p->status = AUDIT_FAIL;
				break;

		}
	}
	else {

		audsvc_p->status = AUDIT_OK;

	}

	/* we unlock before calling AuditWrite (defect 175662) */
	simple_unlock(&audit_lock);

	AuditWrite(audsvc_p->svcnum, 
	audsvc_p->status,
	audsvc_p->args[0],
	audsvc_p->args[1],
	audsvc_p->args[0],
	audsvc_p->args[2]);

	audsvc_p->svcnum = 0;
	free(audsvc_p->args[0]);
	audsvc_p->args[0] = 0;
	audsvc_p->args[1] = 0;
	audsvc_p->args[2] = 0;

	return;

fail:

	simple_unlock(&audit_lock);
}

/*
 * NAME: audit_svcfinis
 *
 * FUNCTION: When called, this function writes the SVC audit records to the
 *	     kernel audit logger. This function need not be called at 
 *	     every system call termination as SVC handler automatically calls
 *	     auditscall() to commit audit records. This should be called
 *	     only if the system call doesn't terminate normally.
 *
 * INPUTS : NONE
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
 * RETURNS: 0
 *
 */

int
audit_svcfinis(){
	
	char	*buf;
	int	len;
	char	*rec;
	char	*tail;
	int	tail_len;
	int	sav_error;
	struct  uthread *ut = curthread->t_uthreadp;
	struct  auddata *audsvc_p = ut->ut_audsvc;


	if (!audsvc_p)
		return(0);

	simple_lock(&audit_lock);

	sav_error = ut->ut_error;

	if(audsvc_p->svcnum <= 1){

		goto fail;

	}

	if(!audit_is_on(audsvc_p->svcnum)){

		audsvc_p->svcnum = 1; /* to force auditscall() to return */
		goto fail;

	}

	/*
	 * tail_len = args + audbuf 
	 */

	tail_len = (audsvc_p->argcnt * sizeof (long)) + audsvc_p->buflen;

	/*
	 * len = record header + tail_len 
	 */

	len = sizeof(struct aud_rec) + tail_len;

	if((buf = malloc(len)) == NULL){

		audsvc_p->svcnum = 1; /* to force auditscall() to return */
		goto fail;

	}

	rec = buf;

	/*
	 * Compute location of tail in this buffer 
	 */

	tail = rec + sizeof (struct aud_rec);

	/*
	 * Copy args 
	 */

	bcopy(&(audsvc_p->args[0]), tail, audsvc_p->argcnt * sizeof(int));
	tail += audsvc_p->argcnt * sizeof(int);

	/*
	 * Copy audbuf 
	 */

	bcopy(audsvc_p->audbuf, tail, audsvc_p->buflen);

	/*
	 * Save args for auditscall
	 */

	audsvc_p->args[0] = (int)buf;
	audsvc_p->args[1] = (int)len;
	audsvc_p->args[2] = (int)tail_len;

	/*
	 * If nested - commit record now 
	 * and Pop stack
 	 */

	if(CheckStack()){
		simple_unlock(&audit_lock);
		auditscall();
		simple_lock(&audit_lock);
		PopStack();

	}
		
fail:

	audsvc_p->buflen = 0;
	audsvc_p->bufcnt = 0;
	audsvc_p->argcnt = 0;
	audsvc_p->status = 0;
	ut->ut_error = sav_error;

	simple_unlock(&audit_lock);

	return(0);
}

/*
 * NAME: AuditWrite
 *
 * FUNCTION: Commits audit records present in the buf.
 *
 * INPUTS :
 *	index : event index
 *	status : event status
 *	buf : buffer in which the audit records are held
 *	len : length of buffer
 *	rlen : length of rec
 *
 * INPUT/OUTPUT : 
 *	rec : buffer which holds the header.
 *
 * OUTPUTS : Only buf is written to the file
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
 * RETURNS:
 *	0 on success
 *	-1 on failure
 *
 */


int
AuditWrite(int index, int status, char *buf, int len, char *rec, int rlen){

	struct	aud_rec	ah;
	int	sav_error;
	int 	rc = 0;
	struct  thread   *t;
	struct  uthread  *ut;

	t = curthread;
	ut = t->t_uthreadp;

	if((audit_bin == NULL) && (audit_panic == AUDIT_PANIC)){

		assert(audit_panic == AUDIT_NO_PANIC);

	}

	/*
	 * Construct the header 
	 */

	bzero(&ah, sizeof (ah));
	ah.ah_magic = AUDIT_HDR1;

	bcopy(be_symtab[index].be_name, ah.ah_event, 
	strlen(be_symtab[index].be_name));

	ah.ah_length = (ushort_t)rlen;

	/* if we already hold the cred lock, for example, we came
	   through the watch cmd setuid() call, then we will crash
	   if we call getuidx, which also needs that lock.  So
	   check first.  If we don't have the lock, then it's safe
	   to call getuidx; else if we do have the lock, then we
	   can examine the cred structure directly. */

	if (!lock_mine(&cred_lock))
	{
		ah.ah_ruid   = getuidx(ID_REAL);
		ah.ah_luid   = getuidx(ID_LOGIN);
	}
	else
	{
		ah.ah_ruid = U.U_cred->cr_ruid;
		ah.ah_luid = U.U_cred->cr_luid;
	}

	ah.ah_result = status;
	bcopy(U.U_comm, ah.ah_name, MAXCOMLEN);
	ah.ah_pid    = U.U_procp->p_pid;
	ah.ah_ppid   = U.U_procp->p_ppid;
	ah.ah_tid    = t->t_tid;
	ah.ah_time   = time;

	/*
	 * Copy it into the buffer 
	 */

	bcopy(&ah, rec, sizeof(ah));

	/*
	 * Make sure there is room in the bin.
	 */

	if(audit_threshold && ((audit_size+len) > audit_threshold)){
		AuditSwitch();
		audit_size = 0;
	}
	audit_size += len;

	sav_error = ut->ut_error;
	ut->ut_error = 0;

	if(audit_bin && (audit_flag & AUDIT_ON)){

		rc = AuditDoWrite(audit_bin, buf, len);

	}


	/*
	 * Device Driver Write
	 */

        if(auditdev && (audit_flag & AUDIT_ON)){

		(*auditdev)(be_symtab[index].be_bitmap, buf, len);

	}

	ut->ut_error = sav_error;

	return(rc);

}

/*
 * NAME: audit_svcstart
 *
 * FUNCTION: This function initiates auditing for a system call event. It
 *	     will store the arguments in ut->ut_audsvc->args and
 *	     ut->ut_audsvc->argcnt is set equal to the number of arguments.
 *	     ut->ut_audsvc->svcnum holds the event index.
 *
 * INPUTS : 
 *	I arg	: name of the event
 *	II arg	: number of arguments
 *	III to n: arguments(event specific information)
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
 *
 * RETURNS:
 *	the event index
 *
 */


audit_svcstart (va_alist)
va_dcl{

	char	*av;
	char	*name;
	int	*audsvcnum;
	int    	numargs;
	int	rc = 0;
	struct  uthread *ut = curthread->t_uthreadp;

	/*
	 * Dynamically allocate audsvc record to this thread
	 */
	if (!ut->ut_audsvc)
	{
		if (audit_flag )
		{
			ut->ut_audsvc = (struct auddata *)
			             xmalloc(sizeof(struct auddata),
				             0, kernel_heap); 
			bzero ( ut->ut_audsvc, sizeof(struct auddata));
			ut->ut_audsvc->svcnum = 1;
		} else
			return(0);
	}

	simple_lock(&audit_lock);

	/*
	 * Check for a nested system call. If true,
	 * save current context, set nested flag, and
	 * push new audit buffer in u block
	 */

	if((audit_flag) && (ut->ut_audsvc->svcnum > 1)){

		PushStack();
	}

	if(ut->ut_audsvc->svcnum == 1){

		int argno;

		va_start(av);
		name = va_arg(av, char *);	
		audsvcnum = va_arg(av, int *);	
		numargs = va_arg(av, int);	

		/*
	 	 * Register event name with auditing 
		 */

		if(*audsvcnum == 0){

			*audsvcnum = AuditLookup(name);

		}

		/*
		 * If registered, make sure its event 
		 * is in the ALL class
		 */

		if(!(be_symtab[*audsvcnum].be_bitmap & 
		(ulong)(1 << ALL_CLASS))){

			be_symtab[*audsvcnum].be_bitmap |= 
			(ulong)(1 << ALL_CLASS);

			be_total_len += strlen(name) + 1;
		}

		/*
		 * Registered and remapped - now return if not auditing 
		 * process for this event
		 */

		if(!audit_is_on(*audsvcnum)){

			if(CheckStack()){
		
				PopStack();

			}
			
			goto out;

		}

		/*
		 * Store initial audit info in u block
		 */

		ut->ut_audsvc->svcnum = *audsvcnum;

		for(argno = 0; argno < numargs, argno < AUD_MAX_ARGS; 
		argno++){

			ut->ut_audsvc->args[argno] = va_arg(av, int);

		}

		ut->ut_audsvc->argcnt = numargs;

		/*
		 * Set return value to "opaque" 
		 */

		rc = *audsvcnum;

	}

out:

	simple_unlock(&audit_lock);

	return(rc);
}


/*
 * NAME: audit_svcbcopy
 *
 * FUNCTION: Appends event information to the current audit event
 *	     buffer. 
 *
 * INPUTS :
 *	buf : specifies the information to append tio the current 
 *	      audit event record buffer.
 *	len : number of bytes in the buffer.
 *
 * OUTPUTS : NONE explicitly
 *
 * ERRORS:
 *	ENOMEM if malloc fails
 *
 * TYPE: int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures.
 *	NONE
 *
 * RETURNS:
 *	-1 on error
 *	0 on success
 *
 */

int
audit_svcbcopy(char *buf, int len){

	register char	*pbp;
	int rc = 0;
	struct  uthread *ut = curthread->t_uthreadp;
	struct  auddata *audsvc_p = ut->ut_audsvc;


	/*
	 * Guard against the case when audit_svcstart was called when
	 * auditing was off
	 */
	if (!audsvc_p)
		return(0);

	simple_lock(&audit_lock);

	if(audsvc_p->svcnum <= 1){

		goto out;

	}

	if(audsvc_p->bufsiz - audsvc_p->buflen <= len){

		/*
		 * Increase buffer to reasonable size 
		 */

		audsvc_p->bufsiz = (audsvc_p->bufsiz+len) * 4;
		pbp = malloc(audsvc_p->bufsiz);
		if(!pbp){

			ut->ut_error = ENOMEM;
			rc = -1;
			goto out;

		}

		if(audsvc_p->audbuf != NULL){

			bcopy(audsvc_p->audbuf, pbp, audsvc_p->buflen);
			free(audsvc_p->audbuf);

		}
		audsvc_p->audbuf = pbp;
	}

	/*
	 * Append input buf to buf 
	 */

	pbp = audsvc_p->audbuf + audsvc_p->buflen;
	bcopy (buf, pbp, len);

	audsvc_p->buflen += len;

	audsvc_p->bufcnt++;

out:

	simple_unlock(&audit_lock);
	return(rc);
}


/*
 * NAME: PushStack
 *
 * FUNCTION: In case of nested system calls the audit record info
 *	     is stored in aud_internal_t structure. The stack is
 *	     the linked list of aud_internal_t structures. The U
 *	     area is free to store the audit info of the latest
 *	     system call.
 *
 * INPUTS : NONE
 *
 * OUTPUTS : NONE
 *
 * ERRORS: NONE
 *
 * TYPE: static int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures
 *	Audit info. from U area is transferred to temporary buffers
 *
 * RETURNS: 0 on a malloc error
 *
 */

static
int
PushStack(){

	struct  uthread *ut = curthread->t_uthreadp;
	struct  auddata *audsvc_p = ut->ut_audsvc;
	struct aud_internal_t *head;
	int i;

	audsvc_p->args[AUD_FLAGS] |= AUD_NESTED;

        if((head = malloc(sizeof(struct aud_internal_t))) == NULL){

		return(0);

	}

        bzero(head, sizeof(struct aud_internal_t));

       /*
        * Link in head structure
       */

	if(audsvc_p->args[AUD_RESERVED] != 0){

              	head->next = (struct aud_internal_t *)
			audsvc_p->args[AUD_RESERVED];

	}

	audsvc_p->args[AUD_RESERVED] = (int)head;

	/*
	 * Copy auddata structure 
	 */

	bcopy(audsvc_p, &head->audsvc, sizeof(struct auddata));

	/*
	 * Set svcnum back to 1 so things work normally
	 */

	audsvc_p->svcnum = 1;
}


/*
 * NAME: PopStack
 *
 * FUNCTION: Copies audit information from the head of linked list into
 *	     the U area and updates the linked list. This function is
 *	     called when exiting from a nested system call.
 *
 * INPUT: NONE
 *	
 * OUTPUTS : NONE
 *
 * ERRORS: NONE
 *
 * TYPE: static int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.
 *	The U area contains audit information of the previous system call
 *
 * RETURNS: Nothing explicitly
 *
 */

static
int
PopStack(){

	struct uthread *ut = curthread->t_uthreadp;
	struct auddata *audsvc_p = ut->ut_audsvc;
	struct aud_internal_t *head;
	int i;

	head = (struct aud_internal_t *)audsvc_p->args[AUD_RESERVED];

	bcopy(&head->audsvc, audsvc_p, sizeof(struct auddata));

	audsvc_p->args[AUD_RESERVED] = (int) head->next;
	if(head->next == (struct aud_internal_t *)NULL){

		audsvc_p->args[AUD_FLAGS] &= ~AUD_NESTED;

	}
	free(head);

}


/*
 * NAME: CheckStack
 *
 * FUNCTION: Checks if it is a nested system call.
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
 *	NONE
 *
 * RETURNS:
 *	1 if nested
 *	0 otherwise
 *
 */

static
int
CheckStack(){
	struct uthread *ut = curthread->t_uthreadp;

	if(ut->ut_audsvc->args[AUD_FLAGS] & AUD_NESTED){

		return(1);
	
	}

	return(0);

}
