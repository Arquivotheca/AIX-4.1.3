/* @(#)24	1.3  src/bos/usr/ccs/bin/ld/bind/stats.h, cmdld, bos411, 9428A410j 1/28/94 11:33:42 */
#ifndef Binder_STATS
#define Binder_STATS
/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifdef STATS

#define STAT_use(ID, count)	Bind_state.memory[ID].used += count

#define STAT_allocations(ID, size)	(Bind_state.memory[ID].allocations++, \
					 Bind_state.memory[ID].alloc += size)

/* Compute maximum allocations when freeing, because freeing almost never
   happens. */
#define	STAT_free(ID,n) \
	(Bind_state.memory[ID].maxused \
	 = max(Bind_state.memory[ID].used,Bind_state.memory[ID].maxused),\
	 Bind_state.memory[ID].used-=n)

#else

/* Null definitions if STATS not defined */
#define STAT_use(ID, count)
#define STAT_allocations(ID, size)
#define	STAT_free(ID,n)

#endif /* STATS */

#endif /* Binder_STATS */
