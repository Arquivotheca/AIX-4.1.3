static char sccsid[] = "@(#)46	1.5  src/bos/usr/lib/asw/mpqp/x21lsd.c, ucodmpqp, bos411, 9428A410j 9/11/91 18:16:14";
/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS:
 *	x21leased		: implements X.21 leased state machine.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "mpqp.def"
#include "mpqp.def"
#include "mpqp.typ"
#include "mpqp.ext"
#include "mpqp.pro"
#include "mpqp.h"
#include "portcb.def"
#include "portcb.typ"
#include "iface.def"
 
/* x21 leased state machine */
/* external data structures */
extern		unsigned short	x21_Stat;	/* current state of PAL's */
extern		unsigned short	tscc_val[NUM_PORT];
extern		unsigned short	tscc_typ[NUM_PORT];

X21leased(register volatile t_pcb	*p_pcb)
{


	FOOTPRINT( 'X', 'l', 'E' );			/* XlE */
	/* decode current state of the network */
	p_pcb->x21_state = SIGNAL_RDY;
	
	p_pcb->port_state = CALL_ESTAB;
	/* signal 0 for 24 bit times */
	p_pcb->cio_reg.data &= DISABLE_422;
	set_cio(p_pcb);
	xmt_24_start( p_pcb );

	while (p_pcb->x21_state == SIGNAL_RDY)
	{	
		FOOTPRINT( 'X', 'l', 's' );			/* Xls */
		sleep( p_pcb );
		if (p_pcb->sleep_rqe.f.rtype == RQE_STAT_UNSOL)
		{
			if ( (p_pcb->sleep_rqe.f.sequence == X_24B_TIMER ) ||
			   (p_pcb->sleep_rqe.f.sequence == X21_INDICATE) )
			{
				/* signal ready */
				p_pcb->cio_reg.data |= ENABLE_422;
				set_cio(p_pcb);
				xmt_24_start( p_pcb );
				p_pcb->x21_state = READY;

				if (x21_Stat && CIO_X21_IA)
					p_pcb->x21_state = SIGNAL_RDY_CTL;
					
			}
		}
		else if (p_pcb->sleep_rqe.f.rtype == RQE_CMD_ACK) 
		{
			if (p_pcb->sleep_rqe.f.status == STOP_PORT)
			{
				stop_duscc_timer(p_pcb);
				p_pcb->start_rqe.f.rtype = RQE_CMD_ACK;
				p_pcb->start_rqe.f.status = STOP_PORT;
				p_pcb->start_rqe.f.sequence = 0;
				p_pcb->port_state = STATE_EXIT;
				return;
			}
		}
	}
					
	while ( ( p_pcb->x21_state == READY ) ||
		( p_pcb->x21_state == SIGNAL_RDY_CTL ) )
	{
		FOOTPRINT( 'X', 'l', 'r' );			/* Xlr */
		sleep( p_pcb );
		if ( p_pcb->sleep_rqe.f.rtype == RQE_STAT_UNSOL )
		{
			switch( p_pcb->sleep_rqe.f.sequence )
			{
				case X_24B_TIMER:
					/* drive control for data phase */
					OUT08(ENREG,IN08(ENREG) | ENR_S_X21_CA);

					if (p_pcb->x21_state == READY)
					{
						/* wait for indicate */
						start_duscc_timer( p_pcb,
						  X_LSD_TIMEOUT, X_LSD_TIMER );
					}
					else /* indicate already on */
					{
						stop_duscc_timer( p_pcb );
						p_pcb->x21_state = DATA_XFER;
						p_pcb->port_state=DATA_TRANSFER;
						return;
					}
					break;
						
				case X21_ZERO:
					p_pcb->x21_state = READY ;
					break;
				case X21_INDICATE:
					stop_duscc_timer( p_pcb );
					p_pcb->x21_state = DATA_XFER;
					p_pcb->port_state=DATA_TRANSFER;
					break;
				case X_LSD_TIMER:
					stop_duscc_timer(p_pcb);
					p_pcb->start_rqe.f.sequence=X_LSD_TIMER;
					p_pcb->port_state = STATE_EXIT;
					clr_x21_lsd( p_pcb );
					break;
				default:
					break;		

		    	}
		}
		else if (p_pcb->sleep_rqe.f.rtype == RQE_CMD_ACK) 
		{
			if (p_pcb->sleep_rqe.f.status == STOP_PORT)
			{
				stop_duscc_timer(p_pcb);
				p_pcb->start_rqe.f.rtype = RQE_CMD_ACK;
				p_pcb->start_rqe.f.status = STOP_PORT;
				p_pcb->start_rqe.f.sequence = 0;
				p_pcb->port_state = STATE_EXIT;
				clr_x21_lsd( p_pcb );

			}
		}	
	}	
	return(0);
} 

clr_x21_lsd( register volatile t_pcb	*p_pcb )
{
	FOOTPRINT( 'X', 'l', 'c' );				/* Xlc */
	/* clear an x.21 leased call by turning of control,
	   signalling ready, then uncontrolled not ready */

	OUT08( ENREG, IN08( ENREG ) & ENR_C_X21_CA );
	/* signal 1 for 24 bit times */
	p_pcb->cio_reg.data |= ENABLE_422;
	set_cio(p_pcb);
	xmt_24_bits( p_pcb );

	/* signal 0 for 24 bit times */
	p_pcb->cio_reg.data &= DISABLE_422;
	set_cio(p_pcb);
	xmt_24_bits( p_pcb );
	p_pcb->x21_state = CALL_TERM;
	return;
}

xmt_24_bits( volatile t_pcb	*p_pcb)
{
	FOOTPRINT( 'X', 'l', 'x' );				/* Xlx */
	xmt_24_start( p_pcb );
	sleep( p_pcb );
	while ( !( p_pcb->sleep_rqe.f.rtype == RQE_STAT_UNSOL ) &&
		 ( p_pcb->sleep_rqe.f.sequence == X_24B_TIMER ) )
		sleep( p_pcb );
}

/* if other things can happen while timer is running that need to be checked,
   xmt_24_start is used.  This allows the while loop conditions to be tailored
   to the particular need of that state.
*/

xmt_24_start( volatile t_pcb	*p_pcb)
{
	FOOTPRINT( 'X', 'l', 't' );				/* Xlt */

	out08( SCC_0A_BASE + SCC_CC, CC_PRESET_PR_CT );
	tscc_val[0] = 1;
	tscc_typ[0] = X_24B_TIMER;

	/* wait for timer to pop */
	p_pcb->sleep_rqe.val = 0;
	out08( SCC_0A_BASE + SCC_CC, CC_START_CT );

	return(0);

}
