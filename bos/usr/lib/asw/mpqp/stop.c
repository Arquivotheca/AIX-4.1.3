static char sccsid[] = "@(#)42	1.6  src/bos/usr/lib/asw/mpqp/stop.c, ucodmpqp, bos411, 9428A410j 12/21/92 15:49:03";

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS: 
 *	stop		: Frees up DMA resources, initiates disconnect,
 *			  turns off drivers and receivers.
 *	stop_p		: Calls stop and queues rqe when stop port issued.
 *	flush_port	: Removes intervening commands when stop is about
 * 		 	  to be issued from the device driver.
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
#include "mpqp.typ"
#include "portcb.def"
#include "portcb.typ"
#include "mpqp.ext"
#include "mpqp.pro"
#include "mpqp.h"
#include "iface.def"

extern	void 	dte_clear_request(volatile t_pcb	*p_pcb);

extern	void	call_term(volatile t_pcb		*p_pcb);

extern	void	clr_x21_lsd(volatile t_pcb		*p_pcb);

/****************************
  Port Shutdown common logic
 ****************************/

void stop ( register volatile t_pcb	*p_pcb )
{
	register ioptr		scc;
	register int		rqeval;
	t_rqe			rqe;

	scc = p_pcb->scc_base;
 
	/* Stop timer */
	p_pcb->timer_type = 0;
	STOP_TIMER(p_pcb);
	stop_duscc_timer(p_pcb);
	set_failsafe(p_pcb->port_no,0);

	/* Disable all sources of interruption */
	mi_enable( p_pcb, 0 );

	/* Reset transmitter and receiver */
	WRITE_CCR( RESET_TX );
	WRITE_CCR( RESET_RX );

	/* Both receive channels may be setup and one running */
	out16( p_pcb->rx_dma_0, 0 );
	out16( p_pcb->rx_dma_1, 0 );
	if ( p_pcb->rx_curr != NO_BUFNO )
	{
		FOOTPRINT( 'S', 'S', 'u' );		/* SSu */
		queue_writeb( &rx_free, p_pcb->rx_curr );
		p_pcb->rx_curr = NO_BUFNO;
	}
	if ( p_pcb->rx_next != NO_BUFNO )
	{
		FOOTPRINT( 'S', 'S', 'v' );		/* SSv */
		queue_writeb( &rx_free, p_pcb->rx_next );
		p_pcb->rx_next = NO_BUFNO;
	}
	p_pcb->rxchan = 0;

	if ( p_pcb->txbuf0 != NO_BUFNO )
	{
    		FOOTPRINT( 'S', 'S', 't' );		/* SSt */
		p_pcb->txbuf0 = NO_BUFNO;		/* Free used channel */
	}
	if ( p_pcb->txbuf1 != NO_BUFNO )
	{
    		FOOTPRINT( 'S', 'S', 'w' );		/* SSw */
		p_pcb->txbuf1 = NO_BUFNO;		/* Free used channel */
	}
	p_pcb->txchan = 0;

	/* Port 3 has one bit used for Port 0.  The 232 Enable line */
	/* must remain in its present state across the termination, */
	/* as it gates Port 0 TxData.				    */

	/* Clean up the port CIO */
	if ( p_pcb->port_no == 3 )
		p_pcb->cio_reg.data &= ENABLE_BLK;

	p_pcb->cio_reg.data |= DISABLE_DTR | DISABLE_HRS | DISABLE_V35;
	set_cio( p_pcb );

	/* Return all receive buffers remaining in the port 	*/
	/* response queue.					*/

	ReRoute[ p_pcb->port_no ] = FALSE;
	while (( rqeval = queue_readl( &port_rq[ p_pcb->port_no ] )) >= 0 )
	{
	    rqe.val = rqeval;
	    if (( rqe.f.rtype == RQE_RX_NODMA ) || 
		( rqe.f.rtype == RQE_RX_DMA )   ||
		( rqe.f.rtype == RQE_CPS_ACK ))
	    {
	        queue_writeb( &rx_free, rqe.f.status );
	    }
	}
	/* Turn off RTS */

	SET_OMR( OMR & OM_RTS_OFF );
	return;
}

/**************************
  Driver Stop Port Command
 **************************/

stop_p ( 
	cmd_blk		        *p_cmd,
	unsigned char far	*p_buf )
{
	volatile t_pcb	*p_pcb;
	t_rqe		q_rqe;
	unsigned char	cmd;

	disable();
	FOOTPRINT( 'S', 'S', 'a' );		/* SSa */
	p_pcb = &pcb [p_cmd->_port]; 

	switch ( p_pcb->phys_link ) {

	    case PCB_PL_X21:

		FOOTPRINT( 'S', 'S', 'x' );	/* SSx */

	        /* if no state machine running */
	        if ( !SleepEn[p_pcb->port_no] )
		{
			/* if in data transfer phase */
                	if (p_pcb->port_state == DATA_TRANSFER )
			{
				/* clear call */
				tx_abort( p_pcb, STOP_PORT );
				sleep_enable( p_pcb );
 
				if ( p_pcb->start_parm & PCB_SP_LEASED)
				{
					/* exit data phase */
					clr_x21_lsd( p_pcb );
				}
				else
				{
					dte_clear_request( p_pcb );
					call_term( p_pcb );
				}
 
				sleep_disable( p_pcb );
			}
			stop( p_pcb );
	        }
		break;

	    default:
		FOOTPRINT( 'S', 'S', 'y' );	/* SSy */

		stop( p_pcb );
		break;
	}

	/* Build response queue element, Command Acknowledgement */

	q_rqe.f.rtype = RQE_CMD_ACK;
	q_rqe.f.status = STOP_PORT;
	q_rqe.f.sequence = 0;

	add_rqe( p_pcb, q_rqe.val );

	free_cmdblk (( cmd_blk *)p_cmd - cmdblk );

	FOOTPRINT( 'S', 'S', 'z' );		/* SSz */
	enable();
	return ( ~PCQ_LOCK );
}

/***************************
  Driver Flush Port Command
 ***************************/

flush_p ( register cmd_blk	*p_cmd )
{
	return;
}
