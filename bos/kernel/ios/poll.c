static char sccsid[] = "@(#)46	1.23.1.1  src/bos/kernel/ios/poll.c, sysios, bos41B, 412_41B_sync 1/19/95 14:30:59";
/*
 * COMPONENT_NAME: (SYSIOS) Poll system call; fp_poll kernel service
 *
 * FUNCTIONS:	poll_cleanup		fp_poll		poll
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
/*
 * LEVEL 1,5 Years Bull Confidential Information
 */

#include <sys/types.h>
#include <sys/syspest.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/device.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/sleep.h>
#include <sys/intr.h>
#include <sys/rtc.h>
#include "selpoll.h"
#include <sys/thread.h>
#include <sys/uthread.h>
#include <sys/atomic_op.h>

/*
 * NAME:  poll_cleanup
 *
 * FUNCTION:	Cleanup control blocks:  take control block off of
 *	the device and thread chains, free the control block's storage,
 *	and update the number of control blocks with non-zero rtnevents
 *	(i.e. the number of control blocks with at least 1 reqevent
 *	that occurred).
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called by fp_poll.
 *	It is pageable.
 *
 * NOTES:	This routine handles correctly both the case of
 *		a) something(s) to cleanup, AND
 *		b) nothing to cleanup.
 *
 * DATA STRUCTURES:	sel_cb,	thread,	uthread,
 *			pollfd
 *
 * RETURN VALUES DESCRIPTION:	none.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	selpoll_cleanup		unchain control block from the device and
 *				thread chains; free the control block's storage.
 *	get_curthread		get current thread pointer
 */
void
poll_cleanup(fds, num_fdesp, num_msgsp, fd_flag)

struct pollfd	*fds;		/* ptr to array of pollfd structures	*/
int		*num_fdesp;	/* #fdes/fp's with non-zero rtnevents	*/
int		*num_msgsp;	/* #msg queue's with non-zero rtnevents	*/
int		fd_flag;	/* flag to decrement fd hold counts	*/
{
	struct sel_cb	*cb;	/* ctl blk at head of thread chain	*/
	extern void	selpoll_cleanup();
	ushort oldevents;
	register struct uthread *temp_uthreadp;/* temp pointer to uthread   */

	temp_uthreadp = CURTHREAD->t_uthreadp;

	/*
	 * Run the thread chain (CURTHREAD->t_uthreadp->ut_selchn).
	 */
	while ((cb = temp_uthreadp->ut_selchn) != NULL)
	{
		/*
		 * Report rtnevents from thread chain.
		 */
		oldevents = (fds + cb->corl)->rtnevents;
		(fds + cb->corl)->rtnevents |= (cb->rtnevents &
		    (cb->reqevents | POLLNVAL | POLLERR | POLLHUP));

		if (((fds + cb->corl)->rtnevents != 0) && (oldevents == 0))
		{
			/*
			 * Increment #elements with non-zero rtnevents.
			 */
			if (cb->dev_id == POLL_MSG)
			{
				(*num_msgsp)++;
			}
			else
			{
				(*num_fdesp)++;
			}
		}

		/*
		 * Clean up this control block.
		 * And decrement fd hold count.
		 */
		selpoll_cleanup(fd_flag ? ((fds + cb->corl)->fd) : -1);
	}

	return;

}  /* end poll_cleanup */       

/*
 * NAME:  fp_poll
 *
 * FUNCTION:	The fp_poll kernel service is used to check the I/O status of
 *	multiple file pointers and message queues to see if they are
 *	ready for reading (receiving) or writing (sending), or if they
 *	have an exceptional condition pending.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called by the POLL system call and by device drivers/
 *	kernel extensions.
 *	It is pageable.
 *
 * NOTES:	The fp_poll kernel service applies only to character devices,
 *	pipes, and message queues.  Not all character devices support it.
 *
 *	The timeout parameter is the length of time (IN MILLISECONDS) to wait
 *	for reqevents to occur.
 *
 * DATA STRUCTURES:	pollfd,	timeval, thread, uthread
 *
 * RETURN VALUES DESCRIPTION:	non-negative value upon successful completion:
 *					>0 -- the number of pollfd structs with
 *					a non-zero revents field;
 *					0 -- the call timed out and no pollfd 
 *					structs have non-zero revents fields;
 *				-EINTR or -EINVAL upon unsuccessful completion.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	selpoll			poll a device
 *	poll_wait		wait for events to occur
 *	poll_cleanup		Unchain control block from the device and
 *				thread chains; free the control block's storage.
 *      msgselect               ipc message select routine
 *      et_wait                 clear EVENT_SYNC flag possibly set by selnotify
 *	get_curthread		get current thread pointer
 *	fetch_and_or		to modify TSELECT
 */
int
fp_poll(void *listptr, ulong nfdsmsgs, long timeout, register uint flags)

/* void		*listptr;	ptr to pollfd/pollmsg/pollist structs*/
/* ulong	nfdsmsgs;	#elements in pollfd/pollmsg arrays	*/
/* long 	timeout;	max time to wait for select criteria	*/
/* register uint flags;		does data contain fd's? or fp's?	*/
{

	register int	rc;	/* return code from various routines	*/
	register int	fdes;	/* number of file descriptors/ptrs	*/
	register int	msgs;	/* number of message queue ids		*/
	register int	elem;	/* element# in fds array		*/
	int		num_fdes;/* #fdes/fp's with non-zero rtnevents	*/
	int		num_msgs;/* #msg queue ids with occurred events	*/
	int		num_req_events;	/* # of requested events	*/
	register int	reqev_msk;/* returned events mask		*/
	struct timeval	timeval;/* timeout parm converted to timeval struct*/
	extern int	selpoll();
	extern int	poll_wait();
	extern void	poll_cleanup();
	register struct thread *temp_threadp;	/* temp pointer to thread */

	/*
	 * If user requested 0 pollfd/pollmsg structs be checked, then
	 * poll() merely acts as a timer (i.e. set a timer for the
	 * specified #ticks).
	 */
	if (nfdsmsgs == 0)
	{
		if (timeout == INF_TIMEOUT)
		{
			rc = poll_wait(NULL, CURTHREAD);
		}
		else
		{
			/*
		 	 * Convert timeout from milliseconds to
			 * seconds and microseconds.
		 	 */
			timeval.tv_sec = timeout / 1000;
			timeval.tv_usec = (timeout % 1000) * 1000;
			rc = poll_wait(&timeval, CURTHREAD);
		}
                if (rc != 0) 
                {
			/* poll_wait() was interrupted by a signal,
			 * clear a possible timer et_post()
			 */
			(void)et_wait(EVENT_NDELAY, EVENT_SYNC, EVENT_SHORT);
                   	return(-rc);
                }
                return(0);

	}

	temp_threadp = CURTHREAD;

	/*
	 * Extract from nfdsmsgs the number of file
	 * descriptors/pointers and message queues.
	 */
	fdes = NFDS(nfdsmsgs);
	msgs = NMSGS(nfdsmsgs);

	/*
	 * Caller specified NO_TIMEOUT, so process input synchronously.
	 */
	if (timeout == NO_TIMEOUT)
	{
		reqev_msk = POLLSYNC;
	}
	else
	{
		reqev_msk = 0;
	}

	/*
	 * The TSELECT flag is used to determine if notification has been
	 * received of events that have occurred (i.e. if the flag is
	 * turned off, then an event has occurred).
	 */
	fetch_and_or(&temp_threadp->t_atomic, TSELECT);

	num_fdes = 0;		/* #fdes/fp's with non-zero rtnevents	*/
	num_msgs = 0;		/* #msg queues with non-zero rtnevents	*/
	num_req_events = 0;		/* # of requested events */

	/*
	 * Process caller's array of pollfd structs.
	 */
	for (elem = 0; elem < fdes; elem++)
	{
		if ((((struct pollfd *)listptr + elem)->fd < 0) ||
		   (((struct pollfd *)listptr + elem)->reqevents == 0) )
		{
			/*
		 	 * Ignore this array element.
		 	 */

			/* svid: if (fd < 0) revents = 0; */
			((struct pollfd *)listptr + elem)->rtnevents = 0;
			continue;
		}

		num_req_events++;
		((struct pollfd *)listptr + elem)->rtnevents = 0;
		/*
		 * Call selpoll to check the status of the
		 * specified file descriptor/ptr.
		 */
		if (flags & POLL_FDMSG)	/* input is file descriptors	*/
		{
			rc = selpoll(((struct pollfd *)listptr + elem)->fd,
				     elem,
				     ((struct pollfd *)listptr + elem)->reqevents | reqev_msk,
				     &(((struct pollfd *)listptr + elem)->rtnevents),
				     POLL_FDMSG,
				     NULL);
		}
		else
		{
			rc = selpoll(((struct pollfd *)listptr + elem)->fd,
				     elem,
				     ((struct pollfd *)listptr + elem)->reqevents | reqev_msk,
				     &(((struct pollfd *)listptr + elem)->rtnevents),
				     0,
				     NULL);
		}

		/*
		 * defect 18359: let selpoll set POLLERR if error
		 */

		if (((struct pollfd *)listptr + elem)->rtnevents != 0)
		{
			num_fdes++;
			/*
			 * At least 1 of the caller's reqevents has occurred,
			 * so process the remaining data synchronously.
			 */
			reqev_msk = POLLSYNC;
		}
	
		/*
	 	 * If TSELECT is off, notification has been received, at least
	 	 * one event has occurred; therefore, all other selpoll() calls
		 * will be synchronous.
	 	 */
		if (!(temp_threadp->t_atomic & TSELECT))
		{
			reqev_msk = POLLSYNC;
		}

	}  /* end for loop */

	/*
	 * Process caller's array of pollmsg structs.
	 */
	for (elem = fdes; elem < (msgs + fdes); elem++)
	{
		if ((((struct pollmsg *)listptr + elem)->msgid < 0) ||
		   (((struct pollmsg *)listptr + elem)->reqevents == 0) )
		{
			/*
		 	 * Ignore this array element.
		 	 */
			continue;
		}

		num_req_events++;
		((struct pollmsg *)listptr + elem)->rtnevents = 0;
		rc = msgselect(((struct pollmsg *)listptr + elem)->msgid,
			       elem,
			       ((struct pollmsg *)listptr + elem)->reqevents | reqev_msk,
			       &(((struct pollmsg *)listptr + elem)->rtnevents));
		if (rc != 0)
		{
			/*
			 * msgselect returned an error.
			 */
			((struct pollmsg *)listptr + elem)->rtnevents |= POLLERR;
		}
		if (((struct pollmsg *)listptr + elem)->rtnevents != 0)
		{
			num_msgs++;
			/*
			 * At least 1 of the caller's reqevents has occurred,
			 * so process the remaining data synchronously.
			 */
			reqev_msk = POLLSYNC;
		}

		/*
		 * If TSELECT is off, notification has been received, at least
	 	 * one event has occurred; therefore, all other selpoll() calls
		 * will be synchronous.
	 	 */
		if (!(temp_threadp->t_atomic & TSELECT))
		{
			reqev_msk = POLLSYNC;
		}

	}  /* end for loop */

	/*
	 * avoid going to sleep waiting for nothing to happen.
	 * i.e. no events requested with infinite timeout
	 */ 
	if ((fdes > 0 || msgs > 0) && (num_req_events == 0) && (timeout == INF_TIMEOUT))
	{
		poll_cleanup((struct pollfd *)listptr, &num_fdes, &num_msgs,
			     flags & POLL_FDMSG);

		/*
		 * Clear TSELECT flag
		 */
		fetch_and_and(&(temp_threadp->t_atomic), ~TSELECT);
		return(-EINVAL);
	}

	rc = 0;
	if ((!reqev_msk) && (temp_threadp->t_atomic & TSELECT))
	{
		/*
		 * None of the caller's reqevents have occurred yet,
		 * so wait the specified length of time for a selnotify.
		 */

		if (timeout == INF_TIMEOUT)
		{
			rc = poll_wait(NULL, temp_threadp);
		}
		else
		{
			/*
		 	 * Convert timeout from milliseconds to
			 * seconds and microseconds.
		 	 */
			timeval.tv_sec = timeout / 1000;
			timeval.tv_usec = (timeout % 1000) * 1000;
			rc = poll_wait(&timeval, temp_threadp);
		}

	}

	/*
	 * Call poll_cleanup to
	 * a) count the #elements in the array of pollfd structs with non-zero
	 *    rtnevents;
	 * b) free any control blocks' storage;
	 * c) decrement file descriptor hold counts.
	 */
	poll_cleanup((struct pollfd *)listptr, &num_fdes, &num_msgs,
							flags & POLL_FDMSG);

	/*
	 * If a selnotify() has occurred, but we didn't have to poll_wait()
	 * for an event to occur, then we must clear the event (EVENT_SYNC)
	 * that selnotify() e_post()ed.
	 */
	if (!(temp_threadp->t_atomic & TSELECT))
	{
		(void)et_wait(EVENT_NDELAY, EVENT_SYNC, EVENT_SHORT);
	}
	else
	{
	    /*
	     * Clear TSELECT flag
	     */
       	     fetch_and_and(&(temp_threadp->t_atomic), ~TSELECT);
        }
		
	
	if (rc == 0)
	{
		/*
		 * Total #events that occurred.
		 */
		return((num_fdes) | (num_msgs << 16));
	}
	else
	{
		/* poll_wait() was interrupted by a signal,
		 * clear a possible timer et_post()
		 */
		(void)et_wait(EVENT_NDELAY, EVENT_SYNC, EVENT_SHORT);
		return(-rc); 	/* return of poll_wait */
	}

}  /* end fp_poll */       

/*
 * NAME:  poll
 *
 * FUNCTION:	The POLL system call is used to check the I/O status of
 *	multiple file pointers/descriptors and message queues to see if they
 *	are ready for reading (receiving) or writing (sending), or if they
 *	have an exceptional condition pending.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called by user programs.
 *	It is pageable.
 *
 * NOTES:	The POLL system call applies only to character devices,
 *	pipes, and message queues.  Not all character devices support it.
 *
 * DATA STRUCTURES:	pollfd/pollmsg,	timeval, thread, uthread
 *
 * RETURN VALUES DESCRIPTION:	non-negative value upon successful completion:
 *					>0 -- the number of pollfd/pollmsg
 *					structs with a non-zero revents field;
 *					0 -- the call timed out and no pollfd/
 *					pollmsg structs have non-zero revents
 *					fields;
 *				-1 upon unsuccessful completion, and errno set
 *				to EAGAIN, EINTR, EINVAL, or EFAULT.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	poll_wait		wait for events to occur
 *	xmalloc			allocate storage for pollfd/pollmsg structs
 *	xmfree			free previously allocated storage
 *	copyin			copy data from user to kernel space
 *	copyout			copy data from kernel to user space
 *	fp_poll			check the I/O status of multiple file pointers
 *				and message queues
 *	get_curthread		get current thread pointer
 */
int
poll(void *listptr, ulong nfdsmsgs, long timeout)

/* void		*listptr;	ptr to pollfd/pollmsg/pollist structs*/
/* ulong	nfdsmsgs;	#elements in pollfd & pollmsg arrays	*/
/* long 	timeout;	max time to wait for select criteria	*/
{

	register int	rc;	/* return code from various routines	*/
	register int	fdes;	/* number of file descriptors/ptrs	*/
	register int	msgs;	/* number of message queue ids		*/
	register int	elem;	/* element# in fds array		*/
	register int	fp_poll_rc;/* return code from fp_poll routine	*/
	struct pollfd	*kfds;	/* kernel's copy of user data		*/
	struct timeval	timeval;/* timeout parm converted to timeval struct*/
	extern int	poll_wait();
	extern int	copyin();
	extern int	copyout();
	register struct thread *temp_threadp;	/* temp pointer to thread */
	register struct uthread *temp_uthreadp;	/* temp pointer to uthread */

	temp_threadp = CURTHREAD;
	temp_uthreadp = temp_threadp->t_uthreadp;

	/* Check for a valid timeout
	 */
	if (timeout < -1)
	{
		temp_uthreadp->ut_error = EINVAL;
		return(-1);
	}

	/*
	 * If user requested 0 pollfd/pollmsg structs be checked, then poll()
	 * merely acts as a timer (i.e. set a timer for the specified
	 * #ticks).
	 */
	if (nfdsmsgs == 0)
	{
		if (timeout == INF_TIMEOUT)
		{
			rc = poll_wait(NULL, temp_threadp);
		}
		else
		{
			/*
		 	 * Convert timeout from milliseconds to
			 * seconds and microseconds.
		 	 */
			timeval.tv_sec = timeout / 1000;
			timeval.tv_usec = (timeout % 1000) * 1000;
			rc = poll_wait(&timeval, temp_threadp);
		}
                if (rc != 0)
  		{  
			temp_uthreadp->ut_error = rc;
			/* poll_wait() was interrupted by a signal,
			 * clear a possible timer et_post()
			 */
			(void)et_wait(EVENT_NDELAY, EVENT_SYNC, EVENT_SHORT);
			return(-1);
		}
		return(0);


	}

	/*
	 * Extract from nfdsmsgs the number of file
	 * descriptors/pointers and message queues.
	 */
	fdes = NFDS(nfdsmsgs);
	msgs = NMSGS(nfdsmsgs);

	/* svid: "EINVAL: The argument nfds is greater than {OPEN_MAX}." */
	if (fdes > OPEN_MAX) {
		temp_uthreadp->ut_error = EINVAL;
		return(-1);
	}

	/*
	 * Allocate memory for the array of pollfd/pollmsg structs.
	 */
	kfds = (struct pollfd *)xmalloc(((fdes + msgs) * sizeof(struct pollfd)),
					0,
					kernel_heap);
	if (kfds == NULL)
	{
		temp_uthreadp->ut_error = EAGAIN;
		return(-1);
	}

	/*
	 * Copy data from user memory to kernel memory.
	 */
	rc = copyin(listptr, kfds, ((fdes + msgs) * sizeof(struct pollfd)));
	if (rc != 0)
	{
		/*
 		 * Free memory for the array of pollfd structs.
 		 */
		(void)xmfree((caddr_t)kfds, kernel_heap);
		temp_uthreadp->ut_error = EFAULT;
		return(-1);
	}

	/*
	 * Call fp_poll to process the array of pollfd structs.
	 */
	fp_poll_rc = fp_poll((void *)kfds, nfdsmsgs, timeout, POLL_FDMSG);
	if ((fp_poll_rc == -EINVAL) || (fp_poll_rc == -EINTR))
	{
		/*
	 	 * Free memory for the array of pollfd structs.
	 	 */
		(void)xmfree((caddr_t)kfds, kernel_heap);
		temp_uthreadp->ut_error = -fp_poll_rc;
		return(-1);
	}

	/*
	 * Copy rtnevents from kernel memory to user memory.
	 */
	for (elem = 0; elem < (fdes + msgs); elem++)
	{
		rc = copyout(&((kfds + elem)->rtnevents), &(((struct pollfd *)listptr + elem)->rtnevents), sizeof(ushort));
		if (rc != 0)
		{
			/*
	 		 * Free memory for the array of pollfd structs.
	 		 */
			(void)xmfree((caddr_t)kfds, kernel_heap);
			temp_uthreadp->ut_error = EFAULT;
			return(-1);
		}
	}

	/*
	 * Free memory for the array of pollfd structs.
	 */
	(void)xmfree((caddr_t)kfds, kernel_heap);

 	/*
	 * #elements with non-negative rtnevents.
	 */
	return(fp_poll_rc);

}  /* end poll */       
