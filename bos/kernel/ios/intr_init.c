static char sccsid[] = "@(#)13        1.14  src/bos/kernel/ios/intr_init.c, sysios, bos41J, 9511A_all 3/7/95 13:13:10";
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: intr_init
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#include <sys/types.h>
#include <sys/intr.h>
#include <sys/syspest.h>
#include <sys/ppda.h>
#include <sys/malloc.h>
#include "interrupt.h"
#include "intr_hw.h"

extern struct i_data	i_data;
extern struct ppda	ppda[];
extern int		soft_int_model;
extern short		(*i_lvlmap)[MAX_LVL_PER_PRI];
#ifdef _RS6K
extern	int	next_poll_grp;
extern	ushort	buid_map[];
#endif /* _RS6K */
#ifdef _POWER_RS
extern struct intr	off_lvlhand;
#endif /* _POWER_RS */
#ifdef _POWER_MP
extern struct intr	intmax_lvlhand;
#endif /* _POWER_MP */

/*
 * NAME:  intr_init
 *
 * FUNCTION:  Initialize interrupt management data structures and define
 *	off-level interrupt handlers.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called during system initialization. It must
 *	be called before any interrupt handlers are defined or any
 *	interrupts taken.
 *
 * NOTES:
 *	This routine must be called before the first interrupt is taken.
 *
 *	This routine is split out of intr.c so that it can be separated
 *	from the rest of interrupt management and put with other system
 *	initialization routines. This removes it from the working set
 *	for normal operation.
 *
 *	This routine does not define all of the system interrupt handlers.
 *	It is expected that each component will define theirs. For example,
 *	it is expected that the timer component will define the timer
 *	interrupt handler and the block I/O component will define the
 *	I/O done off-level interrupt handler.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_init
 *	init_misc
 *	PPDA
 */

void
intr_init( )
{
	int	ipri;		/* interrupt priority */
	int	level;		/* interrupt level */
	int	index;		/* off-level array index */
	int	rc;		/* i_init return code */
	struct ppda *ppda_ptr;	/* ptr to ppda */

	extern	 int	i_softoff();	/* off-level interrupt handler */
#ifdef _POWER_MP
	extern int	i_hwassist_int();
	extern void	ipoll_excl_serv();
	extern mpc_msg_t	ipoll_mpc_msg;
#endif /* _POWER_MP */
	extern	 void	init_misc();	/* machine dependent initialization */

#ifdef _POWER_RS
	/*
	 * Initialize the external interrupt masks that control interrupt
	 * priority.
	 */
#if defined(_POWER_RS1) || defined(_POWER_RSC)
	if( __power_set( POWER_RS1 | POWER_RSC )) {
	    for ( ipri = INTMAX; ipri < NUM_INTR_PRIORITY; ipri++ )
	    {
		i_data.i_pri_map.eim[ipri].eim0 = 0;
		i_data.i_pri_map.eim[ipri].eim1 = 0;
	    }
	}
#endif /* _POWER_RS1 || _POWER_RSC */
#ifdef _POWER_RS2
	if( __power_rs2() ) {
		i_data.i_pri_map.cil[INTMAX] = PRI0_TO_LVL;
		i_data.i_pri_map.cil[INTCLASS0] = PRI1_TO_LVL;
		i_data.i_pri_map.cil[INTCLASS1] = PRI2_TO_LVL;
		i_data.i_pri_map.cil[INTCLASS2] = PRI3_TO_LVL;
		i_data.i_pri_map.cil[INTCLASS3] = PRI4_TO_LVL;
		i_data.i_pri_map.cil[INTTIMER] = PRI5_TO_LVL;
		i_data.i_pri_map.cil[INTOFFL0] = PRI6_TO_LVL;
		i_data.i_pri_map.cil[INTOFFL1] = PRI7_TO_LVL;
		i_data.i_pri_map.cil[INTOFFL2] = PRI8_TO_LVL;
		i_data.i_pri_map.cil[INTOFFL3] = PRI9_TO_LVL;
		i_data.i_pri_map.cil[INTIODONE] = PRI10_TO_LVL;
		i_data.i_pri_map.cil[INTBASE] = PRI11_TO_LVL;
	}
#endif /* _POWER_RS2 */
#endif /* _POWER_RS */

#ifdef _RS6K
        /*
	 * Initialize the buid_map array.
	 *
	 * next_poll_grp is basically an index that indicates
	 * the next group of 16 levels to be allocated out of the
	 * poll array.
	 */
	if( __rs6k() ) {
		for( ipri=1; ipri<=MAX_BUID; ipri++ ) {
			buid_map[ipri] = -1;
		}
		buid_map[0] = 0;
		next_poll_grp = 1;
	}
#endif /* _RS6K */

	/*
	 * Initialize the poll array such that there are no interrupt
	 * handlers defined.
	 */
	for ( level = 0; level < NUM_INTR_SOURCE; level++ )
	{
	  i_data.i_poll[level].poll = (struct intr *)NULL;
	}

	/* Set up i_lvlmap[][] memory, used by SW managed model */
	i_lvlmap = ( void * )xmalloc(
		sizeof(short) * (INTTIMER * MAX_LVL_PER_PRI), 2, pinned_heap);

	assert( i_lvlmap != NULL );

	for( rc=0; rc<INTTIMER; rc++ ) {
		for( index=0; index<MAX_LVL_PER_PRI; index++ ) {
			i_lvlmap[rc][index] = -1;
		}
	}

	/* No need for a off level handler in software managed model */
#ifdef _POWER_RS
	if( !soft_int_model && __power_rs() ) {
	  /* Now set up any off-level handler */
	  off_lvlhand.next = (struct intr *)NULL;
	  off_lvlhand.handler = i_softoff;
	  off_lvlhand.bus_type = BUS_PLANAR;
	  off_lvlhand.flags = INTR_NOT_SHARED | INTR_MPSAFE;
	  off_lvlhand.level = INT_OFFLVL;
	  off_lvlhand.priority = INTOFFLVL;
	  off_lvlhand.bid = 0;
	  rc = i_init(&off_lvlhand);
	  assert( rc == INTR_SUCC );
	}
#endif /* _POWER_RS */

#ifdef _POWER_MP
	if( __rs6k() ) {
	  intmax_lvlhand.next = (struct intr *)NULL;
	  intmax_lvlhand.handler = i_hwassist_int;
	  intmax_lvlhand.bus_type = BUS_PLANAR;
	  intmax_lvlhand.flags = INTR_NOT_SHARED | INTR_MPSAFE;
	  intmax_lvlhand.level = INT_MFRR;
	  intmax_lvlhand.priority = INTMAX;
	  intmax_lvlhand.bid = 0;
	  rc = i_init(&intmax_lvlhand);
	  assert( rc == INTR_SUCC );

	  if ( __power_mp() )
	    ipoll_mpc_msg = mpc_register(INTIODONE, ipoll_excl_serv);
	}
#endif

	/*
	 * Initialize the machine dependent system interrupt handlers.
	 */
	init_misc();

#if defined(_RS6K)
	if (__rs6k()) {
	    /*
	     * If we are on a box with a XIRR, write INTBASE out as the CPPR
	     * so that interrupts can occur.
	     *
	     */
	    volatile struct ppcint *intr;
	    intr = (volatile struct ppcint *)(PPDA->intr);
	    intr->i_cppr = INTBASE;
	    __iospace_sync();
	}
#endif

}
