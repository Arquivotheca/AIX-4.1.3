static char sccsid[] = "@(#)12	1.48  src/bos/kernel/ios/intr.c, sysios, bos41J, 9519A_all 5/5/95 09:51:23";
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS:
 *	i_poll,		i_reset,	i_init,		i_clear,
 *	i_sched,	i_iodone,	i_offlevel,	i_softint
 *	i_softoff,	i_poll_soft,	i_addlvlmap,	i_clrlvlmap
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
#include <sys/syspest.h>
#include <sys/mstsave.h>
#include <sys/low.h>
#include <sys/trchkid.h>
#include <sys/adspace.h>
#include <sys/malloc.h>
#include <sys/intr.h>
#include <sys/inline.h>
#ifndef	 _H_IOACC
#include <sys/ioacc.h>
#endif	 /* _H_IOACC */
#include "interrupt.h"
#include "intr_hw.h"

void	i_softint();

lock_t	intsys_lock = LOCK_AVAIL;
int	soft_int_model = 0;
uint	cppr_random = 0;

struct i_data	i_data = { 0 };		/* interrupt handler data struct*/
#ifdef _POWER_RS
struct intr	off_lvlhand;		/* offlevel interrupt handler */
#endif /* _POWER_RS */
#ifdef _POWER_MP
struct intr	intmax_lvlhand;		/* INTMAX MPC interrupt handler */
struct ppda	*master_ppda_ptr = &ppda[MP_MASTER];
mpc_msg_t	ipoll_mpc_msg =0;
uint		ipoll_excl_cpu =0;
void		ipoll_excl_serv();
void		ipoll_excl_wait();
void		ipoll_excl_clear();
#endif
short	(*i_lvlmap)[MAX_LVL_PER_PRI];	/* source to priority level array */

/*
 * NAME:  i_addlvlmap
 *
 * FUNCTION:
 *	This routine adds processor interrupt levels to the i_lvlmap[][]
 *	arrays.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called by either a process or interrupt
 *	handler.
 *
 *      It does not page fault except on the stack when called under
 *	a process.
 *
 * RETURN VALUE DESCRIPTION:
 *	value to be used as a bit index into i_prilvl[priority]
 *
 * EXTERNAL PROCEDURES CALLED:
 */
static
i_addlvlmap(
	int pri,		/* Priority			*/
	int plvl)		/* processor interrupt level	*/
{
	int	i;		/* loop counter			*/

	ASSERT( pri < INTTIMER );

	/* scan array for first available position */
	for( i=0; i < MAX_LVL_PER_PRI; i++ ) {
		if( i_lvlmap[pri][i] == -1 ) {
			i_lvlmap[pri][i] = plvl;
			return( i );
		}
	}
	assert( 0 );
}

/*
 * NAME:  i_clrlvlmap
 *
 * FUNCTION:
 *	This routine clears processor interrupt levels from the i_lvlmap[]
 *	arrays.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called by either a process or interrupt
 *	handler.
 *
 *      It does not page fault except on the stack when called under
 *	a process.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 */
static void
i_clrlvlmap(
	int pri,		/* Priority			*/
	int index)		/* index into array		*/
{
	ASSERT( pri < INTTIMER );

	i_lvlmap[pri][index] = -1;
}

/*
 * NAME:  i_poll
 *
 * FUNCTION:  Calls each of the the interrupt handlers on the list for
 *	the specified interrupt level until one of them indicates that
 *	it processed the interrupt.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine runs on an interrupt level.
 *
 *      It does not page fault.
 *
 * NOTES:
 *	This routine is only called by the assembler flih for external
 *	interrupts.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	interrupt handler(s)
 */

void
i_poll(
	int plvl )				/* processor interrupt lvl */
{
	register int		rc;		/* slih return code */
	register struct intr	*handler;	/* pointer 1st intr struct*/
	register struct intr	*poll_addr;	/* pointer to intr struct */

	/*
	 * Call each interrupt handler on the poll chain for this
	 * interrupt level, until one claims the interrupt, or we
	 * reach the end of the chain.
	 *
	 * The slih gets passed the address of its "intr" struct.
	 */
	/* Skip over dummy intr struct and get head of poll chain */
	handler = i_data.i_poll[plvl].poll->next;
	poll_addr = handler;
#ifdef DEBUG
	ASSERT( CSA->intpri == handler->priority );
#endif /* DEBUG */

	while ( poll_addr != (struct intr *)NULL )
	{
	  /* slih entry */
	  TRCHKLT(HKWD_KERN_SLIH,poll_addr->handler);

	  rc = (*(poll_addr->handler)) (poll_addr);

	  /* slih exit */
	  TRCHKLT(HKWD_KERN_SLIHRET,rc);

	  if ( rc == INTR_SUCC ) {

	    poll_addr->i_count++;/* increment interrupt count*/
	    return;		/* slih handled interrupt */
	  }

	  ASSERT( rc == INTR_FAIL );
	  poll_addr = poll_addr->next;
	}

	/*
	 * At this point no interrupt handler has claimed the interrupt.
	 * Log an error and carry on.
	 */
	i_loginterr( handler );
}

/*
 * NAME:  i_poll_soft
 *
 * FUNCTION:  Calls each of the the interrupt handlers on the list for
 *	the specified interrupt level until one of them indicates that
 *	it processed the interrupt.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine runs on an interrupt level.
 *
 *      It does not page fault.
 *
 * NOTES:  This function will change the priority before calling the
 *	interrupt handler.  This will only change the priority if
 *	the handler is on a shared PCI interrupt level.  Since the
 *	list is maintained in priority order the more favored handlers
 *	will be called first.  Since this level was called at the
 *	least favored priority i_poll_soft() can exit at a more
 *	favored priority if a higher priority handler claims the
 *	interrupt.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	interrupt handler(s)
 */
void
i_poll_soft(
	int plvl )			/* processor interrupt lvl	*/
{
	int		rc;		/* slih return code		*/
#ifdef DEBUG
	struct intr	*dummy;		/* pointer dummy intr struct */
#endif /* DEBUG */
	struct intr	*poll_addr;	/* pointer to intr struct	*/
	struct mstsave	*c_mst;		/* current mst save are ptr	*/
#ifdef _POWER_MP
	struct ppda *ppda_ptr;		/* pointer to ppda struct	*/
#endif

#ifdef _POWER_MP
	ppda_ptr = PPDA;
#endif
	/*
	 * Call each interrupt handler on the poll chain for this
	 * interrupt level, until one claims the interrupt, or we
	 * reach the end of the chain.
	 *
	 * The slih gets passed the address of its "intr" struct.
	 */
	/* Skip over dummy intr struct and get head of poll chain */
	poll_addr = i_data.i_poll[plvl].poll->next;
	c_mst = CSA;

#if defined(INTRDEBUG)
		/*
		 * mltrace("IPS0", ...);
		 */
		mltrace(0x49505330, plvl, poll_addr, PPDA->i_softpri);
#endif

	do {
#ifdef _POWER_MP
	  /*
	   * Go round funneled handlers if this is not the mpmaster
	   * processor.  Should only happen during a small window.  See
	   * i_init() for detailed explanation.
	   */
	  if( (poll_addr->flags & INTR_MPSAFE) ||
		    (ppda_ptr == master_ppda_ptr) )
#endif /* _POWER_MP */
	    {
		/* slih entry */
		TRCHKLT(HKWD_KERN_SLIH,poll_addr->handler);

		/* This can be done since a MST stack is in use */
		c_mst->intpri = poll_addr->priority;
		/* If calling an INTMAX handler don't turn on MSR(EE) */
		if( c_mst->intpri > INTMAX ) {
			enable();
		}
		rc = (*(poll_addr->handler)) (poll_addr);
		disable();

		/* intpri was lowered to handler's intpri; restore it */
		c_mst->intpri = INTMAX;

#if defined(INTRDEBUG)
		/*
		 * mltrace("IPS1", ...);
		 */
		mltrace(0x49505331, poll_addr->handler, rc, PPDA->i_softpri);
#endif
		/* slih exit */
		TRCHKLT(HKWD_KERN_SLIHRET,rc);

		if ( rc == INTR_SUCC ) {	/* slih handled interrupt */

		    poll_addr->i_count++;	/* increment interrupt count*/
		    /* NEXT release this should change to a branch table
		     * entry point.  Right now it is hardcoded because
		     * there is only one model supported
		     */
		    i_issue_eoi( poll_addr );/* issue an EOI if needed */
		    return;
		}

		ASSERT( rc == INTR_FAIL );
	      }
	  poll_addr = poll_addr->next;

	} while ( poll_addr != (struct intr *)NULL );

	/*
	 * At this point no interrupt handler has claimed the interrupt.
	 * Log an error and carry on.
	 */

#if defined(INTRDEBUG)
	/*
	 * mltrace("IPS2", ...);
	 */
	mltrace(0x49505332, i_data.i_poll[plvl].poll->next , -1, -1);
#endif

	i_loginterr(i_data.i_poll[plvl].poll->next);

	i_issue_eoi( i_data.i_poll[plvl].poll->next );
}

/*
 * NAME:  i_offlevel
 *
 * FUNCTION:
 *	Process each of the pending off-level requests for this
 *	off-level interrupt priority.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called on an off-level interrupt priority
 *	in an interrupt execution environment.
 *
 *      It cannot page fault.
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:  INTR_SUCC
 *
 * EXTERNAL PROCEDURES CALLED:
 *	off-level routine
 *	disable
 *	mtmsr
 */
void
i_offlevel(
	int pri,			/* priority level to process	*/
	struct ppda *ppda_ptr)		/* pointer to ppda struct	*/
{
	register int	oldmsr;		/* current msr value	*/
	register int	index;		/* index into the sched array */
	register struct intr *ip;	/* ptr to intr struct		*/

#ifdef DEBUG
	/*
	 * Verify that the interrupt priority is valid.
	 */
	ASSERT( CSA->prev != NULL );
	ASSERT( CSA->intpri == INTMAX );
#endif

	/*
	 * Run the list for this priority, each time
	 * removing the request at the head, and calling
	 * its handler.
	 */
	index = pri - INTOFFL0;
	while ( ppda_ptr->schedtail[index] != (struct intr *)NULL )
	{
		/*
		 * Remove the off-level request from the head of the list.
		 */
		ip = ppda_ptr->schedtail[index]->next;
		if ( ip == ppda_ptr->schedtail[index] )
			ppda_ptr->schedtail[index] = (struct intr *)NULL;
		else
			ppda_ptr->schedtail[index]->next = ip->next;
		/*
		 * turn off scheduled bit to allow more i_sched's
		 */
		ip->flags &= ~I_SCHED;

		/*
		 * Call off-level handler routine. Set the priority
		 * back down to the appropriate off-level priority.
		 * That's the reason for this service.
		 */
		i_enable( pri );
		(void)(*ip->handler) (ip);
		disable();	/* disable to run list	*/
		ppda_ptr->_csa->intpri = INTMAX;
	}

        /*
	 * if an extra offlevel was scheduled while we were running
	 * then reset it
	 */
	if( soft_int_model )
		RESET_SOFTPRI( pri )
	else
		RESET_SOFTLVL( pri )
}

/*
 * NAME:  i_reset
 *
 * FUNCTION:	Used to reset a bus interrupt level.  That function is
 *		now done by the flih.  This is left if for compatibility.
 *
 *		New/recompiled drivers will not even reference this due
 *		to a #define, in intr.h, that noops the call.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by either a process or an
 *      interrupt handler.
 *
 *      It can page fault if called from a process on a pageable stack.
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 */
void
i_reset(
	struct intr *handler)		/* int handler whose lvl to reset */
{
	return;
}

/*
 * NAME:  i_init
 *
 * FUNCTION:  Define an interrupt handler to the kernel.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can only be called by a process.
 *
 *      It can page fault.
 *
 * NOTES:
 *	If this is the first handler on a level then allocate a
 *	intr struct(dummy) to inherit priority & i_prilvl bit level.
 *
 *	The intr structure must be pinned.
 *
 *	The following routine allows device drivers
 *	to define an interrupt handler to the kernel.  The interrupt
 *	handler structure pointed to by the handler parameter describes
 *	the interrupt handler.  The caller of this service must set up
 *	all of the fields in the intr struct except for the "next" field.
 *
 *	The interrupt handler structure must be pinned prior to calling
 *	this service and remain pinned until after i_clear has been
 *	called to remove the interrupt handler from the poll list.
 *
 *	The interrupt handler may be called anytime after this service
 *	calls i_enable.
 *
 * RETURN VALUE DESCRIPTION:
 *	INTR_SUCC - successful completion
 *	INTR_FAIL - unsuccessful, handler not defined
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_genplvl
 *	i_addlvlmap
 *	i_clrlvlmap
 *	i_enableplvl
 *	i_adjustplvl_ppc
 *	GET_POLL_LOCK
 *	REL_POLL_LOCK
 *	lockl
 *	unlockl
 */

int
i_init(
	struct intr *handler)		/* pointer to intr structure	*/
{
	register int		ipri;	/* input interrupt priority	*/
	register int		plvl;	/* processor interrupt level	*/
	register int		lrc;	/* lockl return code		*/
	register int		rc;	/* return code			*/
	register struct intr	*intpoll; /* intr struct pointer	*/
	register struct intr	*intlast; /* intr struct pointer	*/
	register struct intr	*dummy;	  /* dummy intr struct pointer	*/
	register struct ppda	*ppda_ptr;/* ptr to my ppda		*/
	register int		allsafe;  /* All handlers MP safe	*/
	register int		multi_pri;/* multipriority level	*/

	assert( CSA->prev == NULL );	/* only a process may call this */

#if 0
	/*
	 * For binary compatibility allow these bus_types but convert
	 * them to BUS_BID from then on the bid will control the bus
	 * type.
	 */
	switch( handler->bus_type ) {
	    case 7:
	    case 5:
	    case 6:
		handler->bus_type = BUS_BID;
	}
	if( (handler->bus_type == BUS_BID) && (handler->level == 13) )
		handler->flags |= INTR_INVERT;
#endif

	/*
	 * Check if the intr structure is valid.
	 */
	assert( handler != (struct intr *)NULL );
	assert( handler->handler != ( int (*)())NULL );
	assert( handler->bus_type <= (ushort)BUS_MAXTYPE );
	assert( (handler->flags & ~INTR_FLAG_MSK) == 0 );
	assert( (uint)(handler->priority) < (uint)INTBASE );

	multi_pri = FALSE;

#ifdef _RSPC
	if( __rspc() ) {

		/*
		 * Check trigger mode.  The caller may specify either of
		 * INTR_LEVEL or INTR_EDGE.  Specifying both is an error.
		 *
		 * If caller does not specify either, then it is assumed
		 * that the caller is an old driver; therefore, we will
		 * intuit the trigger mode of the handler based on it's bus
		 * type (i.e. ISA is edge-triggered, PCI is level
		 * triggered.
		 *
		 */
		switch (handler->flags & (INTR_EDGE|INTR_LEVEL)) {
		case 0:
			/*
			 * Old style handler registering.  Intuit which
			 * bits to set based on bus type.  Note: only
			 * do this if it is truly a device driver (i.e.
			 * bus_type == BUS_BID).
			 *
			 */
			if (handler->bus_type == BUS_BID) {
			    switch (BID_TYPE(handler->bid)) {
			    case IO_ISA:
				handler->flags |= INTR_EDGE;
				break;
			    case IO_PCI:
				handler->flags |= INTR_LEVEL;
				multi_pri = TRUE;
				break;
			    default:
				assert(0);
			    }
			}
			break;

		case INTR_EDGE:
			/*
			 * Nothing to do here (currently).
			 *
			 * Can't share an INTR_EDGE so no need to
			 * set multi_pri.
			 */
			 break;

		case INTR_LEVEL:
		        multi_pri = TRUE;
			break;
		default:
			assert(0);
		}
	}
#endif /* _RSPC */

#ifdef _POWER_RS
/*
 * Remove this #define and change check when RS2G defined in sysconfig
 * structure
 */
#define RS2G	0
	/* BUS_60X is only valid for a RS2G box in the POWER_RS set */
	if( handler->bus_type == BUS_60X ) {
		if(( __power_set( POWER_RS1 | POWER_RSC )) ||
		   ( __power_rs2() && !RS2G ) )
		{
			return( INTR_FAIL );
		}
	}
#endif /* _POWER_RS */

	if( soft_int_model && handler->priority >= INTTIMER ) {
		return( INTR_FAIL );
	}

	lrc = lockl( &intsys_lock, LOCK_SHORT );
	ASSERT( lrc == LOCK_SUCC );
	/*
	 * Convert bus level to processor interrupt level
	 */
	plvl = i_genplvl( handler );

	handler->i_count=0;	/* Zero out the i_count field */

	/*
	 * Insert the "intr" struct on the poll chain for the appropriate
	 * interrupt level.
	 * Note that it is inserted at the end of the chain.
	 * This is done because an interrupt could occur on its interrupt
	 * level (if shared with another device) and we don't want to
	 * call this slih until its device is fully initialized.
	 * If an interrupt occurs, another device ahead of this
	 * one in the chain will claim it.
	 */

	intpoll = i_data.i_poll[plvl].poll;	/* get head of poll chain */

	/*
	 * This is the first handler for this interrupt level.
	 */
	if ( intpoll == (struct intr *)NULL )
	{
		dummy = (struct intr *)xmalloc(sizeof(struct intr),2,
						pinned_heap);
		dummy->next = handler;
		dummy->handler = NULL;
		dummy->bus_type = handler->bus_type;
		dummy->flags = handler->flags;
		dummy->priority = handler->priority;
		dummy->bid = handler->bid;
		dummy->i_count = 0;
		if( handler->priority < INTTIMER ) {
			dummy->level = i_addlvlmap( handler->priority, plvl );
		}
		else {
			dummy->level = -1;
		}

		/* Insert it as the only entry in the list.  */
		handler->next = (struct intr *)NULL;
		i_data.i_poll[plvl].poll = dummy;

		/* Enable the interrupt level.  */
		i_enableplvl( handler, plvl );

		/* This forces the EIM/CIL/CPPR to be updated */
		i_enable( i_disable( INTMAX ) );
	}

	else /* At least one other interrupt handler on this poll list. */
	{
		/* Get over dummy struct */
		dummy = intpoll;
		intpoll = intpoll->next;
		/*
		 * Check to see if the new interrupt handler violates
		 * any compatibility rules.
		 *
		 * 1. Must be on same bus, or bus_type
		 * 2. All handlers must be able to share the level
		 *
		 * For latency reasons, any interrupt level can be defined
		 * as non-shared.
		 */
		if( ( handler->bus_type != intpoll->bus_type ) ||
		    ( intpoll->flags & INTR_NOT_SHARED ) ||
		    ( handler->flags & INTR_NOT_SHARED ) )
		{
			unlockl( &intsys_lock );
			return( INTR_FAIL );
		}

		if( handler->bus_type == BUS_BID ) {
			if( BID_TYPE(handler->bid) != BID_TYPE(intpoll->bid) ) {
				unlockl( &intsys_lock );
				return( INTR_FAIL );
			}
		}

		/*
		 * All interrupt handlers for the same interrupt level
		 * must have the same interrupt priority, except on
		 * the PCI bus. Interrupt priorities must be
		 * strictly enforced because of their
		 * effect on interrupt latency.
		 */
		if( (multi_pri == FALSE) &&
		     (intpoll->priority != handler->priority) )
		{
			unlockl( &intsys_lock );
			return( INTR_FAIL );
		}

		/*
		 * Find the last interrupt handler on this poll list.
		 * While scanning list look for a funneled handler.  If
		 * any found, we know this level is already funneled.  If
		 * all handlers are mpsafe but the new one is not we must
		 * adjust the hardware to steer the interrupt to the
		 * mpmaster processor, but only if this is a MP machine.

		 * This means if one handler on this plvl is funneled they
		 * are all funneled.  This opens a window
		 * where an interrupt on this plvl may be being processed
		 * by a funneled processor or is being presented to
		 * another funneled processor while we are changing the
		 * interrupt steering mechanism.  This means that the
		 * handler we are trying to add may be called on a processor
		 * that is not the mpmaster.  This would break funneling.  To
		 * fix this problem i_poll will only call funneled handlers
		 * if it is running on the mpmaster processor.
		 */
		intlast = i_data.i_poll[plvl].poll;
		allsafe = 1;

		if( multi_pri == FALSE ) {
			/* Find end of list */
			do {
				intlast = intlast->next;
				if( allsafe && !(intlast->flags & INTR_MPSAFE) )
				{
					allsafe = 0;
				}
			} while ( intlast->next != (struct intr *)NULL );
		}
		else {
			short	h_pri;		/* handler priority	   */
			ulong	t_prilvl;	/* place to hold old level */

			h_pri = handler->priority;
			intpoll = intlast->next;

			GET_POLL_LOCK( ipri, INTMAX, plvl);
			while( intpoll != (struct intr *)NULL ) {
			    /* Insert in front of this intr */
			    if( intpoll->priority > h_pri ) {
				handler->next = intlast->next;
				intlast->next = handler;
				break;
			    }

			    /* Check for end of list */
			    if( intpoll->next == (struct intr *)NULL ) {
				handler->next = intpoll->next;
				intpoll->next = handler;

				if( h_pri > dummy->priority ) {
				    ppda_ptr = PPDA;
/* We are changing priorities so we must remove old lvlmap and put
 * in new one.  This is doable on UP but the MP issues are more complex
 * since another processor may be using the information as it is being
 * changed.  To fix this a simple lock will have to protect this
 * sequence but then the flih will also have to gain the lock.  This
 * implementation does not address real MP issues.  But should work okay
 * when the MP kernel is running on a UP box.
 *
 * The direction here always moves from more favored to less favored.
 * Since it is possible for an interrupt to be queued between the
 * i_disable() and the turning off of the EE bit.  So also move any
 * queued interrupts for the level from the old priority to the new one.
 */
				    i_clrlvlmap( dummy->priority,dummy->level);

				    t_prilvl = ppda_ptr->
					i_prilvl[dummy->priority] &
						PRILVL_MSK(dummy->level);

				    ppda_ptr->
					i_prilvl[dummy->priority] &=(~t_prilvl);
				    dummy->level = i_addlvlmap(h_pri, plvl);
				    dummy->priority = h_pri;
				    ppda_ptr->i_prilvl[h_pri] |= t_prilvl;
				}
				break;
			    }
			    /* Advance to next intr in list */
			    intlast = intpoll;
			    intpoll = intlast->next;
			}
			REL_POLL_LOCK( ipri, plvl);
			/*
			 * This is the return for PCI bus handlers.  This
			 * algorithm needs work when PCI buses get used
			 * on true MP boxes.  When hanging a lower
			 * priority handler to the list it is possible for
			 * the interrupt to be routed and queued to
			 * another processor at the higher priority.  This
			 * will result in the handler be called at a more
			 * more favored level which should not hurt but
			 * it needs investigating.
			 */
			unlockl( &intsys_lock );
			return( INTR_SUCC );
		}

#ifdef _POWER_MP
		/* Adjust steering if needed */
		if( __power_mp() && allsafe && !(handler->flags & INTR_MPSAFE) )
		{
#ifdef _RS6K
		    if( __rs6k() )
			i_adjustplvl_ppc( handler, plvl, INTR_FUNNEL );
#endif /* _RS6K */
#ifdef _RSPC
		    if( __rspc() )
			i_adjustplvl_rspc( handler, plvl, INTR_FUNNEL );
#endif /* _RSPC */
		}
#endif /* _POWER_MP */

		/*
		 * Insert the new interrupt handler at the end of the list.
		 * We must be disabled to INTMAX whenever we hold the
		 * poll_lock so that this processor does not get an
		 * external interrupt which also might want the poll_lock.
		 * It that happens we will deadlock.
		 */
		handler->next = (struct intr *)NULL;

		GET_POLL_LOCK( ipri, handler->priority, plvl);
		intlast->next = handler;
		REL_POLL_LOCK( ipri, plvl);
	}
	unlockl( &intsys_lock );
	return( INTR_SUCC );
}

/*
 * NAME:  i_clear
 *
 * FUNCTION:  Remove an interrupt handler from the kernel.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can only be called by a process.
 *
 *      It can page fault.
 *
 * NOTES:
 *	The following routine allows device drivers
 *	to remove the specified interrupt handler from the set of interrupt
 *	handlers for that interrupt source.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_genplvl
 *	i_addlvlmap
 *	i_clrlvlmap
 *	i_disableplvl
 *	i_adjustplvl_ppc
 *	i_disable
 *	i_enable
 *	lockl
 *	unlockl
 */

void
i_clear(
	struct intr *handler)		/* pointer to intr structure      */
{
	register int		ipri;	/* input interrupt priority	*/
	register int		plvl;	/* processor interrupt level	*/
	register int		lrc;	/* lockl return code		*/
	register struct intr	*intpoll;	/* intr struct pointer	*/
	register struct intr	*handprev;	/* intr struct pointer	*/
	register struct intr	*dummy;		/* intr struct pointer	*/
	register struct ppda	*ppda_ptr;	/* ptr to ppda		*/

	assert( CSA->prev == NULL );	/* only a process may call this */

	/*
	 * Check if the intr structure is valid.
	 */
	assert( handler != (struct intr *)NULL );

	lrc = lockl( &intsys_lock, LOCK_SHORT );
	ASSERT( lrc == LOCK_SUCC );
	/*
	 * Convert bus level to processor interrupt level.  Then scan
	 * that processor level until the specific intr structure is
	 * found.
	 */
	plvl = i_genplvl( handler );

	/* handprev points to the dummy intr struct */
	dummy = handprev = i_data.i_poll[plvl].poll;
	/* intpoll points to first handler intr struct */
	intpoll = i_data.i_poll[plvl].poll->next;

	/*
	 * Find handler on list
	 */
	while( (intpoll != handler) && (intpoll != (struct intr *)NULL) ) {
		handprev = intpoll;
		intpoll = intpoll->next;
	}

	assert( intpoll != (struct intr *)NULL );

	/*
	 * if handler is only one on list then disable(turn off)
	 * the level, remove it from i_lvlmap[][], free dummy intr struct.
	 *
	 * If not last one...
	 */
	if((intpoll == dummy->next)&&(intpoll->next == (struct intr *)NULL)) {
		i_disableplvl( handler, plvl );
		GET_POLL_LOCK(ipri, handler->priority, plvl);
		if( dummy->priority < INTTIMER ) {
			i_clrlvlmap( dummy->priority, dummy->level );
		}
		i_data.i_poll[plvl].poll = (struct intr *)NULL;
		REL_POLL_LOCK(ipri, plvl);
		xmfree( dummy, pinned_heap );
	}
	else {
		ulong	t_prilvl;	/* place to hold old level */

		assert( handler->priority < INTTIMER );

		/* Here again we may have to adjust the priority,
		 * see i_init().  No reason to check for BUS_PCI here
		 * since it is the only one that will have multiple
		 * priorities per level.
		 */
		/* Remove handler from list and unlock the poll_lock */
		GET_POLL_LOCK(ipri, handler->priority, plvl);
		handprev->next = intpoll->next;

		/* Find the end of list to see if priority needs adjusting */
		for(	; handprev->next != (struct intr *)NULL;
			handprev = handprev->next) ;

		/* priority must be adjusted */
		if( handprev->priority != dummy->priority ) {
		    ppda_ptr = PPDA;
		    i_clrlvlmap( dummy->priority,dummy->level);
		    t_prilvl = ppda_ptr->i_prilvl[dummy->priority] &
			PRILVL_MSK(dummy->level);
		    ppda_ptr->i_prilvl[dummy->priority] &= (~t_prilvl);
		    dummy->level = i_addlvlmap(handprev->priority, plvl);
		    dummy->priority = handprev->priority;
		    ppda_ptr->i_prilvl[dummy->priority] |= t_prilvl;
		}
		REL_POLL_LOCK(ipri, plvl);
	}

#ifdef _POWER_MP
	/*
	 * If the plvl list contains only mpsafe handlers and the one
	 * just removed is a funneled handler then adjust the hardware
	 * so this level is no longer steered to the mpmaster processor.
	 * Of course don't worry about this at all if this is not
	 * a MP machine.
	 */
	if( !(handler->flags & INTR_MPSAFE) &&
	    (i_data.i_poll[plvl].poll != (struct intr *)NULL) &&
	    __power_mp() )
	{
		/* get over dummy intr struct */
		intpoll = i_data.i_poll[plvl].poll->next;
		handprev = intpoll;
		do {
			if( !(intpoll->flags & INTR_MPSAFE) ) {
				handprev = (struct intr *)NULL;
				break;
			}
			intpoll = intpoll->next;
		} while ( intpoll != (struct intr *)NULL );

		if( handprev != (struct intr *)NULL )
		{
#ifdef _RS6K
		    if( __rs6k() )
			i_adjustplvl_ppc( handprev, plvl, INTR_UNFUNNEL );
#endif /* _RS6K */
#ifdef _RSPC
		    if( __rspc() )
			i_adjustplvl_rspc( handprev, plvl, INTR_UNFUNNEL );
#endif /* _RSPC */
		}
	}
#endif /* _POWER_MP */

	unlockl( &intsys_lock );
}

/*
 * NAME:  i_sched
 *
 * FUNCTION:  Schedule off-level processing.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by either a process or an
 *      interrupt handler.
 *
 *      It can page fault if called from a process on a pageable stack.
 *
 * NOTES:
 *	The following routine allows device drivers to schedule some of
 *	their work to be processed at a less-favored interrupt priority.
 *	Thus, if the interrupt handler routine for a device driver must
 *	do some time-consuming processing and this work does not need to
 *	be performed immediately, the processing can be scheduled
 *	OFF-LEVEL.  This service allows interrupt handlers to run as fast
 *	as possible, avoiding interrupt-processing delays and overrun
 *	conditions.
 *
 *	Each off-level interrupt priority has a circular list of requests
 *	waiting to be processed by the i_offlevel.  Entries are processed
 *	in a first in first out order. New requests are added to the
 *	tail of the list by i_sched and i_offlevel processes the requests
 *	starting at the head of the list. The list is as follows:
 *
 *	ppda->schedtail[ipri-INTOFFL0]
 *		|
 *		|	*---------------- . . . ----------------*
 *		|	|					|
 *		|	|	 tail		 head		|
 *		|	|	*-------*	*-------*	|
 *		*-------*----->	* next	* ----> * next	* ------*
 *				*	*	*	*
 *				*	*	*	*
 *				*-------*	*-------*
 *
 *	This list is circular so that entries can be added to the list
 *	without scanning the list. Entries must be added at interrupt
 *	priority INTMAX.
 *
 *      The I_SCHED bit in the flags field is used to tell if a handler
 *      is already scheduled. If it is set i_sched returns without
 *      doing anything. I_offlevel resets this bit when it removes
 *      the handler from the list. Device drivers that need to schedule
 *      a handler to run more than once will need to use different
 *      handler structures that point to the same handler routine.
 *
 *	The actual number of interrupt priorities is machine dependent.
 *	the i_machine.h file may define four unique
 *	interrupt priorities for the off-level handler or only one.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	disable_ints
 *	enable_ints
 *	SET_SOFT_INT
 */
void
i_sched(
	struct intr *handler)
{
	register int	oldmsr;		/* current msr value	*/
	register int	index;		/* index into the sched array */
	register int	ipri;		/* old priority	*/

	register struct ppda	*ppda_ptr;	/* ptr to my ppda */

	ipri = disable_ints();		/* enter critical section */

	/*
	 * Verify that the interrupt priority is valid.
	 */
	assert( handler->priority >= INTOFFL0 );
	assert( handler->priority <= INTOFFL3 );

	/*
	 * Insert the off-level request in the circular list
	 * for that off-level priority.
	 */

	ppda_ptr = PPDA;
#ifdef _POWER_MP
	/* 
	 * Since test_and_set() is working on an uint, bus_type has
	 * to be used as the parameter, and the right casting is applied
	 * to get the I_SCHED bit modified.
	 */
	if ( test_and_set((int *)&(handler->bus_type),(int)I_SCHED)) {
#else
	if (!( handler->flags & I_SCHED ))
	{
		/*
		 * turn on flag to ignore further i_sched calls
		 */
		handler->flags |= I_SCHED;
#endif		

		index = handler->priority - INTOFFL0;
		if ( ppda_ptr->schedtail[index] == (struct intr *)NULL )
			handler->next = handler;
		else
		{
			handler->next = ppda_ptr->schedtail[index]->next;
			ppda_ptr->schedtail[index]->next = handler;
		}
		ppda_ptr->schedtail[index] = handler;

		/*
		 * Schedule the off-level interrupt handler to be run.
		 */
		SET_SOFT_INT( handler->priority );
	}

	/* i_softpri will only be nonzero on the software managed model */
	if (clz32((ppda_ptr->i_softpri) << 16) < ipri) {
		i_dosoft();
	}

	enable_ints(ipri);			/* end critical section */

}

/*
 * NAME:  i_softoff
 *
 * FUNCTION:
 *	Software off-level handler that is called from i_poll when
 *	the off-level interrupt goes off.  It sets up the processor
 *	and calls i_softint to process all the off-level work.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called on an off-level interrupt priority
 *	in an interrupt execution environment.
 *
 *      It cannot page fault.
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:  INTR_SUCC
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_softint
 *	disable
 *	enable
 */
int
i_softoff(
	struct intr *handler)
{
	int	ipri;		/* priority upon entry */

#ifdef INTRDEBUG
	/* Trace it "SOFF" */
	mltrace( 0x534f4646, 0, 0, 0 );
#endif /* INTRDEBUG */

	ipri = i_disable( INTMAX );
	i_softint();
	i_enable( ipri );

	return( INTR_SUCC );
}

#ifdef _POWER_MP
/*
 * NAME:  i_hwassist_int
 *
 * FUNCTION:
 *	Software off-level/Inter Processor Interrupt handler that is called
 *	from i_poll when the off-level/mpc interrupt goes off.
 *	The handler can be called at the INTMAX or INTOFFLVL priorities
 *	At INTMAX, i_hwassist_int() processes the pending INTMAX MPC.
 *	At INTOFFLVL, i_hwassist_int() calls i_hwsoftint() to process all the
 *	off-level work and the pending INTOFFLVL MPC.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called on an INTMAX interrupt priority
 *	in an interrupt execution environment.
 *
 *      It cannot page fault.
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:  INTR_SUCC
 *
 * EXTERNAL PROCEDURES CALLED:
 *	PPDA
 *	i_reset_soft
 *	fetch_and_nop *	fetch_and_and
 */
int
i_hwassist_int(
	struct intr *handler)
{
	uint	mpcis;
	uint	index;
	struct ppda *ppda_ptr;
	extern uint mpc_max_index;
	extern uint	mpc_offlvl_mask;
	extern uint	mpc_timer_mask;
	extern uint	mpc_iodone_mask;

	ppda_ptr = PPDA;

	i_reset_soft(INTMAX);
	if(mpcis=fetch_and_nop(
		   (int *)(&(ppda_ptr->mpc_pend)))) {
	  for (index=0;index<=mpc_max_index;index++) {
	    if ((mpcis & (1<<index)) 
		&& (mpc_reg_array[index].pri==INTMAX)) {
	      fetch_and_and(
                (int *)(&(ppda_ptr->mpc_pend)),~(1<<index));
	      (mpc_reg_array[index].func)();
	    }
	  }
	}

	if (mpcis & mpc_offlvl_mask) {
	  if (mpcis &mpc_timer_mask)
	    ppda_ptr->i_softpri |= PRI_MASK(INTTIMER);
	  if (mpcis &mpc_iodone_mask)
	    ppda_ptr->i_softpri |= PRI_MASK(INTIODONE);
	}

	return( INTR_SUCC );
}
#endif /* _POWER_MP */

/*
 * NAME:  i_softint
 *
 * FUNCTION:
 *	Process each of the pending off-level priority lists until
 *	all are done or until the previous priority level is reached.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called on an off-level interrupt priority
 *	in an interrupt execution environment.
 *
 *	It is assumed the MSR(EE) bit is off upon entry
 *
 *      It cannot page fault.
 *
 * NOTES:
 *      This routine is the off-level scheduler.  It processes
 *      each priority in i_softis.  Can be called from an actual
 *      interrupt via i_poll() on a UP system and i_softoff() or directly
 *	from finish_interrupt().  i_softint() only processes off-level
 *      work if the previous mst->intpri is INTBASE and then it
 *      processes all off-level work.
 *
 * RETURN VALUE DESCRIPTION:  INTR_SUCC
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_offlevel
 *	iodone_offl
 *	GET_MY_PPDA
 *	clz32
 *	disable
 *	mtmsr
#ifdef _POWER_PC
 *	clock
#endif
 */
void
i_softint()
{
	int	softis_pri;	/* most favored off-level priority */
	struct ppda *ppda_ptr;	/* pointer to ppda struct */

	/* Verify that the interrupt priority is valid.  */
	ASSERT( CSA->prev != NULL );
	ASSERT( CSA->intpri == INTMAX );

	/* Get ppda pointer and pointer to priority variable in use */
	ppda_ptr = PPDA;

	ASSERT( ppda_ptr->i_softis != 0 );

#ifdef INTRDEBUG
	/* Trace it "SINT" */
	mltrace( 0x53494e54, ppda_ptr->i_softis, CSA->prev->intpri, 0 );
#endif /* INTRDEBUG */

	/*
	 * Process off=level only when returning to INTBASE and
	 * something to do.
	 */
	if(ppda_ptr->_csa->prev->intpri == INTBASE ) {
		while( ppda_ptr->i_softis ) {

			softis_pri = clz32(ppda_ptr->i_softis << 16);

			/* Turn off level */
			RESET_SOFTLVL( softis_pri );

			switch( softis_pri ) {

			    case INTTIMER:
#ifdef _POWER_PC
				if( __power_pc() ) {
					i_enable( INTTIMER );
					clock( (struct intr *)NULL );
					(void) i_disable( INTMAX );
				}
#endif /* _POWER_PC */
#ifdef _POWER_RS
				/* Should never be called on a RS machine */
				assert( ! __power_rs() )
#endif /* _POWER_RS */
				break;

			    case INTOFFL0:
			    case INTOFFL1:
			    case INTOFFL2:
			    case INTOFFL3:
 				i_offlevel( softis_pri, ppda_ptr );
				break;

			    case INTIODONE:
				i_enable( INTOFFLVL );
				iodone_offl();
				(void) i_disable( INTMAX );
				break;
			}
		}

		/*
		 * Clear the hardware assist
		 */
		CLR_SOFT_INT();
	}

	return;
}

/*
 * NAME:  i_softmod
 *
 * FUNCTION:
 *	Process all of the queued interrupts up to priority of the
 *	previous mst priority.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called in an interrupt environment from
 *	finish_interrupt().
 *
 *	It is assumed the MSR(EE) bit is off upon entry
 *
 *      It cannot page fault.
 *
 * NOTES:
 *      This routine is the queued interrupt handler for the
 *	software managed interrupt model.  It processes
 *      each queued priority in i_softpri.
 *
 * RETURN VALUE DESCRIPTION:  none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_enable
 *	i_disable
 *	i_offlevel
 *	iodone_offl
 *	i_poll
 *	PPDA
 *	clz32
 *	disable
 *	RESET_SOFTPRI
 */
void
i_softmod()
{
	int	q_pri;			/* most favored queued priority */
	int	plvl;			/* processor interrupt level	*/
	int	slvl;			/* source level			*/
	volatile struct ppcint *intr;	/* pointer to hardware registers*/
	struct ppda *ppda_ptr;		/* pointer to ppda struct	*/
#ifdef _POWER_MP
	extern uint mpc_max_index;
	extern uint mpc_timer_mask;
	extern uint mpc_iodone_mask;
	uint	mpcis;
	uint	index;
#endif

	ppda_ptr = PPDA;
	/* Verify that the interrupt priority is valid.  */
	ASSERT( ppda_ptr->_csa->prev != NULL );
	ASSERT( ppda_ptr->_csa->intpri == INTMAX );

	/* Get ppda pointer and pointer to priority variable in use */
	ASSERT( ppda_ptr->i_softpri != 0 );

#ifdef INTRDEBUG
	/* Trace it "SMOD" */
	mltrace(0x534d4f44, ppda_ptr->i_softpri,
		ppda_ptr->_csa->prev->intpri, 0);
#endif /* INTRDEBUG */

	intr = (volatile struct ppcint *)(ppda_ptr->intr);

	while((q_pri=clz32(ppda_ptr->i_softpri << 16))<INTBASE){

	    /* Only process more favored hardware interrupts, process
	     * decrementer, off-levels, and iodone priorities only at
	     * INTBASE
	     */
	    if( ((q_pri < INTTIMER)&&(q_pri < ppda_ptr->_csa->prev->intpri)) ||
	        (ppda_ptr->_csa->prev->intpri == INTBASE) ) {

		/*
		 * if HW interrupt determine processor level and call
		 *    i_poll() to to the work.
		 * if decrementer, off-level, or iodone then handle it here
		 */
		if( q_pri < INTTIMER ) {
		    while( ppda_ptr->i_prilvl[q_pri] ) {
			slvl = clz32( ppda_ptr->i_prilvl[q_pri] );
			ppda_ptr->i_prilvl[q_pri] &= ~PRILVL_MSK( slvl );
			plvl = i_lvlmap[q_pri][slvl];
			i_poll_soft( plvl );
		    }

		    /*
		     * Turn off the queued priority bit (current priority).
		     * Now find next highest priority pending interrupt and
		     * lower CPPR to that priority.  Only write priorities
		     * that could have come from interrupt hardware.
		     *
		     */
		    RESET_SOFTPRI(q_pri);
#ifdef _RS6K
		    if (__rs6k()) {
			q_pri = clz32(ppda_ptr->i_softpri << 16);
			if (q_pri >= INTTIMER) {
#ifdef _POWER_MP
			    if (q_pri > INTOFFLVL) {
			      if ( cppr_random == 0 )
				q_pri = INTBASE;
			      else
				q_pri = INTBASE + (ppda_ptr->syscall & 0xF); 
			    } else {
				q_pri = INTOFFLVL;
			    }
#else
			    q_pri = INTBASE;
#endif
			}
			intr->i_cppr = q_pri;
			__iospace_sync();
		    }
#endif /* _RS6K */

		} else {
		    /* turn off the queued priority bit */
		    RESET_SOFTPRI( q_pri );

		    switch( q_pri ) {

			    case INTTIMER:
				ppda_ptr->_csa->intpri = INTTIMER;
				enable();
				clock( (struct intr *)NULL );
#ifdef _POWER_MP
				if ((mpcis=ppda_ptr->mpc_pend)&mpc_timer_mask) {
				  for (index=0;index<=mpc_max_index;index++) {
				    if ((mpcis & (1<<index)) && 
				      (mpc_reg_array[index].pri==INTTIMER)) {
					fetch_and_and(
					   (int *)(&(ppda_ptr->mpc_pend)),
						     ~(1<<index));
					(mpc_reg_array[index].func)();
				    }
				  }
				}
#endif
				(void) disable();
#ifdef _POWER_RS
#ifdef INTRDEBUG
				/* Should never be called on a RS machine */
				assert( ! __power_rs() )
#endif /* INTRDEBUG */
#endif /* _POWER_RS */
				break;

			    case INTOFFL0:
			    case INTOFFL1:
			    case INTOFFL2:
			    case INTOFFL3:
				/* i_offlevel does its own enabling &
				 * disabling
				 */
				ppda_ptr->_csa->intpri = INTMAX;
				i_offlevel( q_pri, ppda_ptr );
				break;

			    case INTIODONE:
				ppda_ptr->_csa->intpri = INTOFFLVL;
				enable();
				iodone_offl();
#ifdef _POWER_MP
				if ((mpcis=ppda_ptr->mpc_pend)&mpc_iodone_mask) {
				  for (index=0;index<=mpc_max_index;index++) {
				    if ((mpcis & (1<<index)) && 
				      (mpc_reg_array[index].pri==INTIODONE)) {
					fetch_and_and(
					   (int *)(&(ppda_ptr->mpc_pend)),
						     ~(1<<index));
					(mpc_reg_array[index].func)();
				    }
				  }
				}
#endif
				(void) disable();
				break;
		    }

		}

#if defined(_POWER_MP) && defined(_RS6K)
		if (__rs6k()) {
		    if (!ppda_ptr->i_softpri) {
		      if ( cppr_random == 0 )
		        intr->i_cppr = INTBASE;
		      else
		        intr->i_cppr = INTBASE + (ppda_ptr->syscall & 0xF);
		      __iospace_sync();
		    } 
		}
#endif /* _POWER_MP && _RS6K */

	  } else
		/* Can't do anything - just return */
		break;
	}
	return;
}

/*
 * NAME:  i_iodone
 *
 * FUNCTION:
 *	This routine generates an interrupt for the off-level
 *	I/O done routine.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called by either a process or interrupt
 *	handler.
 *
 *      It does not page fault except on the stack when called under
 *	a process.
 *
 *	This routine is provided so that iodone can
 *	perform a soft interrupt without including intr_hw.h.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	SET_SOFT_INT
 */
void
i_iodone()
{
	register struct ppda	*ppda_ptr;	/* ptr to my ppda */

#ifdef DEBUG
	ASSERT( CSA->intpri == INTMAX );
#endif
	ppda_ptr = PPDA;
	SET_SOFT_INT( INTIODONE );
}

#ifdef _POWER_MP

/*
 * NAME:  ipoll_excl_serv
 *
 * FUNCTION:
 *	This routine is a MPC service called by i_hwassist_int.
 *	This routine acknowledge the MPC by setting one bit in
 *	the global variable ipoll_excl_cpu. Then it waits for
 *	ipoll_excl_cpu to be back to a zero value before returning.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called by an interrupt handler.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	disable
 *	enable
 *	fetch_and_add
 *	fetch_and_nop
 */
void
ipoll_excl_serv()
{

  	disable();
	(void) fetch_and_add( (int *)(&ipoll_excl_cpu), 1);
  	while ( !fetch_and_nop ( (int *)(&ipoll_excl_cpu)));
	enable();
  	return;
}

/*
 * NAME:  ipoll_excl_wait
 *
 * FUNCTION:
 *	This function sends a MPC to the other processors and
 *	waits for them to acknowledge this event. This is done
 *	by spining on ipoll_excl_cpu to have NCPU bits set.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called by either a process or interrupt
 *	handler.
 *
 *      It does not page fault except on the stack when called under
 *	a process.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	mpc_send
 *	fetch_and_add
 *	fetch_and_nop
 */
void
ipoll_excl_wait()
{
  	if ( (NCPUS() != 1) && __power_mp() ) {
	  mpc_send( MPC_BROADCAST, ipoll_mpc_msg);
	  (void) fetch_and_add( (int *)(&ipoll_excl_cpu), 1);
	  while ( fetch_and_nop( (int *)(&ipoll_excl_cpu)) != NCPUS() ) ;
	}
	return;
}

/*
 * NAME:  ipoll_excl_clear
 *
 * FUNCTION:
 *	This function clears ipoll_excl_cpu to resume activities
 *	on all processors.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called by either a process or interrupt
 *	handler.
 *
 *      It does not page fault except on the stack when called under
 *	a process.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	fetch_and_and
 */
void
ipoll_excl_clear()
{
  
	if ( (NCPUS() != 1) && __power_mp() ) {
	  (void) fetch_and_and( (int *)(&ipoll_excl_cpu), 0);
	}
	return;
}
#endif
