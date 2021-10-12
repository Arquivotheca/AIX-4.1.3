static char sccsid[] = "@(#)43	1.17.1.1  src/bos/kernel/ios/selnotify.c, sysios, bos41B, 412_41B_sync 1/19/95 14:30:19";
/*
 * COMPONENT_NAME: (SYSIOS) Select/Poll notification routine
 *
 * FUNCTIONS:	selnotify	sel_hash
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
#include <sys/intr.h>
#include <sys/proc.h>
#include <sys/sleep.h>
#include <sys/file.h>
#include <sys/sysmacros.h>
#include <sys/machine.h>
#include "selpoll.h"
#include <sys/thread.h>
#include <sys/lock_def.h>
#include <sys/atomic_op.h>
#include <sys/inline.h>

extern Simple_lock select_lock; /* alloc and init is done in selpollinit() */
				/* under #ifdef _POWER_MP */

/*
 * devsel_hash points to an array of pointers to sel_cb (select control
 * block) structures.  This array is used to chain these control
 * blocks (based on their device id and unique id), using
 * a hashing algorithm.
 */
struct sel_cb	*devsel_hash[MAX_SEL_CBS];

/*
 * NAME:  selnotify
 *
 * FUNCTION:	Wake up thread(s) waiting on a poll or select system call
 *	by specifying the device and 1 or more of the events that the kernel
 *	is being notified of.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called by device drivers and the fileops for
 *	sockets and pipes.
 *      It cannot page fault.
 *
 * NOTES:	This kernel service must be used by device drivers that
 *	support the select or poll system calls.  It should be called
 *	whenever a previous call to the device driver's ddselect entry
 *	point returned with the status of all requested events false and
 *	asynchronous notification of the events was requested.  It is also
 *	used (in the case of sockets and pipes) by the sockets fileops
 *	code (sohasoutofband, sbwakeup) and by the pipes fileops code
 *	(fifo_read, fifo_write, fifo_close).
 *
 *
 *
 * DATA STRUCTURES:	sel_cb -- select control block;
 *			devsel_hash -- hashed array of sel_cb pointers.
 *
 * RETURN VALUES DESCRIPTION:	none.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_disable	disable interrupts
 *	i_enable	enable interrupts
 *	et_post		post events to a thread
 *	disable_lock	disable interrupts, lock select_lock
 *	unlock_enable	unlock select_lock, enable interrupts
 *	fetch_and_and	to modify TSELECT
 */
void
selnotify(sel_id, unique_id, rtnevents)

int	sel_id;			/* dev that req. events occurred on 	*/
int	unique_id;		/* entity that req. events occurred on	*/
ushort	rtnevents;		/* returned events			*/
{

	register int		ipri;	/* current interrupt priority	*/
	register int		ipri2;	/* current interrupt priority	*/
	register unsigned long	index;	/* index into hash table	*/
	register struct sel_cb	*cb;	/* ctl blk at head of thread chain*/
	extern void		et_post();
	void			(*notify)();

	index = sel_id ^ unique_id;
	index = (index ^ (index >> 16)) % MAX_SEL_CBS;

#ifndef _POWER_MP
	ipri = i_disable(INTIODONE);	/* enter critical section	*/
#else /* _POWER_MP */
	ipri = disable_lock(INTMAX, &select_lock);
#endif /* _POWER_MP */
	/*
	 * Calculate the correct element in the devsel_hash
	 * array to look at.  Then run that element's chain,
	 * processing each control block whose sel_id, unique_id,
	 * and rtnevents match the input sel_id, unique_id, and
	 * rtnevents.
	 */

	cb = devsel_hash[index];
	while (cb != NULL)
	{
		if ((cb->dev_id == sel_id) &&
		    (cb->unique_id == unique_id) &&
		    ((cb->reqevents | POLLHUP | POLLERR) & rtnevents))
		{
			/*
			 * This control block matches, so post
			 * its thread of the events that occurred.
			 */
			if (cb->notify != NULL) 
			{	
				notify = cb->notify;
				(*notify)(sel_id, unique_id, rtnevents,
					cb->threadp->t_procp->p_pid);
			}	
			else
			{	
#ifndef _POWER_MP
				ipri2 = disable_ints();
#endif /* _POWER_MP */
				cb->rtnevents |= rtnevents;
#ifndef _POWER_MP
				enable_ints(ipri2);
#else
				/*
				 * We must make sure that the order of updates
				 * to cb->rtnevents and cb->threadp->t_atomic
				 * will be observed by select()
				 */
				SYNC();
#endif /* _POWER_MP */
				fetch_and_and(&cb->threadp->t_atomic, ~TSELECT);
				et_post(EVENT_SYNC, cb->threadp->t_tid);
			}	
		}

		/*
		 * Get the next control block in the chain.
		 */
		cb = cb->dev_chain_f;

	};

#ifndef _POWER_MP
	i_enable(ipri);			/* end of critical section	*/
#else /* _POWER_MP */
	unlock_enable(ipri, &select_lock); /* end of critical section	*/
#endif /* _POWER_MP */
	return;

}  /* end selnotify */       

/*
 * NAME:  sel_hash
 *
 * FUNCTION:	Internal routine to add or delete a select control block
 *	(struct sel_cb) onto or off of the devsel_hash hash table.
 *   
 * EXECUTION ENVIRONMENT:
 *      This routine is called by selreg and selpoll_cleanup to add or
 *	remove control blocks from the device select hash table.
 *      It will not page fault.
 *
 * NOTES:	The devsel_hash table is accessed by selnotify possibly at
 *		interrupt, so interrupts must be disabled to INTMAX before
 *		modifying the hash table. This routine is always called 
 *		from the thread environment.   
 *
 *	  	Valid flags are SEL_HASH_DEL and SEL_HASH_ADD.
 *
 * DATA STRUCTURES:	sel_cb -- select control block;
 *			devsel_hash -- hashed array of sel_cb pointers.
 *
 * RETURN VALUES DESCRIPTION:	none.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	disable_lock	disable interrupts, lock select_lock
 *	unlock_enable	unlock select_lock, enable interrupts
 */
void
sel_hash(sel_cb, flag)
struct sel_cb	*sel_cb;
{
	register int	x;
	register unsigned long	index;	/* index into hash table	*/

	if (flag == SEL_HASH_DEL) {
#ifdef _POWER_MP		/* POWER_MP	*/
		x = disable_lock(INTMAX, &select_lock); /* enter critical  */
#else				/* POWER_MP	*/
		x = disable_ints();
#endif				/* POWER_MP	*/

		if (sel_cb->dev_chain_b->dev_chain_f = sel_cb->dev_chain_f)
			sel_cb->dev_chain_f->dev_chain_b = sel_cb->dev_chain_b;

#ifdef _POWER_MP		/* POWER_MP	*/
		unlock_enable(x, &select_lock);	/* exit critical   */
#else				/* POWER_MP	*/
		enable_ints(x);
#endif				/* POWER_MP	*/
	} else {
		register struct sel_cb *cb, **cbp;

		index = sel_cb->dev_id ^ sel_cb->unique_id;
		index = (index ^ (index >> 16)) % MAX_SEL_CBS;

		cbp = &devsel_hash[index];		/* insertion point */

#ifdef _POWER_MP		/* POWER_MP	*/
		x = disable_lock(INTMAX, &select_lock); /* enter critical  */
#else				/* POWER_MP	*/
		x = disable_ints();
#endif				/* POWER_MP	*/

		sel_cb->dev_chain_b = (void *) cbp;	/* back link	   */

		if (cb = *cbp)				/* not empty?	   */
			cb->dev_chain_b = sel_cb;	/* fixup back link */

		sel_cb->dev_chain_f = cb;		/* forward link	   */

		*cbp = sel_cb;				/* insert	   */

#ifdef _POWER_MP		/* POWER_MP	*/
		unlock_enable(x, &select_lock);	/* exit critical   */
#else				/* POWER_MP	*/
		enable_ints(x);
#endif				/* POWER_MP	*/
	}
}
