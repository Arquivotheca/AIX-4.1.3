static char sccsid[] = "@(#)67	1.3  src/bos/kernel/ios/POWER/intr_pwr.c, sysios, bos411, 9428A410j 4/21/94 18:08:06";
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: 
 *	i_slih,		i_mask_pwr,	i_unmask_pwr,
 *	i_reset_pwr,	i_genplvl_rs1,	i_genplvl_rs2,
 *	i_enableplvl_rs1,	i_enableplvl_rs2,
 * 	i_disableplvl_rs1,	i_disableplvl_rs2,
 *	i_loginterr_pwr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * The intr.c, intr_pwr.c, and intr_ppc.c files contain the code that
 * manages interrupts.  The reason for having multiple files is to split
 * the machine independent function out into intr.c and the machine
 * dependent function out into intr_{pwr|ppc}.c.
 *
 * Functions in intr_pwr.c and intr_ppc.c will generally be accessed 
 * through the low memory branch table.  There are some exceptions, like
 * i_slih().  Because these exception are only called from one place they
 * can be accessed directly.
 */                                                                   

#ifdef _POWER_RS

#include <sys/types.h>
#include <sys/syspest.h>
#include <sys/mstsave.h>
#include <sys/low.h>
#include <sys/intr.h>
#include <sys/trchkid.h>		/* trace hook definitions */
#include <sys/adspace.h>
#include <sys/errids.h>
#include "interrupt.h"
#include "intr_hw.h"

extern	struct i_data	i_data;		/* interrupt handler data struct*/

struct intrerr_pwr {			/* error log description */
        struct  err_rec0        ierr;
                ulong           bid;
                ulong           level;
                ulong           eis0;
		ulong		eis1;
} intr_log_pwr ={ ERRID_INTR_ERR,"SYSINTR", 0, 0, 0, 0 };

#ifdef _SLICER
/*
 * This is the eim_save_area for Slicer Interrupt funelling.
 * The master and non-master CPU eim masks are stored here.
 * The master CPU eim masks need to be updated when i_enableplvl_rs1
 * or i_disableplvl_rs1 are called so as to not lose data if
 * they are called from non-MASTER cpus
 */
extern struct {
	struct imask eim[NUM_INTR_PRIORITY];
} eim_save_area[];
#endif /* _SLICER */

#if defined(_POWER_RS1) || defined(_POWER_RSC)
/*
 * NAME:  i_slih
 *
 * FUNCTION:  Determine the interrupt priority for this external interrupt.
 *
 * ACCESSED:  By name from RS1 ex_flih
 *
 * EXECUTION ENVIRONMENT:
 *      This routine runs on an interrupt level.
 *
 *      It does not page fault.
 *
 * NOTES: 
 *	This routine is only called by the RS1/RSC assembler flih for external
 *	interrupts and it runs with all interrupts disabled (INTMAX).
 *
 * RETURN VALUE DESCRIPTION: The most favored pending processor interrupt
 *	level.  If no interrupt is pending a -1 is returned.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	bitindex
 *	LVL_TO_MASK
 */
struct intr *
i_slih(
	ulong eis0,				/* interrupt source 0 */
	ulong eis1,				/* interrupt source 1 */
	ulong eim0,				/* interrupt mask 0 */
	ulong eim1 )				/* interrupt mask 1 */
{
	ulong			eis_ints[2];	/* enabled pending interrpts*/
	struct imask		mask;		/* mask for a level */
	register int		intpri;		/* interrupt priority */
	register int		new_priority;	/* interrupt priority */
	register int		int_level;	/* interrupt level */
	register int		new_level;	/* interrupt level */
	extern	 int		bitindex();	/* get bit number in word */


	/*
	 * Isolate enabled interrupts that are pending.
	 */
	eis_ints[0] = eis0 & eim0;
	eis_ints[1] = eis1 & eim1;

	if ((eis_ints[0] == 0) && (eis_ints[1] == 0))
		return((struct intr *)NULL);    /* quit if phantom interrupt */

	/*
	 * Find the most favored enabled interrupt that is pending.
	 * This is done by scanning the eis from left to right.
	 * Each enabled pending interrupt has its priority checked
	 * and the most favored interrupt is picked.
	 *
	 * The loop is unrolled such that the body will not execute if
	 * only one enabled interrupt is pending -- presumably a common
	 * occurrence.
	 */
	int_level = bitindex( eis_ints );
	ASSERT(int_level < LVL_PER_SYS);
	ASSERT( i_data.i_poll[int_level].poll != (struct intr *)NULL );
	intpri = i_data.i_poll[int_level].poll->priority;
	LVL_TO_MASK(mask, int_level);
	eis_ints[0] &= ~mask.eim0;
	eis_ints[1] &= ~mask.eim1;

	while ( (eis_ints[0] != 0) || (eis_ints[1] != 0) )
	{
		/*
		 * If here, more than one enabled interrupt is pending.
		 */
		new_level = bitindex( eis_ints );
		ASSERT(new_level < LVL_PER_SYS);
		ASSERT( i_data.i_poll[int_level].poll != (struct intr *)NULL );
		new_priority = i_data.i_poll[new_level].poll->priority;
		LVL_TO_MASK(mask, new_level);
		eis_ints[0] &= ~mask.eim0;
		eis_ints[1] &= ~mask.eim1;

		/*
		 * Update most favored interrupt found if necessary.
		 */
		if ( new_priority < intpri ) { 
			intpri = new_priority;
			int_level = new_level;
		}
	}

	return( i_data.i_poll[int_level].poll );
}
#endif /* _POWER_RS1 || _POWER_RSC */

/*
 * NAME:  i_mask_pwr
 *
 * FUNCTION:  Disable an interrupt source kernel service.
 *
 * ACCESSED:  Branch table entry i_mask
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by either a process or an
 *      interrupt handler.
 *
 *      It can page fault if called from a process on a pageable stack.
 *
 * NOTES:  
 *	The intr structure must be pinned and must be the one passed to
 *	i_init. You should only be masking interrupts that are allocated
 *	to you.
 *
 *	This code will disable a given I/O bus level for all interrupt
 *	priorities.
 *
 *	The bus_type must BUS_MICRO_CHANNEL to mask the interrupt source.
 *
 *	An extra interrupt can occur if the EIM was used to disable the
 *	bus interrupt level.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	io_att
 *	io_det
 *	i_disable
 *	i_enable
 */
void
i_mask_pwr(
	struct intr *handler )		/* int handler whose lvl to mask */
{
	register int	ipri;		/* input interrupt priority  */

	volatile struct iocc *ioptr;

	/*
	 * This service only handles bus interrupt levels.
	 */
	if( handler->bus_type == BUS_MICRO_CHANNEL ) {
	    /*
	     * Mask the bus interrupt level.
	     */
	    ioptr = (volatile struct iocc *)io_att( BUS_TO_IOCC(handler->bid),
							IO_IOCC );

	    ipri = i_disable( INTMAX );	/* enter critical section */
	    ioptr->int_enable &= ~IOCC_IER_MSK( handler->level );

	    i_enable( ipri );		/* end critical section  */
	    io_det( ioptr );
	}
}

/*
 * NAME:  i_unmask_pwr
 *
 * FUNCTION:  Enable an interrupt source kernel service.
 *
 * ACCESSED:  Branch table entry i_unmask
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by either a process or an
 *      interrupt handler.
 *
 *      It can page fault if called from a process on a pageable stack.
 *
 * NOTES:
 *	The intr structure must be pinned and must be the one passed to
 *	i_init. You should only be unmasking interrupts that are allocated
 *	to you.
 *
 *	This will enable an I/O bus level for all I/O priorites that
 *	are strictly less favored than it.
 *
 *	The bus_type must be BUS_MICRO_CHANNEL to unmask the interrupt source.
 *
 *	The IOCC's interrupt mask register is between the bus and the IOCC's
 *	interrupt request register. Therefore, masked interrupts do not cause
 *	a bit to be set in the interrupt request register. A pending interrupt
 *	will be detected by the IOCC when the interrupt level is unmasked.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_disable
 *	i_enable
 */

void
i_unmask_pwr(
	struct intr *handler)		/* int handler whose lvl to enable */
{

	register int	ipri;		/* input interrupt priority  */
	int	plvl;
	union {

		uint	vmask;
		uchar	vbytes[4];
	} vector;

	volatile struct iocc *ioptr;
	/*
	 * This service only handles bus interrupt levels.
	 */
	if( handler->bus_type == BUS_MICRO_CHANNEL ) {
		/*
		 * Unmask the bus interrupt level.
		 */
	    ioptr = (volatile struct iocc *)io_att( BUS_TO_IOCC(handler->bid),
							IO_IOCC );

	    ipri = i_disable( INTMAX );	/* enter critical section */

#ifdef _POWER_RS2
	    /*
	     * The iocc interrupt vector must be programed before the
	     * interrupt level is unmasked.  RS1 and RSC use static
	     * vecotors set up in initiocc()
	     */
	    if( __power_rs2() )
	    {
	        plvl = i_genplvl( handler );
	    	vector.vmask = ioptr->vector[handler->level / sizeof(int)];
	        vector.vbytes[handler->level % sizeof(int)] = plvl;
	        ioptr->vector[handler->level / sizeof(int)] = vector.vmask;
	    }
#endif /* _POWER_RS2 */

	    ioptr->int_enable |= IOCC_IER_MSK( handler-> level );

	    i_enable( ipri );		/* end critical section   */
	    io_det( ioptr );
	}
}

/*
 * NAME:  i_reset_pwr
 *
 * FUNCTION:  Reset a bus interrupt level
 *
 * ACCESSED:  Branch table entry i_reset_int
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by either a process or an
 *      interrupt handler.
 *
 *      It can page fault if called from a process on a pageable stack.
 *
 * NOTES:  
 *	This service should only be called from within the interrupt
 *	subsystem via the function name i_reset_int in the branch table.
 *
 *	The intr structure must be pinned and must be the one passed to
 *	i_init. You should only be resetting interrupts that are allocated
 *	to you.
 *
 *	The bus_type must be BUS_MICRO_CHANNEL to reset the interrupt source.
 *
 * Micro Channel interrupts are level sensitive. The device reports
 * an interrupt to the system by activating its IRQ line on the bus
 * and keeps the line activated until it is reset at the device.
 * 
 * The IOCC presents an interrupt to the system whenever an IRQ line
 * is active and the corresponding interrupt request register bit
 * is 0.
 * 
 * Therefore, the correct sequence for processing interrupts from
 * Micro Channel devices is to reset the interrupt at the device
 * and then reset the interrupt at the IOCC. An extra interrupt
 * will be presented to the system if the IOCC is reset prior to
 * the device.
 *
 * As a result of resetting the interrupt at the IOCC ( EOI ), we
 * could potentially allow an interrupt that has already been presented
 * to the EIS to be re-presented. This will generate an extra interrupt
 * for that * level. To correct this, the EIS bit for the given interrupt
 * level is cleared in the RESET_BUS macro.
 *
 * The kernel maintains a list of interrupt handlers for each
 * interrupt source/level. The kernel calls these interrupt
 * handlers until one of them returns indicating that it processed
 * the interrupt. Subsequent interrupt handlers are not called,
 * since the probability of simultaneous interrupts is small.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	io_att
 *	io_det
 *	i_genplvl
 *	i_eis_reset or i_peis_reset
 *	disable_ints
 *	enable_ints
 */
void
i_reset_pwr(
	struct intr *handler)		/* int handler whose lvl to reset */
{
	register int	ipri;		/* old interrupt priority */
	register int	plvl;		/* processor level */
	register long	trash;

	volatile struct iocc *ioptr;

	/* This service only handles bus interrupt levels.  */
	if( handler->bus_type == BUS_MICRO_CHANNEL ) {

	    /* Reset the bus interrupt level.  */

	    ipri = disable_ints();		/* enter critical section */

	    ioptr = (volatile struct iocc *)io_att( BUS_TO_IOCC(handler->bid),
							 IO_IOCC);
	    plvl = i_genplvl( handler );
	    ASSERT( (plvl >= 0) && (plvl < LVL_PER_SYS) );
#if defined(_POWER_RS1) || defined(_POWER_RSC)
	    if( __power_set( POWER_RS1 | POWER_RSC )) {
		i_eis_reset( plvl );
	    }
#endif /* _POWER_RS1 || _POWER_RSC */

#ifdef _POWER_RS2
	    if( __power_rs2() ) {
		i_peis_reset( plvl );
	    }
#endif /* _POWER_RS2 */
	    trash = ioptr->rfi;
	    io_det( ioptr );

	    enable_ints(ipri);		/* end critical section */
	}
}

#if defined(_POWER_RS1) || defined(_POWER_RSC)
/*
 * NAME:  i_genplvl_rs1
 *
 * FUNCTION:  Generate processor interrupt level value
 *
 * ACCESSED:  Branch table entry i_genplvl
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by either a process or an
 *      interrupt handler.
 *
 *      It can page fault if called from a process on a pageable stack.
 *
 * NOTES:  
 *	The common code does not require a one to one relationship between
 *	bus interrupt levels and external interrupt levels (sources). The
 *	i_genplvl function is used to convert a bus interrupt level into an
 *	processor interrupt level. The bid parameter provides future support
 *	for multiple I/O buses.
 *
 *	Since this is the RS1 version of this function the algorithm is
 *	fairly simple.  If the source is on a micro channel(IOCC), there
 *	is a direct relationship between the bus level of the micro
 *	channel and the processor interrupt level.  IOCC 0 is allocated
 *	processor levels 0-15 while IOCC 1 is allocated processor levels
 *	32-47.  If the interrupt source is not on a micro channel then
 *	the bus level specified in the intr structure is assumed to be
 *	the processor level as well.
 *
 *
 * RETURN VALUE DESCRIPTION:
 *	processor interrupt level for the given interrupt handler
 *
 * EXTERNAL PROCEDURES CALLED:
 *	GET_IOCC_NUM
 */
int
i_genplvl_rs1(
	struct intr *handler)		/* int handler that is seek a plvl */
{
	register int	plvl;		/* processor interrupt level */

	assert( handler->bus_type != BUS_60X );

	if( handler->bus_type == BUS_MICRO_CHANNEL ) {
		assert( (uint)(handler->level) < (uint)NUM_BUS_SOURCE );
		plvl = (GET_IOCC_NUM(handler->bid) * LVL_PER_WORD) +
						handler->level;
	}
	else if( handler->bus_type == BUS_NONE ) {
		/*
		 * For binary compatibility we will have to jump
		 * through a hoop here.
		 *
		 * drivers will be registering an EPOW handler
		 * with a bus_type of BUS_NONE and priority of 
		 * INTEPOW(INTMAX).  So map them to the correct
		 * processor level for this processor.
		 */
		if( handler->priority == INTEPOW ) {
			handler->level = INT_EPOW_RS1;
		}

		plvl = handler->level;
	}
	else if( handler->bus_type == BUS_PLANAR ) {
		plvl = handler->level;
	}
	else {
		/* unknown bus_type */
		assert( handler->bus_type == BUS_NONE );
	}

	return( plvl );
}
#endif /* _POWER_RS1 || _POWER_RSC */

#ifdef _POWER_RS2
/*
 * NAME:  i_genplvl_rs2
 *
 * FUNCTION:  Generate processor interrupt level value
 *
 * ACCESSED:  Branch table entry i_genplvl
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by either a process or an
 *      interrupt handler.
 *
 *      It can page fault if called from a process on a pageable stack.
 *
 * NOTES:  
 *	The common code does not require a one to one relationship between
 *	bus interrupt levels and external interrupt levels (sources). The
 *	i_genplvl function is used to convert a bus interrupt level into an
 *	processor interrupt level. The bid parameter provides future support
 *	for multiple I/O buses.
 *
 *	This is the RS2 version of this function, the algorithm is the
 *	most complex.  The 64 processor interrupt levels have been
 *	broken up into a set of predefined levels and a set of 4 dynamically
 *	maintained pools that correspond to the 4 device interrupt classes.
 *
 *	When a request to i_genplvl is made and the source is check against
 *	the set of predefined or reserved levels.  NOTE: That there may be
 *	reserved levels even in the dynamically maintained pools.
 *	Once it is determined that this is not a predefined level the
 *	priority of the handler is used to scan the pool associated with
 *	that priority.  If it is found that this bus level already occupies
 *	a level that level is then returned, otherwise, the next available
 *	level in the pool is returned.
 *
 * RETURN VALUE DESCRIPTION:
 *	processor interrupt level for the given interrupt handler
 *
 * EXTERNAL PROCEDURES CALLED:
 *	GET_IOCC_NUM
 */
int
i_genplvl_rs2(
	struct intr	*handler)	/* int handler that is seek a plvl */
{
	register int	slvl;		/* starting level for scan	*/
	register int	blvl;		/* bottom level to end scan	*/
	register int	clvl;		/* currentl level of scan	*/
	register int	dir;		/* direction of scan		*/
	register int	plvl;		/* first empty level or matching*/
					/* bid & level			*/
	register int	hpri;		/* handler priority		*/
	register struct intr *cpoll;	/* ptr to clvl intr structure	*/


	hpri = handler->priority;

	if( handler->bus_type != BUS_MICRO_CHANNEL ) {
		/*
		 * It must be BUS_NONE, BUS,PLANAR, or BUS_60X.
		 *
		 * For binary compatibility we will have to jump
		 * through a hoop here.
		 *
		 * drivers will be registering an EPOW handler
		 * with a bus_type of BUS_NONE and priority of 
		 * INTEPOW(INTMAX).  So map them to the correct
		 * processor level for this processor.
		 */
		if( handler->bus_type == BUS_NONE ) {
			if( hpri == INTEPOW ) {
				handler->level = INT_EPOW_RS2;
			}
		}
		else {
			assert( handler->bus_type == BUS_PLANAR || \
				handler->bus_type == BUS_60X );
		}

		return( handler->level );
	}

	switch( hpri ) {
	    case INTMAX:
		assert( handler->level == 0 );
		return( handler->level );

	    case INTCLASS0:
		slvl = STARTCLASS0;
		blvl = STARTCLASS1;
		dir = 1;
		break;

	    case INTCLASS1:
		slvl = STARTCLASS1;
		blvl = STARTCLASS0;
		dir = -1;
		break;

	    case INTCLASS2:
		slvl = STARTCLASS2;
		blvl = STARTCLASS3;
		dir = 1;
		break;

	    case INTCLASS3:
		slvl = STARTCLASS3;
		blvl = STARTCLASS2;
		dir = -1;
		break;

	    default:
		/* die if not device interrupt CLASS0-3 or INTMAX */
		assert( 0 );
	}

	/*
	 * Scan all the handlers in a class for a matching bid & level. 
	 * If a match is found return that level else return the first
	 * available plvl in class.
	 */
	for( plvl = 0, clvl = slvl; clvl != blvl; clvl += dir )
	{
	    /*
	     * If this plvl is empty remember it and advance
	     * to the next plvl
	     */
	    if( i_data.i_poll[clvl].poll == (struct intr *)NULL ) {
		if( !plvl )
			plvl = clvl;
		continue;
	    }

	    /* Step over the dummy intr structure */
	    cpoll = i_data.i_poll[clvl].poll->next;
	    /*
	     * If the priority changes we have backed into the other class
	     * in the pool.  Stop scanning.
	     */
	    if( cpoll->priority != hpri ) {
		break;
	    }

	    /*
	     * If bid and bus level match then this processor
	     * level must be sharing this bus level.
	     */
	    if( GET_IOCC_NUM(cpoll->bid) == GET_IOCC_NUM(handler->bid)	&&
		(cpoll->level) == (handler->level) )
	    {
		plvl = clvl;
		break;
	    }

	}

	/*
	 * if plvl is 0 then the handler was not found or there
	 * were no available entries in the pool.
	 */
	assert( plvl );
	
	return( plvl );
}
#endif /* _POWER_RS2 */

#if defined(_POWER_RS1) || defined(_POWER_RSC)
/*
 * NAME:  i_enableplvl_rs1
 *
 * FUNCTION:  Enable a processor interrupt level
 *
 * ACCESSED:  Branch table entry i_enableplvl
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by a process only
 *
 * NOTES:  
 *	Adjusts the priority mapping array, EIM values, for the
 *	given priority and all less favored priorities.  Then
 *	unmasks or enables the bus level if needed.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_unmask
 *	LVL_TO_MASK
 */
void
i_enableplvl_rs1(
	struct intr *handler,		/* int handler that caused this */
	int	plvl)			/* processor level to enable */
{
	struct imask	mask;		/* eim mask */
	register int	i;

	/*
	 * Enable the interrupt level
	 */
	LVL_TO_MASK(mask, plvl);
	for ( i = handler->priority + 1; i < NUM_INTR_PRIORITY; i++ )
	{
#ifndef _SLICER
		i_data.i_pri_map.eim[i].eim0 |= mask.eim0;
		i_data.i_pri_map.eim[i].eim1 |= mask.eim1;
#else /* _SLICER */
		/* update the master CPU eim mask only */
		eim_save_area[MP_MASTER].eim[i].eim0 |= mask.eim0;
		eim_save_area[MP_MASTER].eim[i].eim1 |= mask.eim1;
		if (CPUID == MP_MASTER) {
			i_data.i_pri_map.eim[i].eim0 |= mask.eim0;
			i_data.i_pri_map.eim[i].eim1 |= mask.eim1;
		}
#endif /* _SLICER */
	}
	if ( handler->bus_type == BUS_MICRO_CHANNEL )
		i_unmask( handler );
}
#endif /* _POWER_RS1 || _POWER_RSC */

#ifdef _POWER_RS2
/*
 * NAME:  i_enableplvl_rs2
 *
 * FUNCTION:  Enable a processor interrupt level
 *
 * ACCESSED:  Branch table entry i_enableplvl
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by a process only
 *
 * NOTES:  
 *	Adjusts the priority mapping array, CIL values, for the
 *	given priority.  The CIL array is initialized so that only
 *	INTCLASS1 & 3 need any adjusting.  The bus level is then unmasked
 *	or enabled, if needed.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_unmask
 */
void
i_enableplvl_rs2(
	struct intr *handler,		/* int handler that caused this */
	int	plvl)			/* processor level to enable */
{
	register int	pri;

	pri = handler->priority;
	switch( pri ) {

	    case INTCLASS1:
	    case INTCLASS3:
		if( plvl < i_data.i_pri_map.cil[pri] ) {
			i_data.i_pri_map.cil[pri] = plvl;
		}
		break;
	    case INTCLASS0:
	    case INTCLASS2:
	    default:
		break;
	}
	if ( handler->bus_type == BUS_MICRO_CHANNEL )
		i_unmask( handler );
}
#endif /* _POWER_RS2 */

#if defined(_POWER_RS1) || defined(_POWER_RSC)
/*
 * NAME:  i_disableplvl_rs1
 *
 * FUNCTION:  Disable a processor interrupt level
 *
 * ACCESSED:  Branch table entry i_disableplvl
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by a process only
 *
 * NOTES:  
 *	Adjusts the priority mapping array, EIM values, for the
 *	given priority and all less favored priorities.  Then
 *	masks and resets bus level if needed.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_mask
 *	i_reset_int
 *	LVL_TO_MASK
 */
void
i_disableplvl_rs1(
	struct intr *handler,		/* int handler that caused this */
	int	plvl)			/* processor level to enable */
{
	struct imask	mask;		/* eim mask */
	register int	i;

	/*
	 * mask and reset the bus level.  No need to check for bus_type
	 * since these functions will do that.
	 */
	i_mask( handler );
	i_reset_int( handler );

	/*
	 * Disable the interrupt level
	 */
	LVL_TO_MASK(mask, plvl);
	for ( i = handler->priority + 1; i < NUM_INTR_PRIORITY; i++ )
	{
#ifndef _SLICER
		i_data.i_pri_map.eim[i].eim0 &= ~mask.eim0;
		i_data.i_pri_map.eim[i].eim1 &= ~mask.eim1;
#else /* _SLICER */
		/* update the master CPU eim mask only */
		eim_save_area[MP_MASTER].eim[i].eim0 &= ~mask.eim0;
		eim_save_area[MP_MASTER].eim[i].eim1 &= ~mask.eim1;
		if (CPUID == MP_MASTER) {
			i_data.i_pri_map.eim[i].eim0 &= ~mask.eim0;
			i_data.i_pri_map.eim[i].eim1 &= ~mask.eim1;
		}
#endif /* _SLICER */
	}
}
#endif /* _POWER_RS1 || _POWER_RSC */

#ifdef _POWER_RS2
/*
 * NAME:  i_disableplvl_rs2
 *
 * FUNCTION:  Disable a processor interrupt level
 *
 * ACCESSED:  Branch table entry i_disableplvl
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by a process only
 *
 * NOTES:  
 *	Adjusts the priority mapping array, CIL values, for the
 *	given priority.  Mask and reset bus level if needed.
 *
 *	only INTCLASS1 & 3 need any adjusting
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_mask
 *	i_reset_int
 *	LVL_TO_MASK
 */
void
i_disableplvl_rs2(
	struct intr *handler,		/* int handler that caused this */
	int	plvl)			/* processor level to enable */
{
	register int	pri;		/* priority of this handler */
	/*
	 * mask and reset the bus level.  No need to check for bus_type
	 * since these functions will do that.
	 */
	i_mask( handler );
	i_reset_int( handler );

	/*
	 * change the CIL array value if needed
	 */
	pri = handler->priority;
	if( pri == INTCLASS1 || pri == INTCLASS3 ) {

		/*
		 * If we are disabling the most favored then we
		 * want to change the CIL value to next most favored
		 * in the pool.  That will generally be the next one
		 * but we must make sure.
		 */
		if( plvl == i_data.i_pri_map.cil[pri] ) {
		    for( plvl++;
			 i_data.i_poll[plvl].poll == (struct intr *)NULL;
			 plvl++ )
		    {

			if( plvl == BASECLASS1 )
			    break;

			if( plvl == BASECLASS3 )
			    break;
		    }
		    i_data.i_pri_map.cil[pri] = plvl;
		}
	}
}
#endif /* _POWER_RS2 */

/*
 * NAME:  i_loginterr_pwr
 *
 * FUNCTION:  Log a phantom interrupt
 *
 * ACCESSED:  Branch table entry i_loginterr
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by i_poll only
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	errsave
 */
void
i_loginterr_pwr(
	struct intr *handler)		/* int handler that caused this */
{
	ulong			eis0, eis1;	/* eis0, eis1      */
	register int		plvl;		/* interrupt level */
	struct imask		mask;

	if ( handler->bus_type == BUS_MICRO_CHANNEL ) {

                i_reset_int( handler );

#if defined(_POWER_RS1) || defined(_POWER_RSC)
		if( __power_set( POWER_RS1 | POWER_RSC )) {
			mfeis( &eis0, &eis1 );
		}
#endif /* _POWER_RS1 || _POWER_RSC */

#ifdef _POWER_RS2
		if( __power_rs2() ) {
			mfpeis( &eis0, &eis1 );
		}
#endif /* _POWER_RS2 */

                /*
		 * Normalize the interrupt level.
		 */
		assert( (uint)(handler->level) < (uint)NUM_BUS_SOURCE );
		plvl = i_genplvl( handler );
		LVL_TO_MASK(mask, plvl);

                if ( (eis0 & mask.eim0) || (eis1 & mask.eim1) ) {

			intr_log_pwr.bid = handler->bid;
			intr_log_pwr.level = handler->level;
			intr_log_pwr.eis0 = eis0;
			intr_log_pwr.eis1 = eis1;
			errsave(&intr_log_pwr, sizeof(intr_log_pwr));
		}
	}
}
#endif /* _POWER_RS */
