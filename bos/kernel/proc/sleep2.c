static char sccsid[] = "@(#)56	1.49  src/bos/kernel/proc/sleep2.c, sysproc, bos41J, 9519B_all 5/11/95 15:02:26";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: e_assert_wait
 *              e_assert_wakeup
 *		e_block_thread
 *		e_clear_wait
 *		e_sleep_thread
 *		e_wakeup_one
 *		e_wakeup_w_result
 *		et_post
 *		et_wait
 *		remove_lock_list
 *		thread_tsleep
 *		thread_tsleep_timeout
 *		thread_twakeup
 *              thread_unlock
 *              thread_waitlock
 *		wait_on_lock
 *		wakeup_lock
 *
 *   ORIGINS: 27, 83
 *
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


/*
 * NOTES:  All event lists have the same format.  The list anchor is either
 *         EVENT_NULL, or contains the thread ID of the last entry in
 *         a circular chain of proc structs.
 *
 *         A circular list was chosen for the following reasons:
 *         a)  prevents starvation/exclusion of threads;
 *         b)  provides for fair scheduling;
 *
 *         Since the list is circular, this tid points to the newest entry,
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
#include <sys/rtc.h>
#include <sys/atomic_op.h>
#include <sys/trchkid.h>
#include "swapper_data.h"
#include "sig.h"

void e_assert_wakeup();
void remove_lock_list();

/* Lock hash queues (for wakeup_lock).
 * This is an array of event list anchors used by sleep/wakeup.
 * Each element needs to be initialized to EVENT_NULL during
 * system initialization (see init_sleep).
 */
int lockhsque[LOCKHSQ] = {EVENT_NULL};
int locklhsque[LOCKLHSQ] = {EVENT_NULL};

/* Event anchor for thread_tsleep */
int tsleep_list = EVENT_NULL;

/*
 * NAME:  e_assert_wait
 *
 * FUNCTION:  asserts that the current thread is about to go to sleep. 
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *	It can page fault if the event word is declared in pageable memory.
 */

void
e_assert_wait(int *event_list, boolean_t interruptible)
{
	register struct thread *t = curthread;  /* pointer to caller         */
        register struct thread *tend;           /* ptr to end of event list  */
        register int waittid;      	        /* thread id on event list   */
	int intpri;	                	/* saved interrupt priority  */
	register int link_register;		/* Return address of caller */
        register int hadbaselock;      	        /* had proc_base_lock on input*/
	
	GET_RETURN_ADDRESS(link_register);
	TRCHKL4T(HKWD_KERN_ASSERTWAIT,t->t_tid,event_list,
		 interruptible,link_register);

	/*
         * check here to be sure that we're not already waiting on another
	 * event
	 */
	assert(t->t_eventlist == NULL);

        intpri = i_disable(INTMAX);

#ifdef _POWER_MP
        /*
         * If we owned the disabled lock already, the anchor should be
         * pinned. Otherwise we need to touch it first before getting
         * the lock since we can't page fault with the lock. Any subsequent
         * touch is guaranteed not to page fault because we are disabled.
         * Note of course that we need to reload the data after getting
         * the lock that protects it.
         */
        if (!(hadbaselock = lock_mine(&proc_base_lock)))
        {
                waittid = LOADWCS(event_list);
                simple_lock(&proc_base_lock);
        }
#endif

        waittid = LOADWCS(event_list);

        ASSERT((t->t_flags & TWAKEONSIG) == 0);

        if (waittid == EVENT_NULL) {

                /* add to empty list */
                t->t_eventlist = t;
        }
        else
        {
		/* validity check event list word - this should be EVENT_NULL
		 * if it is not a valid thread id.
		 */
                /* Other subscribers, just add to list... */
                tend = THREADPTR(waittid);
                ASSERT(tend->t_tid == waittid);
                t->t_eventlist = tend->t_eventlist;
                tend->t_eventlist = t;
        }
        *event_list = t->t_tid;              	/* thread table index */
        t->t_wchan = (char *)event_list;     	/* store wait channel */
	t->t_result = 0;		 	/* the result field   */
	if (interruptible)
		t->t_flags |= TASSERTSIG;
	else
		t->t_flags &= ~TASSERTSIG;

#ifdef _POWER_MP
	if (!hadbaselock)
		simple_unlock(&proc_base_lock);
#endif

        i_enable(intpri);

};

/*
 * NAME:  e_clear_wait
 *
 * FUNCTION:  clears the wait condition for the specified thread.
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can be called from the process or interrupt environments.
 *      It cannot page fault.
 *
 * NOTES:
 *	The thread remains on the event list until it wakes up.
 */

void 
e_clear_wait(tid_t tid, int result)
{
	register struct thread *t;              /* pointer to caller         */
	int intpri;	         		/* saved interrupt priority  */
	register int link_register;		/* Return address of caller */
	register int preempt_option; 

	t = VALIDATE_TID(tid);
	assert(t != NULL);

	GET_RETURN_ADDRESS(link_register);
	TRCHKL4T(HKWD_KERN_CLEARWAIT,t->t_tid,t->t_wchan,result,link_register);

	preempt_option = result < 0 ? result = -result, E_WKX_NO_PREEMPT :
							E_WKX_PREEMPT;

        intpri = disable_lock(INTMAX, &proc_base_lock);
#ifdef _POWER_MP
	simple_lock(&proc_int_lock);
#endif

	/* If we are still on the wait list, remove ourselves */
        if (t->t_eventlist != NULL)
	{
                remove_e_list(t, (int *)t->t_wchan);

		t->t_result = result;
		t->t_flags &= ~(TWAKEONSIG|TASSERTSIG|TSIGWAIT);

		/*
		 * Thread asleep or stopped AND Waiting for event
		 */
		if (t->t_wtype == TWEVENT)
		{
			switch(t->t_state)
			{
			 case TSSLEEP:
				if (t->t_policy == SCHED_OTHER)
					setrq(t, preempt_option, RQTAIL);
				else
					setrq(t, E_WKX_PREEMPT, RQTAIL);
				break;
			 case TSRUN:
			 case TSSTOP:
				t->t_wtype = TNOWAIT;
				break;
			 default:
				ASSERT(FALSE);
			}
		}
	}

#ifdef _POWER_MP
	simple_unlock(&proc_int_lock);
#endif
	unlock_enable(intpri, &proc_base_lock);
};

/*
 * NAME:  e_block_thread
 *
 * FUNCTION:  blocks the current thread. 
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can be called from the process environment only.
 *      It cannot page fault.
 */

int 
e_block_thread()
{
	register struct thread *t = curthread;  /* pointer to caller         */
	register struct proc *p;
	register int hadkernel_lock;     /* this thread held the kernel lock */
	register int link_register;		/* Return address of caller  */
	register int i;				/* loop counter		     */
	int intpri;	                	/* saved interrupt priority  */

	GET_RETURN_ADDRESS(link_register);
	TRCHKL4T(HKWD_KERN_THREADBLOCK,t->t_tid,t->t_wchan,t->t_flags,
		 link_register);

	p = t->t_procp;

        if (p->p_pid > 0)			        /* if not booting */
		fetch_and_add(&(U.U_ru.ru_nvcsw), 1);  	/* voluntary++    */

	/* 
	 * Releasing lock because the proc_base_lock is used
	 * within lockl to go to sleep after having taken
	 * the interlock bit.  Don't want to hold proc_base_lock
	 * while spinning on the interlock bit.
	 */
	hadkernel_lock = IS_LOCKED(&kernel_lock);
        if (hadkernel_lock) unlockl(&kernel_lock);

        intpri = disable_lock(INTMAX, &proc_base_lock);

	/* Check signals before sleeping */
	if ((t->t_flags & TASSERTSIG) && SIG_MAYBE(t,p)) {
		if (check_sig(t, 1))
			goto after_sig_recheck;
		else
			t->t_flags &= ~TSIGAVAIL;
	}

	if (t->t_eventlist != NULL) {
		if (t->t_flags & TASSERTSIG) 
			t->t_flags |= TWAKEONSIG;

       		/*
       	 	 * record local threads sleeping
       	 	 */
       		if (t->t_flags & TLOCAL)
       			p->p_local--;
        	ASSERT(p->p_local >=0);

		t->t_wtype = TWEVENT; 

                /*
                 * The proc_base_lock needs to be held into the dispatcher
                 * for interruptible sleeps to forestall intervening SIGSTOPs
                 * and SIGCONTs to the process.  Otherwise the thread will
                 * be put onto the runqueue twice.  The first time by cont()
                 * the second time by the dispatcher.
                 */
#ifdef _POWER_MP
		if (t->t_flags & TWAKEONSIG)
			locked_swtch();
		else {
			simple_unlock(&proc_base_lock);
			swtch();
		}
		simple_lock(&proc_base_lock);
#else
		swtch();
#endif
        	if (t->t_flags & TLOCAL)
        		p->p_local++;
	}

	/* Check signals before sleeping */
	if ((t->t_flags & TASSERTSIG) && SIG_MAYBE(t,p)) {
		if (check_sig(t, 1)) {
after_sig_recheck:
			t->t_result = THREAD_INTERRUPTED;
			t->t_flags |= TSIGAVAIL;
		}
		else
			t->t_flags &= ~TSIGAVAIL;
	}

	t->t_flags &= ~(TWAKEONSIG|TASSERTSIG|TSIGWAIT);

	/*
	 * The thread may still be on the list, if awakened
	 * because of a signal or because of intra-process
	 * synchronization (ie. exit, core, exec).
	 */
	if (t->t_eventlist != NULL)
		remove_e_list(t, (int *)t->t_wchan);

	unlock_enable(intpri, &proc_base_lock);

       	/*
         * this can't deadlock since NOONE can hold the kernel_lock
         * while waiting for any other lock - or in fact anything
         * else except a kernel page fault.  The page fault handler
         * and device drivers never wait for the kernel_lock
         */
        if (hadkernel_lock) lockl(&kernel_lock,LOCK_SHORT);

	ASSERT(t->t_eventlist == NULL);

	return(t->t_result);
};


/*
 * NAME:  e_sleep_thread
 *
 * FUNCTION:  sleep and atomically release a lock. 
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can be called from the process environment only.
 *      It cannot page fault.
 *
 */

e_sleep_thread(int *event_list, void *lockp, int flags)
{
        register int rc;             /* return code from e_sleep */
        volatile int m;                 /* content of word */

	ASSERT((flags&(LOCK_SIMPLE|LOCK_HANDLER)) != (LOCK_SIMPLE|LOCK_HANDLER));

	e_assert_wait(event_list, flags & INTERRUPTIBLE);

        if (lockp) {
#ifndef _POWER_MP
                if (flags & LOCK_SIMPLE)
#else
                if (flags & (LOCK_SIMPLE|LOCK_HANDLER))
#endif
                        simple_unlock(lockp);
                else if (flags & (LOCK_READ|LOCK_WRITE))
                        lock_done(lockp);
        }

	rc = e_block_thread();

        if (lockp) {
#ifndef _POWER_MP
                if (flags & LOCK_SIMPLE)
#else
                if (flags & (LOCK_SIMPLE|LOCK_HANDLER))
#endif
			simple_lock(lockp);
                else if (flags & LOCK_READ)
                        lock_read(lockp);
                else if (flags & LOCK_WRITE)
                        lock_write(lockp);
        }

        return(rc);
};

/*
 * NAME:  e_wakeup_w_result
 *
 * FUNCTION:  
 *	Wakeup the threads on the specified event list.  Caller can specify 
 *	the reason the wakeup is occuring with the result parameter.  
 *
 * EXECUTION ENVIRONMENT:
 *	This procedure can be called from the process or interrupt environments.
 *	It cannot page fault.
 *
 * NOTES:
 *	If called from the interrupt environment, the event list anchor cannot
 *	be pageable.
 */

void
e_wakeup_w_result(int *event_list, int result)
{
	kwakeup(event_list, E_WKX_PREEMPT, result);
};

/*
 * NAME:  e_wakeup_one
 *
 * FUNCTION: Wakeup the highest priority sleeper on the specified event list.  
 *
 * EXECUTION ENVIRONMENT:
 *	This procedure can be called from the process or interrupt environments.
 *	It cannot page fault.
 *
 * NOTES:
 *	If called from the interrupt environment, the event list anchor cannot
 *	be pageable.
 */

void
e_wakeup_one(int *event_list)
{
	register struct thread *firstt; /* first thread on the event list     */
	register struct thread *t;      /* thrd ptr to traverse the event list*/
	register struct thread *prevt;  /* thread preceding t in event list   */
	register struct thread *waket;	/* most favored thread in event list  */
	register struct thread *waketprev;/* thread preceding waket in event l*/
        register int waittid;      	        /* thread id on event list    */
	int intpri;	                	/* saved interrupt priority   */
	register int link_register;	/* Return address of the caller */

	intpri = i_disable(INTMAX);
	
        /* get list anchor (may page fault) */
        waittid = LOADWCS(event_list);

#ifdef _POWER_MP
        simple_lock(&proc_base_lock);

        waittid = LOADWCS(event_list);
#endif

	if (waittid != EVENT_NULL) {

#ifdef _POWER_MP
		simple_lock(&proc_int_lock);
#endif

		/* 
		 * Find most favored thread.  waittid identifies the last
		 * thread added to the list.  The next thread the first.
		 */
	 	waketprev = THREADPTR(waittid);
		if ((waket = waketprev->t_eventlist) == waketprev) {
			/* only one element on the list */
			*event_list = EVENT_NULL;
		}
		else {
			/* several elements on the list */
			prevt = firstt = waket;
	 		t = prevt->t_eventlist;
			do {
				if (t->t_pri < waket->t_pri) {
					waket = t;
					waketprev = prevt;
				}
				prevt = t;
				t = t->t_eventlist;
			} while (t != firstt);
                	waketprev->t_eventlist = waket->t_eventlist;
                	if (waittid == (int)waket->t_tid)   /* we are anchor */
                        	*event_list = (int)waketprev->t_tid;
        	}

		GET_RETURN_ADDRESS(link_register);
		TRCHKL3T(HKWD_KERN_EWAKEUPONE,waket->t_tid,waket->t_wchan,
			 link_register);

        	waket->t_eventlist = NULL;
        	waket->t_wchan = NULL;
                waket->t_result = THREAD_AWAKENED;

		if (waket->t_wtype == TWEVENT)
		{
			switch(waket->t_state)
			{
			 case TSSLEEP:
				setrq(waket, E_WKX_PREEMPT, RQTAIL);
				break;
			 case TSRUN:
			 case TSSTOP:
				waket->t_wtype = TNOWAIT;
				break;
			 default:
				ASSERT(FALSE);
			}
         	}
#ifdef _POWER_MP
		simple_unlock(&proc_int_lock);
#endif
	}

#ifdef _POWER_MP
        simple_unlock(&proc_base_lock);
#endif

	i_enable(intpri);
};

/*
 * NAME:  wait_on_lock
 *
 * FUNCTION: puts the current thread on a lock list.  
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *	This routine will not page fault.	
 */

void
wait_on_lock(int *event_list, boolean_t interruptible)
{
        register struct thread *t = curthread;  /* pointer to caller         */
        register struct thread *tend;           /* ptr to end of event list  */
        int intpri;                             /* saved interrupt priority  */

	TRCHKL3T(HKWD_KERN_WAITLOCK, t->t_procp->p_pid, t->t_tid, t->t_wchan1);

        if (t->t_procp->p_pid > 0) 			/* if not booting */
		fetch_and_add(&(U.U_ru.ru_nivcsw), 1);	/* involuntary++  */

        ASSERT(csa->intpri == INTMAX);
#ifdef _POWER_MP
	ASSERT(lock_mine(&proc_base_lock));
#endif

        if (*event_list == EVENT_NULL) {

                /* add to empty list */
                t->t_slist = t;
        }
        else
        {
                /* Other subscribers, just add to list... */
                tend = THREADPTR(*event_list);
                ASSERT(tend->t_tid == *event_list);
                t->t_slist = tend->t_slist;
                tend->t_slist = t;
        }
        *event_list = t->t_tid;                 /* thread table index */
        t->t_swchan = (char *)event_list;      /* store wait channel */

	if (interruptible)
        	t->t_flags |= TWAKEONSIG;
};

/*
 * NAME:  wakeup_lock
 *
 * FUNCTION: Wakes up threads that are waiting for locks. 
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can be called from the process or interrupt environments.
 *      It cannot page fault.
 *
 * NOTES:
 *	This function returns TRUE, if there are sleepers still waiting 
 *	for the lock.
 */

boolean_t 
wakeup_lock(int *event_list, void *lockp, int flags)
{
	register struct thread *t;	/* pointer to threads in event list */
	register struct thread *prevt;	/* pointer to previous thread in el */
	register struct thread *lastt;	/* last thread in el                */
	register struct thread *minwrt;	/* highest priority writer          */
	register struct thread *minwrprevt;/* thrd before highest pri writer */
        register int waittid;           /* thread id on event list          */
	register char mypri;
	register char minwrpri = PIDLE;
	register boolean_t sleepers = FALSE;
	register int lock_sid;
	register int lock_offset;
	register int count = 0;		/* No. of waiters                   */

        ASSERT(csa->intpri == INTMAX);
#ifdef _POWER_MP
        ASSERT(lock_mine(&proc_base_lock));
        ASSERT(lock_mine(&proc_int_lock));
#endif

       	/* get list anchor */
       	waittid = LOADWCS((int *)event_list);

	if (waittid == EVENT_NULL) 
		return(FALSE);

	/*
	 * search key is a combination of the SID of the effective address
	 * plus the offset in the segment of the lock word
	 */
	lock_sid = SRTOSID(mfsri(lockp));  /* SID */
	lock_offset = SEGOFFSET(lockp);    /* OFFSET */

        switch(flags) {
        case WAKEUP_LOCK_SIMPLE :		/* simple lock wakeup */
        case WAKEUP_LOCK_WRITER :		/* complex lock wakeup */
		minwrt = NULL;
		lastt = prevt = THREADPTR(waittid);

		do {
			t = prevt->t_slist;

			/* locks are hashed */
			if (t->t_wchan1offset == lock_offset) {	
				if (t->t_wchan1sid == lock_sid) {
					count++;
					
					/* wakeup most favored sleeper */ 
					if (t->t_wtype == TWLOCK) {  
						mypri = t->t_pri;
						if (mypri < minwrpri) {
							minwrpri = mypri;
							minwrt = t;
							minwrprevt = prevt;
						}
					}
				}
			}
			
			prevt = t;

		} while (t != lastt);

		if (count > 1)
			sleepers = TRUE;

		if (minwrt != NULL) {
			/* wakeup highest priority writer */
			remove_lock_list(minwrprevt, minwrt);
			break;
		}

		if (flags == WAKEUP_LOCK_SIMPLE)
			break;

                /* Fall through if there are no writers */

        case WAKEUP_LOCK_ALL :

		sleepers = FALSE;
                lastt = prevt = THREADPTR(waittid);
		count = 0;

                do {
                        t = prevt->t_slist;
                        if ((t->t_wchan1offset == lock_offset) &&
			    (t->t_wchan1sid == lock_sid)) {
				count++;
				remove_lock_list(prevt, t);
			}
                        else
                                prevt = t;
                } while (t != lastt);

                break;

        default :

                assert(0);

	}

	TRCHKL2T(HKWD_KERN_WAKEUPLOCK, lockp, count);
	return(sleepers);
}

/*
 * NAME:  remove_lock_list
 *
 * FUNCTION: removes a thread from a hashed event list.
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can be called from the process or interrupt environments.
 *      It cannot page fault.
 *      It is assumed that the caller has disabled to INTMAX
 *      and acquired the PROC_INT_LOCK for _POWER_MP
 *
 * NOTES:
 *      The event word anchor is reset in this routine.  If called from the
 *      interrupt level, this anchor mustbe declared in pinned memory.
 *      This function is like remove_e_list, except the event list has been
 *      traversed by the caller.  prevt should not equal NULL.
 *
 */

void
remove_lock_list(struct thread *prevt, struct thread *t)
{
        ASSERT(prevt != NULL);
        ASSERT(prevt->t_slist == t);
        ASSERT(csa->intpri == INTMAX);
        ASSERT(t->t_wtype == TWLOCK || t->t_wtype == TWLOCKREAD);
#ifdef _POWER_MP
        ASSERT(lock_mine(&proc_base_lock));
        ASSERT(lock_mine(&proc_int_lock));
#endif

	/* Reset event words */
        if (t->t_slist == t) {          /* Only one element on list */
                *(int *)t->t_swchan = EVENT_NULL;
        }
        else {
                prevt->t_slist = t->t_slist;
                if (*(int *)t->t_swchan == (int)t->t_tid)   
                        *(int *)t->t_swchan = (int)t->t_slist->t_tid;
        }

        t->t_slist = NULL;
        t->t_swchan = NULL;
        t->t_wchan1 = NULL;

#if 0					/* optimize out for perf reasons */
	t->t_wchan1sid = NULLSEGID;
	t->t_wchan1offset = NULL;
#endif

	switch(t->t_state)
	{
	 case TSSLEEP:
		setrq(t, E_WKX_PREEMPT, RQTAIL);
		break;
	 case TSRUN:
	 case TSSTOP:
		t->t_wtype = TNOWAIT;
		break;
	}
}

/*
 * NAME:  et_wait
 *
 * FUNCTION:  Sleep until event occurs
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *
 *      This procedure is pinned and will only fault if called on a
 *      pageable stack or passed a pageable event word.
 *
 * NOTES:
 *      This routine saves the wait mask (an input parm) in the thread
 *      struct t's t_wevent field.  If this wait mask is empty (i.e.
 *      no events to wait on), then the clear mask (an input parm) is
 *      applied to t_pevent; this value is then returned to the caller.
 *
 *      If the wait mask ws not empty, then we enter a loop:
 *      A check is made to see if the events that we are waiting on have
 *      occurred.  If they have, then the awaited events that have
 *      occurred (i.e. t_wevent & t_pevent) are returned to the caller.
 *
 *      If the awaited events have not occurred, then a check is made to
 *      see if we return on a signal.  If we do, then a check is made to
 *      see if there is a signal to deliver.  If there is, then EVENT_SIG
 *      is returned to the caller.
 *
 *      If there is no signal to deliver (but we do return on a signal),
 *      then the SWAKEONSIG flag is set in field t_flag.  The thread is
 *      then put to sleep in an event wait state.  When the thread is
 *      awakened, we loop back and check again to see if the awaited
 *      events have occurred, etc.
 *
 * DATA STRUCTURES:  proc (Process table entries)
 *
 * RETURNS:
 *      This routine returns a ulong value (the variable
 *      'happened'), which reflects one of the following cases:
 *
 *      If there were no waited events, then the
 *      return value will be a mask of the events
 *      that occurred with the clear-mask bits applied;
 *
 *      If et_wait() returns on a signal, and a signal
 *      occurred, then the return value will be EVENT_SIG.
 *
 *      Otherwise, the return value will be a mask
 *      of waited events that occurred.
 *
 */

ulong
et_wait(ulong wait_mask, ulong clear_mask, int flags)
/* register ulong   wait_mask;  events that will wake me up */
/* register ulong   clear_mask; clear these bits before waiting */
/* register int     flags;      wait option flags */
{
        register struct thread *t;		/* the current thread */
	register struct proc *p;		/* The current process */
        register ulong happened;       		/* return value */
        register int ipri;           		/* saved interrupt priority */
        register int hadkernel_lock;		/* held the kernel lock */
        register int hadint_lock;		/* held the proc_int_lock */

	t = curthread;
	p = t->t_procp;

        if (p->p_pid > 0)		     	/* if we're not booting */
		fetch_and_add(&(U.U_ru.ru_nvcsw),1);  	/* voluntary++  */

        /*
         * The kernel lock is an automatic short term lock -
         * it is never held across waits EXCEPT for kernel data
         * page faults.   It should be released prior to acquiring
         * the proc_int_lock, because the latter is used internally 
	 * within lockl/unlockl.  Don't want to hold proc_int_lock 
	 * and spin on the interlock bit, because the interlock bit 
	 * is supposed to be acquired before the proc_int_lock.
         */
        hadkernel_lock = IS_LOCKED(&kernel_lock);
        if (hadkernel_lock) unlockl(&kernel_lock);

        ipri = i_disable(INTMAX);       	/* begin critical section */

#ifdef _POWER_MP
        if (!(hadint_lock = lock_mine(&proc_base_lock)))
                simple_lock(&proc_base_lock);
#endif

        ASSERT((t->t_flags & TWAKEONSIG) == 0);

        t->t_wevent = wait_mask;        	/* awaited events */
        if (t->t_wevent == EVENT_NDELAY) 	/* any events awaited? */
                happened = t->t_pevent & clear_mask;
        else
        {       /* something to wait on? */

                for (;;)
                {
                        /*
                         * If this is not a short sleep, check for
                         * signals that may preempt the sleep
                         */
                        if (flags != EVENT_SHORT)
                        {       /* return on a signal?   */

                                ASSERT( (flags & EVENT_SIGRET) ||
                                        (t->t_uthreadp->ut_save.kjmpbuf));

                                if (SIG_MAYBE(t,p) && check_sig(t, 1)) 
				{ 		/* signal to deliver */

                                        t->t_flags &= ~TWAKEONSIG;
					t->t_flags |= TSIGAVAIL;

                                        if (flags & EVENT_SIGRET) {
                                                happened = EVENT_SIG;
                                                break;
                                        }
                                        else {
						t->t_wevent = EVENT_NDELAY;
                                                t->t_pevent &= (~clear_mask);
                                                unlock_enable(INTBASE,
							&proc_base_lock);

						/* version 3 semantics */
						if (hadkernel_lock)
							lockl(&kernel_lock,
								LOCK_SHORT);
                                                longjmpx(EXSIG);
                                        }
                                }
				/* no signal to deliver */
				else  {
                                        t->t_flags &= ~TSIGAVAIL;
                                        t->t_flags |= TWAKEONSIG;
				}
                        }

                        if (happened = t->t_wevent & t->t_pevent)
                                break;

                        /*
                         * record local threads sleeping
                         */
                        if (t->t_flags & TLOCAL)
                                t->t_procp->p_local--;
                        ASSERT(t->t_procp->p_local >=0);

                        /*
                         *  Put this process to sleep in an event wait.  The
                         *  process regains control after the call to swtch().
                         *  If the desired events have not yet occurred, it
                         *  just waits some more.
                         */
                        t->t_wtype = TWEVENT;

	                /*
                 	 * The proc_base_lock needs to be held into the 
			 * dispatcher for interruptible sleeps to forestall 
			 * intervening SIGSTOPs and SIGCONTs to the process.  
			 * Otherwise the thread will be put onto the runqueue 
			 * twice.  The first time by cont() the second time 
			 * by the dispatcher.
                 	 */
#ifdef _POWER_MP
                	if (t->t_flags & TWAKEONSIG)
                        	locked_swtch();
                	else {
                        	simple_unlock(&proc_base_lock);
                        	swtch();
                	}
                	simple_lock(&proc_base_lock);
#else
                	swtch();
#endif

                        if (t->t_flags & TLOCAL)
                                t->t_procp->p_local++;
                }
                /*
                 * Unset SWAKEONSIG flag; it is less efficient to record
                 * and check if SWAKEONSIG was set before the swtch().
                 */
                t->t_flags &= ~TWAKEONSIG;
        }

        /*
         *  clear non-persistent posted events
         *  (queue events remain posted until queue is empty)
	 *  clear waited events since et_posts are asyncronous
	 *  to sleeps.  This prevents an et_post from waking an
	 *  e_sleep because of left over waiting bits.
         */
        t->t_pevent &= (~clear_mask);
	t->t_wevent = EVENT_NDELAY;

#ifdef _POWER_MP
        if (!hadint_lock)
                simple_unlock(&proc_base_lock);
#endif

        i_enable(ipri);                 /* end critical section */

        if (hadkernel_lock) lockl(&kernel_lock,LOCK_SHORT);

        return(happened);
}
/*
 * NAME:  et_post
 *
 * FUNCTION:  Post events to a thread
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can be called by a process or an interrupt handler.
 *
 *      This procedure is pinned and will only fault if called on a
 *      pageable stack or passed a pageable event word.
 *
 * NOTES:
 *      This routine checks the thread id (input parm) to see if the
 *      thread is asleep and waiting to be posted, and if all the
 *      events it is waiting on have occurred.  If so, the thread is
 *      put on the ready queue.
 */

void
et_post(ulong events, tid_t tid)
/* register ulong   events;     event_list anchor pointer */
/* register pid_t   pid;        ID of process to post     */
{
        register struct thread  *t;     /* thread structure             */
	register struct proc	*p;	/* process structure		*/
        register int            ipri;   /* saved interrupt priority     */

	if (tid == NULL_TID)
		return;

	t = THREADPTR(tid);
        ASSERT(t->t_tid == tid);

        ipri = i_disable(INTMAX);       /* begin critical section       */

#ifdef _POWER_MP
        simple_lock(&proc_base_lock);
        simple_lock(&proc_int_lock);
#endif

        t->t_pevent |= events;
	p = t->t_procp;

	/* Check if the thread is waiting for an event */
	if (t->t_wtype == TWEVENT)
	{
		switch(t->t_state)
		{
		 case TSSLEEP:
			if (t->t_pevent & t->t_wevent)
				setrq(t, E_WKX_PREEMPT, RQTAIL);
			break;
		 case TSRUN:
		 case TSSTOP:
			if (t->t_pevent & t->t_wevent)
				t->t_wtype = TNOWAIT;
			break;
		 default:
			ASSERT(FALSE);
		}
        }

#ifdef _POWER_MP
       	simple_unlock(&proc_int_lock);
       	simple_unlock(&proc_base_lock);
#endif

        i_enable(ipri);                 /* exit critical section        */
}

/*
 * NAME:  thread_tsleep_timeout
 *
 * FUNCTION: the watchdog routine for thread_tsleep.
 *
 * EXECUTION ENVIRONMENT:
 *	This procedure is called from the interrupt environment only.
 *	It cannot page fault.
 *
 * NOTES: 
 *	This routine is called from the clock handler.  It clears the wait
 *	condition for the specified thread passing it a return code of
 *	THREAD_TIMED_OUT.  The specified thread is scheduled for the 
 *	processor.
 */

void
thread_tsleep_timeout(struct trb *trb)
{
	e_clear_wait((tid_t)trb->func_data, THREAD_TIMED_OUT);
}

/*
 * NAME:  thread_tsleep
 *
 * FUNCTION:  puts the current thread to sleep for some time.
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *	It cannot page fault during the disabled section.
 */

int
thread_tsleep(int ticks, atomic_p user_lock, const sigset_t *block)
{
	register struct thread *t = curthread;
	register struct trb *trb;
	char *errorp;				/* current errno	   */
	atomic_p kuser_lock;			/* kernel-mapped user lock */
	int user_lock_value;			/* user lock value	   */
	int rc;					/* return code		   */
	int srval;
	label_t buf;
	int didgeth;
	tid_t waitertid;
	int waitersleft;
	int oldpri;
	sigset_t sigmask;
	sigset_t oldmask;

	/*
	 * Make sure this is a user thread.
	 */
	ASSERT(csa->intpri == INTBASE);
	errorp = &t->t_uthreadp->ut_error;

	/*
	 * Get signals to unblock/reblock.
	 */
	if (block) {
		if (copyin((caddr_t)block,(caddr_t)&sigmask,sizeof(sigset_t))){
			*errorp = EFAULT;
			return(-1);
		}
		/* do not allow SIGKILL, or SIGSTOP to be masked */
		SIGDELSET(sigmask,SIGKILL);
		SIGDELSET(sigmask,SIGSTOP);
	}

	/*
	 * Preattach user space. We can not sleep for a second item
	 * once we are placed on the event list.
	 */
	if (user_lock) {
		srval = (didgeth = (((uint)user_lock >> SEGSHIFT) != PRIVSEG)) ?
			as_geth(&U.U_adspace, user_lock) :
			as_getsrval(&U.U_adspace, user_lock);
		kuser_lock = (int *)vm_att(srval, user_lock);
		simple_lock(&U.U_handy_lock);
	}

	/* 
	 * Preallocate trb.  We can not sleep for a second item 
	 * once we are placed on the event list.
	 */
	if (ticks) {
		trb = talloc();
		assert(trb != NULL);
	}

	/*
	 * Go on the event list.
	 */
	e_assert_wait(&t->t_tsleep, INTERRUPTIBLE);

	/*
	 * Release the user lock.
	 */
	if (user_lock) {
		/* Inline thread_unlock */
		if (setjmpx(&buf)) {
			simple_unlock(&U.U_handy_lock);
			e_clear_wait(t->t_tid, 0);
			if (ticks)
				tfree(trb);
			vm_det(kuser_lock);
			if (didgeth)
				as_puth(&U.U_adspace, srval);
			*errorp = EFAULT;
			return(-1);
		}
		if (!(*kuser_lock & UL_WAIT)) {
			_clear_lock(kuser_lock, UL_FREE); /* does a SYNC */
			rc = 0;
		} else {
			_clear_lock(kuser_lock, UL_WAIT); /* does a SYNC */
			e_assert_wakeup(user_lock, &waitertid, &waitersleft);
			TRCHKGT(HKWD_KERN_WAKEUPLOCK,
				user_lock, waitersleft, 0,0,0);
			if (waitersleft == 0)
				fetch_and_and(kuser_lock, ~UL_WAIT);
			rc = (waitertid == NULL_TID) ? 0 : 1;
		}
		simple_unlock(&U.U_handy_lock);
		if (rc == 1)
			e_clear_wait(waitertid, THREAD_AWAKENED);
		clrjmpx(&buf);
		vm_det(kuser_lock);
		if (didgeth)
			as_puth(&U.U_adspace, srval);
	}

	/*
	 * Install the new signal mask.
	 */
	if (block) {
		oldpri = disable_lock(INTMAX, &proc_base_lock);
		oldmask = t->t_sigmask;	
		t->t_sigmask = sigmask;	
		t->t_flags |= TSIGWAIT;
		unlock_enable(oldpri, &proc_base_lock);
	}

	/*
	 * Start the timer and sleep.
	 */
	if (ticks) {

		/*
		 * Convert the number of ticks specified to a time 
		 * value that can be provided to the timer services.
		 */
		TICKS_TO_TIME(trb->timeout.it_value, ticks);

		/*
		 * Ensure that this is treated as an incremental timer, 
		 * not an absolute one.
		 */
		trb->flags      =  T_INCINTERVAL;
		trb->func       =  thread_tsleep_timeout;
		trb->eventlist  =  EVENT_NULL;
		trb->func_data  =  t->t_tid;
		trb->ipri       =  INTTIMER;

		tstart(trb);			/* start the timer	*/
		rc = e_block_thread();		/* go to sleep		*/
		while (tstop(trb));		/* deactivate trb	*/
		tfree(trb);			/* free memory		*/
	}
	else {
		rc = e_block_thread();
	}

	switch (rc) {
	case THREAD_INTERRUPTED :		/* got signal */
		t->t_uthreadp->ut_error = EINTR;
		return(-1);
	case THREAD_TIMED_OUT :		/* slept all the way thru */
		return(0);
	default :
        	return(rc);
	}
}

/*
 * NAME:  thread_twakeup
 *
 * FUNCTION:  wake a thread up.
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *	It can page fault.
 */

int
thread_twakeup(tid_t tid, int wakeup)
{
	register struct thread *t;      /* pointer to caller         */

	assert(csa->intpri == INTBASE);	/* make sure this is a process  */

	if (wakeup <= 0)
	{
		u.u_error = EINVAL;
		return(-1);
	}

	t = VALIDATE_TID(tid);
	if ((t == NULL ) ||		/* invalid tid */
	    (t->t_procp != curproc))	/* not in same process */
	{
		u.u_error = ESRCH;
		return(-1);
	}

	e_clear_wait(tid, -wakeup); 	/* non preemptible */

	return(0);
}

/*
 * NAME:  thread_waitlock
 *
 * FUNCTION: sleep on a user lock
 *
 * EXECUTION ENVIRONMENT:
 *	This procedure can only be called by a user thread.
 *	It can page fault.
 *
 * RETURNS:
 *	-1, an error occurred
 *		EFAULT, the user lock address is bad
 *	0, successful, the lock was granted
 *	1, unsuccessful, the lock was not granted and we slept
 *		(we could have used -1 and EAGAIN, but it is tedious to
 *		 check errno in a multithreaded process)
 *
 * NOTE:
 *
 *	Lock values :
 *	The user lock word is divided into two flags :
 *		UL_BUSY		: lock free or not
 *		UL_WAIT		: waiters present or not
 *
 *	The convenient value UL_FREE is the absence of those two flags.
 *	!!! There is an assumption (look for check_lock below) that all the
 *	other bits are always zero. If they are not the user state may be
 *	inconsistent (missed ISYNC) but that would be an application
 *	programming error anyway. !!!
 *
 *	Anchorage system :
 *	When a thread requests a busy user lock, it has to wait on an event
 *	word, which is an anchor to an event list. Unfortunately, it is very
 *	impracticable to have user words as anchors. To overcome that, we use
 *	a single anchor (U_ulocks), hoping that there won't be that many
 *	waiters, and we record the real wait channel.
 *	The serialization of the UL_WAIT flag / U_ulocks list is done with the
 *	U_handy_lock.
 *
 *	Fork aftermath:
 *	When a child is forked, it inherits its parent's data, therefore
 *	a parent's contended lock becomes a child's contended lock, even
 *	though it is not contended in the child. It is not a problem though
 *	since the number of waiters is updated after a wake-up.
 *
 *	Sleep options:
 *	The sleep must be interruptible otherwise we may get stuck forever
 *	in the kernel if the user program never calls thread_unlock(). However
 *	it should not be a cancellation point, therefore we do not return EINTR
 *	nor ERESTART.
 */
int
thread_waitlock(atomic_p user_lock)
{
	atomic_p kuser_lock;			/* kernel-mapped user lock */
	int user_lock_value;			/* user lock value	   */
	int rc;					/* return code		   */
	int srval;
	label_t buf;
	int did_as_geth;
	int link_register;

	/*
	 * Make sure this is a user thread.
	 */
	ASSERT(csa->intpri == INTBASE);
	ASSERT(!(curthread->t_flags & TKTHREAD));

	/*
	 * Attach to user space.
	 */
	srval = (did_as_geth = (((uint)user_lock >> SEGSHIFT) != PRIVSEG)) ?
		as_geth(&U.U_adspace, user_lock) :
		as_getsrval(&U.U_adspace, user_lock);
	kuser_lock = (atomic_p)vm_att(srval, user_lock);

	/*
	 * Protect against bad user address
	 */
	if (setjmpx(&buf)) {

		/* Bad user address (the U_handy_lock has been locked though)*/
		curthread->t_uthreadp->ut_error = EFAULT;
		rc = -1;

	} else {

		simple_lock(&U.U_handy_lock);
		user_lock_value = fetch_and_or(kuser_lock, UL_BUSY|UL_WAIT);
		if (!(user_lock_value & UL_BUSY)) {
			/* We got the lock since it was free */
#ifdef _POWER_MP
			_check_lock(kuser_lock,UL_BUSY|UL_WAIT,UL_BUSY|UL_WAIT);
			/* It was a no-op check_lock, except it did an ISYNC */
#endif
			if (!(user_lock_value & UL_WAIT))
				*kuser_lock = UL_BUSY;
			rc = 0;
		} else {
			/* We didn't get the lock, we'll have to wait */
			GET_RETURN_ADDRESS(link_register);
			TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_MISS,
				user_lock, user_lock_value, LOCK_SWRITE_TRACE,
				link_register, NULL);
			TRCHKGT(HKWD_KERN_WAITLOCK,
				U.U_procp->p_pid, curthread->t_tid,
				user_lock, 0,0);
			curthread->t_ulock = user_lock;
			e_assert_wait(&U.U_ulocks, INTERRUPTIBLE);
			rc = 1;
		}

	}
	simple_unlock(&U.U_handy_lock);

	/*
	 * Sleep if we have to.
	 */
	if (rc == 1)
		e_block_thread();

	/*
	 * Detach from user space.
	 */
	if (rc >= 0)
		clrjmpx(&buf);
	vm_det(kuser_lock);
	if (did_as_geth)
		as_puth(&U.U_adspace, srval);

	return(rc);
}

/*
 * NAME:  thread_unlock
 *
 * FUNCTION: wake up sleepers on a user lock
 *
 * EXECUTION ENVIRONMENT:
 *	This procedure can only be called by a user thread.
 *	It can page fault.
 *
 * RETURNS:
 *	-1, an error occurred
 *		EFAULT, the user lock address is bad
 *	0, successful, the lock was freed, nobody was woken up
 *	1, successful, the lock was freed, someone was woken up
 *
 * NOTE:
 *	See note for thread_request()
 */
int
thread_unlock(atomic_p user_lock)
{
	atomic_p kuser_lock;			/* kernel-mapped user lock */
	int rc;					/* return code		   */
	int srval;
	label_t buf;
	int did_as_geth;
	tid_t waitertid;			/* tid of thread to wake up*/
	int waitersleft;			/* # of waiters left on que*/

	/*
	 * Make sure this is a user thread.
	 */
	ASSERT(csa->intpri == INTBASE);
	ASSERT(!(curthread->t_flags & TKTHREAD));

	/*
	 * Attach to user space.
	 */
	srval = (did_as_geth = (((uint)user_lock >> SEGSHIFT) != PRIVSEG)) ?
		as_geth(&U.U_adspace, user_lock) :
		as_getsrval(&U.U_adspace, user_lock);
	kuser_lock = (int *)vm_att(srval, user_lock);

	/*
	 * Protect against bad user address
	 */
	if (setjmpx(&buf)) {

		/* Bad user address (the U_handy_lock has been locked though)*/
		curthread->t_uthreadp->ut_error = EFAULT;
		rc = -1;

	} else {

		simple_lock(&U.U_handy_lock);
		if (!(*kuser_lock & UL_WAIT)) {
			_clear_lock(kuser_lock, UL_FREE); /* does a SYNC */
			rc = 0;
		} else {
			_clear_lock(kuser_lock, UL_WAIT); /* does a SYNC */
			e_assert_wakeup(user_lock, &waitertid, &waitersleft);
			TRCHKGT(HKWD_KERN_WAKEUPLOCK,
				user_lock, waitersleft, 0,0,0);
			if (waitersleft == 0)
				fetch_and_and(kuser_lock, ~UL_WAIT);
			rc = (waitertid == NULL_TID) ? 0 : 1;
		}

	}
	simple_unlock(&U.U_handy_lock);
			
	/*
	 * Wake up if we have to.
	 */
	if (rc == 1)
		e_clear_wait(waitertid, THREAD_AWAKENED);     /* preemptible */

	/*
	 * Detach from user space.
	 */
	if (rc >= 0)
		clrjmpx(&buf);
	vm_det(kuser_lock);
	if (did_as_geth)
		as_puth(&U.U_adspace, srval);

	return(rc);
}

/*
 * NAME:  e_assert_wakeup
 *
 * FUNCTION: asserts that the highest priority sleeper on the specified user
 *	lock is about to be woken up.
 *
 * RETURNS:
 *	tid of sleeper to be woken up.
 *	number of waiters left.
 *
 * EXECUTION ENVIRONMENT:
 *	Called by a thread.
 *	Cannot page fault.
 */

void
e_assert_wakeup(atomic_p user_lock, tid_t *awaking, int *sleepers)
{
	register struct thread *lastt;	/* last thread on the event list      */
	register struct thread *t;	/* thrd ptr to traverse the event list*/
	register struct thread *waket;	/* most favored thread in event list  */
	tid_t waittid;			/* thread id on event list	      */
	int count;			/* number of waiters left	      */
	int intpri;			/* saved interrupt priority	      */


	count = 0;
	intpri = disable_lock(INTMAX, &proc_base_lock);

	/* get list anchor (won't page fault) */
	waittid = LOADWCS(&U.U_ulocks);

	if (waittid != EVENT_NULL) {
		/*
		 * Find most favored thread.  waittid identifies the last
		 * thread added to the list.  The next thread the first.
		 */
		lastt = THREADPTR(waittid);
		if ((waket = lastt->t_eventlist) == lastt) {
			/* only one element on the list */
			if (waket->t_ulock == user_lock)
				count = 1;
		} else {
			/* several elements on the list */
			if (waket->t_ulock == user_lock)
				count = 1;
			t = waket;
			do {
				t = t->t_eventlist;
				if (t->t_ulock == user_lock) {
					if (count) {
						if (t->t_pri < waket->t_pri)
							waket = t;
					} else
						waket = t;
					count++;
				}
			} while (t != lastt);
		}
	}

	if (count) {
		waket->t_ulock = NULL;		/* assert wakeup */
		*awaking = waket->t_tid;
		*sleepers = count - 1;		/* we'll be off soon */
	} else {
		*awaking = NULL_TID;
		*sleepers = 0;
	}
			
	unlock_enable(intpri, &proc_base_lock);
}
