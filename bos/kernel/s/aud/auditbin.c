static char sccsid[] = "@(#)85        1.24.1.3  src/bos/kernel/s/aud/auditbin.c, syssaud, bos411, 9428A410j 10/15/93 12:44:34";

/*
 * COMPONENT_NAME: (SYSAUDIT) Auditing Management
 *
 * FUNCTIONS: auditbin() system call
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
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
#include	<sys/file.h>
#include	<sys/systm.h>
#include	<sys/vnode.h>
#include	<sys/auditk.h>
#include	<sys/audit.h>
#include	<sys/uio.h>
#include	<sys/priv.h>
#include	<sys/syspest.h>
#include	<sys/lockl.h>

/*
 * Forward declarations 
 */

static int AuditHead(struct file *);
int AuditTail(struct file *);
int AuditSwitch();
int AuditDoWrite(struct file *, char *, int);

/*
 * NAME: auditbin()
 *
 * FUNCTION: the auditbin() system call establishes an audit bin into 
 *	     which the kernel writes audit records. Optionally, it may
 *	     be used to establish an overflow bin into which records are
 *	     written when the current bin reaches the specified threshold.
 *
 * INPUTS :
 *	cmd : This could be either
 *	      AUDIT_EXCL : If the file specified by cur is not the kernel's
 *	      		   current bin file, the auditbin system call fails
 *			   immediately with errno set to EBUSY.
 *	      AUDIT_WAIT : The auditbin system call should not return until
 *			   i) bin full ii) bin failure iii) bin contention
 *			   iv) system shutdown
 *	cur : Specifies the descriptor which the kernel audit logger 
 *	      component should use as the current bin.
 *	next : Specifies the file descriptor which will be used as the 
 *	       current auditbin if the threshold is exceeded or if a
 *	       write to the current bin should fail.
 *	threshold : specifies the maximum size of the current bin.
 *
 * OUTPUTS : NONE explicitly
 *
 * ERRORS:
 *	EINVAL : invalid parameter values
 *	EPERM : The caller doesn't have the AUDIT_CONFIG privilege.
 *	EBADF : The file descriptors are not valid.
 *	EBUSY : The file specified by cur is not the current bin file.
 *	EFBIG : The threshold was reached buit the switch to the new bin
 *	 	was not possible
 *	EINTR : The auditting subsystem was shutdown while the caller was
 *		waiting for a bin.
 *
 * TYPE: int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures.
 *	audit_bin audit_next and thresh values are set.
 *
 * RETURNS:
 *	-1 in case of error
 *	0 in success
 *
 */
int
auditbin(int cmd, int CurrentFd, int NextFd, int thresh){

	struct file	*FileCurrent;	
	struct file	*FileNext;	

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

	/*
	 * Check flags
	 */

	if ((-1 ^ AUDIT_WAIT ^ AUDIT_EXCL) & cmd){

		u.u_error = EINVAL;
                goto out;

	}

        if(fp_getf(CurrentFd, &FileCurrent)){

                u.u_error = EBADF;
                goto out;

        }

	if(((FileCurrent->f_flag & FWRITE) == 0) || 
        (FileCurrent->f_type != DTYPE_VNODE) ||
	(FileCurrent->f_vnode->v_vntype != VREG)){
		u.u_error = EBADF;
		goto outf1;
	}

 	/* 	
	 * "NextFile" fd, if != -1, must be for an open, 
	 * writable, regular file 
	 */

	if(NextFd != -1){
	        if(fp_getf(NextFd, &FileNext)){
	                u.u_error = EBADF;
	                goto outf1;
	        }
		
		if(((FileNext->f_flag & FWRITE) == 0) || 
        	(FileNext->f_type != DTYPE_VNODE) ||
		(FileNext->f_vnode->v_vntype != VREG)){

			u.u_error = EBADF;
			goto outf2;

		}
	}
	else {
		FileNext = NULL;
	}

 	/*
	 * Threshold, if non-zero, must be at least 
	 * large enough  to hold the head and tail 
	 */

	if((thresh) && (thresh < (2 * sizeof(struct aud_bin)))){

		u.u_error = EINVAL;
		goto outf2;

	}

 	/*
	 * If auditing has been disabled 
	 * (invalidating the current bin),
 	 * report to the bin manager that auditing was reset.
	 */
	if((audit_bin == NULL) && 
	(audit_flag == AUDIT_OFF) && (cmd & AUDIT_EXCL)){

		u.u_error = EINTR;
		goto outf2;

	}

 	/*
	 * If the old current bin and the new current 
	 * bin are not the same,
 	 * swap the new current bin for the old current bin (if any) 
	 */

	if((audit_bin == NULL) || 
	(FileCurrent->f_vnode->v_gnode != audit_bin->f_vnode->v_gnode)){

		if(audit_bin){

			if(cmd & AUDIT_EXCL){
				u.u_error = EBUSY;
				goto outf2;
			}

		}

		fp_hold(FileCurrent);
		if(AuditHead(FileCurrent)){
			goto outf2;
		}

 	  	/*
		 * If there was one, write a tail into it 
		 * (AuditTail releases the file) 
		 */

		if(audit_bin){

			AuditTail(audit_bin);

		}

		audit_bin = FileCurrent;
		audit_size = 0;

	}

 	/*
	 * Release the old next bin and install 
	 * the new next bin this must not be done 
	 * until after we know the current bin matched (above).
	 */

	if(audit_next){
		fp_close(audit_next);
		audit_next = NULL;
	}
	if(FileNext){
		fp_hold(FileNext);
	}
	audit_next = FileNext;

 	/*
	 * Set threshold 
	 */

	audit_threshold = thresh;

	if(audit_threshold && audit_bin->f_offset >= audit_threshold){

		AuditSwitch();

	}
	else if (cmd & AUDIT_WAIT){

		audit_anchor.lock = EVENT_NULL;

		/*
		 * Give up lock before sleeping 
		 */
		simple_unlock(&audit_lock);

		e_sleep(&audit_anchor.lock, EVENT_SIGRET);

		u.u_error = audit_anchor.error;

		if (NextFd != -1)
		{
			/* Decrement the next file descriptor count */
			ufdrele(NextFd);
		}
		
		/* Decrement the current file descriptor count */
		ufdrele(CurrentFd);
		
		if(u.u_error){
			return (-1);
		}
		else {
			return (0);
		}
	}

outf2:
	if (NextFd != -1)
	{
		/* Decrement the next file descriptor count */
		ufdrele(NextFd);
	}

outf1:
	/* Decrement the current file descriptor count */
	ufdrele(CurrentFd);

out:

	simple_unlock(&audit_lock);

	if(u.u_error){
		return (-1);
	}
	else {
		return (0);
	}
}

/*
 * NAME: AuditHead()
 *
 * FUNCTION: Inserts the audit header in the file 
 *
 * INPUTS :
 *	FilePtr : File pointer into which audit header has to be put.	
 *
 * OUTPUTS : NONE explicitly
 *
 * ERRORS: NONE
 *
 * TYPE: static int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures.
 *	NONE
 *
 * RETURNS:
 *	-1 on failure
 *	0 on success
 *
 */
static
int
AuditHead(struct file *FilePtr){

	static	struct	aud_bin	AuditHeader = {
		/* magic */	AUDIT_MAGIC,
		/* version */	AUDIT_VERSION,
		/* tail */	AUDIT_HEAD,
		/* len */	0,
		/* plen */	0,
		/* time */	0,
		/* reserved1 */ 0,
		/* reserved2 */	0
	};

	AuditHeader.bin_time = (ulong_t)time;

	/*
	 * Write head record to the new bin 
	 */

	if(AuditDoWrite(FilePtr, (char *)&AuditHeader, sizeof(AuditHeader))){

		fp_close(FilePtr);
		return(-1);

	}

	return(0);
}

/*
 * NAME: AuditTail()
 *
 * FUNCTION: Insert audit tailer in the file and close it.
 *
 * INPUTS : 
 *	FilePtr: Pointer to the file in which tailer is written
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
 * RETURNS: returns errors generated in the functions it calls else returns
 *	    0.
 *
 */
int
AuditTail(struct file *FilePtr){

	int	TmpError;
	static	struct	aud_bin	AuditTailer = {
		/* magic */	AUDIT_MAGIC,
		/* version */	AUDIT_VERSION,
		/* tail */	AUDIT_BIN_END,
		/* len */	0,
		/* plen */	0,
		/* time */	0,
		/* reserved1 */	0,
		/* reserved2 */	0
	};

	AuditTailer.bin_len = FilePtr->f_offset - sizeof(struct aud_bin);
	AuditTailer.bin_time = (ulong_t)time;

	/*
	 * Write end record to current bin 
	 */

	AuditDoWrite(FilePtr, (char *)&AuditTailer, sizeof(AuditTailer));

	/*
	 * Release this bin (and notify the manager) 
	 */

	fp_close(FilePtr);

	audit_anchor.error = 0;
	e_wakeup(&audit_anchor.lock);

	TmpError = u.u_error;
	u.u_error = 0;
	return(TmpError);
}

/*
 * NAME: AuditSwitch()
 *
 * FUNCTION: switches bins. It closes the current bin after inserting the
 *	     audit tailer at the end of file. It inserts the audit 
 *	     header into the next audit bin which becomes the current
 *	     bin.
 *
 * INPUTS : NONE
 *
 * OUTPUTS : NONE
 *
 * ERRORS : NONE
 *
 * TYPE: int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures.
 *	audit_bin, audit_next and audit_size are affected
 *
 * RETURNS:
 *
 */
int
AuditSwitch(){

	/*
	 * If there is no "next" bin or 
	 * we can't write to it for some reason
 	 * then we want to tell the bin manager and 
	 * continue with the current one 
	 */

	if(audit_next == NULL){

		audit_anchor.error = 0;
		e_wakeup(&audit_anchor.lock);
		audit_size = 0;
		return;

	}

	if(AuditHead(audit_next)){

		audit_next = NULL;
		audit_anchor.error =  EFBIG;
		e_wakeup(&audit_anchor.lock);
		audit_size = 0;
		return;

	}

	/*
	 * Set size to size of head + tail 
	 */

	audit_size = 2 * sizeof(struct aud_bin);

	/*
	 * Terminate the old bin 
	 */

	AuditTail(audit_bin);

	audit_bin = audit_next;
	audit_next = NULL;
}

/*
 * NAME: AuditDoWrite()
 *
 * FUNCTION: write into the bin file specified.
 *
 * INPUTS :
 *	bin : bin file into which buffer contents have to be written.
 *	buf : contents of which are written to the bin file.
 *	len : length of buffer.
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
 * RETURNS:
 *	0 on success.
 *	-1 on error.
 *
 */
int 
AuditDoWrite(struct file *bin, char *buf, int len){

	int status;
	int cnt;

	/*
	 * No need to enhance ulimit value for this process 
	 * because the kernel is writing into the bin file
	 * here. This fact is transmitted though the argument
	 * UIO_SYSSPACE. A special flag in the thread structure
	 * will be set which will bypass ulimit checking
	 */
        status = fp_write(bin, buf, len, 0, UIO_SYSSPACE, &cnt);

	if(status != 0 || cnt < len){

		if(audit_panic == AUDIT_PANIC){

        		assert(audit_panic == AUDIT_NO_PANIC);

		}
		else {
			return(-1);
		}
	}
	return(0);
}
