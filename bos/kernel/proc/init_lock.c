static char sccsid[] = "@(#)82	1.8  src/bos/kernel/proc/init_lock.c, sysproc, bos41B, 412_41B_sync 12/1/94 10:42:26";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: init_lock_instr_branch
 *		init_lock_instr_overlay
 *		init_locks
 *
 *   ORIGINS: 27 83
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * NOTES:
 */

#include <sys/param.h>
#include <sys/atomic_op.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/init_lock.h>
#include <sys/lockname.h>
#include <sys/sleep.h>
#include <sys/timer.h>
#include <sys/m_intr.h>
#include <sys/syspest.h>
#include <sys/systemcfg.h>
#include <sys/syspest.h>
#include <sys/proc.h>

extern unsigned int maxspin;

#ifdef _POWER_MP
#define BA_MASK 0x48000002              /* branch absolute opcode */

/* low memory variable; if set instrumentation is to be turned on */
int instr_avail = 0;  /* set via bosboot/mkboot in dbg_avail (see hardinit.c) */

void init_lock_instr_overlay();
void init_lock_instr_branch();

/* pinned and pageable pools control block */
extern struct	lock_pool	pinned_cb;
extern struct	lock_pool	pageable_cb;

/* lock on family lock statistics table */
extern Simple_lock	family_table_lock;

/* instrumented lockl data area */
extern struct lock_data_instrumented lockl_hstat[LOCKLHS];

#endif /* _POWER_MP */

/*
 * NAME:    init_locks 
 *
 * FUNCTION: Initiliaze lock hashed wait queues and if 
 *	     instrumentation is turned on allocate all lock tables, 
 *	     initialize the pinned and the paged lock pools, and
 *	     allocate the locks needed to serialize the accesses to
 *	     the tables and the pools.
 *           The klockd daemon is started.
 *          
 *
 * NOTES:    This function should be invoked during the initialization
 *	     phase, before any lock is used.
 *         
 */

void
init_locks()

{
	register int i;
	register int ipri;
	struct lock_data_instrumented *l;

#ifdef _POWER_MP
	/* runtime optimization--no need to check on non-_POWER_MP kernels */
	if (__power_mp()) {
		maxspin = MAXSPIN_MP;
	}
	else {
		maxspin = MAXSPIN_UP;
	}
#endif /* _POWER_MP */

	/* initialize lock hash queue */
        for (i = 0; i < LOCKHSQ; i++)
                lockhsque[i] = EVENT_NULL;

	/* initialize lockl hash queue */
        for (i = 0; i < LOCKLHSQ; i++)
                locklhsque[i] = EVENT_NULL;

#if defined(_POWER_MP) && defined(_POWER_PC)
	if ((instr_avail) && __power_pc()) {

		/* pin lockl_hstat structure */
		ltpin (&lockl_hstat, sizeof(lockl_hstat));

		/* initialize family lock statistics table */
		ltpin (family_lock_statistics,FAMILY_LOCK_SIZE);
		for (i=0; i < MAX_FAMILY; i++) {
			family_lock_statistics[i].acquisitions = 0;
			family_lock_statistics[i].misses = 0;
			family_lock_statistics[i].sleeps = 0;
		}	
		
		/* initialize lock_pinned pool */
		ltpin (lock_pinned, POOL_INITIAL_SIZE);
		for( l = &(lock_pinned[0]); l < &(lock_pinned[INITIAL_LOCK_NUMBER]); l++) {
			l->lo_next = l + 1;
			l->lockname = NULL;
#ifdef DEBUG
			l->dbg_zero = 0;
			l->dbg_flags = 0;
#endif
		}
		pinned_cb.start = l;
		(--l)->lo_next = NULL;
		pinned_cb.free_list = lock_pinned;
		pinned_cb.end_list = l;
		pinned_cb.allocated = INITIAL_LOCK_NUMBER;
		pinned_cb.free = INITIAL_LOCK_NUMBER;
		
		/* initialize lock_pageable pool */
		for( l = &(lock_pageable[0]); l < &(lock_pageable[INITIAL_LOCK_NUMBER]); l++) {
			l->lo_next = l + 1;
			l->lockname = NULL;
#ifdef DEBUG
			l->dbg_zero = 0;
			l->dbg_flags = 0;
#endif
		}
                pageable_cb.start = l;
                (--l)->lo_next = NULL;
                pageable_cb.free_list = lock_pageable;
                pageable_cb.end_list = l;
                pageable_cb.allocated = INITIAL_LOCK_NUMBER;
                pageable_cb.free = INITIAL_LOCK_NUMBER;
		
		/* initialize instrumented lock primitive overlays */
		ipri = i_disable(INTMAX);
		init_lock_instr_overlay();
		
		/* initialize instrumented lock primitive branch */
		init_lock_instr_branch();
		i_enable(ipri);
		
		/* alloc lock for table management */
		/* since lock_alloc requires the lock on the control block 
		 * allocate the lock for the pinned pool control block by hand
		 */
		
		pinned_cb.pool_lock._slockp = pinned_cb.free_list;
                pinned_cb.free_list = ((struct lock_data_instrumented *)pinned_cb.pool_lock._slockp)->lo_next;
                pinned_cb.free--;
                bzero(pinned_cb.pool_lock,sizeof(struct lock_data_instrumented));
#ifdef DEBUG
		pinned_cb.pool_lock._slockp->dbg_flags |= LOCK_IS_ALLOCATED;
#endif
		pinned_cb.pool_lock._slockp->lock_id = LOCK_TABLE_CLASS;
		pinned_cb.pool_lock._slockp->_occurrence = 1;
		
		lock_alloc((void *)&pageable_cb.pool_lock,LOCK_ALLOC_PIN,LOCK_TABLE_CLASS,2);
		lock_alloc((void *)&family_table_lock,LOCK_ALLOC_PIN,LOCK_TABLE_CLASS,3);
	}
#endif /* _POWER_MP */
}

#ifdef _POWER_MP
/*
 * NAME: init_lock_instr_overlay
 *
 * FUNCTION:
 *      This routine initializes lock overlay section.
 * Each entry in the table consists of the external function pointer
 * followed by is address and size.  A text address for each
 * possible implementation follows.  
 *
 * EXECUTION ENVIRONMENT:
 *      This function is called early in system initialization.
 * Interrupts are disabled and xlate is off
 *
 * RETURNS:
 *      None
 */
void
init_lock_instr_overlay()
{
        int i;
        int *funct_addr;        /* address of first instruction of function */

#if defined(_POWER_MP) && defined(_POWER_PC)
	if ((instr_avail) && __power_pc()) {
		for (i = 0; i < LOCK_OVERLAY_NUMBER; i++)
		{
			/* determine the machine dependent function to use based
			 * on the machine type we are running on
			 */
			funct_addr = lock_overlay_data[i].addr_instr;

			/* If null there is no overlay for this implementation
			 */
			if (funct_addr == NULL)
				continue;

			/* copy the overlay to its correct address.
			 */
			si_bcopy(funct_addr, lock_overlay_data[i].ov_addr,
					lock_overlay_data[i].size);
			si_cflush(lock_overlay_data[i].ov_addr, lock_overlay_data[i].size);
		}
        }
#endif /* _POWER_MP && _POWER_PC */
}


/*
 * NAME:     init_lock_instr_branch
 *
 * FUNCTION:
 *      This routine initializes the memory branch table for instrumented
 * functions.  Each entry in the table consists of a external
 * function name followed by instrumented implementation of the
 * function.  This routine puts a branch absolute pointing to the 
 * instrumented function into the branch table.  The function
 * descriptor for the external name is also patched to point to the
 * instrumented function.  This allows indirect call to go directly
 * to the correct function.
 *
 * NOTES:
 *      This function manipulates function descriptors.  Function descriptors
 * are what "C" function addresses resolve to.  This code manipulate the
 * first two words were the first word contains the actual address of the
 * function and the second contains a run-time TOC value.
 *
 *
 * EXECUTION ENVIRONMENT:
 *      This function is called from hardinit().  It runs once during
 * system initialization.  init_config() must have already executed.
 * This function runs with ALL interrupts disabled and in real mode.
 * There are no system interrupt handlers established yet, so even
 * something as simple as an alignment int is not recoverable.
 *          
 * RETURNS:	None
 *
 */

void
init_lock_instr_branch()
{
        int i;
        int *ba_addr;           /* address to place the BA instruction */
        int (*funct_ptr)();     /* machine dependent function implementation */
        int funct_addr;         /* address of first instruction of function */

#if defined(_POWER_MP) && defined(_POWER_PC)
	if ((instr_avail) && __power_pc()) {
		for (i = 0; i < LOCK_PRIM_NUMBER; i++)
		{
			funct_ptr = lock_branch_data[i].instr_name;

			/* Get the address where the branch absolute instruction
			 * must go.  This is contained in the fist word of the
			 * function descriptor
			 */
			ba_addr = (int *)*((int *)lock_branch_data[i].ext_name);

			/* get the machine dependent function address.  This address
			 * is in the first word of the function descriptor
			 */
			funct_addr = *((int *)funct_ptr);

			/* store a branch absolute into branch table.  Also patch
			 * the function descriptor to point directly to the machine
			 * dependent function
			 */
			*ba_addr = funct_addr | BA_MASK;
			*(int *)lock_branch_data[i].ext_name = funct_addr;

			/* code has been modified so do a cache flush.  A special
			 * cache flush routine is used as the cache flush services
			 * are in the branch table
			 */
			si_cflush(ba_addr, sizeof(int));
		}
	}
#endif /* _POWER_MP && _POWER_PC */
}
#endif /* _POWER_MP */
