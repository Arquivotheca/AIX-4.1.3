static char sccsid[] = "@(#)44	1.15.1.12  src/bos/kernel/ios/selpoll.c, sysios, bos411, 9428A410j 6/23/94 13:55:55";
/*
 * COMPONENT_NAME: (SYSIOS) Kernel routines to support Select/Poll
 *
 * FUNCTIONS:	selpoll_cleanup		poll_wait		selreg
 *		selpoll			fp_select		unselect
 *		selpoll_timeout		selpoll_untimeout	devselect
 *
 * ORIGINS: 27, 83
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
 * LEVEL 1,5 Years Bull Confidential Information
 */

#include <sys/types.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <sys/device.h>
#include <sys/priv.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/intr.h>
#include <sys/sleep.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/timer.h>		/* define the timer request block - trb	*/
#include <sys/poll.h>
#include <sys/machine.h>
#include "selpoll.h"
#include <sys/thread.h>
#include <sys/uthread.h>
#include <sys/lock_def.h>

Simple_lock select_lock;	/* alloc and init is done in selpollinit() */
				/* under #ifdef _POWER_MP */
/*
 * q struct
 */
struct que {
	struct que *nxt;
	struct que *prv;
};

/*
 * These values MUST be maintained to reflect the selstuff struct.
 */
#define	HDR_BYTES	(3*sizeof (void *) + 2*sizeof (short))
#define	NCB		((PAGESIZE - HDR_BYTES) / sizeof (struct sel_cb))

struct selstuff {
	struct que q;
	struct sel_cb *freelist;
	short nfree;
	short pad;
	struct sel_cb cbs[NCB];
};

static struct que stuff = { &stuff, &stuff }, empty = { &empty, &empty };
static nstuff;			/* depth of stuff queue	*/

/*
 * cb_to_hdr -	get back to the selstuff header given a control block
 */
#define	cb_to_hdr(cb)	(struct selstuff *) (~(PAGESIZE-1) & (ulong) (cb))

/*
 * init_q -	initialize a queue (they aren't textbook queues)
 */
#define init_q(q)	((q)->nxt = (q)->prv = (q))

/*
 * en_q -	add a queue element `q' just after `p'
 */
#define en_q(p, q) {						\
	register struct que *x = (p), *n = (q);			\
								\
	(x->nxt = ((n->nxt = x->nxt)->prv = n))->prv = x;	\
}

/*
 * de_q -	remove `q' from the queue
 *	`q' is left pointing to where it used to be.
 */
#define de_q(q)		(((q)->nxt->prv = (q)->prv)->nxt = (q)->nxt)

/*
 * devsel_hash, a pointer to an array of sel_cb (select control block)
 * structures, is declared in selnotify.c.  devsel_hash is used to chain
 * control blocks (based on their device id and unique id), using a
 * hashing algorithm.
 */
extern struct sel_cb	*devsel_hash[];

/*
 * NAME:	selpoll_cleanup
 *
 * FUNCTION:	Cleanup:  take control block off of the device and
 *	thread chains, and then free the control block's storage.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called by select and poll_cleanup.
 *	It cannot page fault.
 *
 * NOTES:	The fd_flag parameter has the value -1 when file pointers
 *	were specified and the file descriptor number when file descriptors
 *	were specified.
 *
 * DATA STRUCTURES:	sel_cb
 *
 * RETURN VALUES DESCRIPTION:	none.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	xmfree			free previously allocate storage
 *	sel_hash		remove control block from select hash table
 *	disable_lock            disable to INTMAX and take select_lock
 *	unlock_enable           release select_lock and enable
 *	get_curthread
 *	ufdrele                 decrement hold count on file descriptor
 */
void
selpoll_cleanup(int fd_flag)
{
	register struct sel_cb	*cb;	/* ctl blk at head of thread chain*/
	register struct sel_cb	*next_cb;/* ctl blk after head of thread chain*/
	register struct selstuff *ss;
	register x;
	register struct uthread *temp_uthreadp;/* temp pointer to uthread   */
	register int	cb_dev_id;		/* current dev_id     */
	register int	cb_corl;		/* current correlator */

	temp_uthreadp = CURTHREAD->t_uthreadp;

	/*
	 * Take the control block off the device chain.
	 */
	if (temp_uthreadp->ut_selchn != NULL) {
		cb = temp_uthreadp->ut_selchn;
		sel_hash(cb, SEL_HASH_DEL);	/* del ctl blk from hash chain*/
		/*
		 * Take the control block off the thread chain.
		 */
		temp_uthreadp->ut_selchn = cb->thread_chain;
		cb_dev_id = cb->dev_id;	/* once the cb is put on the freelist */
		cb_corl = cb->corl;	/* it will be reused by selreg().     */

		ss = cb_to_hdr(cb);		/* get back to header	*/
#ifdef _POWER_MP	/* POWER_MP	*/
		x = disable_lock(INTMAX, &select_lock);
#else			/* POWER_MP	*/
		x = disable_ints();
#endif			/* POWER_MP	*/

		cb->thread_chain = ss->freelist;
		ss->freelist = cb;

		switch (++ss->nfree) {
		    case NCB:			/* !full => full	*/
			if (nstuff > 1) {	/* at least 2 stuffi	*/
				--nstuff;	/* reduce by 1		*/
				de_q(&ss->q);	/* remove from queue	*/
			} else
				ss = 0;		/* nothing to xmfree	*/
			break;

		    case 1:			/* empty => !full	*/
			de_q(&ss->q);		/* remove from empty	*/
			en_q(stuff.prv, &ss->q);/* add to end		*/
			ss = 0;			/* don't xmfree		*/
			++nstuff;
			break;

		    default:
			ss = 0;			/* don't xmfree		*/
			break;
		}

#ifdef _POWER_MP	/* POWER_MP	*/
		unlock_enable(x, &select_lock);
#else			/* POWER_MP	*/
		enable_ints(x);
#endif			/* POWER_MP	*/

		/*
		 * Check if a file descriptor hold count must be decremented.
		 * The following is further complicated by the possibility of
		 * nested control blocks originating from device driver calls
		 * to fp_select for which selpoll() cannot increment a count.
		 */
		if (fd_flag != -1) {
			if (cb_dev_id != POLL_MSG) {
				next_cb = temp_uthreadp->ut_selchn;
				if (next_cb == NULL ||
				    next_cb->corl != cb_corl) {
					(void) ufdrele(fd_flag);
				}
			}
		}

		if (ss)
			xmfree((void *) ss, pinned_heap);
	}
}

/*
 * NAME:	poll_wait
 *
 * FUNCTION:	Wait for events to occur.  This includes calling the
 *	selpoll_timeout() function (if a non-infinite timeout value was
 *	specified), calling et_wait(), and then calling selpoll_untimeout()
 *	(if a non-infinite timeout value was specified).
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called by fp_poll, poll, and select.
 *	It cannot page fault.
 *
 * NOTES:	This routine disables interrupts across et_wait to make sure
 *              timer does not go off in between return from et_wait and
 *		disabling of timer.
 *
 * DATA STRUCTURES:	thread,
 *			trb
 *
 * RETURN VALUES DESCRIPTION:	0 upon successful completion;
 *				EINTR if et_wait() returns EVENT_SIG.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	assert
 *	selpoll_timeout		wait for the specified length of time
 *	selpoll_untimeout	remove the previously-specified timer
 *	et_wait			wait for an event to occur
 */
int
poll_wait(timeout, threadp)

struct timeval	*timeout;	/* timeout value in secs/usecs		*/
struct thread	*threadp;	/* pointer to thread structure		*/
{
/*	register int		ipri;	/* interupt priority level */
	register int		rc;	/* return code from et_wait	*/
	register struct trb	*timer;	/* return value from selpoll_timeout*/
	extern struct trb	*selpoll_timeout();
	extern void		selpoll_untimeout();
	extern void		unselect();
	extern ulong		et_wait();

	if (timeout != NULL)
	{
		/*
		 * Start a timer that will "go off" in "ticks" #clock ticks.
		 */
		timer = selpoll_timeout((void(*)())unselect, threadp, timeout);
	}

	/*
	 * Wait for the event(s) to occur.
	 */
	/*
	 * If poll_wait() returns EINTR, select/poll/fp_poll will call
	 * et_wait(EVENT_NDELAY, EVENT_SYNC, EVENT_SHORT) to clear a possible
	 * timer et_post(). The i_disable()/i_enable() are no longer necessary.
	 */
	rc = (int)et_wait(EVENT_SYNC, EVENT_SYNC, EVENT_SIGRET);

	if (timeout != NULL)
	{
		/*
		 * Stop the timer that was started by selpoll_timeout.
		 */
		selpoll_untimeout(timer);
	}


		/* now check return code */
	if (rc == EVENT_SIG)		/* a signal was received	*/
	{
		rc = EINTR;
	}
	else
	{
		assert(rc == EVENT_SYNC);
		/*
		 * Either the timer went off and unselect posted us,
		 * or selnotify was called and posted an event.
		 */
		rc = 0;
	}

	return(rc);

}  /* end poll_wait */

/*
 * NAME:	selreg
 *
 * FUNCTION:	Register a select request in order to support asynchronous
 *	notification that the selected event has occurred for the select or
 *	poll system call.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called by file systems.
 *	It cannot page fault.
 *
 * NOTES:	This service is used to allocate and initialize a control block
 *	used by the selnotify kernel service to find and post any threads
 *	waiting on the notification of the event. The dev_id and unique_id
 *	parameters uniquely identify the specific instance of the function
 *	being selected (i.e. devno and chan for device drivers, socket address
 *	for sockets, or control block pointer for pipes).  The reqevents
 *	parameter is stored in the control block so that the selnotify service
 *	can find all threads waiting on the specific instance of the function
 *	matching one or more of the events being indicated.  The corl parameter
 *	parameter to this routine is stored in the allocated control block so
 *	that the poll and select system calls can correlate the returned events
 *	in a specific control block with a file descriptor or file pointer.
 *
 *	This service will not allocate a control block if the POLLSYNC flag
 *	is set in the reqevents parameter.
 *
 * DATA STRUCTURES:	sel_cb
 *
 * RETURN VALUES DESCRIPTION:	0 upon successful completion;
 *				EAGAIN if unable to allocate the control block
 *
 * EXTERNAL PROCEDURES CALLED:
 *	assert
 *	xmalloc		allocate pinned storage for a sel_cb control block
 *      sel_hash	add control block to select hash table
 *	disable_lock    disable to INTMAX and take select_lock
 *	unlock_enable   release select_lock and enable
 *	get_curthread
 */
int
selreg(corl, dev_id, unique_id, reqevents, notify)

register int		corl;		/* correlator: file ptr, etc.	*/
register int		dev_id;		/* device id: devno, etc.	*/
register int		unique_id;	/* unique id: chan, etc.	*/
ushort			reqevents;	/* requested events		*/
void  			(*notify)(); 	/* routine for cascaded selects) */
{
	register struct sel_cb	*cb;	/* ctl blk to allocate/initialize*/
	register struct uthread *temp_uthreadp;/* temp pointer to uthread   */

	temp_uthreadp = CURTHREAD->t_uthreadp;


	if (!(reqevents & POLLSYNC)) {
		register struct selstuff *ss;
		register struct sel_cb **cbp;
		register x;

		/*
		 * Allocate a control block.
		 */
	    retry:
		if (nstuff == 0) {
			/*
			 * empty master list.  try to get more space from
			 * the system.
			 */
			assert(sizeof (*ss) <= PAGESIZE);

			ss = (void *) xmalloc(sizeof (*ss)
					      , PGSHIFT, pinned_heap);
			if (!ss)
				return EAGAIN;

			ss->nfree = NCB-1;

			/*
			 * construct the free list
			 */
			for (cbp = &ss->freelist, cb = &ss->cbs[1]
			     ; cb < &ss->cbs[NCB]
			     ; ++cb) {
				*cbp = cb;
				 cbp = &cb->thread_chain;
			}

			*cbp = 0;		/* terminate last one	*/
			 cb  = &ss->cbs[0];	/* use first one	*/

#ifdef _POWER_MP	/* POWER_MP	*/
			x = disable_lock(INTMAX, &select_lock);
#else			/* POWER_MP	*/
			x = disable_ints();
#endif			/* POWER_MP	*/

			en_q(&stuff, &ss->q);	/* add to head of q	*/
			++nstuff;		/* keep queue depth	*/

#ifdef _POWER_MP	/* POWER_MP	*/
			unlock_enable(x, &select_lock);
#else			/* POWER_MP	*/
			enable_ints(x);
#endif			/* POWER_MP	*/
		} else {
			/*
			 * we feed from stuff.nxt, until exhausted, at which
			 * time we remove from stuff.nxt and add to empty.
			 *
			 * there is always at least one available on stuff.nxt,
			 * unless we lost the race on "nstuff", in which case
			 * we just retry.
			 */
#ifdef _POWER_MP	/* POWER_MP	*/
			x = disable_lock(INTMAX, &select_lock);
#else			/* POWER_MP	*/
			x = disable_ints();
#endif			/* POWER_MP	*/
			ss = (void *) stuff.nxt;

			if (ss == (struct selstuff *) &stuff) {
				/*
				 * race on "nstuff".
				 */
				assert(nstuff == 0);
#ifdef _POWER_MP	/* POWER_MP	*/
				unlock_enable(x, &select_lock);
#else			/* POWER_MP	*/
				enable_ints(x);
#endif			/* POWER_MP	*/
				goto retry;
			}

			cb = ss->freelist;
			assert(cb);
			ss->freelist = cb->thread_chain;

			/*
			 * if this is a full one, and we are just depleting it
			 */
			switch (--ss->nfree) {
			    default:		/* !full >= !full	*/
			    case NCB-1:		/*  full => !full	*/
				break;

			    case 0:		/* !full => empty	*/
				/*
				 * move to the empty list, if
				 * appropriate
				 */
				--nstuff;
				de_q(&ss->q);
				en_q(&empty, &ss->q);
				break;
			}

#ifdef _POWER_MP	/* POWER_MP	*/
			unlock_enable(x, &select_lock);
#else			/* POWER_MP	*/
			enable_ints(x);
#endif			/* POWER_MP	*/
		}

		/*
		 * Initialize the control block.
		 */
		cb->reqevents = reqevents;
		cb->rtnevents = 0;
		cb->dev_id    = dev_id;
		cb->unique_id = unique_id;
		cb->threadp   = CURTHREAD;
		cb->corl      = corl;
		cb->notify    = notify;

		/*
		 * Chain the control block onto the thread chain.
		 */
		cb->thread_chain = (struct sel_cb *) temp_uthreadp->ut_selchn;
		temp_uthreadp->ut_selchn = cb;
		sel_hash(cb, SEL_HASH_ADD);	/* add ctl blk to hash chain*/
	}

	return(0);
}

/*
 * NAME:	selpoll
 *
 * FUNCTION:	This kernel service provides a select/poll function for an
 *	open device driver instance.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called by fp_select, fp_poll, and select.
 *	It cannot page fault.
 *
 * NOTES:	This service is used to determine if a particular device has
 *	data available to be read, is capable of writing data, or has an
 *	exceptional condition outstanding.
 *
 *	The file_id parameter to this routine can be either a file descriptor
 *	or a pointer to a file structure.  If the flags parameter has POLL_FDMSG
 *	set, the file_id is a file descriptor; if POLL_NESTED is set, then the
 *	caller has nested calls to select/poll.
 *
 * DATA STRUCTURES:	file,
 *			sel_cb
 *
 * RETURN VALUES DESCRIPTION:	0 upon successful completion;
 *				the non-zero return code from
 *				a) the device driver's ddselect entry point,
 *				b) getf, or
 *				upon unsuccessful completion.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	selpoll_cleanup	unchain the control block from the device and
 *			thread chains; free the control block's storage
 *	file_ops select entry point
 *	get_curthread
 *	ufdgetf		convert file descriptor to a file pointer
 *	ufdhold		increment hold count on file descriptor
 *	ufdrele		decrement hold count on file descriptor
 */
int
selpoll(file_id, corl, reqevents, rtneventsp, flags, notify)

register int 		file_id;	/* file ptr or file desc (see flags)*/
register int 		corl;		/* correlator:  file ptr, etc.	*/
ushort			reqevents;	/* requested events		*/
ushort			*rtneventsp;	/* ptr to occurred events	*/
register uint		flags;		/* POLL_FDMSG/POLL_NESTED	*/
void  			(*notify)(); 	/* routine for cascaded selects) */
{
	struct file		*fp;	/* file ptr; used to call ddselect*/
	register int		rc;	/* return code from various routines*/
	struct sel_cb		*sel_cb;/* ctl blk at head of thread chain*/
	extern int		getf();
	extern void		selpoll_cleanup();
	register struct uthread *temp_uthreadp;/* temp pointer to uthread   */

	if (flags & POLL_FDMSG)
	{
		/*
		 * file_id is a file descriptor.
		 */
		rc = ufdhold(file_id);	/* increment hold count */
		if (rc != 0)
		{
			*rtneventsp = POLLNVAL;
			return(rc);
		}
		rc = ufdgetf(file_id, &fp);
		if (rc != 0)
		{
			(void) ufdrele(file_id);    /* decrement hold count */
			*rtneventsp = POLLNVAL;
			return(rc);
		}
	}
	else
	{
		/*
		 * file_id is a file pointer.
		 */
		fp = (struct file *)file_id;
	}

	/*
	 * If nested poll flag is set, then copy the correlator from the control
	 * block on the top of the pending event chain to be used as the 
	 * correlator for this request; otherwise, ensure that no notify 
	 * routine is called.
	 */
       	temp_uthreadp = CURTHREAD->t_uthreadp;
	sel_cb = temp_uthreadp->ut_selchn;    /* current head of thread chain */
	if (flags & POLL_NESTED)
	{	
		corl = sel_cb->corl;
	}	
	else
	{
		notify = NULL;
	}

	if (!(reqevents & POLLSYNC))
	{
		/*
		 * This is an asynchronous request (i.e. a ctl blk
		 * has previously been allocated/initialized and
		 * put on the device and thread chains).
		 */

		/*
		 * Call the file ops select entry point.
		 */
		rc = (*fp->f_ops->fo_select)(fp, corl, reqevents, 
					     rtneventsp, notify);
		if (rc != 0)
		{
			*rtneventsp |= POLLERR;
			/*
			 * Perform cleanup functions only if a sel_cb was
			 * added to the thread chain by this call to the
			 * file ops select entry point (via selreg() ).
			 * This will ensure that a sel_cb is not removed
			 * too early, thereby loosing the information
			 * "corl/fd" required for the call to ufdrele().
			 */
			if (flags & POLL_FDMSG) {
				if ((temp_uthreadp->ut_selchn != NULL) &&
					(temp_uthreadp->ut_selchn != sel_cb) &&
					/* A sel_cb has been added */
					((struct sel_cb *)
					temp_uthreadp->ut_selchn)->corl == corl)
					selpoll_cleanup(file_id);
				else
					ufdrele(file_id);
			} else if (temp_uthreadp->ut_selchn != sel_cb)
				/* Clean the sel_cb which has been added */
				selpoll_cleanup(-1);

			return(rc);

		}

		if (*rtneventsp != 0)
		{
			/*
			 * At least 1 reqevent has occurred, so
			 * we can get rid of the ctl blk.
			 */
			if (flags & POLL_FDMSG) {
				if ((temp_uthreadp->ut_selchn != NULL) &&
					(temp_uthreadp->ut_selchn != sel_cb) &&
					/* A sel_cb has been added */
					((struct sel_cb *)
					temp_uthreadp->ut_selchn)->corl == corl)
					selpoll_cleanup(file_id);
				else
					ufdrele(file_id);
			} else if (temp_uthreadp->ut_selchn != sel_cb)
				/* Clean the sel_cb which has been added */
				selpoll_cleanup(-1);	/* cleanup */
		}

	} /* end if (!(reqevents & POLLSYNC)) */

	else
	{
		/*
		 * POLLSYNC is set -- i.e. this is a synchronous request.
		 */

		/*
		 * Call the device's ddselect entry point.
		 */
		rc = (*fp->f_ops->fo_select)(fp, corl, reqevents, 
					     rtneventsp, NULL);

		if (rc != 0)
		{
			*rtneventsp |= POLLERR;
		}

		/* Decrement the file descriptor count */
		if (flags & POLL_FDMSG)
			ufdrele(file_id);

	}

	return(rc);
}  /* end selpoll */

/*
 * NAME:	fp_select
 *
 * FUNCTION:	This kernel service provides a select/poll function for an
 *	open device driver instance.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called by device drivers/kernel extensions.
 *	It cannot page fault.
 *
 * NOTES:	This service is typically used to determine if a particular
 *	device has data available to be read, is capable of writing data,
 *	or has an exceptional condition outstanding.
 *
 * DATA STRUCTURES:	file
 *
 * RETURN VALUES DESCRIPTION:	0 upon successful completion;
 *				the non-zero return code from selpoll upon
 *				unsuccessful completion.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	selpoll			poll a device
 *	get_curthread
 */
int
fp_select(fp, reqevents, rtneventsp, notify)

register struct file	*fp;		/* file ptr to check status of	*/
ushort			reqevents;	/* requested events		*/
ushort			*rtneventsp;	/* ptr to occurred events	*/
void			(*notify)();	/* function ptr for nested poll */
{
	register int		rc;	/* return code from selpoll	*/
	register int		flags = 0; /* flags for selpoll routine */
	extern int		selpoll();

	if (CURTHREAD->t_uthreadp->ut_selchn != NULL)
	{
		flags = POLL_NESTED;
	}
	else
	{
		reqevents |= POLLSYNC;
	}

	/*
	 * Call selpoll to check the status of the specified file pointer.
	 */
	*rtneventsp = 0;
	rc = selpoll(fp, fp, reqevents, rtneventsp, flags, notify);

	return(rc);

}  /* end fp_select */

/*
 * NAME:	unselect
 *
 * FUNCTION:	Post an event to a thread when a timer "goes off".
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called by clock.
 *	It cannot page fault.
 *
 * NOTES:	This routine is called by clock() when the timer started by
 *	selpoll_timeout "goes off".
 *
 * DATA STRUCTURES:	thread,
 *			trb
 *
 * RETURN VALUES DESCRIPTION:	none.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	et_post			notify waiters that event(s) occurred
 */
void
unselect(timer)

register struct trb	*timer;
{
	extern void	et_post();

	et_post(EVENT_SYNC, ((struct thread *)timer->func_data)->t_tid);

	return;

}  /* end unselect */

/*
 * NAME:	selpoll_timeout
 *
 * FUNCTION:	Start a timer that will "go off" in the specified #clock ticks.
 *	At that time, execute a routine called "func", passing "arg" as the
 *	parameter. Only allow fine granularity timeouts if user is priviledged.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called by poll_wait.
 *	It cannot page fault.
 *
 * NOTES:
 *
 * DATA STRUCTURES:	trb
 *
 * RETURN VALUES DESCRIPTION:	ptr to timer request structure
 *
 * EXTERNAL PROCEDURES CALLED:
 *	privcheck	Check privilege
 *	talloc		Obtain a timer request structure
 *	tstart		Submit a timer request
 */
struct trb *
selpoll_timeout(func, arg, timeout)

void		(*func)();	/* function to execute when timer goes off*/
caddr_t		arg;		/* parameter to pass to "func" function	*/
struct timeval	*timeout;	/* #secs/usecs in which timer should go off*/
{
	register struct trb	*timer;	/* timer req. struct (see talloc)*/
	extern struct trb	*talloc();
	extern void		tstart();

	timer = talloc();	/* obtain a timer request struct	*/

	/*
	 * Initialize timer request struct's fields.
	 */
	timer->flags = 0;
	if(privcheck(SET_PROC_RAC) == EPERM) {
	 	/* only allow timeouts > one millisecond */
		if((timeout->tv_sec == 0 ) &&
		   (timeout->tv_usec != 0 ) &&
		   (timeout->tv_usec < (uS_PER_SEC / 1000)))  {
			timeout->tv_usec = (uS_PER_SEC / 1000);
		}
	}
	timer->timeout.it_value.tv_sec = timeout->tv_sec;
	/* convert from micro seconds to nanoseconds */
	timer->timeout.it_value.tv_nsec = timeout->tv_usec * NS_PER_uS;
	timer->func = func;
	timer->func_data = (ulong)arg;
	timer->ipri = INTBASE;

	tstart(timer);		/* submit the timer request		*/

	/*
	 * The timer request struct returned by talloc will be used
	 * later as input to selpoll_untimeout to stop the timer.
	 */
	return(timer);

}  /* end selpoll_timeout */

/*
 * NAME:	selpoll_untimeout
 *
 * FUNCTION:	Stop the timer that was started by selpoll_timeout.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called by poll_wait.
 *	It cannot page fault.
 *
 * NOTES:
 *
 * DATA STRUCTURES:	trb
 *
 * RETURN VALUES DESCRIPTION:	none.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	tstop		Cancel a pending timer request
 *	tfree		Relinquish a timer request structure
 */
void
selpoll_untimeout(timer)

struct trb	*timer;	/* timer req. struct returned by selpoll_timeout*/
{

/*
 *** tstop() correctly handles the case of an already-stopped timer ***
 */
	while(tstop(timer)){    /* cancel a pending timer request	*/
	  ;
	}

	tfree(timer);		/* relinquish a timer request struct	*/

	return;

}  /* end selpoll_untimeout */


/*
 * NAME:	devselect
 *
 * FUNCTION:	This kernel service provides a select/poll function for an
 *	open device driver instance referenced by devno and channel.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called by sys.c.
 *	It cannot page fault.
 *
 * NOTES:	This service is typically used to determine if a particular
 *	device has data available to be read, is capable of writing data,
 *	or has an exceptional condition outstanding.
 *
 * DATA STRUCTURES:	thread and uthread struct, select control block
 *
 * RETURN VALUES DESCRIPTION:	0 upon successful completion;
 *				the non-zero return code from selpoll upon
 *				unsuccessful completion.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	rdevselect	devswqry	ddselect
 *	get_curthread	selpoll_cleanup
 */
int
devselect(dev_t devno, int chan, ushort reqevents, ushort *rtneventsp,
	  void (*notify)())

{
	register int		rc;	/* return code from selpoll	*/
	register int		flags = 0; /* flags for selpoll routine */
	struct sel_cb		*sel_cb; /* ctl blk at head of thread chain*/
	register int 		corl;	/* correlator:  file ptr, etc.	*/
	int			status; /* status from devswqru 	*/
	extern int		devswqry();
	extern int		rdevselect();
	extern void		selpoll_cleanup();
	register struct uthread *temp_uthreadp;/* temp pointer to uthread    */

	temp_uthreadp = CURTHREAD->t_uthreadp;

	/*
	 * If nested poll flag is set, then copy the correlator from
	 * the control
	 * block on the top of the pending event chain to be used as the 
	 * correlator for this request; otherwise, ensure that no notify 
	 * routine is called.
	 */

	if (temp_uthreadp->ut_selchn != NULL)
	{
		sel_cb = temp_uthreadp->ut_selchn;
		corl = sel_cb->corl;
	}
	else
	{
		notify = NULL;
		reqevents |= POLLSYNC;
	}
	*rtneventsp = 0;

	/*
	 * Check that the device number is valid
	 */
	if (devswqry (devno, &status, NULL) || (status == DSW_UNDEFINED))
	  {
	    	*rtneventsp |= POLLERR;
	    	rc = ENXIO;
	    	return (rc);
	  }
	/*
	 * Call the pfs routine that will register the select request
	 * using selreg and will call the device drivers ddselect entry
	 * point.
	 */
	 rc = rdevselect(devno, corl, reqevents, rtneventsp,
			notify,chan);

	if (!(reqevents & POLLSYNC))
	{
		/*
		 * This is an asynchronous request (i.e. a ctl blk
		 * has previously been allocated/initialized and
		 * put on the device and thread chains).
		 */

		if (rc != 0)
		{
			*rtneventsp |= POLLERR;
			selpoll_cleanup(-1); /*perform cleanup functions,no fd*/
			return(rc);
		}

		if (*rtneventsp != 0)
		{
			/*
			 * At least 1 reqevent has occurred, so
			 * we can get rid of the ctl blk.
			 */
			selpoll_cleanup(-1);	/* cleanup, no fd	*/
		}

	} /* end if (!(reqevents & POLLSYNC)) */

	else
	{
		/*
		 * POLLSYNC is set -- i.e. this is a synchronous request.
		 */
		if (rc != 0)
		{
			*rtneventsp |= POLLERR;
		}
	}

	return(rc);
 } /* end of devselect */
