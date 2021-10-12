static char sccsid[] = "@(#)60	1.72  src/bos/kernel/proc/sleep.c, sysproc, bos41J, 9515B_all 4/10/95 13:51:58";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: ep_post
 *		e_sleep
 *		e_sleepl
 *		e_wakeup
 *		e_wakeup_w_sig
 *		e_wakeupx
 *		init_sleep
 *		kwakeup
 *		remove_e_list
 *		sleepx
 *		wakeup
 *
 *   ORIGINS: 3, 27, 83
 *
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


/*
 * NOTES:  All event lists have the same format.  The list anchor is either
 *         EVENT_NULL, or contains the process ID of the last entry in
 *         a circular chain of proc structs.
 *
 *        [A circular list was chosen for the following reasons:
 *         a)  prevents starvation/exclusion of processes;
 *         b)  provides for fair scheduling]
 *
 *         Since the list is circular, this pid points to the newest entry,
 *         allowing FIFO processing.
 *
 *         These procedures are pinned and will only fault if called on a
 *         pageable stack or passed a pageable event word.
 *
 */

#include <sys/types.h>
#include <sys/proc.h>
#include <sys/sleep.h>
#include <sys/pri.h>
#include <sys/prio_calc.h>
#include <sys/systm.h>
#include <sys/syspest.h>
#include <sys/lockl.h>
#include <sys/intr.h>
#include <sys/param.h>
#include <sys/user.h>
#include <sys/context.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/atomic_op.h>
#include "swapper_data.h"
#include "sig.h"

int hsque[NHSQUE];

/*
 * NAME:  init_sleep
 *
 * FUNCTION:  Initialize hsque
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *
 * NOTES:  This routine initializes each element of the event-list anchor
 *         array to EVENT_NULL.
 */
void
init_sleep(void)
{
	register int i;

	for (i = 0; i < NHSQUE; i++)
		hsque[i] = EVENT_NULL;
}

/*
 * NAME:  sleepx
 *
 * FUNCTION:  Wait for the event to occur.
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *
 * NOTES:  
 *	Callers of this service must be prepared for premature return
 *      and check that the reason for waiting has gone away.  This is
 *      even more true with this implementation since the wakeup is
 *      per hash class as opposed to wait channel.
 *
 *      Note that the priority parameter will be the priority of the
 *      process when is becomes runnable again (if that priority is more
 *      favorable). The process will remain keep that priority until it
 *      is dispatched. The range the wakeup priority  0 <= pri <= PRI_LOW.
 *      If the priority parameter is outside of that range it is forced
 *      to the lower/upper boundary.
 *      The call to lockl to acquire the global non-preemptable device
 *      driver lock ensures that the owner of the lock has the same
 *      priority as the most favored waiter.
 *
 * DATA STRUCTURES:  none
 *
 * RETURNS:  
 *	This routine returns an integer value:
 *	(0): event occurred
 *	(1): signalled out
 */

sleepx(int chan, int pri, int flags)
/* int chan;		wait channel                */
/* int pri;		ignore, see note above      */
/* int flags;		signal control flags        */
{
	register	rc;		/* e_sleep return code         */
	register	sflags;		/* e_sleep flags               */
	register struct thread *t = curthread;
	
	/*
	 * Convert the signal control flags
	 */
	if (flags & SWAKEONSIG) {
		if (flags & PCATCH)
			sflags = EVENT_SIGRET;
		else
			sflags = EVENT_SIGWAKE;
	}
	else
	        sflags = EVENT_SHORT;

	/*
	 * Force the process to wait for the event to occur.
	 * Unlock the non-preemptable device driver lock during the
	 * wait.  It is intended to simulate a non-preemptable kernel.
	 * Set the wakeup priority of the process.
	 */
	t->t_wchan1 = (char *)chan;		/* store real wait channel */
	t->t_wakepri = MAX(0,MIN(PRI_LOW,pri));	/* set the wakeup priority */
	rc = e_sleepl(&kernel_lock, sqhash(chan), sflags);
	t->t_wakepri = PIDLE;			/* reset wake up priority  */
	t->t_pri = prio_calc(t);		/* recalculate priority    */
	if (!t->t_boosted)
		t->t_sav_pri = t->t_pri;
	t->t_wchan1 = NULL;		/* reset real wait channel */

	/*
	 * Return to the caller.  The sleep may have been terminated
	 * by a signal, so we need to determine the type of return.
	 */
	return (rc == EVENT_SIG);	/* 1 if signalled, else 0 */
}

/*
 * NAME:  wakeup
 *
 * FUNCTION:  Wake up sleeping processes.
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can be called by a process or an interrupt handler.
 *
 * NOTES:  
 *	Wake up all processes sleeping on the wait channel.  This
 *      implementation over-achieves in that it wakes up all processes
 *      waiting on a channel with the same hash class as opposed to just
 *      the same channel.
 */

void
wakeup(int chan)
{
	e_wakeup(sqhash(chan));
}

/*
 * NAME:  e_sleep
 *
 * FUNCTION:  Add current process to an event list
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *
 *      This procedure is pinned and will only fault if called on a
 *      pageable stack or passed a pageable event word.
 *
 * NOTES:  
 *	This routine, within its critical section, can only fault on
 *      the event word.  Once the load of this word succeeds, no other
 *      pageable data are touched.
 *
 *      This routine both subscribes to an event and sleeps on it.  The
 *      event-list (input parm) is 'touched'.  If the event-list is
 *      empty, the thread is added to the empty list; otherwise, the
 *      thread is added to the front of the list.
 *
 *      The e_sleep_thread routine is then called.  If its return code is
 *      not EVENT_SIG, then EVENT_SUCC is returned to the caller of
 *      e_sleep.
 *
 *      If e_sleep_thread did return a value of EVENT_SIG, and e_sleep 
 *	was called with the EVENT_SIGRET flag, then the thread is removed 
 *	from the event-list, and EVENT_SIG is returned to the caller of 
 *	e_sleep.
 *
 * RETURN VALUE DESCRIPTION:  
 *	This routine returns an int value of EVENT_SIG if the return code 
 *	from the call to e_sleep_thread is EVENT_SIG and e_sleep was called 
 * 	with flag EVENT_SIGRET.  If the above is not true, then a ulong value 
 *	of EVENT_SUCC is returned.
 */

int
e_sleep(int *event_list, int flags)
/* register int *event_list;	event_list anchor pointer */
/* register int flags;		wait option flags */
{
	register int     rc;			/* return code */
	register int     interruptible;		/* wake on signal */

	ASSERT(csa->prev == NULL);		/* MUST be a thread to sleep */
	
	interruptible = ((flags == EVENT_SIGWAKE) || (flags == EVENT_SIGRET));

	rc = e_sleep_thread(event_list, NULL, interruptible); 

	if (rc == THREAD_INTERRUPTED)
	{
		if (flags == EVENT_SIGWAKE) 
		{
			longjmpx(EINTR);
		}
		rc = EVENT_SIG;
	}
	else
	{
	    	rc = EVENT_SUCC;
	}

	return(rc);
}

/*
 * NAME:  e_sleepl
 *
 * FUNCTION:  
 *	Conditionally unlock the user-specified lock, call e_sleep,
 *      and then (conditionally) re-lock the lock before returning.
 *      The code is pinned since a critical section is required.
 *
 *      This routine was added so that the caller would not have to
 *      be pinned.
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *
 *      This procedure is pinned and will only fault if called on a
 *      pageable stack or passed a pageable event word or lock word.
 *
 *      Since this routine could possibly page fault on BOTH the lock
 *      word and the event-list, we could be in trouble...  However,
 *      since only one of these items is needed at a time, we are safe
 *      (i.e. when lock_word is being used, we don't need event_list;
 *      when event_list is being used, lock_word is not needed).
 *
 * NOTES:  See notes for e_sleep.
 *
 * RETURN VALUE DESCRIPTION:  The value of e_sleep is passed to the caller.
 */

int
e_sleepl(int *lock_word, int *event_list, int flags)
/* register int *lock_word;	caller's lock word */
/* register int *event_list;	event_list anchor pointer */
/* register int flags;		wait option flags */
{
	register int    interruptible;	/* wakeu on signal */
	register int	rc;		/* return code from e_sleep */

	interruptible = (flags == EVENT_SIGRET)  || (flags == EVENT_SIGWAKE);

        e_assert_wait(event_list, interruptible);

	unlockl(lock_word);

        rc = e_block_thread();

	(void) lockl(lock_word, LOCK_SHORT);

	switch(rc){
	case THREAD_INTERRUPTED :
		rc = EVENT_SIG;
		break;
	default :
		rc = EVENT_SUCC;
		break;
	}

	return(rc);
}

/*
 * NAME:  ep_post
 *
 * FUNCTION:  Post events to a process
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can be called by a process or an interrupt handler.
 *
 *      This procedure is pinned and will only fault if called on a
 *      pageable stack or passed a pageable event word.
 *
 * NOTES:  
 *	This routine wakes each thread in the designated process
 *	that is waiting for the specified event(s).  This routine is
 *	intended for process based interactions and is used internally 
 *	to support exit, kwaitpid, and stop.  More than one thread is
 *	awakened, because a thread can sleep for a specific thread
 * 	to be stopped or exit.  State must be examined in kwaitpid
 *	before and after sleeping. 
 */

void
ep_post(ulong events, pid_t pid)
/* register ulong   events;	event_list anchor pointer */
/* register pid_t   pid;	ID of process to post     */
{
	register struct proc	*p;	/* process structure		*/
	register struct thread	*t;	/* thread structure		*/
	register int		ipri;	/* saved interrupt priority	*/
        register int            hadintlock;
        register int            hadbaselock;


	p=PROCPTR(pid);
	ASSERT(p->p_pid == pid);

	ipri = i_disable(INTMAX);	/* begin critical section	*/

#ifdef _POWER_MP
        if (!(hadbaselock = lock_mine(&proc_base_lock)))
                simple_lock(&proc_base_lock);
        if (!(hadintlock = lock_mine(&proc_int_lock)))
                simple_lock(&proc_int_lock);
#endif

	t = p->p_threadlist;
	do
	{
		/* Check if the thread is waiting for an event */
		if (t->t_wtype == TWEVENT)
		{
			switch(t->t_state)
			{
			 case TSSLEEP:
				if (events & t->t_wevent)
					setrq(t, E_WKX_PREEMPT, RQTAIL);
				break;
			 case TSRUN:
			 case TSSTOP:
				if (events & t->t_wevent)
					t->t_wtype = TNOWAIT;
				break;
			 default:
				ASSERT(FALSE);
			}
		}
		t = t->t_nextthread;
	}
	while (t != p->p_threadlist);

#ifdef _POWER_MP
        if (!hadintlock)
                simple_unlock(&proc_int_lock);
        if (!hadbaselock)
                simple_unlock(&proc_base_lock);
#endif

	i_enable(ipri);			/* exit critical section	*/
}

/*
 * NAME:  e_wakeup, e_wakeupx
 *
 * FUNCTION:  
 *	Wake up all processes on an event wait-list.  e_wakeup and e_wakeupx
 *	are the same, except the caller can specify whether runrun is preserved 
 * 	across the call.  Or in other words, whether another process should 
 *	be chosen to run following this call. 
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can be called by a process or an interrupt handler.
 *
 *      This procedure is pinned and will only fault if called on a
 *      pageable stack or passed a pageable event word.
 *
 * NOTES:  
 *	This routine first 'touches' the event-list (input parm).
 *      If the event-list is empty, we immediately return to the caller.
 *
 *      Otherwise, the event-list is made NULL.  Each process in the
 *      event-list is then checked to see if it is asleep and waiting
 *      to be posted, and if all the events it is waiting on have
 *      occurred.  If so, the process is put on the ready queue.
 *
 *	When a process is put on the ready queue, the runrun flag may
 *	be set, which forces a call to the dispatcher, when interrupts
 *	are enabled.  The dispatcher may choose another process to run.
 *	If the process that is chosen, needs a lock that is held by the
 *	caller, then that process will go to sleep again, resulting in 
 *	a wasted context switch.  e_wakeupx is provided for this reason. 
 *	e_wakeupx emulates the SYSTEM V approach.
 */

void
e_wakeup(int *event_list)
/* int *event_list;	event_list anchor pointer */
{
	void kwakeup();

	kwakeup(event_list, E_WKX_PREEMPT, THREAD_AWAKENED);
}

void
e_wakeupx(int *event_list, int option)
/* int *event_list;	event_list anchor pointer */
{
	void kwakeup();

	if (option != E_WKX_NO_PREEMPT)
		option = E_WKX_PREEMPT;

	kwakeup(event_list, option, THREAD_AWAKENED);
}

void
kwakeup(int *event_list, int option, int result)
/* int *event_list;	event_list anchor pointer */
/* int option;		indicates if runrun should be preserved */
{
	register int	waittid;	/* waiting thread ID */
	register struct thread *lastt;	/* last thread on list */
	register struct thread *t;	/* thread being posted */
	register struct thread *nextt;	/* next thread on list */
	register int	ipri;		/* saved interrupt priority */
        register int    hadintlock;     /* had proc_int_lock on input */
        register int    hadbaselock;    /* had proc_base_lock on input */

        ipri = i_disable(INTMAX);       /* begin critical section */

	/* proc_int_lock and proc_base_lock are lost over page faults */
#ifdef _POWER_MP
	hadbaselock = lock_mine(&proc_base_lock);
	hadintlock = lock_mine(&proc_int_lock);
#endif

	/*
	 * Last waiting process
	 * WARNING:  may page fault on event-list
	 */
	waittid = LOADWCS(event_list);

#ifdef _POWER_MP
	if (!lock_mine(&proc_base_lock))
		simple_lock(&proc_base_lock);
        if (!lock_mine(&proc_int_lock))
                simple_lock(&proc_int_lock);

	/* Reload word after acquiring proc_int_lock for atomicity. */
	waittid = LOADWCS(event_list);
#endif

	if (waittid != EVENT_NULL)  		/* is there anyone waiting?  */
	{	
		lastt = THREADPTR(waittid);
		ASSERT(waittid == lastt->t_tid);

		*event_list = EVENT_NULL; 		/* empty the list */

		/* lastt points to a list of subscribers, post them */
		nextt = lastt->t_eventlist;
		do
		{
			t = nextt;			/* post this thread */
			nextt = t->t_eventlist;   	/* remember the next */
			t->t_eventlist = NULL;
			t->t_wchan = NULL;
			t->t_result = result;

			/*
		 	 * Thread asleep or stopped AND Waiting for event  
		 	 */
			if (t->t_wtype == TWEVENT)
			{
				switch(t->t_state) 
				{
				 case TSSLEEP:
					if (t->t_policy != SCHED_OTHER)
						setrq(t, E_WKX_PREEMPT, RQTAIL);
					else
						setrq(t, option, RQTAIL);
					break;
				 case TSRUN:
				 case TSSTOP:
					t->t_wtype = TNOWAIT;
					break;
				 default:
					ASSERT(FALSE);
				}
			}

		}  while (t != lastt);
	}  				/* end if waitpid != EVENT_NULL */

#ifdef _POWER_MP
        if (!hadintlock)
                simple_unlock(&proc_int_lock);
	if (!hadbaselock)
        	simple_unlock(&proc_base_lock);
#endif

	i_enable(ipri);			/* end critical section */
}

/*
 * NAME:  remove_e_list
 *
 * FUNCTION:  Remove the specified process/thread from the event list. 
 *
 * EXECUTION ENVIRONMENT:
 * 	This procedure is called only from process environment.
 *	It is called disabled.
 *
 */
void
remove_e_list(struct thread *t, int *event_list)
{
        register struct thread 	*tend;        /* pointer to end of event list */
        register struct thread 	*prev;        /* ptr to prev event list thread*/
        register int           	waittid;      /* thread id on event list      */
	register int		i;	      /* counter                      */
	register int		hadintlock;   /* held proc_int_lock on input  */

	ASSERT(csa->intpri == INTMAX);

#ifdef _POWER_MP
	if ((hadintlock = lock_mine(&proc_int_lock)))
       		simple_unlock(&proc_int_lock);
       	simple_unlock(&proc_base_lock);
#endif 

	/*
	 * Last waiting process
	 * WARNING:  may page fault on event-list
	 */
	waittid = LOADWCS(event_list);

#ifdef _POWER_MP
	/* Serialize event list. */
       	simple_lock(&proc_base_lock);
	if (hadintlock)
       		simple_lock(&proc_int_lock);

	/* Reload word after acquiring proc_int_lock for atomicity. */
        waittid = LOADWCS(event_list);
#endif

        /*
         * if still something there and we're still on list
         * the volatile definition ensures that the test is
         * not optimized out
         */
        if ( (waittid != EVENT_NULL) && (t->t_eventlist != NULL) )
        {
                tend = prev = THREADPTR(waittid);   /* get tail of list */
                i = NTHREAD;                        /* max times thru */

                while (1) {
                        if (prev->t_eventlist == t)
                                break;
                        prev = prev->t_eventlist;
                        assert(i-- > 0);     /* infinite loop catcher */
                }

                /* remove us from list */
                prev->t_eventlist = t->t_eventlist;

                /* if list anchor points to us: */
                if (tend == t) {
                        /*
                         * if list is now empty, then mark empty,
                         * else set new tail pointer
                         */
                        if (prev == t)
                                *event_list = EVENT_NULL;
                        else
                                *event_list = prev->t_tid;
                }
        }
        t->t_eventlist = NULL;                  /* we're no longer on list */
        t->t_wchan = NULL;                      /* reset wait channel      */
}

/*
 * NAME: e_wakeup_w_sig
 *
 * FUNCTION:
 *	Posts a signal to each thread sleeping
 *	interruptably on the designated event list.
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *	This procedure disables interrupts.
 *
 * NOTE:
 *	This service will only wake up processes
 *	that are sleeping interruptibly.  Processes
 *	doing short sleeps will not be affected.
 *
 * RETURNS:
 *	None.
 */
void
e_wakeup_w_sig( int *event_list, int signo )
{
	register int	waittid;	/* waiting thread ID */
	register struct thread *lastt;	/* last thread on list */
	register struct thread *t;	/* thread being posted */
	register int	ipri;		/* saved interrupt priority */

	ipri = i_disable(INTMAX);	/* begin critical section */

	/*
	 * Last waiting process
	 * WARNING:  may page fault on event-list
	 */
	waittid = LOADWCS(event_list);

#ifdef _POWER_MP
        simple_lock(&proc_base_lock);
        simple_lock(&proc_int_lock);

	/* Reload word after acquiring proc_int_lock for atomicity. */
	waittid = LOADWCS(event_list);
#endif

	/* NULL event list has no effect */
	if (waittid != EVENT_NULL) {

		t = lastt = THREADPTR(waittid);
		ASSERT(waittid == lastt->t_tid);

		do {
			tidsig(t->t_tid, signo);
			t = t->t_eventlist;
		} while (t != lastt);
	}

#ifdef _POWER_MP
        simple_unlock(&proc_int_lock);
        simple_unlock(&proc_base_lock);
#endif

	i_enable(ipri);			/* end critical section */

}

/*
 * NAME:  getptid
 *
 * FUNCTION:  Gets the first tid in the designated process. 
 *
 * NOTES:
 *	This kernel service is used to support e_post in V4
 *	drivers supplied by IBM which have not been upgraded
 *	to support multi-threaded processes.  It is an undocumented
 *	kernel service and may be removed at any time.  NOBODY
 *	SHOULD BE USING THIS KERNEL SERVICE.
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure is callable from process or interrupt environments.
 *
 */

tid_t getptid(pid_t pid)
{
	struct proc *p;

	p = VALIDATE_PID(pid);

	if (!p || p->p_threadcount != 1)
		return(NULL_TID);

	return(p->p_threadlist->t_tid);
}
