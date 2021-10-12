/* @(#)99	1.11  src/bos/kernel/sys/lockl.h, sysproc, bos411, 9428A410j 11/3/93 16:23:11 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#ifndef _H_LOCKL
#define _H_LOCKL

#include <sys/lock_def.h>

/*
 *  Global kernel locks.
 *
 *  These lock words are pinned.  Other lock words, imbedded in
 *  various kernel structures, need not be.
 *
 *  Locks are defined here in resource order.  The rule is that
 *  lower-order locks must NOT be acquired when a higher-order lock is
 *  already owned.  Following this rule prevents deadlocks.  Declaring
 *  the lock words in ascending resource order allows DEBUG code to
 *  check that the rules are actually being followed.
 */

/*
 *  Since it is not normally held across waits, the kernel non-premption
 *  lock is exempted from the resource ordering rule.
 */
extern lock_t kernel_lock;	/* kernel mode non-preemption lock */


#endif /* _H_LOCKL */
