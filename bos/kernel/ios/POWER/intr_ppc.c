static char sccsid[] = "@(#)68        1.12  src/bos/kernel/ios/POWER/intr_ppc.c, sysios, bos41J, 9517A_all 4/25/95 17:47:24";
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS:
 *	i_mask_ppc,	i_unmask_ppc,	i_reset_ppc,
 *	i_genplvl_ppc,	i_enableplvl_ppc, i_disableplvl_ppc
 *	i_loginterr_ppc
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1995
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

/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#ifdef _POWER_PC

#include <sys/types.h>
#include <sys/syspest.h>
#include <sys/mstsave.h>
#include <sys/low.h>
#include <sys/intr.h>
#include <sys/trchkid.h>		/* trace hook definitions */
#include <sys/adspace.h>
#include <sys/errids.h>
#include <sys/ppda.h>
#include "interrupt.h"
#include "intr_hw.h"
#include <sys/inline.h>

#ifdef _POWER_MP
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#endif /* _POWER_MP */

extern	lock_t	intsys_lock;
extern	struct i_data	i_data;		/* interrupt handler data struct*/
extern	struct ppda	ppda[];

struct	intrerr_ppc intr_log_ppc ={ ERRID_INTRPPC_ERR,"SYSINTR", 0, 0 };
/*
 * next_poll_grp is an index that indicates the next group of 16 levels
 * available in the poll_array.  It is incremented for each new BUID
 * as they pass through i_init() during interrupt handler registration.
 * It should never exceed MAX_BUC_POLL_GRP.
 */
int	next_poll_grp;			/* next available poll group	*/
short	buid_map[MAX_BUID+1];		/* BUID to poll array mapping	*/
Simple_lock	int_hw_lock;		/* XIVR/IER/IRR global lock 	*/

struct	intr	intr_soft_dummy = {0, 0, 0, 0 };

/*
 * NAME:  i_mask_ppc
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
i_mask_ppc(
	struct intr *handler )		/* int handler whose lvl to mask */
{
	register int	ipri;		/* input interrupt priority  */

	volatile struct iocc_ppc *ioptr;

	/*
	 * This service only handles micro channel bus interrupt levels.
	 */
	if( handler->bus_type == BUS_MICRO_CHANNEL ) {
		/*
		 * Mask the bus interrupt level.
		 */
		ioptr = (volatile struct iocc_ppc *)
			io_att( BID_TO_IOCC(handler->bid), IO_IOCC_PPC );

#ifdef _POWER_MP
		ipri = disable_lock( INTMAX, &int_hw_lock);
#else
		ipri = i_disable( INTMAX );	/* enter critical section */
#endif

		ioptr->int_enable &= ~IOCC_IER_MSK( handler->level );

#ifdef _POWER_MP
		unlock_enable( ipri, &int_hw_lock);
#else
		i_enable( ipri );		/* end critical section  */
#endif
		io_det( ioptr );
	}
}

/*
 * NAME:  i_unmask_ppc
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
i_unmask_ppc(
	struct intr *handler)		/* int handler whose lvl to enable */
{

	register int	ipri;		/* input interrupt priority  */
	int	plvl;
#ifdef _POWER_MP
	extern uint mproc_physid;
#endif

	volatile struct iocc_ppc *ioptr;

	/*
	 * This service only handles micro channel bus interrupt levels.
	 */
	if( handler->bus_type == BUS_MICRO_CHANNEL ) {
		/*
		 * Unmask the bus interrupt level.
		 */
		ioptr = (volatile struct iocc_ppc *)
			io_att( BID_TO_IOCC(handler->bid), IO_IOCC_PPC );

#ifdef _POWER_MP
		ipri = disable_lock( INTMAX, &int_hw_lock);
#else
		ipri = i_disable( INTMAX );	/* enter critical section */
#endif

		/*
		 * Setup XIVR to interrupt at correct priority
		 */
#ifndef _POWER_MP
		ioptr->xivr[handler->level] = 0xFF00 | handler->priority;
#else
		if ( handler->flags & INTR_MPSAFE )
			/* Any processor may service MPSAFE handlers */
			ioptr->xivr[handler->level] = ( 0xFF << 8 ) |
							handler->priority;
		else
			/* Only MP_MASTER may service funnelled handlers */
			ioptr->xivr[handler->level] = ( mproc_physid << 8 ) |
							handler->priority;
#endif
		ioptr->int_enable |= IOCC_IER_MSK( handler->level );

#ifdef _POWER_MP
		unlock_enable( ipri, &int_hw_lock);
#else
		i_enable( ipri );		/* end critical section   */
#endif
		io_det( ioptr );
	}
}

/*
 * NAME:  i_reset_ppc
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
 *	i_init.
 *
 *	The bus_type must be BUS_MICRO_CHANNEL to reset the interrupt source.
 *
 *	Micro Channel interrupts are level sensitive. The device reports
 *	an interrupt to the system by activating its IRQ line on the bus
 *	and keeps the line activated until it is reset at the device.
 *
 *	The IOCC presents an interrupt to the system whenever an IRQ line
 *	is active and the corresponding interrupt request register bit
 *	is 0.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	disable_ints
 *	enable_ints
 */
void
i_reset_ppc(
	struct intr *handler)		/* int handler whose lvl to reset */
{
	int	ipri;		/* old interrupt priority */
	ulong	xirr;		/* place to build XIRR value	*/

	struct ppda	*ppda_ptr;	/* PPC interrupt registers */
	volatile struct ppcint *intr;

	/* This service only handles bus interrupt levels.  */
	if( handler->bus_type == BUS_MICRO_CHANNEL ) {

		/* Find address of interrupt hardware */
		ppda_ptr = PPDA;
		intr = (volatile struct ppcint *)(ppda_ptr->intr);

		/*
		 * Build an XIRR value then use that to reset the IOCC level
		 */

		/* build an XISR value first */
		xirr = (GET_BUID_NUM( handler->bid ) << 4) | handler->level;
#ifdef _POWER_MP
		ipri = disable_lock( INTMAX, &int_hw_lock);
#else
		ipri = i_disable( INTMAX );	/* enter critical section */
#endif

		/* add in current CPPR and blast it back out */
		xirr |= (intr->xirr_poll & 0xFF000000);

		intr->i_xirr = xirr;

		__iospace_sync();

#ifdef _POWER_MP
		unlock_enable( ipri, &int_hw_lock);
#else
		i_enable( ipri );		/* end critical section */
#endif
	}
}

/*
 * NAME:  i_genplvl_ppc
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
 * RETURN VALUE DESCRIPTION:
 *	processor interrupt level for the given interrupt handler
 *
 * EXTERNAL PROCEDURES CALLED:
 */
int
i_genplvl_ppc(
	struct intr *handler)		/* int handler that is seek a plvl */
{
	int	plvl;		/* processor interrupt level	*/
	int	buid;		/* BUC BUID from bid		*/
	int	index;		/* index into poll array group  */


	if( handler->bus_type == BUS_NONE ) {
		/*
		 * For binary compatibility we will have to jump
		 * through a hoop here.
		 *
		 * drivers will be registering an EPOW handler
		 * with a bus_type of BUS_NONE and priority of
		 * INTEPOW(INTMAX).  So map them to the correct
		 * processor level for this processor.
		 *
		 * The only other BUS_NONE through here should be
		 * the local MFRR handler which is level 2.
		 */
		if( handler->priority == INTEPOW ) {
			handler->level = INT_EPOW_PPC;
		}

		return( handler->level );
	}

	ASSERT( handler->bus_type == BUS_MICRO_CHANNEL || \
		handler->bus_type == BUS_PLANAR || \
		handler->bus_type == BUS_60X );

	assert( (uint)(handler->level) < (uint)NUM_BUS_SOURCE );

	buid = GET_BUID_NUM( handler->bid );
	assert( buid <= MAX_BUID );

	/*
	 * See if BUID is new and must be allocated to a group in
	 * the poll array.  BUID 0 is reservered and setup at
	 * initialization.
	 */
	if( buid_map[buid] == -1 ) {
		assert( next_poll_grp < MAX_BUC_POLL_GRP );
		buid_map[buid] = next_poll_grp;
		next_poll_grp++;
	}

	index = buid_map[ buid ];

	plvl = (index * NUM_BUS_SOURCE) + handler->level;

	return( plvl );
}

/*
 * NAME:  i_enableplvl_ppc
 *
 * FUNCTION:  Enable a processor interrupt level
 *
 * ACCESSED:  Branch table entry i_enableplvl
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by a process only
 *
 * NOTES:
 *	This is mostly a noop on PPC machines unless the BUC is
 *	an IOCC in which case i_unmask is called to setup the
 *	XIVR and enable the bus level.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_unmask
 */
void
i_enableplvl_ppc(
	struct intr *handler,		/* int handler that caused this */
	int	plvl)			/* processor level to enable */
{

	if ( handler->bus_type == BUS_MICRO_CHANNEL )
		i_unmask( handler );
}

/*
 * NAME:  i_disableplvl_ppc
 *
 * FUNCTION:  Disable a processor interrupt level
 *
 * ACCESSED:  Branch table entry i_disableplvl
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by a process only
 *
 * NOTES:
 *	Calls i_mask() & i_reset_int() to do the dirty work
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_mask
 *	i_reset_int
 */
void
i_disableplvl_ppc(
	struct intr *handler,		/* int handler that caused this */
	int	plvl)			/* processor level to enable */
{
	/*
	 * mask and reset the bus level.  No need to check for bus_type
	 * since these functions will do that.
	 */
	i_mask( handler );
	i_reset_int( handler );
}

/*
 * NAME:  i_adjustplvl_ppc
 *
 * FUNCTION:  Change the steering characteristics of an interrupt level
 *
 * ACCESSED:  Called directly, PowerPC only.  Multiprocessor ONLY!
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by a process only
 *
 * NOTES:
 *	Passed 'operation' tells whether to Funnel or Unfunnel the level.
 *
 *	Includes significant parts of both i_mask() and i_unmask().
 *
 *	Processor number 0xFF means any processor to the hardware.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 */
#ifdef _POWER_MP
void
i_adjustplvl_ppc(
	struct intr *handler,		/* int handler that caused this */
	int	plvl,			/* processor level to enable */
	int	operation)		/* what to do */
{
	register int	ipri;		/* input interrupt priority  */
	unsigned long	ier_mask;
#ifdef _POWER_MP
	extern uint mproc_physid;
#endif

	volatile struct iocc_ppc *ioptr;

	/* Mask the bus interrupt level. */

	ioptr = (volatile struct iocc_ppc *)
		io_att( BID_TO_IOCC(handler->bid), IO_IOCC_PPC );

	ier_mask = IOCC_IER_MSK( handler->level );
#ifdef _POWER_MP
	ipri = disable_lock( INTMAX, &int_hw_lock);
#else
	ipri = i_disable( INTMAX );	/* enter critical section */
#endif
	ioptr->int_enable &= ~ier_mask;
	if ( operation == INTR_FUNNEL )
	{
		/* Only MP_MASTER may service funnelled handlers */
		ioptr->xivr[handler->level] = ( mproc_physid << 8 ) |
						handler->priority;
	}
	else /* operation == INTR_UNFUNNEL */
	{
		/* Any processor may service MPSAFE handlers */
		ioptr->xivr[handler->level] = ( 0xFF << 8 ) |
						handler->priority;
	}

	ioptr->int_enable |= ier_mask;

#ifdef _POWER_MP
	unlock_enable( ipri, &int_hw_lock);
#else
	i_enable( ipri );		/* end critical section  */
#endif
	io_det( ioptr );
}
#endif /* _POWER_MP */

/*
 * NAME:  i_loginterr_ppc
 *
 * FUNCTION:  Log a phantom interrupt
 *
 * ACCESSED:  Branch table entry i_loginterr
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by i_poll only WITH MSR(EE) OFF.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	errsave
 */
void
i_loginterr_ppc(
	struct intr *handler)		/* int handler that caused this */
{
	int	ipri;		/* interrupt priority	*/
	ulong	irr;		/* IOCC IRR register	*/
	volatile struct iocc_ppc *ioptr;	/* IOCC pointer		*/

#ifdef INTRDEBUG
	brkpoint( handler, handler->bid, handler->level );
#endif /* INTRDEBUG */

	if( handler->bus_type == BUS_MICRO_CHANNEL ) {
		/*
		 * Attempt to reset the level in the IOCC by issueing
		 * a EOI to the level.  Then check the IRR bit.  If
		 * still set then log the error.
		 */
		ioptr = (volatile struct iocc_ppc *)
			io_att( BID_TO_IOCC(handler->bid), IO_IOCC_PPC );
#ifdef INTRDEBUG
		irr = ioptr->int_request;
		/* Trace it "LOGE" */
		mltrace( 0x4c4f4745, handler->bid, handler->level, irr );
#endif /* INTRDEBUG */

		ioptr->eoi[ handler->level ] = 0;
		irr = ioptr->int_request;

		if( irr & IOCC_IER_MSK( handler->level ) ) {
			intr_log_ppc.bid = handler->bid;
			intr_log_ppc.level = handler->level;
			errsave(&intr_log_ppc, sizeof(intr_log_ppc));
		}

		io_det( ioptr );
	}
}

  /*
 * NAME: i_issue_eoi_ppc
 *
 * FUNCTION:  Issue EOI to interrupt source
 *
 * ACCESSED:  Branch table entry i_issue_eoi
 *
 * EXECUTION ENVIRONMENT:
 *
 *      It can page fault if called from a process on a pageable stack.
 *
 * NOTES:
 *	See i_reset_ppc()
 *
 *	This function is much like i_reset_ppc() with the following
 *	exception.  The CPPR (LSB of the XIRR) is CSA()->prev->mstinpri.
 *	(i.e. we are EOI'ing an interrupt and transitioning the hardware
 *	to a less-favored priority.
 *
 *	We are MSR_EE == 0 when called.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	disable_ints
 *	enable_ints
 */
void
i_issue_eoi_ppc(
	struct intr *handler) 		/* int handler to get EOI'ed */
{
	volatile struct ppcint *intr;	/* PPC interrupt registers */
	struct ppda *ppda_ptr;
	unsigned int intpri;

	ASSERT(CSA->prev != 0);

	ppda_ptr = PPDA;

	/*
	 * Get address of interrupt hardware.
	 */
	intr = (volatile struct ppcint *)(ppda_ptr->intr);

	/*
	 * The new XIRR will be the highest priority interrupt outstandinding
	 * (should be the current priority) concatenated with the BID and
	 * level of the interrupt we are going to EOI.
	 *
	 */
	intpri = clz32(ppda_ptr->i_softpri << 16);

#if defined(INTRDEBUG)
	/*
	 * mltrace("EOI1", ...);
	 */
	mltrace(0x454f4931,
		ppda_ptr->i_softpri,
		intpri,
		(GET_BUID_NUM(handler->bid) << 4) | handler->level);
#endif

	intr->i_xirr = (intpri << 24) |
	               (GET_BUID_NUM(handler->bid) << 4) |
		       handler->level;

	__iospace_sync();
}
#endif /* _POWER_PC */

