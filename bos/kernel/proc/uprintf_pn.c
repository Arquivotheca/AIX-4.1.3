static char sccsid[] = "@(#)84  1.4  uprintf_pn.c, sysproc, bos410 10/19/93 14:00:39";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: NLuprintfx
 *		upcheck
 *		updone
 *		upfexit
 *		upfget
 *		upfinit
 *		upquemsg
 *		chkfmt
 *		NLuprintf
 *
 *   ORIGINS: 27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/uprintf.h>
#include <sys/malloc.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/vnode.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/intr.h>
#include <sys/sleep.h>
#include <sys/lockl.h>
#include <sys/specnode.h>
#include <sys/syspest.h>
#include <sys/systm.h>

struct upfbuf *upffree = NULL;	/* pinned upfbuf free list anchor         */
struct upfbuf *upfmlist = NULL;	/* message list	anchor (pageable upfbufs) */
struct upfbuf *upfmlisti = NULL; /* message list anchor (pinned upfbufs)  */

int upfsleep = EVENT_NULL;	/* waitlist for master uprintf process      */
int upfmsgwait = EVENT_NULL;	/* waitlist for procs with pending messages */

uint upfseqnum = 0;		/* message arrival sequence number   */
pid_t upfmaster = 0;		/* pid of the master uprintf process */

/*
 * NAME: 	upfinit()
 *
 * FUNCTION:	called at system initialization time to initialize
 *		the pinned upfbuf free list.
 *		
 *		
 * PARAMETERS:
 *            	none.
 *
 * RETURN VALUE:
 *              none.
 *	
 */

upfinit()
{
	struct upfbuf *uptab, *up;

	/* get pinned storage for upfbuf list entries.
	 */
	uptab = (struct upfbuf *)
		   xmalloc((UP_NBUFS * sizeof(struct upfbuf)),0,pinned_heap);
	assert(uptab != NULL);

	/* init the free list.
	 */
	for (up = uptab; up < uptab + (UP_NBUFS - 1); up++)
		up->up_next = (up + 1);
	up->up_next = NULL;

	/* update free list pointer.
	 */
	upffree = uptab;

	return(0);
}

/*
 * NAME: 	NLuprintfx(uprintf)
 *
 * FUNCTION:	queue a NLS message to the process's controlling tty.
 *		
 *		the NLS message described the specified uprintf struct
 *		is queued for delivery by the uprintf process. 
 *		
 *		this procedure is similiar to NLuprintf();  however, all
 *		strings parameters (catalog name, default message,
 *		print arguments) must reside in pinned, unloadable,
 *		static storage. also, the passed uprintf struct must
 *		not reside in pagable storage.
 *
 *		this procedure does not page fault or block and
 *		must be called under a process.
 *
 * PARAMETERS:
 *            	uprintf	- pointer to a uprintf struct describing the
 *			  message to be printed.
 *
 * RETURN VALUE:
 *              0 	- success
 *		ESRCH	- uprintf process not active
 *		ENOMEM	- no free upfbufs 
 *		ENODEV	- no controlling tty
 *		EINVAL	- process cannot receive messages or invalid
 *			  parameter.
 *	
 */

NLuprintfx(uprintf)
struct uprintf *uprintf;
{
	int ipri, rc, nargs;
	struct upfbuf *up;
	ushort fmtvec;

	/* check if the message can be sent.
	 */
	if (rc = upcheck(uprintf,(UP_NLS|UP_INTR),&fmtvec,&nargs))
		return(rc);

	/* make sure the master uprintf process is active.
	 * WHY???
	 */
	ipri = disable_lock(INTMAX, &uprintf_lock);
	if (upfmaster == 0)
	{
		unlock_enable(ipri, &uprintf_lock);
		return(ESRCH);
	}

	/* allocate a upfbuf from the free list.
	 */
	if (upffree == NULL)
	{
		unlock_enable(ipri, &uprintf_lock);
		return(ENOMEM);
	}
	up = upffree;
	upffree = up->up_next;
	unlock_enable(ipri, &uprintf_lock);

	/* fill out the upfbuf.
	 */
	bcopy(uprintf,&up->up_uprintf,sizeof(struct uprintf));
	up->up_fmtvec = fmtvec;
	up->up_nargs = nargs;
	up->up_pid = curproc->p_pid;
	up->up_ttyd = U.U_ttyd;
	up->up_ttyc = U.U_ttympx;
	up->up_flags = (UP_INTR|UP_NLS);

	ipri = disable_lock(INTMAX, &uprintf_lock);

	/* get the sequence number of the upfbuf and
	 * increment to next.
	 */
	up->up_seq = upfseqnum;
	upfseqnum += 1;

	/* add the upfbuf to the pinned list.
	 */
	if (upfmlisti != NULL)
	{
		up->up_next = upfmlisti->up_next;
		upfmlisti->up_next = up;
	}
	else
	{
		up->up_next = up;
	}
	upfmlisti = up;

	/* increment the current proccess's message count
	 * and wakeup the uprintf process.
	 */
	curproc->p_msgcnt++;
	e_wakeup(&upfsleep);	

	unlock_enable(ipri, &uprintf_lock);

	return(0);
}

/*
 * NAME:        upcheck(upf,type,fmtvec)
 *
 * FUNCTION:    determines if the process can receive messages and
 *              validates uprintf parameters.
 *
 * PARAMETERS:
 *              upf     - uprintf struct pointer
 *              type    - message type
 *              fmtvec  - pointer to a returned format vector describing
 *                        the print arguments
 *              nargs   - pointer to a the returned number of print
 *                        arguments found in the format string
 *
 * RETURN VALUE:
 *              0       - success
 *              ENODEV  - no controlling tty
 *              EINVAL  - process cannot receive messages, invalid
 *                        catalog name, invalid format string, or
 *                        invalid number of print arguments
 *              errors from subroutines
 *
 */

upcheck(upf,type,fmtvec,nargs)
struct uprintf *upf;
int type;
ushort *fmtvec;
int *nargs;
{
        int rc, ipri;

        /* make sure the current process is valid and
         * can receive messages.
         */
        ipri = disable_lock(INTMAX, &uprintf_lock);

        if (curproc->p_flag & (SEXIT | SKPROC )) {
                unlock_enable(ipri, &uprintf_lock);
                return(EINVAL);
        }
        unlock_enable(ipri, &uprintf_lock);

        /* check if the process has a controlling tty.
         */
        if (U.U_ttyd == NODEVICE)
                return(ENODEV);

        if (!U.U_ttysid)
                return(ENODEV);

        /* check to see if the listed controlling device is the actual yours */
        if (U.U_procp->p_sid != *(U.U_ttysid))
                return(ENODEV);

        /* check if the catalog name exists and has a
         * valid length if NLS support required.
         */
        if (type & UP_NLS)
        {
                if (upf->upf_NLcatname == NULL ||
                    strlen(upf->upf_NLcatname) > UP_MAXCAT)
                        return(EINVAL);
        }

        /* check if the default message exists and has a
         * valid length.
         */
        if (upf->upf_defmsg == NULL || strlen(upf->upf_defmsg) > UP_MAXSTR)
                return(EINVAL);

        /* examine the format string.
         */
        if (rc = chkfmt(upf->upf_defmsg,nargs,fmtvec))
                        return(rc);

        /* all is ok.
         */
        return(0);

}

/*
 * NAME: 	upquemsg(upf,type,fmtvec,nargs)
 *
 * FUNCTION:	constucts a upfbuf for the specified uprintf struct
 *		and type and adds the upbuf to the pageable message
 *		list.  the process's (current) message count is
 *		incremented to reflect the queued message.
 *
 * PARAMETERS:
 *            	upf	- pointer to uprintf struct
 *		type	- message type
 *		fmtvec	- format vector for default format string
 *		nargs	- number of arguments in format string
 *
 * RETURN VALUE:
 *		0	- success
 *		ESRCH	- uprintf process not active
 *		ENOMEM	- memory allocation for upfbuf failed
 *              errors from subroutines
 *	
 */

upquemsg(upf, type, fmtvec, nargs)
struct uprintf *upf;
int type;
ushort fmtvec;
int nargs;
{
	int ipri, rc;
	struct upfbuf *up;

	/* check if the uprintf master process is active.
	 */
	if (upfmaster == 0)
		return(ESRCH);

	if ((up = (struct upfbuf *) malloc(sizeof(struct upfbuf))) == NULL)
		return(ENOMEM);

	/* init the upfbuf.
	 */
	up->up_NLcatname = NULL;
	up->up_defmsg = NULL;
	up->up_nargs = 0;
	up->up_fmtvec = 0;

	/* setup catalog name if NLS required.
	 */
	if (type & UP_NLS)
	{
		if (rc = getstr(upf->upf_NLcatname,&up->up_NLcatname,
				UP_MAXCAT))
			goto errout;
	}

	/* setup default message.
	 */
	if (rc = getstr(upf->upf_defmsg,&up->up_defmsg,UP_MAXSTR))
		goto errout;

	/* setup print arguments.
	 */
	if (rc = getpargs(upf,up,fmtvec,nargs))
		goto errout;

	/* finish updating the entry.
	 */
	up->up_pid = curproc->p_pid;
	up->up_NLsetno = upf->upf_NLsetno;
	up->up_NLmsgno = upf->upf_NLmsgno;
	up->up_ttyd = U.U_ttyd;
	up->up_ttyc = U.U_ttympx;
	up->up_flags = type;

	ipri = disable_lock(INTMAX, &uprintf_lock);
	
	/* get the sequence number and increment
	 * to next.
	 */
	up->up_seq = upfseqnum;
	upfseqnum += 1;

	/* add the upfbuf to the message list.
	 */
	if (upfmlist != NULL)
	{
		up->up_next = upfmlist->up_next;
		upfmlist->up_next = up;
	}
	else
	{
		up->up_next = up;
	}
	upfmlist = up;

	/* increment the current proccess's message count
	 * and wakeup the uprintf master process.
	 */
	curproc->p_msgcnt++;
	e_wakeup(&upfsleep);	

	unlock_enable(ipri, &uprintf_lock);

	return(0);


	/* errout. free any memory used to hold string and
	 * the upfbuf.
	 */
	errout:

	freestrs(up);
	free(up);
	return(rc);
}

/*
 * NAME: 	upfget(upfbuf,upfdata)
 *
 * FUNCTION:	message get system call to get a upfbuf and the
 *		uprintf data associated with the message.
 *
 *		upfget() checks if the calling process is the master
 *		uprintf process.  if not and there is no master process
 *		the calling process will become the master process.
 *		
 *		the message lists are check for upfbuf to be processed.
 *		if both lists are empty, the process sleep; otherwise,
 *		a upfbuf is gotten from one lists and assign to the 
 *		process.  the upfbuf and the uprintf data associated
 *		with the upfbuf is copied to the user supplied buffers.
 *
 * PARAMETERS:
 *            	upfbuf	- pointer to a user supplied upfbuf
 *            	upfdata	- pointer to a user supplied upfdata buffer.
 *
 * RETURN VALUE:
 *	0 	- success
 *	-1	- error occurred and u.u_error set to one of the following
 *		  errnos:
 *
 *		 EBUSY	- uprintf master process exists, or
 *		 EFAULT	- error in copying out upfbuf or upfdata
 *		 EPERM	- caller did not have required privilege 
 *	
 */

upfget(upfbuf, upfdata)
struct upfbuf *upfbuf;
struct upfdata *upfdata;
{
	int ipri, len, rc, arg;
	struct proc *procp;
	struct upfbuf *up, **list;
	char *dptr;
	struct uthread *ut;

	ut = curthread->t_uthreadp;

	/* check if caller has required privilege.
	 */
	if (privcheck(SET_PROC_RAC) == EPERM)
	{
		ut->ut_error = EPERM;
		return(-1);
	}

	ipri = disable_lock(INTMAX, &uprintf_lock);

	/* check if this is the master uprintf process.  if not,
	 * make the current process the uprintf process if none
	 * exists.
	 */
	if (upfmaster != curproc->p_pid)
	{
		if (upfmaster == 0)
		{
			upfmaster = curproc->p_pid;
		}
		else
		{
			unlock_enable(ipri, &uprintf_lock);
			ut->ut_error = EBUSY;
			return(-1);
		}
	}

	/* sleep if both message lists are empty.
	 */
	if (upfmlisti == NULL && upfmlist == NULL)
	{
		/* return if the sleep was interrupted by a signal.
	   	 */
		if (e_sleep_thread(&upfsleep, &uprintf_lock, 
			LOCK_HANDLER|INTERRUPTIBLE) == THREAD_INTERRUPTED)
		{
			unlock_enable(ipri, &uprintf_lock);
			ut->ut_error = EINTR;
			return(-1);
		}
	}

	/* determine which list to take the upfbuf from. if both
	 * lists have entries, examine the sequence numbers for
	 * the upfbufs at the head of the list.  otherwise, get
	 * the upfbuf from the list with entries.
	 */
	if (upfmlisti != NULL && upfmlist != NULL)
	{
		/* both list have entries. choose the list with
	 	 * the lowest sequence number.
		 */
		if (upfmlisti->up_seq < upfmlist->up_seq &&
		    upfmlist->up_seq < upfseqnum)
			list = &upfmlisti;
		else
			list = &upfmlist;
	}
	else if (upfmlisti != NULL)
	{
		list = &upfmlisti;
	}
	else
	{
		assert(upfmlist != NULL)
		list = &upfmlist;
	}
			
	/* get a upfbuf off of the appropriate message list.
	 */
	up = (*list)->up_next;
       	if (*list == up)
       		*list = NULL;      
       	else                            
       		(*list)->up_next = up->up_next;

	unlock_enable(ipri, &uprintf_lock);

	/* set the upfbuf pointer in the uprintf process's
	 * ublock.  This field is not protected by a lock
	 * because it is assumed that the uprintf demon 
	 * is single threaded, which allows this field to
	 * referenced in pageable text, resulting in a 
	 * slightly lighter system.
	 */
	U.U_message = (void *) up;

	/* copy out the upfbuf.
	 */
	if (u.u_error = copyout(up,upfbuf,sizeof(struct upfbuf)))
	{
		ut->ut_error = EFAULT;
		return(-1);
	}

	/* copy out the catalog name if NLS message.
	 */
	if (up->up_flags & UP_NLS)
	{
		len = strlen(up->up_NLcatname);
		assert(len <= UP_MAXCAT);
		if (copyout(up->up_NLcatname,upfdata->upd_NLcatname,len+1))
		{
			ut->ut_error = EFAULT;
			return(-1);
		}
	}

	/* copy out the default format string.
	 */
	len = strlen(up->up_defmsg);
	assert(len <= UP_MAXSTR);
	if (copyout(up->up_defmsg,upfdata->upd_defmsg,len+1))
	{
		ut->ut_error = EFAULT;
		return(-1);
	}

	/* get the print arguments.
	 */
	dptr = upfdata->upd_prtargs; 
	for (arg = 0; arg < up->up_nargs; arg++)
	{
		/* check if the argument is a string argument or
		 * a 'by value' argument.
		 */
		if (UPF_ISSTR(up->up_fmtvec,arg))
		{
			len = strlen(up->up_args[arg]);	
			assert(len <= UP_MAXSTR);
			if (copyout(up->up_args[arg],dptr,len+1))
			{
				ut->ut_error = EFAULT;
				return(-1);
			}
		}
		else
		{
			if (suword(dptr,up->up_args[arg]))
			{
				ut->ut_error = EFAULT;
				return(-1);
			}

		}
		dptr = dptr + (UP_MAXSTR+1);
	}

	return(0);
}

/*
 * NAME: 	upfexit(procp)
 *
 * FUNCTION:	exit termination handler - called by kexit() for all
 *		exiting processes.  the process will be mark so that
 *		no more messages can be queued for it.  if the exiting
 *		process is the master uprintf process, message completeion
 *		processing will be done for all upfbufs on both message
 *		lists (upfmlist and upfmlisti).  if the process is a
 *		slave uprintf process holding a upfbuf, message completion
 *		processing will be perform on the upfbuf.  if the exiting
 *		process has pending messages, the process will be waited.
 *		
 * PARAMETERS:
 *            	procp	- pointer to proc struct for exiting process.
 *
 * RETURN VALUE:
 *              NONE
 *	
 */

upfexit(procp)
struct proc *procp;
{
	struct upfbuf *up, *next;
	int ipri, klock, plock;

	ipri = disable_lock(INTMAX, &uprintf_lock);

	/* if this is the master uprintf process, we must clean up
	 * the upfbufs on the message lists that have not been gotten.
	 */
	if (procp->p_pid == upfmaster)
	{
		/* zero upfmaster (no master uprintf process).
		 */
		upfmaster = 0;

		/* perform message done processing for all upfbufs
		 * on pinned list.
	   	 */
		while (upfmlisti != NULL) {
			up = upfmlisti;
			if (upfmlisti->up_next == upfmlisti)
				upfmlisti = NULL;
			else
				upfmlisti = upfmlisti->up_next;
			unlock_enable(ipri, &uprintf_lock);
			updone(up);
			disable_lock(INTMAX, &uprintf_lock);
		}	

		/* now take care of the other list.
	   	 */
		while (upfmlist != NULL) {
			up = upfmlist;
			if (upfmlist->up_next == upfmlist)
				upfmlist = NULL;
			else
				upfmlist = upfmlist->up_next;
			unlock_enable(ipri, &uprintf_lock);
			updone(up);
			disable_lock(INTMAX, &uprintf_lock);
		}	
	}

	unlock_enable(ipri, &uprintf_lock);

	/* check if this is a slave uprintf process which exited
	 * without sending it's message.  if so, perform message
	 * done processing for the upfbuf.
	 */
	if ((up = U.U_message) != NULL)
	{
		U.U_message = NULL;
		updone(up);
	}

	disable_lock(INTMAX, &uprintf_lock);

	/* check if the exiting process has pending messages.
	 * if so, the process must wait for the messages to
	 * be processed.
	 */
	if (procp->p_msgcnt != 0)
	{
		/* sleep until all messages have been proccessed.
		 */
		while (procp->p_msgcnt != 0)
			(void)e_sleep_thread(&upfmsgwait, 
					&uprintf_lock, LOCK_HANDLER);
	}
	unlock_enable(ipri, &uprintf_lock);	
	return;
}

/*
 * NAME: 	updone(up)
 *
 * FUNCTION:	message done processing.  free the resource associated
 *		a upfbuf, decrement the target proccess's message count,
 *		and ready the process if the message count falls to
 *		zero and the process was waiting for message delivery.
 *		
 *
 * PARAMETERS:
 *            	up	- pointer to upfbuf
 *
 * RETURN VALUE:
 *              NONE
 *	
 */

updone(up)
struct upfbuf *up;
{
	int ipri, arg;
	struct proc *procp;
	short flags;

	procp = PROCPTR(up->up_pid);
	flags = up->up_flags;

	/* if this is not a pinned upfbuf, free any memory used to
	 * hold strings and free the upfbuf.  pinned upfbufs will
	 * be dealt with below when disabled.
	 */
	if (!(flags & UP_INTR))
	{
		freestrs(up);
		free(up);
	}

	ipri = disable_lock(INTMAX, &uprintf_lock);

	/* if this is a pinned upfbuf, add it to the
	 * free list.
	 */
	if (flags & UP_INTR)
	{
		up->up_next = upffree;
		upffree = up;
	}

	/* decrement the process's message count and wakeup
	 * waitors if the message count is 0 and there are
	 * any waitors.
	 */
	if (--procp->p_msgcnt == 0 && upfmsgwait != EVENT_NULL) {
		e_wakeup(&upfmsgwait);	
		unlock_enable(ipri, &uprintf_lock);
		return;
	}

	unlock_enable(ipri, &uprintf_lock);
	return;
}

/*
 * NAME: 	chkfmt(fmt,nargs,fmtvec)
 *
 * FUNCTION:	examines the format string for format characters,
 *		determines the number of print arguments described
 *		by the string and contructs a bit vector describing
 *		the print arguments found.
 *
 * PARAMETERS:
 *            	fmt	- pointer to print format string
 *		nargs	- returned number of print arguments contained
 *			  within the format string. 
 *		fmtvec  - returned bit vector describing the print
 *			  arguments
 *
 * RETURN VALUE:
 *              0	- success
 *		EINVAL	- invalid format string
 *	
 */

chkfmt(fmt,nargs,fmtvec)
char *fmt;
int *nargs;
ushort *fmtvec;
{
	char *c;
	int arg;
	ushort vec;
	
	*fmtvec = 0;
	*nargs = 0;

	/* examine the format string for format characters.
	 */
	for (c = fmt, arg = 0, vec = 0; *c != '\0'; c++)
	{
		if (*c != '%')
			continue;

		c++;
		switch (*c)
		{
		case '%':
			continue;

		case 'x':
		case 'd':
		case 'u':
		case 'o':
		case 'c':
			/* 'by value' argument.
			 */
			break;

		case 's':
			/* string argument.
			 * mark this argument as a string in the
			 * format vector.
			 */	
			vec |= UPF_VBIT(arg);
			break;

		default:
			return(EINVAL);
			break;
		}
		arg++;
		
		/* check if we are over the maximum number of argument.
		 */
		if (arg > UP_MAXARGS)
			return(EINVAL);
	}

	/* update the format vector and number of argument
	 * with what we have found.
	 */
	*fmtvec = vec;
	*nargs = arg;
	return(0);
}

/*
 * NAME: 	NLuprintf(uprintf)
 *
 * FUNCTION:	queue a NLS message to the process's controlling tty.
 *		
 *		the NLS message described the specified uprintf struct
 *		is queued for delivery by the uprintf process.
 *		
 *		this procedure does not page fault or block and
 *		must be called under a process.
 *
 *
 * PARAMETERS:
 *            	uprintf	- pointer to a uprintf struct describing the
 *			  message to be printed.
 *
 * RETURN VALUE:
 *              0 	- success
 *		ESRCH	- uprintf process not active
 *		ENOMEM	- no memory for upfbuf or string buffers
 *		ENODEV	- no controlling tty
 *		EINVAL	- process cannot receive messages or invalid
 *			  parameter.
 *	
 */

NLuprintf(uprintf)
struct uprintf *uprintf;
{
	int nargs, rc;
	ushort fmtvec;

	/* check if the message can be sent.
	 */
	if (rc = upcheck(uprintf,UP_NLS,&fmtvec,&nargs))
		return(rc);

	/* queue the message.
	 */
	rc = upquemsg(uprintf,UP_NLS,fmtvec,nargs);

	return(rc);
}
