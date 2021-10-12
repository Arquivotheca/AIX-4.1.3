static char sccsid[] = "@(#)02	1.15  src/bos/kernel/proc/watchdog.c, sysproc, bos411, 9437B411a 9/12/94 13:39:49";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: w_clear
 *		w_init
 *		w_start
 *		w_stop
 *		watchdog
 *
 *   ORIGINS: 27,83
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


/*
 * The watchdog timer services provide a fast way to start and stop
 * a timer. They are intended for situations where the timeout
 * condition almost never occurs and more that one operation is
 * performed each second. They trade off the coarse granularity of
 * timeout operations vs the efficieny of starting and stoping
 * their timer.
 */

#include <sys/types.h>
#include <sys/watchdog.h>
#include <sys/intr.h>
#include <sys/systm.h>
#include <sys/mstsave.h>
#include <sys/syspest.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/systemcfg.h>

/*
 *	The following is the anchor of the watch dog timers. The
 *	watchdog timer list is a doubly linked circular list.
 *
 *	*--------------------------------------------*
 *	| *----------------------------------------* |
 *	| |    wtimer        head         tail     | |
 *	| |   *------*     *------*     *------*   | |
 *	| *-> | next | --> | next | --> | next | --* |
 *	*---- | prev | <-- | prev | <-- | prev | <---*
 *	      |      |     |      |     |      |
 *	      *------*     *------*     *------*
 */
#ifndef _POWER_MP
extern int brkpoint();
struct watchdog wtimer = { &wtimer, &wtimer, (void (*)())brkpoint, 0, 0 };
#endif /* _POWER_MP */


/*
 * NAME:  watchdog
 *
 * FUNCTION:  Decrement each of the active watchdog timers and call
 *	their timeout routine if their timer expires.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine runs at INTTIMER interrupt priority on the timer
 *	interrupt level.
 *
 *      It does not page fault.
 *
 * NOTES:
 *	This routine is only called by the system timer once each second.
 *
 * RETURN VALUE DESCRIPTION: None
 *
 * EXTERNAL PROCEDURES CALLED:
 *	watchdog timeout routines
 */

void
watchdog()
{
	register struct watchdog *w;		/* next watchdog timer */
	register struct watchdog *wanchor;	/* addr of anchor */
	cpu_t my_cpu=CPUID;			
	ulong count;

	/*
	 *  Only caller is the clock interrupt handler via the system
	 *  timeout routine.
	 */
	ASSERT( csa->prev != NULL );
	ASSERT( csa->intpri == INTTIMER );

	/*
	 *  Decrement the current count for each of the active watchdog
	 *  timers and call their timeout routine if they have now expired.
	 */
#ifdef _POWER_MP
	simple_lock(&watchdog_lock);
#endif

	wanchor = &TIMER(my_cpu)->wtimer;
	for (w = wanchor->next; w != wanchor; w = w->next)
	{
		/*
		 * An atomic operation is needed because w->count can be
		 * modified by w_start() or w_stop(), either by another
		 * processor, or by a driver responding to a higher level
		 * interrupt.
		 */
		count = w->count;
		while((count>0) && !compare_and_swap(&w->count,&count,count-1));
		if (count == 1)
		{
			TIMER(my_cpu)->w_called = w;
#ifdef _POWER_MP
		  	simple_unlock(&watchdog_lock);
#endif

		  	(*(w->func))(w);

			if (csa->intpri < INTTIMER)
				i_enable(INTTIMER);
				
#ifdef _POWER_MP
	  		simple_lock(&watchdog_lock);
#endif
			TIMER(my_cpu)->w_called = NULL;
		}
	}
#ifdef _POWER_MP
	simple_unlock(&watchdog_lock);
#endif
}


/*
 * NAME:  w_init
 *
 * FUNCTION:  Add a watchdog timer to the list of watchdog timers
 *	defined to the system.
 *
 * EXECUTION ENVIRONMENT:
 *	This service can only be called under a process.
 *
 *      It can page fault.
 *
 * RETURN VALUE DESCRIPTION: Zero if succesful, otherwise one
 *
 * EXTERNAL PROCEDURES CALLED:
 *	disable_lock	begin watchdog timer critical section
 *	unlock_enable	end watchdog timer critical section
 */

int
w_init(register struct watchdog *w)
/* register struct watchdog	*w;		 next watchdog timer */
{
	register int		intpri;		/* current interrupt priority */
	cpu_t 			my_cpu=CPUID;

	/*
	 *  Watchdog timers can only be defined when executing under a
	 *  process. They are typically defined by the device driver's
	 *  open routine.
	 */
	assert( csa->prev == NULL );

	/*
	 *  Verify that this watchdog timer is not already defined to
	 *  the system.
	 */
	ASSERT( w->next == (struct watchdog *)NULL );
	ASSERT( w->prev == (struct watchdog *)NULL );

	/*
	 *  Insert the new watchdog timer at the end of the list.
	 */
	intpri = disable_lock(INTTIMER, &watchdog_lock);

	/*
	 * Currently calling a watchdog routine.
	 */
	if (TIMER(my_cpu)->w_called != NULL){
		/*
		 * This should never happen because this routine
		 * can only be called at the process level and
		 * watchdog timeout functions are called only at
		 * the interrupt level.  All interrupt level 
		 * processing must be completed before a thread
		 * or a process is scheduled.
		 */
		assert(0);
	}
	w->next = &(TIMER(my_cpu)->wtimer);
	w->prev = TIMER(my_cpu)->wtimer.prev;
	w->next->prev = w;
	w->prev->next = w;

	unlock_enable(intpri, &watchdog_lock);
	return 0; 					/* success */
}

/*
 * NAME:  w_clear
 *
 * FUNCTION:  Remove a watchdog timer from the list of watchdog timers
 *	defined to the system.
 *
 * EXECUTION ENVIRONMENT:
 *	This service can only be called under a process.
 *
 *      It does page fault.
 *
 * RETURN VALUE DESCRIPTION: Zero if success, otherwise one
 *
 * EXTERNAL PROCEDURES CALLED:
 *	disable_lock	begin watchdog timer critical section
 *	unlock_enable	end watchdog timer critical section
 */

int
w_clear(register struct watchdog *w)
{
	register int		intpri;		/* current interrupt priority */
	int 			i;
	int 			ncpus=NCPUS();
	cpu_t			my_cpu=CPUID;

	/*
	 *  Watchdog timers can only be removed when executing under a
	 *  process. They are typically defined by the device driver's
	 *  open routine.
	 */
	assert( csa->prev == NULL );

        /* 
         *  Insure that we have valid pointers
         *    (e.g. not calling w_clear() twice)
         */
        assert(w->next != NULL);
        assert(w->prev != NULL);

	/*
	 *  Remove the watchdog timer from the list of watchdog timers.
	 */
	intpri = disable_lock(INTTIMER, &watchdog_lock);
	for (i = 0; i < ncpus; i++){ 

		/* See if this watchdog is in use */
	  	if (TIMER(i)->w_called == w){
			ASSERT(i != my_cpu);
	    		unlock_enable(intpri, &watchdog_lock);
	    		return -1;
	  	}

	}

	w->next->prev = w->prev;
	w->prev->next = w->next;
	w->next = (struct watchdog *)NULL;
	w->prev = (struct watchdog *)NULL;

	unlock_enable(intpri, &watchdog_lock);
	return 0; 					/* success */
}

/*
 * NAME:  w_start
 *
 * FUNCTION:  Restart a watchdog timer
 *
 * EXECUTION ENVIRONMENT:
 *	This service can be called under a process or interrupt level.
 *
 * RETURN VALUE DESCRIPTION: None
 *
 * NOTE:
 *	w->count is also modified by watchdog() and w_stop(). Synchronization
 *	is provided by atomic primitives : a compare_and_swap in watchdog(),
 *	simple stores in w_start() and w_stop(). 
 *	(For MP, simple stores are atomic primitives on PPC systems because
 *	they cancel any pending reservation, they are not on PWR systems where
 *	that mechanism does not exist, but MP and PWR is not supported.)
 */
void
w_start(struct watchdog *w)
{
	w->count = w->restart;
}

/*
 * NAME:  w_stop
 *
 * FUNCTION:  Stop a watchdog timer
 *
 * EXECUTION ENVIRONMENT:
 *	This service can be called under a process or interrupt level.
 *
 * RETURN VALUE DESCRIPTION: None
 *
 * NOTE:
 *	w->count is also modified by watchdog() and w_start(). Synchronization
 *	is provided by atomic primitives : a compare_and_swap in watchdog(),
 *	simple stores in w_start() and w_stop().
 *      (For MP, simple stores are atomic primitives on PPC systems because
 *      they cancel any pending reservation, they are not on PWR systems where
 *      that mechanism does not exist, but MP and PWR is not supported.)
 */
void
w_stop(struct watchdog *w)
{
	w->count = 0;
}
