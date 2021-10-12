static char sccsid[] = "@(#)84	1.4  src/bos/kernel/proc/simple_lock_com.c, sysproc, bos411, 9428A410j 4/5/94 14:19:56";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: simple_lock_init
 *		simple_lock_init_instr
 *
 *   ORIGINS: 27 83
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * NOTES:
 *      There is no functional difference between the POWER and the
 *      POWERPC version of these functions.  These functions will only be
 *      defined here for both POWER and POWERPC (no extension or branch
 *      table is necessary).
 *
 *      The lock word can be pageable, but must be in ordinary working
 *      storage.  It must not be in a persistent segment with journalling
 *      applied.  This is because the deadlock signal is not handled by
 *      these service routines.  Paging errors on the lock word cause
 *      the kernel to panic.
 *
 * EXECUTION ENVIRONMENT:
 *      These functions are pinned and will only fault if called on a
 *      pageable stack or passed a pageable lock word.
 */

#include <sys/types.h>
#include <sys/proc.h>
#include <sys/thread.h>
#include <sys/ppda.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/atomic_op.h>
#include <sys/intr.h>
#include <sys/syspest.h>
#include <sys/systm.h>
#include <sys/signal.h>
#include <sys/prio_calc.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/trchkid.h>
#include <sys/sleep.h>

#ifdef _INSTRUMENTATION
#ifdef DEBUG
int lock_instr_dbg_wanted = 0; /* set to true if allocation debug wanted */
#endif
#endif

/*
 * NAME: simple_lock_init
 *
 * FUNCTION: initialize simple lock
 *
 * NOTE: when using instrumentation counters, all field are reset by 
 *	 simple_lock_init; in order to record statistics by instance
 *	 and by lock class before resetting the counters old values
 *	 are stored in the family_lock_statistics table, holding
 *	 per class statistics.
 *
 * RETURNS: no return 
 */

#ifdef _INSTRUMENTATION
void
simple_lock_init_instr(simple_lock_t lockaddr)
#else

#if defined(_KERNSYS)
#ifdef simple_lock_init
/* need to undo MACRO definition--if this function changes, so must MACRO */
#undef simple_lock_init
#endif
#endif

void
simple_lock_init(simple_lock_t l)
#endif

{
#ifdef _INSTRUMENTATION
	struct lock_data_instrumented	*l;
	
	if ( lockaddr->_slock & INSTR_ON) {
		l = lockaddr->_slockp;
	/* when reusing a lock_data_instrumented structure 
	 * old statistics are accrued in the family_lock_statistics
	 * table
	 */
		fetch_and_add(&family_lock_statistics[l->lock_id].acquisitions, l->acquisitions);
		fetch_and_add(&family_lock_statistics[l->lock_id].misses, l->misses);
		fetch_and_add(&family_lock_statistics[l->lock_id].sleeps, l->sleeps);

		l->acquisitions = 0;
		l->misses = 0;
		l->sleeps = 0;
	}
	else {
#ifdef DEBUG
		if (lock_instr_dbg_wanted) {
			printf("Warning:  simple_lock not allocated\n");
			brkpoint();
		}
#endif
		l = (struct lock_data_instrumented *)lockaddr;
	}
#endif
	
	*((simple_lock_data *)l) = SIMPLE_LOCK_AVAIL;
}
