static char sccsid[] = "@(#)24	1.4  src/bos/usr/lib/asw/mpqp/autoresp.c, ucodmpqp, bos411, 9428A410j 8/23/93 13:18:52";

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS: 
 *	start_ar	: Start Auto-response mode.
 *	stop_ar		: Stop Auto-response mode
 *	add_to_arq	: Add rqe to arq
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
 *
 * FUNCTION: start_ar - Receives frames and compares them with the
 *	     address and control
 *           bytes given by the device driver.  If they match, a frame is
 *           transmitted with the same address byte and the given control   
 *           byte.  This code will exit auto-response if the receive timer
 *           expires, a frame with data is received, or a transmit or link
 *           error occurs.  If it exits, it will stay in receive mode.  An
 *           exit status is sent to the driver as receive status to maintain
 *           synchronization with any receive data that may have caused the
 *           termination of auto-response.
 *
 *	     stop_ar - When issued by the device driver, takes the port out
 *	     of auto-response mode.
 *
 */

#include "mpqp.def"
#include "mpqp.typ"
#include "mpqp.ext"
#include "mpqp.pro"
#include "mpqp.h"
#include "portcb.def"
#include "portcb.typ"
#include "iface.def"

extern void	add_to_arq();

start_ar ( t_tx_common		*p_tx_comn,
	   unsigned char far	*p_tx_bfr )

{
register volatile t_pcb	*p_pcb;                                            
unsigned char		a_r_exit=0;
unsigned char		port;
t_rqe			local_rqe;
rx_buf_hdr far		*p_rx_buf_hdr;
unsigned short		ar_rx_timer;
unsigned char		n_cmd_blk;
cmd_t			*p_cmd = (cmd_t *)p_tx_comn;
#define	RQE_POLL_FRAME	RQE_CPS_ACK

	FOOTPRINT( 'A', 's', 'a' );
	/* extract port number from command block */
	port = p_tx_comn->_port;
	n_cmd_blk = (cmd_blk *)p_tx_comn - &cmdblk[0];

	/* copy auto response parameters into port cntl block */
	p_pcb = &pcb[port];
	p_pcb->auto_resp = ( (cmd_blk *)p_tx_comn )->p._aresp;

	/* build rqe for command acknowledgement */
	local_rqe.f.rtype = RQE_CMD_ACK;
	local_rqe.f.sequence = p_tx_comn->_sequence;
	local_rqe.f.port = port;
	local_rqe.f.status = p_tx_comn->_type;

	/* Build transmit short in cmd blk from start auto resp cmd */
	p_cmd->type = TX_LONG;
	p_cmd->length = 2;
	p_cmd->flags = TX_NO_FREE;
	p_cmd->card_addr = (unsigned char far *)p_cmd->cs.data;
	p_cmd->cs.data[0] = p_pcb->auto_resp._addr;
	p_cmd->cs.data[1] = p_pcb->auto_resp._txctl;

	/* Start receive timer.  (must build rx timer from chars) */
	ar_rx_timer = ( p_pcb->auto_resp._thi << 8 ) + p_pcb->auto_resp._tlo;
	start_duscc_timer( p_pcb, ar_rx_timer, AR_RX_TIMER );

	/* send rqe to ack command */
	add_to_arq( local_rqe );
	local_rqe.f.sequence = 0;

	sleep_enable( p_pcb );
	/* While not exit condition */
	while (1)
	{

       		/* sleep waiting on receive data or error condition */
       		p_pcb = sleep( p_pcb );
		
       		/* if 2 byte frame */
        	local_rqe = p_pcb->sleep_rqe;
        	if ( local_rqe.f.rtype == RQE_POLL_FRAME )
		{
			/* get addr of rx bfr, sequence = bfr number */
        		p_rx_buf_hdr = ( rx_buf_hdr far *)RECV_BUFFER( local_rqe.f.status );
			/* if frame matches */
			if ( *( (unsigned char far *)p_rx_buf_hdr +
			   p_rx_buf_hdr->offset+ 1) == p_pcb->auto_resp._rxctl )
			{
				/* restart timer */
        			start_duscc_timer( p_pcb, ar_rx_timer, AR_RX_TIMER );

				/* put transmit command on port cmd queue */
				queue_writeb( p_pcb->cmd_q, n_cmd_blk );
				work_q [PCQ_WORK] |= p_pcb->mask_on;
				queue_writeb(&rx_free, (unsigned char)local_rqe.f.status);
				continue;
			}
			else
			{
				local_rqe.f.rtype = RQE_RX_DMA;
				ReRoute [p_pcb->port_no] = 0xFF;
				stop_duscc_timer( p_pcb);
				break;
			}
		}

		/* else port status */
		else if (local_rqe.f.rtype == RQE_STAT_UNSOL)
		{
        		/* if timer expired */
        		if ( local_rqe.f.sequence == AR_RX_TIMER )
				/* put rqe in arq */
  				break;

			/* if DSR Dropped or X.21 network error */ 
			if ((local_rqe.f.sequence == DSR_OFF) ||
		    		(local_rqe.f.sequence == X21_ZERO) )
				/* send up rqe, exit auto-response */
				stop_duscc_timer( p_pcb );
				break;
		}
		else if (local_rqe.f.rtype == RQE_CMD_ACK)
		{
	    		if (local_rqe.f.status == STOP_AR)
				stop_duscc_timer( p_pcb );
				break;
		}

    		/* if not an rqe we're waiting for, send rqe up to driver */
		if ( local_rqe.f.rtype != RQE_POLL_FRAME )
    			add_to_arq( local_rqe );
	}
	sleep_disable(p_pcb);

	/* if out of while loop, exit condition must have been met
	write adapter rqe to arq, interrupt host if empty or full (error) */

	/* reset auto-response receive timer */
	stop_duscc_timer( p_pcb ); 
	p_pcb->timer_type = 0;

	/* add rqe, type = receive, buffer number */
	if ( local_rqe.f.rtype == RQE_RX_DMA )
		add_rqe( p_pcb, local_rqe.val );
	else
		add_to_arq( local_rqe );

	free_cmdblk( (cmd_blk *)p_tx_comn - cmdblk);
	FOOTPRINT( 'A', 's', 'z' );
	return(~PCQ_LOCK);	
}         

int stop_ar ( cmd_blk		*p_cmd,
	   unsigned char far	*p_tx_bfr )
{
	t_rqe		q_rqe;
	FOOTPRINT( 'A', 'p', 'a' );

	/* build rqe to ack command */
	/* note: function performed in start_ar */
	q_rqe.f.rtype    = RQE_CMD_ACK;
	q_rqe.f.status   = p_cmd->_type;
	q_rqe.f.sequence = p_cmd->_sequence;
	
	/* send out rqe */
	add_rqe( &pcb [p_cmd->_port], q_rqe.val );
	free_cmdblk( (cmd_blk *)p_cmd - cmdblk);

	FOOTPRINT( 'A', 'p', 'z' );
	return(~PCQ_LOCK);
}

	
void add_to_arq ( t_rqe 	local_rqe )
{
int	rc;

	/* add rqe, type = receive, buffer number */
	disable();
	queue_writel( &arqueue, local_rqe.val );
	/* if queue was empty, generate host interrupt */
	host_intr( TREG_ARQ );
	enable();

	return;
}
