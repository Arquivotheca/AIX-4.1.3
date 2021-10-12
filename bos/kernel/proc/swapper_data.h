/* @(#)62	1.9  src/bos/kernel/proc/swapper_data.h, sysproc, bos411, 9428A410j 1/26/94 15:19:33 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: MASKBIT
 *		
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#ifndef _H_SWAPPER_DATA
#define _H_SWAPPER_DATA

#define sched_event_timer    0x00000001  /* 1 second timer event         */
#define sched_event_swapin   0x00000002  /* swapin runnable process event*/

/*
 *	Ready process queues.
 *
 *	The ready queue consists of two data structures, run_mask
 *	and proc_run.  The actual queues are anchored in proc_run,
 *	which is an array containing the heads of circularly-linked
 *	lists of proc structures that are ready, but not running.
 *	Each process priority has its own ready queue, so there is
 *	no sorting of entries within these lists.  New arrivals are
 *	always added to the end, and the dispatcher always selects
 *	the entry at the head of the most-favored priority queue.
 *	
 *	To speed up selecting the highest priority non-empty queue,
 *	run_mask stores a parallel bit mask indexed by priority level.
 *	This mask contains a one bit for every priority that has ready
 *	processes, i.e. for every non-null queue head in proc_run.
 *	
 *	Note that running processes are not on the ready queue.
 *	This permits use of the p_next pointer for wait lists and
 *	lock lists.  It also allows changing the current process's
 *	priority without rechaining it.
 *	
 *	The dispatcher logic assumes that there is ALWAYS at least
 *	one ready process.  The wait process ensures that this
 *	will be true.  It runs at the lowest possible process
 *	priority, PMASK, and never waits.  Since the best way to
 *	do nothing is generally machine-dependent, a separate
 *	module, `waitproc' is assigned this task.  An infinite
 *	loop would be a logically correct implementation for this
 *	function.
 *
 *	These structures are shared between the sched.c and dispatch.c. 
 */

#define RQTAIL		0
#define RQHEAD		1

#define	BITS_PER_WORD	(NBPB*NBPW)		 /* number of bits per word   */
#define MASKBIT(lvl)	(1<<(BITS_PER_WORD-1-(lvl)%BITS_PER_WORD))
#define	NRUNMASK	((PMASK+1)/BITS_PER_WORD)/* number of runmask words   */

struct ready_queue_flags {
	unsigned int word[NRUNMASK];
};

extern struct ready_queue_flags run_mask;	 /* ready queue flags         */
extern struct thread *thread_run[PMASK+1];	 /* ready queue pointer table */

#ifndef _POWER_MP
#define INC_RUNRUN(inc)	(runrun = ((inc) <= 0) ? 0 : 1)
#else /* _POWER_MP */
#define INC_RUNRUN(inc)						\
{								\
	/* This macro assumes caller owns the proc_int_lock */	\
	register int ncpus, new;				\
	if ((new = (runrun + inc)) <= 0)			\
		new = 0;					\
	else {							\
		ncpus = NCPUS();				\
		if (new > ncpus)				\
			new = ncpus;				\
	}							\
	runrun = new;						\
}
#endif

extern int timeslice;				/* timeslice for threads */

#endif /* _H_SWAPPER_DATA */
