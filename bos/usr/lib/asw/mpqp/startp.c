static char sccsid[] = "@(#)41	1.18  src/bos/usr/lib/asw/mpqp/startp.c, ucodmpqp, bos411, 9434B411a 6/16/94 10:04:12";
/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS: 
 *	startp		: See description below.
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

/*
 * FUNCTION: To establish a physical connection to a switched
 *           or leased line. Physical connections supported
 *           include RS232-D, V.35, EIA422-A, X.21. Hayes auto-
 *           dial, V.25bis and X.21 switched and non-switched
 *           call protocols are also supported.
 */

#include "mpqp.def"
#include "mpqp.typ"
#include "mpqp.ext"
#include "mpqp.pro"
#include "mpqp.h"
#include "portcb.def"
#include "portcb.typ"
#include "iface.def"


extern void	add_rqe( volatile t_pcb *, unsigned long );
extern void init_rcv( volatile t_pcb 	*p_pcb );

/*
   Because of far parameter problems with taking the address of stack frame
   variables, the following external definitions for functions within this
   file were added.
*/

extern int	leased( volatile t_pcb * );
extern int	switched( volatile t_pcb *, cmd_t * );
extern int	smrtmod( volatile t_pcb *, cmd_t * );
extern int	autocal( volatile t_pcb *, cmd_t * );
extern int	autoans( volatile t_pcb *, cmd_t * );
extern int	X21switched(volatile t_pcb *, cmd_t * );
extern int	X21leased(volatile t_pcb * );

extern unsigned char		x21_Stat;	/* last value returned by PAL */
extern 	t_x21_ctl		x21_ctl;	/* global x.21 control struct */

startp (cmd_t			*p_cmd,		/* pointer to command block */
	unsigned char far	*p_buf)		/* pointer to dial data buf */

{
register volatile t_pcb		*p_pcb;		/* port control blk */
unsigned short			r_u;		/* Return value */
extern unsigned short		rx_bsiz;

	FOOTPRINT( 'S', 's', 'a' );		/* Ssa */
	p_pcb = &pcb [p_cmd->port];

	/* set up error RQE for possible NAK to start cmd */
	p_pcb->start_rqe.f.rtype = RQE_CMD_NAK;
	p_pcb->start_rqe.f.status = START_PORT;
	p_pcb->start_rqe.f.port = p_cmd->port;
	p_pcb->start_rqe.f.sequence = 0;

	/* Copy start port parameters to pcb */
	p_pcb->start_parm = p_cmd->cs.start_port.parm;

	/* 5/17/94: Added capability to make max receive frame size config- */
	/* urable to improve on recovery of overflowed received frames in   */
	/* BiSync, see defect# 149486.					    */
	/* Currently RDTO is modifiable in ODM, PdAt.  However, the default */
	/* in PdAt is decimal 92.  I have never seen it change from that    */
	/* value.							    */
	/* Set maximum frame length.  It has to be <= (rx_bsiz - RDTO) so   */
	/* it won't overflow into the next buffer.			    */

	p_pcb->rx_flgt = p_cmd->cs.start_port.max_rx_bsiz;
	if ( p_pcb->rx_flgt > 
		( rx_bsiz - ( p_pcb->rx_off = p_cmd->cs.start_port.RDTO )) )
	{
	    p_pcb->rx_flgt = ( rx_bsiz - p_cmd->cs.start_port.RDTO );
	}

	/* If dial data, copy dial buffer pointer */
	if (p_cmd->cs.start_port.parm & PCB_SP_DIALBFR)
	{
		p_pcb->dial_data = p_buf;
	}
	if (p_pcb->phys_link & PCB_PL_X21)
	{
		x21_ctl = *(t_x21_ctl far *)p_buf;
		init_rcv( p_pcb );
	}
	p_pcb->sleep_rqe.val = RQE_NULL;

	sleep_enable(p_pcb);
	mi_enable(p_pcb,p_pcb->modem_int);

	/* if not X.21 */
	if (!(p_pcb->phys_link & PCB_PL_X21))
	{
		if (p_pcb->start_parm & PCB_SP_LEASED)
			leased( p_pcb );
		else
			switched( p_pcb, p_cmd );
	}
	else
	{
		if (p_pcb->start_parm & PCB_SP_LEASED)
			X21leased(p_pcb );
		else
			X21switched(p_pcb, p_cmd );
	}

	free_cmdblk( (cmd_blk *)p_cmd - cmdblk);
	sleep_disable(p_pcb);

/************************************************************************/
/*									*/
/* 			Data Transfer State				*/
/*									*/
/* Enable receivers, change protocols if required, build rqe.		*/
/*									*/
/************************************************************************/

	if ( p_pcb->port_state == DATA_TRANSFER)
	{
		FOOTPRINT( 'S', 's', 'b' );		/* Ssb */
		/* if done with autodial */
		if ( p_pcb->phys_link <= PCB_PL_X21 )

		{
			/* set up data transfer protocol */
			p_pcb->proto = p_pcb->data_proto;
			p_pcb->flags = p_pcb->data_flags;
			/* if dial was internally clocked */
	   		if (!(p_pcb->phys_link & PCB_PL_X21))
			{
				p_pcb->baud_rate = 0;
			    	/* turn off DTE_CLK in CIO enabling DCE clock*/
			    	p_pcb->cio_reg.data &= DISABLE_DTE_CLK;
			    	set_cio(p_pcb);
			}
			if ( p_pcb->data_proto & PCB_PRO_BSC )
				set_bsc( p_pcb );
			else if ( p_pcb->data_proto & PCB_PRO_ASYNC )
				set_asc( p_pcb );
			else
				set_sdlc( p_pcb );
			/* We don't want to see interrupts for CD/CTS anymore */
			/* Call mi_enable here to set IER (interrupt enable   */
			/* register, rsMask and trsMask changed in set_xxx    */
			p_pcb->modem_int &= ~(PCB_MM_CD | PCB_MM_CT);
			mi_enable(p_pcb,p_pcb->modem_int);
			p_pcb->modem_state = modem_flags( p_pcb );
		}

		init_rcv( p_pcb );

		/* enable channels for receive */
		p_pcb->start_rqe.f.rtype = RQE_CMD_ACK;
		p_pcb->start_rqe.f.sequence = 0;

	} /* Successful or failed starts */
	else
	{
		stop( p_pcb );
	}

	/* Build adapter rqe with successful call establishment */
	add_rqe( p_pcb, p_pcb->start_rqe.val );
	FOOTPRINT( 'S', 's', 'z' );		/* Ssz */
	return( ~PCQ_LOCK );
}

/************************************************************************/
/*									*/
/* State machine for leased operation					*/
/*									*/
/************************************************************************/

leased ( volatile t_pcb		*p_pcb )
{
	p_pcb->port_state = WAIT_FOR_DSR;
	p_pcb->sleep_rqe.val = RQE_NULL;

	/* Get latest modem status and save it in PCB */
	p_pcb->modem_state = modem_flags( p_pcb );

	p_pcb->cio_reg.data &= ENABLE_DTR;
	set_cio( p_pcb );

	/* if DSR is active then advance to the next state */
	if (p_pcb->modem_state & PCB_MM_DS )          
	{
		p_pcb->port_state = DATA_TRANSFER;        
		return;
	}

	/* Start DSR timer */
	start_duscc_timer( p_pcb, DSR_ON_TIMEOUT, DSR_ON_TIMER );

	/* Wait until connection is made or timer expires */
	while ( p_pcb->port_state == WAIT_FOR_DSR )
	{
		p_pcb = sleep( p_pcb );

		if ( p_pcb->sleep_rqe.f.rtype == RQE_STAT_UNSOL)
		{
			if ( p_pcb->sleep_rqe.f.sequence == DSR_ON )
			{

				stop_duscc_timer( p_pcb );
				p_pcb->port_state = DATA_TRANSFER;
			}
			else if ( p_pcb->sleep_rqe.f.sequence == DSR_ON_TIMER )
			{
				p_pcb->start_rqe.f.sequence = DSR_ON_TIMER;
				p_pcb->port_state = STATE_EXIT;
			}
		}
		else if ( ( p_pcb->sleep_rqe.f.rtype == RQE_CMD_ACK )
			&& ( p_pcb->sleep_rqe.f.status == STOP_PORT ) )
		{
			p_pcb->start_rqe.f.rtype = RQE_CMD_ACK;
			p_pcb->start_rqe.f.status = STOP_PORT;
			p_pcb->start_rqe.f.sequence = 0;
			p_pcb->port_state = STATE_EXIT;
		}
	}
	return;
}

/************************************************************************/
/*									*/
/* State machine for switched operation					*/
/*									*/
/************************************************************************/

switched ( volatile t_pcb		*p_pcb ,
	cmd_t				*p_cmd )
{
	if ( p_pcb->phys_link & PCB_PL_HAYES )
		smrtmod( p_pcb, p_cmd );

	else if ( p_pcb->phys_link & PCB_PL_V25 )
		autocal( p_pcb, p_cmd );

	else /* must be manual */
	{
		/* Start 1 sec timer and wait for it to expire */
		p_pcb->sleep_rqe.val = RQE_NULL;
		start_duscc_timer( p_pcb, SW_TIMEOUT, SW_TIMER );
		while (!(( p_pcb->sleep_rqe.f.rtype == RQE_STAT_UNSOL ) &&
			   ( p_pcb->sleep_rqe.f.sequence == SW_TIMER )))
			p_pcb = sleep ( p_pcb );
	
		stop_duscc_timer( p_pcb );
	
		/* Get modem status */
		p_pcb->modem_state = modem_flags( p_pcb );
	
		if /* DSR is active */
		   ( p_pcb->modem_state & PCB_MM_DS )
		{
			/* Report error and return */
			p_pcb->start_rqe.f.sequence = DSR_ON;
			p_pcb->port_state = STATE_EXIT;
		}
		else /* start call establishing operation */
		{
			if /* This is a out going call */
			   ( p_pcb->start_parm & PCB_SP_CALL )
			{
				/* Indicate ready for dial */
				/* Build RQE           */
				p_pcb->start_rqe.f.rtype = RQE_STAT_UNSOL;
				p_pcb->start_rqe.f.sequence = READY_FOR_DIAL;
				queue_writel( &arqueue, p_pcb->start_rqe.val );
				host_intr( TREG_ARQ );
				p_pcb->start_rqe.f.rtype = RQE_CMD_NAK;
				manual( p_pcb );
			}	
			else		/* answer operation */
			{
				if /* auto-answer, CDSTL is selected */
				   ( p_pcb->start_parm & PCB_SP_CDSTL ) 
				{
				        p_pcb->port_state = WAIT_FOR_DSR;
    					/* Raise DTR */
    					p_pcb->cio_reg.data &= ENABLE_DTR;
    					set_cio(p_pcb);
					autoans(p_pcb, p_cmd);
				}
				else
					manual( p_pcb );
			}
		}
	}
}
	
/************************************************************************/
/*									*/
/* State machine for smart modem					*/
/*									*/
/************************************************************************/
extern	int	Ascii_To_Op( unsigned char far *, unsigned short );

smrtmod ( volatile t_pcb	*p_pcb ,
	cmd_t			*p_cmd )
{
unsigned char			n_cmd_blk;

    /* init channels for receive */
    init_rcv( p_pcb );

    p_pcb->port_state = WAIT_FOR_CONNECT;
    p_pcb->modem_state = modem_flags( p_pcb );
    /* Raise DTR */
    p_pcb->cio_reg.data &= ENABLE_DTR;
    set_cio(p_pcb);

    if ( p_cmd->length != 0 )
    {
	p_cmd->type = TX_LONG;
	p_cmd->flags = TX_NO_FREE;
	p_cmd->card_addr = XMIT_BUFFER((cmd_blk *)p_cmd - &cmdblk[0]);
	Ascii_To_Op( p_cmd->card_addr, p_cmd->length );

	/* put transmit on port cmd queue */
	n_cmd_blk = (cmd_blk *)p_cmd - &cmdblk[0];
	queue_writeb( p_pcb->cmd_q, n_cmd_blk );
	work_q [PCQ_WORK] |= p_pcb->mask_on;
    }
    /* if answer mode */
    if (!(p_pcb->start_parm & PCB_SP_CALL))
    {
	/* Get modem latest status and save it in PCB */
	if ( !( p_pcb->modem_state & DSR_ON ) )
	{
	    p_pcb->port_state = WAIT_FOR_DSR;
	    smrtans(p_pcb);
	    return;
	}
	else /* DSR already on */
	{
	    /* start timer to allow transmit to go out */
	    start_duscc_timer( p_pcb, SM_TX_TIMEOUT , SM_TX_TIMER);
	    while ( p_pcb->port_state == WAIT_FOR_CONNECT )
	    {
	        p_pcb = sleep( p_pcb );
	        if ((p_pcb->sleep_rqe.f.rtype == RQE_STAT_UNSOL) &&
		    (p_pcb->sleep_rqe.f.sequence == SM_TX_TIMER))
	        {
		    p_pcb->start_rqe.f.sequence = DSR_ON;
		    p_pcb->port_state = DATA_TRANSFER;
	        }
	    }
	    return;
	}
    }

    /* call mode */
    if ( p_cmd->cs.auto_dial.connect_timer != 0 )
        start_duscc_timer( p_pcb, (p_cmd->cs.auto_dial.connect_timer), 
	    SM_TIMER);
    else
        start_duscc_timer( p_pcb, SM_TIMEOUT, SM_TIMER );

    /* Clear response field */
    p_pcb->sleep_rqe.val = RQE_NULL;

    /* Wait until transmit is complete or timer is expired */
    while (p_pcb->port_state == WAIT_FOR_CONNECT)
    {
        p_pcb = sleep(p_pcb);
        FOOTPRINT( 'S', 'm', 'w' );		/* Smw */

        /* If transmit is complete then go to next state */
        if ( p_pcb->sleep_rqe.f.rtype == RQE_STAT_UNSOL )
        {
	    /* if timer expired, NAK start command */
	    if  ( p_pcb->sleep_rqe.f.sequence == SM_TIMER )
	    {
	        FOOTPRINT( 'S', 'm', 't' );	/* Smt */
	        /* Report as DSR timer */
	        p_pcb->start_rqe.f.sequence = DSR_ON_TIMER;
	        p_pcb->port_state = STATE_EXIT;
	    }
	    /* else if modem line change */
	    else if ( p_pcb->sleep_rqe.f.sequence & DSR_ON )
	    {
	        FOOTPRINT( 'S', 'm', 'd' );	/* Smd */
	        /* if modem line turned on */
	        if (!( p_pcb->sleep_rqe.f.sequence & OFF_BIT ))
	        {
		    p_pcb->modem_state |= p_pcb->sleep_rqe.f.sequence;
		    if ( p_pcb->modem_state & DSR_ON )
		    {
		        /* reset rcv timer */
		        stop_duscc_timer(p_pcb);
		        /* Wait 5 seconds before allowing transfer of data,
		           else some modems can't handle */
        	        start_duscc_timer(p_pcb,XMIT_DELAY_TIME,
			    XMIT_DELAY_TIMER);
	    		while (p_pcb->sleep_rqe.f.sequence != XMIT_DELAY_TIMER)
	    		{
			    p_pcb = sleep(p_pcb);
			    FOOTPRINT( 'S', 'm', 'x' );	/* Smx */
	    		}
			p_pcb->port_state=DATA_TRANSFER;
		    }
		}
		/* turn off bit in modem save value */
		else 
		    p_pcb->modem_state &= ~p_pcb->sleep_rqe.f.sequence;
	    }
	}
	/* if call data, send to host */
	else if (p_pcb->sleep_rqe.f.rtype == RQE_CPS_ACK)
	{
	    FOOTPRINT( 'S', 'm', 'c' );	/* Smc */
	    p_pcb->sleep_rqe.f.rtype = RQE_RX_DMA;
	    add_rqe( p_pcb, p_pcb->sleep_rqe.val );
	}
	else if ( ( p_pcb->sleep_rqe.f.rtype == RQE_CMD_ACK )
	    && ( p_pcb->sleep_rqe.f.status == STOP_PORT ) )
	{
	    FOOTPRINT( 'S', 'm', 's' );	/* Sms */
	    p_pcb->start_rqe.f.rtype = RQE_CMD_ACK;
	    p_pcb->start_rqe.f.status = STOP_PORT;
	    p_pcb->start_rqe.f.sequence = 0;
	    p_pcb->port_state = STATE_EXIT;
	}
    }
    return;
}

/************************************************************************/
/*									*/
/* State machine for V.25 BIS auto call					*/
/*									*/
/************************************************************************/

autocal( volatile t_pcb		*p_pcb, 
	cmd_t			*p_cmd)
{
unsigned char			n_cmd_blk;

	FOOTPRINT( 'S', 'c', 'a' );		/* Sca */
	/* init channels for receive */
	init_rcv( p_pcb );

	p_pcb->port_state = V25BIS_CALL;	/* Set current state    */
	p_pcb->sleep_rqe.val = RQE_NULL;	/* Clear response field */
	/* Get modem latest status and save it in PCB */
	p_pcb->modem_state = modem_flags(p_pcb);

	/* Raise DTR */
	p_pcb->cio_reg.data &= ENABLE_DTR;
	set_cio(p_pcb);

/************************************************************************/
/*									*/
/* This state handles the first auto call sequence			*/
/*									*/
/************************************************************************/

    /* if CTS is active and RI is not then advance to next state */
    if ((p_pcb->modem_state & PCB_MM_CT) && 
    	(!(p_pcb->modem_state & PCB_MM_RI) )) 
    {
	FOOTPRINT( 'S', 'c', 'b' );		/* Scb */
	/* Advance to next state */
	p_pcb->port_state = V25_CALL_DIALING;
    } 
    else 
    {
  	/* Start CTS timer */
	start_duscc_timer(p_pcb,V25_DIAL_CTS_TIME,V25_DIAL_CTS_TIMER);
  
  	/* Wait until CTS is active or timer is expired */
  	while (p_pcb->port_state == V25BIS_CALL)
  	{
  	    p_pcb = sleep(p_pcb);
	    FOOTPRINT( 'S', 'c', 'c' );		/* Scc */
   
  	    /* if a status interrupt */
  	    if (p_pcb->sleep_rqe.f.rtype == RQE_STAT_UNSOL) 
  	    {
  		/* Ring indicate gone active ? */
  		if (p_pcb->sleep_rqe.f.sequence == RI_ON)
  		{
		    FOOTPRINT( 'S', 'c', 'd' );		/* Scd */
		    stop_duscc_timer(p_pcb);
  		    /* This is a call collision */
		    p_pcb->start_rqe.f.sequence = p_pcb->sleep_rqe.f.sequence;
  		    p_pcb->port_state=STATE_EXIT;
  		}
		/* Check for Clear To Send signal */
		else if (p_pcb->sleep_rqe.f.sequence & CTS_ON)
  		{
		    FOOTPRINT( 'S', 'c', 'e' );		/* Sce */
		    /* if CTS turned on */
		    if (!(p_pcb->sleep_rqe.f.sequence & OFF_BIT ))
		    {
			stop_duscc_timer(p_pcb);
			p_pcb->modem_state |= p_pcb->sleep_rqe.f.sequence;
			/* Modem ready to receive dial data, go to next state*/
  			p_pcb->port_state = V25_CALL_DIALING;
  		    }
		    /* turn off bit in modem save value */
		    else p_pcb->modem_state &= ~p_pcb->sleep_rqe.f.sequence;
		}
		/* Check if timer expired */
  		else if (p_pcb->sleep_rqe.f.sequence == V25_DIAL_CTS_TIMER)
  		{
		    FOOTPRINT( 'S', 'c', 'f' );		/* Scf */
  		    /* Timer expired, no CTS */
  		    /* save timer type in sequence field */
		    p_pcb->start_rqe.f.sequence = p_pcb->sleep_rqe.f.sequence;
		    p_pcb->port_state = STATE_EXIT;
  		}
  	    } /* if status interrupt */
	}  /* while port_state == V25BIS_CALL */
    }	/* wait for CTS */
/************************************************************************/
/*									*/
/* This state handles the second auto call sequence			*/
/*									*/
/************************************************************************/

    if (p_pcb->port_state == V25_CALL_DIALING)
    {
	/* Transmit dial command and data */

	if ( p_cmd->length != 0 )
	{
	    FOOTPRINT( 'S', 'c', 'g' );		/* Scg */
	    p_cmd->type = TX_LONG;
	    p_cmd->flags = TX_NO_FREE;
	    p_cmd->card_addr = XMIT_BUFFER( (cmd_blk *)p_cmd - &cmdblk[0]);
	    if ( (p_pcb->dial_proto & PCB_PRO_ASYNC) &&
		(p_pcb->dial_flags & PCB_AF_PAREN) )
	    {
		if (p_pcb->dial_flags & PCB_AF_ODDPAR) {
	    	    Ascii_To_Op( p_cmd->card_addr, p_cmd->length );
	    	    FOOTPRINT( 'S', 'c', 'j' );	/*Scj*/
		} else {
	    	    Ascii_To_Ep( p_cmd->card_addr, p_cmd->length );
	    	    FOOTPRINT( 'S', 'c', 'k' ); /*Sck*/
		}
	    } 

	    /* put transmit on port cmd queue */
	    n_cmd_blk = (cmd_blk *)p_cmd - &cmdblk[0];
	    queue_writeb( p_pcb->cmd_q, n_cmd_blk );
	    work_q [PCQ_WORK] |= p_pcb->mask_on;
	}

	if (!(p_pcb->start_parm & PCB_SP_CALL)) /* answer mode */
	{
	    /* Leave port_state as V25_CALL_DIALING to differentiate	*/
	    /* from Hayes autodial.					*/
	    autoans(p_pcb,p_cmd);
	    return;
	}

        /* call mode */
        if ( p_cmd->cs.auto_dial.connect_timer != 0 )
	    /* start connect timer for specified amount of time */
            start_duscc_timer( p_pcb, (p_cmd->cs.auto_dial.connect_timer), 
	        DSR_ON_TIMER);
        else
	    /* Start connect timer for 60 sec */
	    start_duscc_timer(p_pcb,V25_CALL_CONNECT_TIME,DSR_ON_TIMER);
    
	/* Wait until connection is made or timer is expired */
	while (p_pcb->port_state == V25_CALL_DIALING)
	{
	    p_pcb = sleep(p_pcb);
	    FOOTPRINT( 'S', 'c', 'm' );		/* Scm */

	    /* if status interrupt */
	    if (p_pcb->sleep_rqe.f.rtype == RQE_STAT_UNSOL)
	    {
		/* Ring indicate active ? */
		if (p_pcb->sleep_rqe.f.sequence == RI_ON)
		{
		    FOOTPRINT( 'S', 'c', 'n' );		/* Scn */
		    stop_duscc_timer(p_pcb);
		    /* This is a call collision */
		    p_pcb->start_rqe.f.sequence = p_pcb->sleep_rqe.f.sequence;
		    p_pcb->port_state = STATE_EXIT;
		}
		/* Check for DSR signal */
		else if (p_pcb->sleep_rqe.f.sequence & DSR_ON)
		{
		    FOOTPRINT( 'S', 'c', 'o' );		/* Sco */
		    /* if modem line turned on */
		    if (!(p_pcb->sleep_rqe.f.sequence & OFF_BIT ))
		    {
			p_pcb->modem_state |= p_pcb->sleep_rqe.f.sequence;
			/* Connection successful */
			stop_duscc_timer(p_pcb);
			/* Clear response field */
			p_pcb->sleep_rqe.val = RQE_NULL;

			if (p_cmd->cs.auto_dial.v25b_tx_timer == 0)
			    /* Start 5 sec transmit delay timer */
			    start_duscc_timer(p_pcb,XMIT_DELAY_TIME,
				XMIT_DELAY_TIMER);
			else
			    start_duscc_timer(p_pcb,
				(p_cmd->cs.auto_dial.v25b_tx_timer),
				XMIT_DELAY_TIMER);
			while (p_pcb->sleep_rqe.f.sequence != XMIT_DELAY_TIMER)
			{
			    p_pcb = sleep(p_pcb);
			    FOOTPRINT( 'S', 'c', 'r' );	/* Scr */
			}
			p_pcb->port_state = DATA_TRANSFER;
		    }
		    /* turn off bit in modem save value */
		    else
			p_pcb->modem_state &= ~p_pcb->sleep_rqe.f.sequence;
		}
		/* Timer expired? */
		else if (p_pcb->sleep_rqe.f.sequence == DSR_ON_TIMER)
		{
		    FOOTPRINT( 'S', 'c', 'p' );		/* Scp */
		    /* Timer expired without DSR */
		    p_pcb->start_rqe.f.sequence = DSR_ON_TIMER;
		    p_pcb->port_state = STATE_EXIT;
		}
	    }
	    else if ( ( p_pcb->sleep_rqe.f.rtype == RQE_CMD_ACK )
		 && ( p_pcb->sleep_rqe.f.status == STOP_PORT ) )
	    {
		FOOTPRINT( 'S', 'c', 'q' );		/* Scq */
		p_pcb->start_rqe.f.rtype = RQE_CMD_ACK;
		p_pcb->start_rqe.f.status = STOP_PORT;
		p_pcb->start_rqe.f.sequence = 0;
		p_pcb->port_state = STATE_EXIT;
	    }
        }
    }
    return;
}

/************************************************************************/
/*									*/
/* State machine for auto-answer operation				*/
/*									*/
/************************************************************************/

autoans ( volatile t_pcb	*p_pcb, 
	  cmd_t			*p_cmd)
{
    FOOTPRINT( 'S', 'u', 'a' );		/* Sua */
    p_pcb->sleep_rqe.val = RQE_NULL;	/* Clear response field */

    /* Some modems don't change the RI signal to high, even in V25BIS
	mode;  so don't wait for the RI signal, just wait for DSR */

/************************************************************************/
/*									*/
/* This state handles waiting for DSR					*/
/*									*/
/************************************************************************/

    if ( (p_pcb->port_state == WAIT_FOR_DSR) ||
    	(p_pcb->port_state == V25_CALL_DIALING) )
    {
	/* Get modem latest status and save it in PCB */
	p_pcb->modem_state = modem_flags(p_pcb);

	/* if DSR is active then advance to the next state */
	if (p_pcb->modem_state & PCB_MM_DS)
	{
	    FOOTPRINT( 'S', 'u', 'd' );		/* Sud */
	    /* Indicate successful call establishment */
	    p_pcb->port_state = DATA_TRANSFER;
	}

	/* Wait forever until connection is made */
	while ( (p_pcb->port_state == WAIT_FOR_DSR) ||
    	    (p_pcb->port_state == V25_CALL_DIALING) )
	{
	    FOOTPRINT( 'S', 'u', 'g' );		/* Sug */
	    p_pcb = sleep(p_pcb);

	    /* if modem signal has changed */
	    if (p_pcb->sleep_rqe.f.rtype == RQE_STAT_UNSOL)
	    {
		/* Check for DSR signal */
		if ( (p_pcb->sleep_rqe.f.sequence == DSR_ON) &&
		    (!( p_pcb->sleep_rqe.f.sequence & OFF_BIT )))
		{
		    /* Connection successful */
		    FOOTPRINT( 'S', 'u', 'h' ); /* Suh */
		    stop_duscc_timer(p_pcb);
		    /* Clear response field */
		    p_pcb->sleep_rqe.val = RQE_NULL;

		    /* Start transmit delay timer */
		    if (p_cmd->cs.auto_dial.v25b_tx_timer)
		    {
            		start_duscc_timer(p_pcb,
			    (p_cmd->cs.auto_dial.v25b_tx_timer),
			    XMIT_DELAY_TIMER);
		    	while (p_pcb->sleep_rqe.f.sequence != XMIT_DELAY_TIMER)
			{
			    p_pcb = sleep(p_pcb);
	            	    FOOTPRINT( 'S', 'u', 'j' );	    /* Suj */
			}
		    }
	    	    p_pcb->port_state = DATA_TRANSFER;
		}
	    }
	    else if (p_pcb->sleep_rqe.f.rtype == RQE_CMD_ACK) 
	    {
	        if (p_pcb->sleep_rqe.f.status == STOP_PORT) 
	        {
	            FOOTPRINT( 'S', 'u', 'i' );	    /* Sui */
	            p_pcb->start_rqe.f.rtype = RQE_CMD_ACK;
	            p_pcb->start_rqe.f.status = STOP_PORT;
	            p_pcb->start_rqe.f.sequence = 0;
	            p_pcb->port_state = STATE_EXIT;
	            return;
	        }
	    }
	}
    }
    return;
}

smrtans ( volatile t_pcb *p_pcb ) 
{
	FOOTPRINT( 'S', 'a', 'a' );		/* Saa */
	p_pcb->sleep_rqe.val = RQE_NULL;	/* Clear response field */

	/* Get modem latest status and save it in PCB */
	p_pcb->modem_state = modem_flags(p_pcb);

/************************************************************************/
/*									*/
/* This state handles the second auto answer sequence			*/
/*									*/
/************************************************************************/

    if (p_pcb->port_state == WAIT_FOR_DSR)
    {
	/* Clear response field */
	p_pcb->sleep_rqe.val = RQE_NULL;

	/* Wait until connection is made or timer expires */
	while (p_pcb->port_state == WAIT_FOR_DSR)
	{
	    p_pcb = sleep(p_pcb);
	    FOOTPRINT( 'S', 'a', 'w' );	    /* Saw */

	    /* if modem signal has changed */
	    if (p_pcb->sleep_rqe.f.rtype == RQE_STAT_UNSOL)
	    {
	        if ( (p_pcb->sleep_rqe.f.sequence & DSR_ON)&&
	            (!( p_pcb->sleep_rqe.f.sequence & OFF_BIT )))
	        {
	            /* Connection successful */
	            FOOTPRINT( 'S', 'a', 'd' );	    /* Sad */

		    stop_duscc_timer(p_pcb);
		    /* Clear response field */
		    p_pcb->sleep_rqe.val = RQE_NULL;

	            p_pcb->port_state = DATA_TRANSFER;
	        }
	    }
	    else if (p_pcb->sleep_rqe.f.rtype == RQE_CMD_ACK) 
	    {
	        if (p_pcb->sleep_rqe.f.status == STOP_PORT) 
	        {
	            FOOTPRINT( 'S', 'a', 's' );	    /* Sas */
	            p_pcb->start_rqe.f.rtype = RQE_CMD_ACK;
	            p_pcb->start_rqe.f.status = STOP_PORT;
	            p_pcb->start_rqe.f.sequence = 0;
	            p_pcb->port_state = STATE_EXIT;
	            return;
	        }
	    }
	}
    }
    return;
}

/************************************************************************/
/*									*/
/* State machine for manual answer operation				*/
/*									*/
/************************************************************************/

manual ( volatile t_pcb		*p_pcb )
{
	p_pcb->port_state = WAIT_FOR_DSR;
	/* Raise DTR */
	p_pcb->cio_reg.data &= ENABLE_DTR;
	set_cio(p_pcb);

	while (p_pcb->port_state == WAIT_FOR_DSR)
	{
		p_pcb = sleep(p_pcb);

		/* DSR is active */
		if ((p_pcb->sleep_rqe.f.rtype == RQE_STAT_UNSOL)
			&& (p_pcb->sleep_rqe.f.sequence == DSR_ON))
		{
			p_pcb->port_state = DATA_TRANSFER;
		}
		else    
		{
			if ( ( p_pcb->sleep_rqe.f.rtype == RQE_CMD_ACK ) &&
			   ( p_pcb->sleep_rqe.f.status == STOP_PORT ) )
			{
				p_pcb->start_rqe.f.rtype = RQE_CMD_ACK;
				p_pcb->start_rqe.f.status = STOP_PORT;
				p_pcb->start_rqe.f.sequence = 0;
				p_pcb->port_state = STATE_EXIT;
			}
		}
	}
	return;
} 

extern unsigned short rx_count[];

void init_rcv( volatile t_pcb	*p_pcb )
{
	/* make sure the port is stoped */
	out08( p_pcb->scc_base + SCC_CC, RESET_RX );

	/* put the current set of rcv buffers back on the free list */
        if ( p_pcb->rx_curr != NO_BUFNO )
                queue_writeb( &rx_free, p_pcb->rx_curr );
        if ( p_pcb->rx_next != NO_BUFNO )
                queue_writeb( &rx_free, p_pcb->rx_next );

	/* init the sched stuff for this port */
        rx_count[p_pcb->port_no]=0;
        UNSCHED_RX_WORK(p_pcb);

	/* enable channels for receive */

	p_pcb->rxchan = 0x00;	
	p_pcb->rx_curr = enable_rx( p_pcb, p_pcb->rx_dma_1 );
	p_pcb->rxchan = 0x01;	
	p_pcb->rx_next = enable_rx( p_pcb, p_pcb->rx_dma_0 );
	p_pcb->rxchan = 0x00;	

	/* enable the port to receive */
	InitPDMA( p_pcb->rx_dma_1 );
	out08( p_pcb->scc_base + SCC_CC, ENABLE_RX );

}
