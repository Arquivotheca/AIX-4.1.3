static char sccsid[] = "@(#)64	1.13  src/bos/kernel/ios/POWER/intr_rspc.c, sysios, bos41J 5/2/95 16:53:39";
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS:
 *      i_mask_rspc,		i_unmask_rspc,
 *      i_reset_rspc,		i_genplvl_rspc,
 *	i_enableplvl_rspc,	i_disableplvl_rspc,
 *	i_loginterr_rspc,	i_issue_eoi_rspc,
 *	i_adjustplvl_rspc
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifdef _RSPC

#include <sys/types.h>
#include <sys/syspest.h>
#include <sys/mstsave.h>
#include <sys/low.h>
#include <sys/intr.h>
#include <sys/trchkid.h>
#include <sys/adspace.h>
#include <sys/errids.h>
#include <sys/ppda.h>
#include <sys/inline.h>
#include <sys/ioacc.h>

#ifndef _H_SYSTEM_RSPC
#include <sys/system_rspc.h>
#endif /* _H_SYSTEM_RSPC */

#include "interrupt.h"
#include "mpic.h"

#ifndef _h_INTR_HW_PPC
#include "intr_hw_ppc.h"
#endif /*_h_INTR_HW_PPC */

extern struct io_map		nio_map;
extern struct intrerr_ppc	intr_log_ppc;	/* found in intr_ppc.c */

struct io_map			mpic_map;

/* A place to keep track of missing interrupts by level			*/
unsigned long i_missing[16] = {0};

#ifdef INTR_DEBUG
/* A place to keep track of total interrupt count by level		*/
unsigned long i_total[32] = {0};
#endif /* INTR_DEBUG */

/*
 * RSPC systems with MPIC class interrupt controllers are MP capable.
 * In such machines, a mechanism must exist to lock the interrupt
 * controller hardware, from a software perspective, to keep multiple CPUs
 * from accessing the chips simultaneously.  The int_hw_lock, used orig-
 * inally on PowerPC IO compliant MP systems, serializes hardware access.
 */
#ifdef _POWER_MP
#define	INT_HW_LOCK(pri)	disable_lock(pri,&int_hw_lock)
#define	INT_HW_UNLOCK(pri)	unlock_enable(pri,&int_hw_lock)
extern Simple_lock		int_hw_lock;
#else
#define	INT_HW_LOCK(pri)	i_disable(pri)
#define	INT_HW_UNLOCK(pri)	i_enable(pri)
#endif /* _POWER_MP */

/*
 * MODULE NOTES:
 *   This file contains the pinned low level interrupt code for the DAKOTA
 * class of machines. That means we have a couple of major problems to overcome:
 *   1 - Interrupts are edge triggered.  We use this unfortunate fact to
 *       justify a rule that says hardware interrupt levels are non-shareable
 *	 on ISA bus adapters.
 *	 PCI bus interrupt level are sharable since they are level triggered.
 *   2 - The 8259s provide only 16 hardware interrupt levels, and it is a very
 *       old & mean chip.  It does not take kindly to constant reading and
 *       writing of it's registers.  That means we have to put quite a bit of
 *       time between register accesses if we don't want to mess up the whole
 *       system. (note: the part used in Sandalfoot models and on do
 *       not exhibit this behavior; therefore, the io_delay calls have been
 *       removed as of 6/1/94)
 *
 * When a routine registers an interrupt handler with i_init() the routine
 * i_enableplvl(), in this module, will be called.  Normally i_enableplvl would
 * set up a bunch of priority specific hardware interrupt masks.  Now all it
 * will do is modify the 8259 interrupt masks to ALWAYS enable this hardware
 * interrupt level.  No more reloading of interrupt mask registers on every
 * system priority change.
 *
 * Since hardware interrupts for a given device are ALWAYS enabled, the
 * ex_flih can be entered any time the MSR(EE) bit is on.  This is OK because
 * all the ex_flih will do is queue the interrupt by setting the appropriate
 * bit in ppda->i_softpri, ppda->i_prilvl[], and acking the 8259. 
 * Since we are edge triggered, we will not get another interrupt
 * at that level until the real slih has been called to reset the
 * interrupt, and it pops again.  (The exception to this
 * would be any periodic interrupt sources generating pulses fast enough that
 * the real slih didn't get scheduled and run between two pulses.  In this
 * case one or more interrupts could be lost.
 *
 * The routine finish_interrupt() is called at the end of all of the system
 * first level interrupt handlers.  This code will check ppda->i_softpri
 * and call a routine that will determine if interrupt processing can be 
 * done.  This means that if a hardware
 * interrupt has just been logged, and we were at a priority level that
 * allows processing that interrupt, we will get to it now.  If the priority
 * level we were at is not able to accept the interrupt, then nothing
 * will be done until we are at a priority that allows processing this
 * interrupt.
 *
 * The final piece of this mess is in i_enable() and a couple of other
 * places where the priority is enabled to a less favored value.  
 * These place will check i_softpri just before re-enabling MSR(EE)
 * if it is nonzero and the priority is enabled enough to allow the
 * queued interrupts to be processed then they are.
 */


/*
 * This local variable contains a copy of what has been written to the 8259
 * mask registers.  It will be updated, and as a result the real hardware mask
 * registers, only by i_enableplvl_rspc() and i_disableplvl_rspc().
 *
 * The 8259 enable is backward.  A 1 disables the line while a 0 enables it.
 */

uint rspc_8259_enables = 0xffff;	/* Start with no interrupts enabled -
					   this maintains the "processor" view
					   of the interrupt mask.  High half
					   never used, padded for lwarx/stwcx.
					 */

ushort rspc_8259_mask = 0x0;		/* this maintains the "device driver"
					   view of the interrupt mask.  Used
					   for i_mask, i_unmask
					*/

ushort rspc_8259_mask_sys = 0xfffb;	/* this maintains the "system" view
					   of the interrupt mask.  Used for 
					   i_enableplvl, i_disableplvl & 
					   i_issue_eoi.
					*/

ushort rspc_8259_elcr = 0;		/* Everybody is initialized to be
					   edge triggered.
					 */

/* Construct and write the Primary/Secondary 8259 Interrupt Masks */
extern void	i_8259mask( volatile char *ioptr );

/* Write the Primary/Secondary 8259 ELC Registers */
extern void	i_8259elcr( volatile char *ioptr );

/*
 * NAME:  i_enableplvl_rspc
 *
 * FUNCTION:  Enable a processor interrupt level
 *
 * ACCESSED:  Branch table entry i_enableplvl
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by a process only (INTBASE).
 *	This routine is not performance critical.
 *
 * NOTES:
 *      Adjusts the priority mapping array, EIM values, for the
 *      given priority and all less favored priorities.  Then
 *      unmasks or enables the bus level if needed.
 *
 * NOTES:
 *        On these machines we are not able to generate interrupts through
 *      software.  That means that all references to the 8259s must be in
 *      support of real hardware devices.  Any requests that come to this
 *      module for non-hardware devices will have to be investigated.
 *
 *        The mask fields in an 8259 are different than IOCC/XIO and friends.
 *      When a bit is 0, the interrupt is enabled.  When 1, the interrupt is
 *      masked off.  That means enable uses 'and' and disable uses 'or', just
 *      backwards from the other guys.
 *
 *        Since hardware interrupt levels are not shared, we will set the
 *      flag INTR_NOT_SHARED when they enable one of the bus interrupt
 *      levels.  This means we don't have to play in i_init() from the
 *      common code.  It also means i_init() will correctly reject any
 *      attempts to add a 2nd interrupt handler.  (If this is a PCI bus
 *      handler, we will allow shared interrupts.  This may be a mistake,
 *      but since PCI is level triggered it SHOULD work.)
 *
 *	  For MP to actually work, a mechanism with an MPC must be implemented,
 *	as on Pegasus, to serialize data structure accesses between processors.
 *
 * RETURN VALUE DESCRIPTION:
 *      none
 *
 * EXTERNAL PROCEDURES CALLED:
 *      I8259_IMR_MSK
 *      i_8259elcr,	i_8259mask
 *	iomem_att,	iomem_det
 *	INT_HW_LOCK,	INT_HW_UNLOCK
 */
void
i_enableplvl_rspc(
        struct intr	*handler,	/* int handler that caused this */
        int		 plvl)		/* processor level to enable */
{
	int		 b_type;
	int		 pri;

	/* BUS_NONE should only be used by EPOW handlers */
	if( handler->bus_type == BUS_NONE )
		return;

	b_type = BID_TYPE(handler->bid);

	if( b_type == IO_DIAG )
		return;

	ASSERT( b_type == IO_ISA || b_type == IO_PCI );

	pri = INT_HW_LOCK(INTMAX);

	if (handler->flags & INTR_EDGE )
		handler->flags |= INTR_NOT_SHARED;

	if( plvl > 0x10 )	/* 0x10 to 0xFF (Max) are from the MPIC */
	{
		int				 lvl;
		union { struct vecpri		 s;
			unsigned int		 i; } vp;
		volatile struct s_mpic		*Mpic;

		Mpic = (volatile struct s_mpic *)iomem_att( &mpic_map );
		lvl = handler->level;

		/* Disable the MPIC source */
		vp.i = MpicRead( &Mpic->intsource[lvl].vecpri );
		vp.s.mask = MPIC_MASK_DISABLED;
		MpicWrite( &Mpic->intsource[lvl].vecpri, vp.i );
		__iospace_sync();

		/* Ensure that the Vector/Priority and Destination registers
		   are not in use */
		vp.i = MpicRead( &Mpic->intsource[lvl].vecpri );
		if ( vp.s.activity == MPIC_ACTIVITY_INUSE )
		{
			INT_HW_UNLOCK(pri);
			INT_HW_LOCK(INTMAX);
		}
		vp.i = MpicRead( &Mpic->intsource[lvl].vecpri );
		ASSERT(vp.s.activity == MPIC_ACTIVITY_IDLE);

		if (handler->flags & INTR_EDGE )
			vp.s.sense = MPIC_SENSE_EDGE;

		if (handler->flags & INTR_POLARITY )
			vp.s.polarity = MPIC_POLARITY_HIGH;

		/* Set the priority for this interrupt source in MPIC */
		vp.s.priority = MPIC_PRI(handler->priority);
		MpicWrite( &Mpic->intsource[lvl].vecpri, vp.i );
		eieio();

		/* Enable the MPIC source */
		vp.s.mask = MPIC_MASK_ENABLED;
		MpicWrite( &Mpic->intsource[lvl].vecpri, vp.i );
		__iospace_sync();

		INT_HW_UNLOCK(pri);
		iomem_det( (void *)Mpic );
	}
	else			/* 0-F are in the 8259s, 10 is EPOW */
	{
		volatile char	*iop;

		iop = iomem_att( &nio_map );
		if (handler->flags & INTR_LEVEL )
		{
			rspc_8259_elcr |= I8259_IMR_MSK(plvl);
			i_8259elcr( iop );
		}

		/* You may need to adjust the priority of the 8259 interrupt
		   source (source 0) on the MPIC.  (Raise it; more favored) */

		/* Enable the IRQ through the 8259 */
		rspc_8259_mask_sys &= (~I8259_IMR_MSK(plvl));
		i_8259mask( iop );

		INT_HW_UNLOCK(pri);
		iomem_det( (void *)iop );
	}
}

/*
 * NAME:  i_disableplvl_rspc
 *
 * FUNCTION:  Disable a processor interrupt level
 *
 * ACCESSED:  Branch table entry i_disableplvl
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by a process only (INTBASE).
 *	This routine is not performance critical.
 *
 * NOTES:
 *      The mask fields in an 8259 are different than IOCC/XIO and friends.
 *      When a bit is 0, the interrupt is enabled.  When 1, the interrupt is
 *      masked off.  That means enable uses 'and' and disable uses 'or', just
 *      backwards from the other guys.
 *
 *	  For MP to actually work, a mechanism with an MPC must be implemented,
 *	as on Pegasus, to serialize data structure accesses between processors.
 *
 * RETURN VALUE DESCRIPTION:
 *      none
 *
 * EXTERNAL PROCEDURES CALLED:
 *      I8259_IMR_MSK
 *      i_8259elcr,	i_8259mask
 *	iomem_att,	iomem_det
 *	INT_HW_LOCK,	INT_HW_UNLOCK
 */
void
i_disableplvl_rspc(
        struct intr	*handler,	/* int handler that caused this */
        int		 plvl)		/* processor level to enable */
{
	int		 b_type;
	int		 pri;

	/* BUS_NONE should only be used by EPOW handlers */
	if( handler->bus_type == BUS_NONE )
		return;

	b_type = BID_TYPE(handler->bid);

	if( b_type == IO_DIAG )
		return;

	ASSERT( b_type == IO_ISA || b_type == IO_PCI );

	pri = INT_HW_LOCK(INTMAX);

	if( plvl > 0x10 )	/* 0x10 to 0xFF (Max) are from the MPIC */
	{
		int				 lvl;
		union { struct vecpri		 s;
			unsigned int		 i; } vp;
		volatile struct s_mpic		*Mpic;

		Mpic = (volatile struct s_mpic *)iomem_att( &mpic_map );
		lvl = handler->level;

		/* Disable the MPIC source */
		vp.i = MpicRead( &Mpic->intsource[lvl].vecpri );
		vp.s.mask = MPIC_MASK_DISABLED;
		MpicWrite( &Mpic->intsource[lvl].vecpri, vp.i );
		__iospace_sync();

		/* Ensure that the Vector/Priority and Destination registers
		   are not in use */
		vp.i = MpicRead( &Mpic->intsource[lvl].vecpri );
		if ( vp.s.activity == MPIC_ACTIVITY_INUSE )
		{
			INT_HW_UNLOCK(pri);
			INT_HW_LOCK(INTMAX);
		}
		vp.i = MpicRead( &Mpic->intsource[lvl].vecpri );
		ASSERT(vp.s.activity == MPIC_ACTIVITY_IDLE);

		/* Set interrupt trigger mode back to level sensitive */
		if (handler->flags & INTR_EDGE )
			vp.s.sense = MPIC_SENSE_LEVEL;

		/* Return polarity to High */
		if (handler->flags & INTR_POLARITY )
			vp.s.polarity = MPIC_POLARITY_HIGH;

		MpicWrite( &Mpic->intsource[lvl].vecpri, vp.i );

		INT_HW_UNLOCK(pri);
		iomem_det( (void *)Mpic );
	}
	else			/* 0-F are in the 8259s, 10 is EPOW */
	{
		volatile char	*iop;

		iop = iomem_att( &nio_map );
		if (handler->flags & INTR_LEVEL)
		{
			/* Set interrupt trigger mode back to edge */
			rspc_8259_elcr &= (~I8259_IMR_MSK(plvl));
			i_8259elcr( iop );
		}

		/* You may need to adjust the priority of the 8259 interrupt
		   source (source 0) on the MPIC.  (Lower it; less favored) */

		/* Disable the IRQ through the 8259 */
		rspc_8259_mask_sys |= I8259_IMR_MSK(plvl);
		i_8259mask( iop );

		INT_HW_UNLOCK(pri);
		iomem_det( (void *)iop );
	}
}

/*
 * NAME:  i_mask_rspc
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
 *      The intr structure must be pinned and must be the one passed to
 *      i_init. You should only be masking interrupts that are allocated
 *      to you.
 *
 *      This code will disable a given I/O bus level for all interrupt
 *      priorities.
 *
 *      The DAKOTA I/O subsystem does not have two levels of interrupt
 *      source masking as do the IOCC/XIO based I/O subsystems.  That means
 *      that i_mask, and i_unmask will have to directly modify the
 *      stored 8259 masks, as well as diddle the chips.  This will be a little
 *      slower perhaps than the IOCC based versions, and may result in some
 *      'strange' behavior as seen from interrupt handlers.  Take your time
 *      and think it through if things seem strange.
 *
 * RETURN VALUE DESCRIPTION:
 *      none
 *
 * EXTERNAL PROCEDURES CALLED:
 *      i_8259mask
 *	i_genplvl
 *	iomem_att,  iomem_det
 *	INT_HW_LOCK, INT_HW_UNLOCK
 */
void
i_mask_rspc( struct intr *handler )	/* int handler whose lvl to mask */
{
	int		 plvl;
	int		 lvl;
	int		 pri;

	plvl = i_genplvl( handler );
	pri  = INT_HW_LOCK(INTMAX);

	ASSERT( plvl != 0x10 );			/* The whole ISA bus */

	if( plvl > 0x10 )	/* 0x10 to 0xFF (Max) are from the MPIC */
	{
		int				 lvl;
		union { struct vecpri		 s;
			unsigned int		 i; } vp;
		volatile struct s_mpic		*Mpic;

		Mpic = (volatile struct s_mpic *)iomem_att( &mpic_map );
		lvl = handler->level;

		/* Disable the MPIC source */
		vp.i = MpicRead( &Mpic->intsource[lvl].vecpri );
		vp.s.mask = MPIC_MASK_DISABLED;
		MpicWrite( &Mpic->intsource[lvl].vecpri, vp.i );
		__iospace_sync();

		INT_HW_UNLOCK(pri);
		iomem_det( (void *)Mpic );
	}
	else			/* 0-F are in the 8259s, 10 is EPOW */
	{
		volatile char	*iop;

		/* Disable the IRQ through the 8259 */
		iop = iomem_att( &nio_map );
		rspc_8259_mask |= I8259_IMR_MSK(plvl);
		i_8259mask( iop );

		INT_HW_UNLOCK(pri);
		iomem_det( (void *)iop );
	}
}

/*
 * NAME:  i_unmask_rspc
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
 *      The intr structure must be pinned and must be the one passed to
 *      i_init. You should only be unmasking interrupts that are allocated
 *      to you.
 *
 *      This will enable an I/O bus level for all I/O priorites that
 *      are strictly less favored than it.
 *
 * RETURN VALUE DESCRIPTION:
 *      none
 *
 * EXTERNAL PROCEDURES CALLED:
 *      i_8259mask
 *	i_genplvl
 *	iomem_att,  iomem_det
 *	INT_HW_LOCK, INT_HW_UNLOCK
 */

void
i_unmask_rspc( struct intr *handler )  /* int handler whose lvl to enable */
{
	int		 plvl;
	int		 lvl;
	int		 pri;

	plvl = i_genplvl( handler );
	pri = INT_HW_LOCK(INTMAX);

	ASSERT( plvl != 0x10 );

	if( plvl > 0x10 )	/* 0x10 to 0xFF (Max) are from the MPIC */
	{
		int				 lvl;
		union { struct vecpri		 s;
			unsigned int		 i; } vp;
		volatile struct s_mpic		*Mpic;

		Mpic = (volatile struct s_mpic *)iomem_att( &mpic_map );
		lvl = handler->level;

		/* Enable the MPIC source */
		vp.i = MpicRead( &Mpic->intsource[lvl].vecpri );
		vp.s.mask = MPIC_MASK_ENABLED;
		MpicWrite( &Mpic->intsource[lvl].vecpri, vp.i );
		__iospace_sync();

		INT_HW_UNLOCK(pri);
		iomem_det( (void *)Mpic );
	}
	else			/* 0-F are in the 8259s, 10 is EPOW */
	{
		volatile char	*iop;

		/* Enable the IRQ through the 8259 */
		iop = iomem_att( &nio_map );
		rspc_8259_mask &= (~I8259_IMR_MSK(plvl));
		i_8259mask( iop );

		INT_HW_UNLOCK(pri);
		iomem_det( (void *)iop );
	}
}

/*
 * NAME:  i_reset_rspc
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
 *      This service should only be called from within the interrupt
 *      subsystem, i_clear(), via the function name i_reset_int in
 *	the branch table.
 *
 *      The intr structure must be pinned and must be the one passed to
 *      i_init. You should only be resetting interrupts that are allocated
 *      to you.
 *
 *      This code is a NOOP because the there is nothing to reset on
 *	either an 8259 or an MPIC.
 *
 * RETURN VALUE DESCRIPTION:
 *      none
 *
 * EXTERNAL PROCEDURES CALLED:
 */
void
i_reset_rspc(struct intr *handler)
{
	return;
}

/*
 * NAME:  i_issue_eoi_rspc
 *
 * FUNCTION:  Issue an EOI to the interrupt hardware
 *
 * ACCESSED:  Branch table entry i_issue_eoi
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from an interrupt environment.
 *
 * NOTES:
 *      This service should only be called from within the interrupt
 *      subsystem, i_poll_soft(), via the function name i_issue_eoi in
 *	the branch table.
 *
 *      The intr structure must be pinned and must be the one passed to
 *      i_init.
 *
 *	On RSPC models IRQ 15 is set up for use by PCI agents.  This
 *	level is level sensitive.  The flih masks this level and issues
 *	an EOI to the 8259 and interrupt time.  This function
 *	only has to unmask the line now.
 *
 *	The EOI Register, Int. Ack. Register and, arguably, Interrupt
 *	Source Mask Bit are not serialized with the int_hw_lock.
 *
 * RETURN VALUE DESCRIPTION:
 *      none
 *
 * EXTERNAL PROCEDURES CALLED:
 *      i_8259mask
 *	i_genplvl
 *	iomem_att,  iomem_det
 */
void
i_issue_eoi_rspc( struct intr *handler )
{
	int		 plvl;

	plvl   = i_genplvl( handler );

	if( plvl > 0x10 )	/* 0x10 to 0xFF (Max) are from the MPIC */
	{
		/* This could, arguably should, be hand coded since iomem_att
		 * (etc.) are generic and, thus, costly in cycles. */

		volatile struct s_mpic		*Mpic;

		Mpic = (volatile struct s_mpic *)iomem_att( &mpic_map );

		MpicWrite( &Mpic->procreg[CPUID].eoi, 0 );
		eieio();

		iomem_det( (void *)Mpic );
	}
	else			/* 0-F are in the 8259s, 10 is EPOW */
	{
		volatile char	*iop;

		if( handler->flags & INTR_EDGE )
			return;

		/* 
		 * IRQ was set up for level sensitive triggering
		 * so unmask IRQ line so more can be presented
		 */
		iop = iomem_att( &nio_map );
		rspc_8259_mask_sys &= (~I8259_IMR_MSK(handler->level));
		i_8259mask( iop );

		iomem_det( (void *)iop );
	}
}

/*
 * NAME:  i_genplvl_rspc
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
 *      The common code does not require a one to one relationship between
 *      bus interrupt levels and external interrupt levels (sources). The
 *      i_genplvl function is used to convert a bus interrupt level into an
 *      processor interrupt level. The bid parameter provides future support
 *      for multiple I/O buses.
 *
 * RETURN VALUE DESCRIPTION:
 *      processor interrupt level for the given interrupt handler
 *
 * EXTERNAL PROCEDURES CALLED:
 */

int
i_genplvl_rspc( struct intr *handler )
{
	int		 b_type = BID_TYPE(handler->bid);

        if( handler->bus_type == BUS_NONE )
        {
		/*
		 * For binary compatibility we have to jump through a hoop.
		 *
		 * Drivers will be registering an EPOW handler with a bus_type
		 * of BUS_NONE and priority of INTEPOW(INTMAX).  Map them to
		 * the correct processor level for this processor.
		 */
		if( handler->priority == INTEPOW )
			handler->level = INT_EPOW_RSPC;
		return(handler->level);
	}

	switch( b_type )
	{
		case IO_ISA:
				return(handler->level);

		case IO_PCI:
				/* Victory */
				if ( intctl_pri == INT_CTL_MPIC )
					return( MPIC_VECT(handler->level) );
				else /* Dakota */
					return( handler->level );
		default:
				assert(0);
	}
	assert(0);
}

#ifdef _POWER_MP
/*
 * NAME:  i_adjustplvl_rspc
 *
 * FUNCTION:  Change the steering characteristics of an interrupt level
 *
 * ACCESSED:  Called directly, PowerPC only.  Multiprocessor ONLY!
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by a process only (INTBASE).
 *	This service is not performance critical.
 *	You must have interrupt steering hardware like the MPIC for this
 *	to work.
 *	No check is made to see if there's an MPIC - it's assumed.
 *
 * NOTES:  
 *	Passed 'operation' tells whether to Funnel or Unfunnel the level.
 *
 *	The Destination register for the affected interrupt source is a
 *	bitmask where every 1 bit marks a processor which is allowed to
 *	service the interrupt.  Thus a FUNNELLED interrupt will have exactly
 *	one bit set, while an UNFUNNELLED one will have physical NCPUs
 *	bits set.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	iomem_att,	iomem_det,
 *	INT_HW_LOCK,	INT_HW_UNLOCK
 */
void
i_adjustplvl_rspc(
	struct intr *handler,		/* int handler that caused this */
	int	plvl,			/* Not Used */
	int	operation)		/* what to do */
{
	extern uint	 mproc_physid;	/* Master processor physical ID */
	int		 pri;		/* entry interrupt priority  */
	int		 lvl;		/* MPIC Source to manipulate */
	uint		 cpumask;	/* CPU Mask bit array */
	int		 disabled;	/* Entry condition of MASK bit */

	union { struct vecpri		 s;
		unsigned int		 i; } vp;
	union { struct feature0		 s;
		unsigned int		 i; } feat0;
	volatile struct s_mpic		*Mpic;

	/* All ISA bus sources, plvl[0-F], share MPIC Source 0.  All other
	 * plvl values are individual MPIC sources. */
	lvl = ( plvl < 0x10 ) ? 0 : handler->level;
	Mpic = (volatile struct s_mpic *)iomem_att( &mpic_map );

	pri = INT_HW_LOCK(INTMAX);

	/* Disable the MPIC source */
	vp.i = MpicRead( &Mpic->intsource[lvl].vecpri );
	disabled = vp.s.mask;
	vp.s.mask = MPIC_MASK_DISABLED;
	MpicWrite( &Mpic->intsource[lvl].vecpri, vp.i );
	__iospace_sync();

	/* Ensure that the Vector/Priority and Destination registers
	   are not in use */
	vp.i = MpicRead( &Mpic->intsource[lvl].vecpri );
	if ( vp.s.activity == MPIC_ACTIVITY_INUSE )
	{
		INT_HW_UNLOCK(pri);
		INT_HW_LOCK(INTMAX);
	}
	vp.i = MpicRead( &Mpic->intsource[lvl].vecpri );
	ASSERT(vp.s.activity == MPIC_ACTIVITY_IDLE);

	if ( operation == INTR_FUNNEL )
	{
		/* Only MP_MASTER may service funnelled handlers */
		cpumask = ( 1 << mproc_physid );
	}
	else /* operation == INTR_UNFUNNEL */
	{
		/* Any processor may service MPSAFE handlers */
		feat0.i = MpicRead( &Mpic->global.feature0 );
		cpumask = ( 1 << ( feat0.s.numCPU + 1 ) ) - 1;
	}
	MpicWrite( &Mpic->intsource[lvl].destination, cpumask );
	eieio();			/* MP_int_dest is always written */

	/* Enable the MPIC source (only if we Disabled it above) */
	if ( !disabled )
	{
		vp.s.mask = MPIC_MASK_ENABLED;
		MpicWrite( &Mpic->intsource[lvl].vecpri, vp.i );
	}

	__iospace_sync();
	INT_HW_UNLOCK(pri);
	iomem_det( (void *)Mpic );
}
#endif /* _POWER_MP */

/*
 * NAME:  i_loginterr_soft
 *
 * FUNCTION:  Log an unclaimed interrupt
 *
 * ACCESSED:  Branch table entry i_loginterr
 *
 * NOTE: This function uses the standard PowerPC error log structure
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by i_poll_soft only
 *
 * RETURN VALUE DESCRIPTION:
 *      none
 *
 * EXTERNAL PROCEDURES CALLED:
 *      errsave
 */
void
i_loginterr_soft(
	struct intr *handler)           /* int handler that caused this */
{
#ifdef INTRDEBUG
	brkpoint( handler, handler->bid, handler->level );
#endif /* INTRDEBUG */

	/* Since the 8259 indicates IRQ 7 for spurious interrupts
	 * they are not logged.
	 * 
	 * NOTE: if lots of spurious(unclaimed) interrupts hit IRQ15,
	 * which is IRQ 7 on the slave then we should not log them
	 * either.  If we are getting lots of either then it may be
	 * better to change the flih to check the ISR register in the
	 * 8259 before even queueing the interrupt.
	 */
	if( handler->level != INT_SFIRQ7 ) {
#ifdef INTRDEBUG
		/* Trace it "LOGE" */
		mltrace( 0x4c4f4745, handler->bid, handler->level, 0 );
#endif /* INTRDEBUG */

		intr_log_ppc.bid = handler->bid;
		intr_log_ppc.level = handler->level;
		errsave(&intr_log_ppc, sizeof(intr_log_ppc));
	}
}

#endif /* _RSPC */
