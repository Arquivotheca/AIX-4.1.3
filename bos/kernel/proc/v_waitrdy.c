static char sccsid[] = "@(#)63	1.24  src/bos/kernel/proc/v_waitrdy.c, sysproc, bos41J, 9512A_all 3/20/95 19:38:01";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: v_ready
 *              v_readyp
 *              v_readyt
 *              v_wait
 *
 *   ORIGINS: 27, 83
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
 *
 * All VMM wait lists have the same format.  The list anchor
 * is either NULL or contains a pointer to the last entry in
 * a circular chain of proc structs.  Since the list is circular, 
 * this index points to the newest entry, allowing FIFO processing
 * when desired.  Some lists do not require this feature, but
 * they are all handled alike for convenience.	The chain pointer
 * is p_next, which is a proc structure pointer.
 *								
 * Processes are always queued at the end of the wait list.
 * This gives that process priority on free page frames to 
 * avoid storage deadlocks.	    
 *
 * EXECUTION ENVIRONMENT
 *
 * ATTRIBUTES:
 *
 *   REUSEABLE	 = Yes			
 *   REENTRANT	 = No				
 *   RECURSIVE	 = No					
 *								
 * NOTES:	
 *	These procedures are called within a VMM critical section 
 *	and will not fault.
 *
 *	Processes on this list cannot terminate.  At most they may be
 *	signalled to terminate, but the signal will be deferred until
 *	after they are runnable again.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	setrun		place process on ready queue
 *	unready		prepare current process for sleep
 *
 */

#include <sys/types.h>
#include <sys/proc.h>
#include <sys/systm.h>
#include <sys/var.h>
#include <sys/syspest.h>
#include <sys/intr.h>
#include <sys/sleep.h>
#include <sys/atomic_op.h>
#include "swapper_data.h"


/*
 *  v_wait -- enqueue current thread on VMM wait list.
 */
void 
v_wait(struct thread **wait_list)

/* struct thread **wait_list;		pointer to wait list anchor */
{
	register struct ppda *myppda = PPDA;	/* current ppda pointer */
	register struct thread *t;		/* current thread pointer */
	register int save_ipri;	

	t = myppda->_curthread;

	save_ipri = disable_lock(INTMAX, &proc_base_lock);

	/*
	 * If the thread is already on a wait list or ppda_no_vwait
	 * is set then this wait request is a result of a page-ahead
	 * operation so just ignore it.
	 */
	if (t->t_wchan2 != NULL || myppda->ppda_no_vwait) {
	  	ASSERT(t->t_wchan2 == NULL || t->t_wtype == TWPGIN);
		unlock_enable(save_ipri, &proc_base_lock);
	  	return;
	}

	t->t_flags &= ~TWAKEONSIG;	/* unconditionally turn it off*/

	if (*wait_list) {		/* if list isn't empty: */
		t->t_next = (*wait_list)->t_next; /* point curthread to head */
		(*wait_list)->t_next = t;         /* point tail to curthread */
	}
	else {				/* list is currently empty: */
		t->t_next = t;		/* point to self (1-element list) */
	}
	*wait_list = t;		     /* update anchor curthread becomes tail */
	
	/*
	 *  Put this thread to sleep in a page wait.
	 *  The exit from VMM critical sections simulates an interrupt return.
	 *  dispatch() will choose another ready thread.
	 *  The current thread is suspended in the act of returning.
	 */
	ASSERT(t->t_state == TSRUN);	/* dispatch will put TSSLEEP here */
	t->t_wtype = TWPGIN;
	t->t_wchan2 = (char *)wait_list;

	myppda->_ficd = 1;		/* call dispatch at end of crit sect */

	unlock_enable(save_ipri, &proc_base_lock);
}


/*
 *  v_ready -- resume the first thread on a VMM wait list.
 */
void 
v_ready(struct thread **wait_list)

/* struct thread **wait_list;		pointer to wait list anchor */
{
	register struct thread *waitt;		/* last thread on list */
	register struct thread *waket;		/* thread being awakened */
	register int save_ipri;

	save_ipri = disable_lock(INTMAX, &proc_base_lock);
#ifdef _POWER_MP
	simple_lock(&proc_int_lock);
#endif

	/*
	 * Check tail of the list.  Could have already been made
	 * runnable.  Threads can be readied on the interrupt
	 * level via v_interrupt.
	 */
	waitt = *wait_list;
	if (waitt == NULL) {	
#ifdef _POWER_MP
		simple_unlock(&proc_int_lock);
#endif
		unlock_enable(save_ipri, &proc_base_lock);
		return;	
	}

	waket = waitt->t_next;		/* head of wait list */
	ASSERT(waket);
	
	/* remove from wait list */
	if (waitt == waket)		/* only one thread on list? */
		*wait_list = NULL;	/* mark list empty */
	else				/* other waiters remain	*/
		waitt->t_next = waket->t_next;

	/* 
	 * If the thread has been stopped while waiting on this queue,
	 * we do not ready the thread, rather, we set t_wtype to TNOWAIT
	 * so that when the thread receives the SIGCONT, it will know
	 * that the event has occurred. 
	 */
	waket->t_wchan2 = NULL;

	if (waket->t_state == TSSTOP) {
		waket->t_wtype = TNOWAIT;
	}
	else if (waket->t_state == TSRUN) {
		/*
		 * This state is introduced to prevent the thread from being
		 * put on the runqueue twice.  The dispatcher is run after the 
		 * exception and queues the current thread on the runqueue.
		 * This code assumes that the dispatcher will change the
		 * state of the thread to TSSLEEP if an intervening v_ready
		 * does not occur.
		 */
	  	ASSERT(waket->t_wtype == TWPGIN);
	  	waket->t_wtype = TNOWAIT;
	} else {
	  	ASSERT((waket->t_wtype != TWPGIN)||(waket->t_state == TSSLEEP));
	  	setrq(waket, E_WKX_PREEMPT, RQTAIL); /* make first thread rdy */
	}

#ifdef _POWER_MP
	simple_unlock(&proc_int_lock);
#endif
	unlock_enable(save_ipri, &proc_base_lock);

}


/*
 *  v_readyt -- resume a specified thread on a VMM wait list.
 */
void 
v_readyt(wait_list,waket)

struct thread **wait_list;		/* pointer to wait list anchor */
struct thread *waket;			/* thread being awakened */
{
	register struct thread *prevt;		/* last thread on list */
	register struct thread *wlist;		/* wait list anchor */
	register int save_ipri;
	register int hadintlock;
	register int hadbaselock;

	save_ipri = i_disable(INTMAX);

#ifdef _POWER_MP
	/* may have lock if called from vcs_interrupt via signal delivery */
        if (!(hadbaselock = lock_mine(&proc_base_lock)))
                simple_lock(&proc_base_lock);
	if (!(hadintlock = lock_mine(&proc_int_lock)))
		simple_lock(&proc_int_lock);
#endif

	ASSERT(waket);			/* should not be NULL if we're here */

        /*
         * Check tail of the list.  Could have already been made
         * runnable.  Threads can be readied on the interrupt
         * level via v_interrupt.
         */
        if ((prevt = wlist = *wait_list) == NULL)
		goto done;

	for (;;) {
		if (prevt->t_next == waket)
			break;
		if ((prevt = prevt->t_next) == wlist)
			goto done;
	}
		
	prevt->t_next = waket->t_next;

	if (wlist == waket) {
		if (waket == prevt)
			*wait_list = NULL;
		else
			*wait_list = prevt;
	}

	/* If the thread has been stopped while waiting on this queue,
	 * we do not ready the thread, rather, we set t_wtype to SNOWAIT
	 * so that when the threads receives the SIGCONT, it will
	 * know that the event has occurred.
	 */
	waket->t_wchan2 = NULL;

	if (waket->t_state == TSSTOP)
	  	waket->t_wtype = TNOWAIT;
	else if (waket->t_state == TSRUN) {
                /*
                 * This state is introduced to prevent the thread from being
                 * put on the runqueue twice.  The dispatcher is run after the
                 * exception and queues the current thread on the runqueue.
                 * This code assumes that the dispatcher will change the
                 * state of the thread to TSSLEEP if an intervening v_ready
                 * does not occur.
                 */
	  	ASSERT(waket->t_wtype == TWPGIN);
	  	waket->t_wtype = TNOWAIT;
	} else {
	  	ASSERT((waket->t_wtype != TWPGIN)||(waket->t_state == TSSLEEP));
	  	setrq(waket, E_WKX_PREEMPT, RQTAIL); /* make first thread rdy */
	}

done:
#ifdef _POWER_MP
	if (!hadintlock)
		simple_unlock(&proc_int_lock);
	if (!hadbaselock)
		simple_unlock(&proc_base_lock);
#endif
	i_enable(save_ipri);

}
