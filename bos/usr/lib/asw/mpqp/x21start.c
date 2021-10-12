static char sccsid[] = "@(#)48	1.8  src/bos/usr/lib/asw/mpqp/x21start.c, ucodmpqp, bos411, 9428A410j 5/26/92 11:19:40";
/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS:
 *	x21switched		: Starts state machine by testing whether
 *				  network is already in ready state.
 *	call_establishment	: implements X.21 circuit switched state
 *				  machine
 *	call_term		: If error condition happens, switched to
 *				  call termination state machine.
 *	dte_clear_request	: Initiates clear sequence, usually after 
 *				  timeout or stop command received.
 *	dte_clear_confirm	: Responds to clear from the network.
 *	x21_rcv_data		: Processes call progress signals and
 *				  initiates retries.
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
#include "mpqp.ext"
#include "mpqp.pro"
#include "mpqp.h"
#include "portcb.def"
#include "portcb.typ"
#include "iface.def"

/* x21 adapter state machine                 */
/* external data structures */
extern		unsigned short	x21_Stat;	/* current state of PAL's */
extern 		t_x21_ctl	x21_ctl;	/*global x21 control structure*/
extern		unsigned short	tscc_val[NUM_PORT];	/* value of scc timer */
extern		unsigned short	tscc_typ[NUM_PORT];	/* type of scc timer */
/* external routines */

extern	void	call_establishment(volatile t_pcb	*p_pcb,
	       			cmd_t			*p_cmd); 

extern	void	call_term(volatile t_pcb		*p_pcb);

extern	void 	dte_clear_request(volatile t_pcb	*p_pcb);

extern	void 	dte_clear_confirm(volatile t_pcb	*p_pcb);

extern	void	x21_rcv_data(volatile t_pcb		*p_pcb);


extern 	void	add_to_arq ( t_rqe 	local_rqe );

extern	void	start_duscc_timer( t_pcb *p_pcb,
				unsigned short	timer_val,
				unsigned char	timer_type);

x21switched(register volatile t_pcb	*p_pcb,
		 cmd_t			*p_cmd) 
{
int		r_u;		/* return code for q_add_rqe */
unsigned char	cmd_num;	/* cmd block number */


	FOOTPRINT( 'X', 's', 'e' );				/* Xse */
	/* decode current state of the network */
	p_pcb->port_state = CALL_ESTAB;

	/* if call out, prepare transmit command */
	if ( p_pcb->start_parm & PCB_SP_CALL )
	{
		/* if buffer has dial data */
		if (x21_ctl.sel_lgt != 0)
		{
			FOOTPRINT( 'X', 's', 't' );		/* Xst */
			/* Build transmit in start cmd blk */
			p_cmd->type = TX_LONG;
			p_cmd->length = x21_ctl.sel_lgt;
			p_cmd->flags = TX_NO_FREE;
			p_cmd->card_addr = (unsigned char far *)
			  x21_ctl.sel_sig;
			Ascii_To_Op( p_cmd->card_addr, p_cmd->length );
		}
	}
	
	p_pcb->x21_state = SIGNAL_RDY;

	while ((p_pcb->port_state != STATE_EXIT) && 
               (p_pcb->port_state != DATA_TRANSFER))
	{
		/* stay in while until start completes */
		if (p_pcb->port_state == CALL_ESTAB)
			call_establishment(p_pcb, p_cmd);
		if (p_pcb->port_state == CALL_TERM)
			call_term(p_pcb);
	}
	FOOTPRINT( 'X', 's', 'x' );				/* Xsx */
}


void	call_establishment(register volatile t_pcb 	*p_pcb,
		           cmd_t			*p_cmd) 
{
unsigned char	n_cmd_blk;	/* command block number 	*/


	FOOTPRINT ( 'X', 'c', 'e' );				/* Xce */
	if ( p_pcb->x21_state == SIGNAL_RDY )
	{
		p_pcb->x21_state = READY;
		/* signal 0 for 24 bit times */
		out08(SCC_0A_BASE + SCC_CC, DISABLE_TX);
		p_pcb->cio_reg.data &= DISABLE_422;
		set_cio(p_pcb);
		xmt_24_start( p_pcb );

	}

	while (p_pcb->x21_state == READY)
	{
		FOOTPRINT ( 'X', 'c', 'r' );			/* Xcr */
		p_pcb = sleep(p_pcb);
		if ( p_pcb->sleep_rqe.f.rtype == RQE_STAT_UNSOL)
		{
			switch(p_pcb->sleep_rqe.f.sequence)
			{
			case X21_PLUS:
				FOOTPRINT ( 'X', 'c', 'p' );	/* Xcp */
				/* if buffer has dial data */
				if (x21_ctl.sel_lgt != 0)
				{
					p_pcb->x21_state = DTE_WAITING;
					start_duscc_timer(p_pcb, XT2_TIMEOUT,
								 XT2_TIMER);

		    			/* put transmit on port cmd queue */
					n_cmd_blk =
					      (cmd_blk *)p_cmd - &cmdblk[0];
		   			queue_writeb( p_pcb->cmd_q, n_cmd_blk );
		    			work_q [PCQ_WORK] |= p_pcb->mask_on;
				}
				else /* direct call */
				{
					FOOTPRINT ( 'X', 'c', 'D' ); /* XcD */
					p_pcb->x21_state = DTE_WAITING;
					p_pcb->timer_type = XT2_TIMER;
					start_duscc_timer(p_pcb, XT2_TIMEOUT,
								 XT2_TIMER);
					p_pcb->cio_reg.data |= ENABLE_422;
					set_cio(p_pcb);
				}
				break;

			case X21_BEL:
				FOOTPRINT ( 'X', 'c', 'b' ); 	/* Xcb */
				/* if set up for answer */
		   		if (! (p_pcb->start_parm & PCB_SP_CALL))
				{
					p_pcb->x21_state = DTE_WAITING;
					start_duscc_timer(p_pcb,XT4_TIMEOUT,
								XT4_TIMER);
					/* c = on */
					out08(ENREG,(in08(ENREG)|ENR_S_X21_CA));
				}
				/* ignore bel for outgoing call */
				break;

			case X21_ZERO:
				FOOTPRINT ( 'X', 'c', 'z' ); 	/* Xcz */
				dte_clear_confirm(p_pcb);
				break;

			case XT1_TIMER:
				FOOTPRINT ( 'X', 'c', 't' ); 	/* Xct */
				p_pcb->start_rqe.f.sequence =
					 p_pcb->sleep_rqe.f.sequence;
				dte_clear_request( p_pcb );
				break;

			case X_24B_TIMER:
				FOOTPRINT ( 'X', 'c', '2' ); 	/* Xc2 */
				/* if finished with UNR */
				if ( !(p_pcb->cio_reg.data & ENABLE_422 ))
				{

					/* signal ready */
					p_pcb->cio_reg.data|=ENABLE_422;
					set_cio(p_pcb);

					/* if this is a out going call */
					if (p_pcb->start_parm & PCB_SP_CALL)
						xmt_24_start( p_pcb );
				}	
				else /* done signalling ready (call out only) */
				{
		   			if ( p_pcb->start_parm & PCB_SP_CALL)
					{
						FOOTPRINT ('X','c','c');/*Xcc */
						start_duscc_timer( p_pcb, 
						  XT1_TIMEOUT, XT1_TIMER );
						p_pcb->cio_reg.data &= 
							DISABLE_422;
						set_cio(p_pcb);
						out08( ENREG, in08(ENREG)|								ENR_S_X21_CA);
					}
				}
				break;
			default:
				break;
			}
		}
		else if (p_pcb->sleep_rqe.f.rtype == RQE_CMD_ACK) 
		{
			if (p_pcb->sleep_rqe.f.status == STOP_PORT)
			{
				FOOTPRINT ('X','c','s');	/*Xcs */
				stop_duscc_timer(p_pcb);
				p_pcb->start_rqe.f.rtype = RQE_CMD_ACK;
				p_pcb->start_rqe.f.status = STOP_PORT;
				p_pcb->start_rqe.f.sequence = 0;
				p_pcb->port_state = STATE_EXIT;

				/* signal ready, then unc not ready */
				out08( ENREG, in08( ENREG ) & ENR_C_X21_CA );
				p_pcb->cio_reg.data |= ENABLE_422;
				set_cio(p_pcb);
				xmt_24_bits( p_pcb );

				/* force T to 0 */
				p_pcb->cio_reg.data &= DISABLE_422;
				set_cio(p_pcb);
				return;
			}
		}
	}
	while ( p_pcb->x21_state == DTE_WAITING )
	{
		FOOTPRINT ( 'X', 'c', 'd' );			/* Xcd */
		p_pcb = sleep( p_pcb );
		switch( p_pcb->sleep_rqe.f.rtype) 
		{
		case RQE_STAT_UNSOL: 

			switch(p_pcb->sleep_rqe.f.sequence)
			{
			case X21_INDICATE:
				FOOTPRINT ( 'X', 'c', 'i' );	/* Xci */
				/* start completed successfully! */
				stop_duscc_timer( p_pcb );
				p_pcb->port_state = DATA_TRANSFER;
				p_pcb->x21_state = DATA_XFER;
				break;

			case XT2_TIMER:
			case XT4_TIMER:
			case X_GR0_TIMER:
				FOOTPRINT ( 'X', 'c', 'g' );	/* Xcg */
				p_pcb->start_rqe.f.sequence =
					 p_pcb->sleep_rqe.f.sequence;
				dte_clear_request( p_pcb );
				break;

			case X_CPS_TIMER:
				FOOTPRINT ( 'X', 'c', '1' );	/* Xc1 */
				dte_clear_confirm( p_pcb );
				
			case X21_ZERO:
				FOOTPRINT ( 'X', 'c', 'y' );	/* Xcy */
				if (! p_pcb->retry_in_prog )
					dte_clear_confirm( p_pcb );
				break;
			default:
				break;
			}
			break;

		case RQE_CPS_ACK:
			FOOTPRINT ( 'X', 'c', 'C' );		/* XcC */
			if ( p_pcb->timer_type == XT2_TIMER )
				start_duscc_timer(p_pcb,XT2_TIMEOUT,XT2_TIMER);
			else
				start_duscc_timer(p_pcb,XT4_TIMEOUT,XT4_TIMER);

			x21_rcv_data( p_pcb );
			break;
		case RQE_CMD_ACK: 
			if ( p_pcb->sleep_rqe.f.status == STOP_PORT )
			{
				FOOTPRINT ( 'X', 'c', 'S' );	/* XcS */
				p_pcb->start_rqe.f.rtype = RQE_CMD_ACK;
				p_pcb->start_rqe.f.status = STOP_PORT;
				p_pcb->start_rqe.f.sequence = 0;
				dte_clear_request( p_pcb );
			}
			break;
		default:
			break;
		}
	}	
}

extern clrTC();

void	call_term(register volatile	t_pcb	*p_pcb)
{

  FOOTPRINT ( 'X', 't', 'e' );			/* Xte */
  /* send clear confirmation T=0, C=off */

  clrTC();

  while ((p_pcb->x21_state == WAIT_DCE_CLR_CONF) ||
         (p_pcb->x21_state == WAIT_CLR_DCE_RDY) )	
  {
  	FOOTPRINT ( 'X', 't', 'w' );			/* Xtw */
	p_pcb = sleep(p_pcb);

	switch( p_pcb->sleep_rqe.f.rtype)
	{
	case RQE_STAT_UNSOL:

		switch(p_pcb->sleep_rqe.f.sequence)
		{
			/* if dce does not respond */
			case XT5_TIMER:
		   	case XT6_TIMER:

  				FOOTPRINT ( 'X', 't', 't' );	/* Xtt */
				/* indicate forced termination in port state */
				p_pcb->x21_state = FORCED_TERM;
				/* signal ready, then uncontrolled not ready */
				p_pcb->cio_reg.data |= ENABLE_422;
				set_cio(p_pcb);
				xmt_24_bits( p_pcb );
	
				p_pcb->cio_reg.data &= DISABLE_422;
				set_cio(p_pcb);
				break;
			 
			case X21_ZERO:
			 
				FOOTPRINT( 'X', 't', 'z' );	/* Xtz */
				if (p_pcb->timer_type == XT5_TIMER)
				{
					p_pcb->x21_state = WAIT_CLR_DCE_RDY;
					start_duscc_timer(p_pcb, XT6_TIMEOUT,
						XT6_TIMER);
	
				}
				break;
			case X21_ONE:
	
				FOOTPRINT('X','t','o');			/*Xto*/
				if (p_pcb->x21_state == WAIT_CLR_DCE_RDY)
				{
  					if ( p_pcb->retry_in_prog )
  					{
						FOOTPRINT('X','t','r');/*Xtr*/
						/* start retry operation */
						p_pcb->x21_state = 
							WAIT_RETRY1_TIMER;
						p_pcb->retry_in_prog = 0;
						start_duscc_timer(p_pcb, 
						  x21_ctl.retry_delay, 
							X_RETRY1_TIMER);
						p_pcb->cio_reg.data |= 
							ENABLE_422;
						set_cio(p_pcb);
  					}
					else
					{
						stop_duscc_timer(p_pcb);
						p_pcb->x21_state = NORMAL_TERM;
						/* signal ready, then unc */
						p_pcb->cio_reg.data|=ENABLE_422;
						set_cio(p_pcb);
						xmt_24_bits( p_pcb );
	
						p_pcb->cio_reg.data &= 
								DISABLE_422;
						set_cio(p_pcb);
  					}
				}
				break;
			default:
				break;
		}

	case RQE_CMD_ACK: 

		if (p_pcb->sleep_rqe.f.status == STOP_PORT)
		{
			FOOTPRINT( 'X', 't', 's' );		/* Xts */
			p_pcb->start_rqe.f.rtype = RQE_CMD_ACK;
			p_pcb->start_rqe.f.status = STOP_PORT;
			p_pcb->start_rqe.f.sequence = 0;
		}
		break;

	case RQE_CPS_ACK:
	
		FOOTPRINT( 'X', 't', 'c' );			/* Xtc */
		x21_rcv_data( p_pcb );
		break;
	}
  }
  while ( p_pcb->x21_state == WAIT_RETRY1_TIMER )
  {
	sleep( p_pcb );
	if ( p_pcb->sleep_rqe.f.rtype == RQE_STAT_UNSOL )
	{
		if ( p_pcb->sleep_rqe.f.sequence == X_RETRY1_TIMER )
		{
			p_pcb->x21_state = READY;
			p_pcb->port_state = CALL_ESTAB;
			xmt_24_start( p_pcb );
		}
	}	
	else if (p_pcb->sleep_rqe.f.rtype == RQE_CMD_ACK) 
	{
		if (p_pcb->sleep_rqe.f.status == STOP_PORT)
		{
			p_pcb->start_rqe.f.rtype = RQE_CMD_ACK;
			p_pcb->start_rqe.f.status = STOP_PORT;
			p_pcb->start_rqe.f.sequence = 0;
			p_pcb->x21_state = NORMAL_TERM;
		}
	}
  }

  /* if normal term or forced term, exit and send rqe */
  if ( ( p_pcb->x21_state == NORMAL_TERM ) ||  
       ( p_pcb->x21_state == FORCED_TERM ) )
  {
 	/* reset tx and rx, stop timers, release buffers, etc. */
  	stop(p_pcb);
  	p_pcb->port_state = STATE_EXIT;
  }
	FOOTPRINT( 'X', 't', 'x' ); 			/* Xtx */
}

  
void	dte_clear_confirm(register volatile t_pcb	*p_pcb)
{

	FOOTPRINT( 'X', 'd', 'E' ); 			/* XdE */
	p_pcb->port_state = CALL_TERM;
	p_pcb->x21_state = WAIT_CLR_DCE_RDY;
	/* if error code not already set */
	if ( p_pcb->start_rqe.f.sequence == 0)
	{
		p_pcb->start_rqe.f.sequence = X_DCE_CLEAR;
	}

	start_duscc_timer(p_pcb, XT6_TIMEOUT, XT6_TIMER);
	return;
}

void	dte_clear_request(volatile t_pcb	*p_pcb)
{

	FOOTPRINT( 'X', 'D', 'E' ); 			/* XDE */
	p_pcb->x21_state = WAIT_DCE_CLR_CONF;

	start_duscc_timer( p_pcb, XT5_TIMEOUT, XT5_TIMER);
	p_pcb->port_state = CALL_TERM;
	return;
}


void	x21_rcv_data(register volatile t_pcb	*p_pcb)
{
rx_buf_hdr far           *p_rx;		/* pointer to rx buf */

unsigned char far	*p_data;		/* data character from rqe */
unsigned int		search_length, Group_num, retry_group,cps_mask;

FOOTPRINT( 'X', 'r', 'E' ); 			/* XrE */
/* get receive buffer pointer */
p_rx = (rx_buf_hdr far *)RECV_BUFFER( p_pcb->sleep_rqe.f.status ); 

/* get pointer to first character of data (not header) */
p_data = (unsigned char far *)p_rx + p_rx->offset;

/* if first character of data '3x' where x is a digit from 0 to 9 */
if (!(( *p_data >= 0x30 ) && ( *p_data <= 0x39 )))
{ 
	FOOTPRINT( 'X', 'r', 'd' ); 			/* Xrd */
	/* DPI data */
	p_rx->type = BUF_STAT_X_DPI;
	p_pcb->sleep_rqe.f.rtype = RQE_RX_DMA;
}
else
{
	/* CPS data */

	FOOTPRINT( 'X', 'r', 'c' ); 				/* Xrc */
	/* if data length > 3 then more than one cps */
	if ( p_rx->length > 3 )
	{
		FOOTPRINT( 'X', 'r', 'm' );			/* Xrm */
		/* set pointer to end of data and search backward for a ',' */
		p_data = (unsigned char far *)p_rx + p_rx->offset +
						( p_rx->length-1 );
		/* set pointer to first character after last ',' */
		search_length = 1;
		while (( *p_data != ',' ) && ( search_length <= p_rx->length ))
		{
			p_data--;
			search_length++;
		}
		p_data++;
	}
   
	/* Check cps for valid entry */
	if (( *p_data >= 0x30 ) && ( *p_data <= 0x39 ))
	{
		FOOTPRINT( 'X', 'r', 'v' );			/* Xrv */
		p_pcb->sleep_rqe.f.rtype = RQE_RX_DMA;
		/* Retry if necessary */
		Group_num = ( *p_data ) - 0x30;

		if ( Group_num == 0 )
		{
			FOOTPRINT( 'X', 'r', '0' );		/* Xr0 */
			start_duscc_timer( p_pcb, X_GR0_TIMEOUT, X_GR0_TIMER);
		}
		else
		{
			stop_duscc_timer( p_pcb );
			p_pcb->retry_in_prog = 0;

			if ( x21_ctl.retry_count > 0 )
			{
				FOOTPRINT( 'X', 'r', 'r' );	/* Xrr */
				p_data++;
				cps_mask = 0x8000 >> ( *p_data - '0' );
				retry_group = x21_ctl.retry[Group_num];
				if ( retry_group && cps_mask ) 
				{
					FOOTPRINT( 'X', 'r', 'm' );  /* Xrm */
					x21_ctl.retry_count--;
					p_pcb->retry_in_prog = B_TRUE;
				}
			}
		        else
			{
				if (Group_num == 8)
				{
					FOOTPRINT( 'X', 'r', '8' );  /* Xr8 */
					/* no error - Ack_start to llc */
					p_rx->type = BUF_STAT_CPS_OK;
					p_pcb->sleep_rqe.f.sequence =
						BUF_STAT_CPS_OK;
				}
				else
				{
					FOOTPRINT( 'X', 'r', 'R' );  /* XrR */
					/* error */
					p_rx->type = BUF_STAT_CPS_E;
					p_pcb->sleep_rqe.f.sequence =
						 BUF_STAT_CPS_E;
				}
			}

			/* indicate reason for call terminating */
			p_pcb->start_rqe.f.sequence = X_RETRY_EXP;

			/* if clear not already in process, start it */
			if ( p_pcb->port_state != CALL_TERM )
				start_duscc_timer( p_pcb, X_CPS_TIMEOUT, 
					X_CPS_TIMER );
		}
	}
}
/* queue rx_ack for cps */
add_rqe( p_pcb, p_pcb->sleep_rqe.val );
}

