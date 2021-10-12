static char sccsid[] = "@(#)29	1.9  src/bos/usr/lib/asw/mpqp/errsts.c, ucodmpqp, bos411, 9428A410j 8/23/93 13:19:22";

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS: 
 *	f_errsts	: The scheduler service function for the ERRSTS
 *			:  level, serving timers, SCCs, CIOs.
 *	scc_delta	: Handles SCC timer pops and line changes (CTS,CD)
 *	cio_delta	: Handles CIO line changes (DSR,RI)
 *	add_rqe		: Place an RQE into either the Adapter Response
 *			:  Queue or ReRoute it to a Port Response Queue
 *			:  for sleep service wakeup.
 *	f_prqwork	: The Port Response Queue work processing scheduler
 *			:  service function.  Dequeues RQE from Port Response
 *			:  Queues and does wakeup, schedules Bus Master,
 *			:  whatever is appropriate.
 *	sleep		: Place an offlevel service process to sleep and
 *			:  return to the scheduler.
 *	sleep_enable	: Setup to allow an offlevel service process to use
 *			:  sleep services.  Starts RQE re-routing.
 *	sleep_disable	: Cleanup after an offlevel has enabled sleeping.
 *			:  Flushes the Port Response Queue as necessary.
 *	mi_enable	: Enable or Disable interrupt sources for a port.
 *	modem_flags	: Create the bit-mapped line status value and return.
 *	get_modem_stats	: Get modem_flags and return an RQE to the host.
 *	tx_abort	: Clean up after a transmit error of any type.  Stops
 *			:  a running Port DMA channel, NAKs the command to
 *			:  the driver and frees the associated resources.
 *			:  May start the next queued operation.
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
   This module answers the scheduler level ERRSTS, port Error/Status
   (unsolicited) for every port.  It performs essential offlevel services
   for Line Changes (i.e. Port DMA management, Rx/Tx control, etc.)

   f_errsts : Module entry point from scheduler for level ERRSTS

   Input  : port, unsigned character port number
   Output : nothing significant, always returns Zero
*/

#include "mpqp.def"
#include "mpqp.typ"
#include "mpqp.ext"
#include "mpqp.pro"
#include "mpqp.h"
#include "portcb.def"
#include "portcb.typ"
#include "iface.def"

void tx_abort ( volatile t_pcb	*, unsigned char );

extern unsigned short		Recv_Enable;	/* Recv enable/disable */

extern void asy_decode( volatile t_pcb * );
extern void bop_decode( volatile t_pcb * );
extern void cop_decode( volatile t_pcb * );
extern void x21_decode( volatile t_pcb * );

void add_rqe ( volatile t_pcb		*p_pcb,
  	       unsigned long		rqeval )
{
	unsigned char	port = p_pcb->port_no;
	rqe_q		*p_rq = p_pcb->rsp_q;
	t_rqe		rqe;
	int		rc;

	rqe.val    = rqeval;
	rqe.f.port = port;

	disable();
	FOOTPRINT( 'e', 'a', 'a' );		/* eaa */
	if ( ReRoute [port] || SleepEn [port] || !Q_EMPTY( p_rq ) )
	{
	    /* If the RQE is to be routed to the port response queue	*/
	    /* (because of ReRoute flag, sleep enable, or something	*/
	    /* is already on the queue), attempt to write it there. 	*/

	    rc = queue_writel( p_rq, rqe.val );
	    if (rc < 0)
		FOOTPRINT( 'e', 'a', 'P' );	/* eaP Queue is full */
	    else { 
		if ( rc == 2 )			/* Queue was empty */
		{
		    FOOTPRINT( 'e', 'a', 'p' );	/* eap */
		    if ( !( work_q[SLEEPWAKE] & p_pcb->mask_on)  )
			SCHED_PRQ_WORK( p_pcb );
		} else 
		    FOOTPRINT( 'e', 'a', 'N' );	/* eaN Queue was non-empty */
	    }
	} else {

	    /* Otherwise, unless this is a receive and receive is not	*/
	    /* enabled, write the RQE directly to the adapter response	*/
	    /* queue; if the queue is full, log an error trace, else,	*/
	    /* interrupt the host (an RQE is available).		*/

	    if (( rqe.f.rtype != RQE_RX_NODMA ) || Recv_Enable )
	    {
	        rc = queue_writel( &arqueue, rqe.val );
	        if (rc < 0)
	            FOOTPRINT( 'e', 'a', 'A' ); 	/* eaA */
	        else {
	            FOOTPRINT( 'e', 'a', 'i' );	/* eai */
	            host_intr( 0 );
	        }
	    }
	}
	FOOTPRINT( 'e', 'a', 'z' );		/* eaz */
	enable();
	return;
}

f_errsts ( unsigned char   port )
{
	volatile t_pcb		*p_pcb;
	eblock			*p_eblk;
	unsigned short		etype;		/* Type from e_block for port */
	t_rqe			rqe;

	p_pcb = &pcb [port];
	p_eblk = &estat [port];

	FOOTPRINT( 'e', 'f', 'a' );		/* efa */
	disable();
	UNSCHED_ERRS_WORK( p_pcb );
	p_pcb->estat = *p_eblk;
	p_eblk->e_type = ETYP_EMPTY;
	enable();

	p_eblk = &p_pcb->estat;

	if ( p_eblk->e_type == ETYP_EMPTY )
	{
	    FOOTPRINT( 'e', 'f', 'A' );		/* efA */
	    return ( 0 );
	}
	if ( p_eblk->e_type & ( ETYP_TIMER_POP | ETYP_FAILSAFE ) )
	{

	    rqe.f.rtype  = RQE_STAT_UNSOL;
	    rqe.f.status = 0;

	    if ( p_pcb->timer_type )
	    {
		rqe.f.sequence = p_pcb->timer_type;
		rqe.f.status = 0;
		p_pcb->timer_type = 0;
		add_rqe( p_pcb, rqe.val );
	    }
	}
	if ( p_pcb->phys_link == PCB_PL_X21 )
		x21_decode ( p_pcb );
	if ( p_pcb->proto & PCB_PRO_SDLC )
		bop_decode( p_pcb );
	else
	    switch( p_pcb->proto ) 
	    {
	        case PCB_PRO_BSC:
		    cop_decode( p_pcb );
		    break;
	        case PCB_PRO_X21:
		    cop_decode( p_pcb );
		    break;
	        case PCB_PRO_ASYNC:
		    asy_decode( p_pcb );
		    break;
	        default:
		    FOOTPRINT( 'e', 'f', 'E' );		/* efE */
		    break;
	    }
	FOOTPRINT( 'e', 'f', 'z' );			/* efz */
	return ( 0 );
}

free_cmdblk ( unsigned char	blk)
{
	FOOTPRINT( 'e', 'c', 'a');			/* eca */
	queue_writeb( &tx_free, blk );
	if ( tx_int_en )
	{
	    tx_int_en--;
	    if ( tx_int_en )
		host_intr( TREG_TXFREE );
	}
}

extern unsigned short	tscc_typ[NUM_PORT];

void  scc_delta( volatile t_pcb		*p_pcb,
		 register unsigned int	 e_ictsr )
{
	register t_rqe		*p_rqe = &( p_pcb->error_rqe );
	bufno			*p_last;

	FOOTPRINT( 'e', 's', 'E' );		/* esE */
	p_rqe->f.rtype = RQE_STAT_UNSOL;
	if ( e_ictsr & ICS_dDCD )		/* Carrier Detect Changed */
	{
		if ( p_pcb->modem_int & MASK_CD )
		{
		    p_rqe->f.sequence = ( e_ictsr & ICS_DCD ) ? CD_ON : CD_OFF;
		    add_rqe ( p_pcb, p_rqe->val );
		}
	}
	if ( e_ictsr & ICS_dCTS )		/* Clear to Send Changed */
	{
		if ( p_pcb->modem_int & MASK_CTS )
		{
		    p_rqe->f.sequence = (e_ictsr & ICS_CTS) ? CTS_ON : CTS_OFF;
		    add_rqe ( p_pcb, p_rqe->val );
		}
	}
	if ( e_ictsr & ICS_CZERO )
	{
		/* if timer running */
		FOOTPRINT( 'e', 's', 'a' );		/* esa */
		if (tscc_typ[p_pcb->port_no] )
		{
			/* make sure no more interrupts come in as this tmr */
			p_pcb->timer_type = 0;
			if (tscc_typ[p_pcb->port_no] == TX_FS_TIMER )
			{
				FOOTPRINT( 'e', 's', 'b' );	/* esb */

				/* If the transmit failsafe timer expired,  */
				/* schedule tx_work to clean up the failed  */
				/* transmit, build tx nak rqe, and ensure   */
				/* transmits continue.  */
				p_pcb->port_status = TX_FS_TIMER;
				SCHED_TX_WORK( p_pcb );
			}
			else
			{
				FOOTPRINT( 'e', 's', 'c' );	/* esc */
				p_rqe->f.sequence = tscc_typ[p_pcb->port_no];
				add_rqe ( p_pcb, p_rqe->val );
			}
			tscc_typ[p_pcb->port_no] = 0;
		}
	}
	FOOTPRINT( 'e', 's', 'X' );			/* esX */
	return;
}

void  cio_delta( volatile t_pcb		*p_pcb,
		 unsigned short		 e_ciodata )
{
	register t_rqe		*p_rqe = &( p_pcb->error_rqe );
	unsigned short		q_temp;

	FOOTPRINT( 'e', 'C', 'a' );			/* eCa */
	p_rqe->f.rtype = RQE_STAT_UNSOL;
	q_temp = p_pcb->cio_last_data ^ e_ciodata;
	if ( q_temp & ~RING_INDICATE )		/* Ring Indicate Line */
	{
	    if ( p_pcb->modem_int & MASK_RI )
	    {
		FOOTPRINT( 'e', 'C', 'r' );			/* eCr */
		p_rqe->f.sequence = 
			( e_ciodata & ~RING_INDICATE ) ? RI_OFF : RI_ON;
		add_rqe ( p_pcb, p_rqe->val );
	    }
	}
	if ( q_temp & ~DATA_SET_READY )		/* Data Set Ready Line */
	{
	    if ( p_pcb->modem_int & MASK_DSR )
	    {
		FOOTPRINT( 'e', 'C', 'd' );			/* eCd */
		p_rqe->f.sequence = 
			( e_ciodata & ~DATA_SET_READY ) ? DSR_OFF : DSR_ON;
		add_rqe ( p_pcb, p_rqe->val );
	    }
	}
	p_pcb->cio_last_data = e_ciodata;

	return;
}

/*
   Offlevel port response queue (PRQ) work.  Several sources exist.
   Any time response queue re-routing is active, which occurs when a
   port service function is sleeping or responses are bottled up pending
   Bus Master DMA service.
*/

void 
f_prqwork( unsigned short     p_no )
{
	register volatile t_pcb		*p_pcb = &pcb [p_no];
	rqe_q				*p_rqe = p_pcb->rsp_q;
	t_rqe				 l_rqe;
	int				 rc;

	FOOTPRINT( 'p', 'f', 'a' );		/* pfa */
	
	/* First check PRQ with queue_previewl to prevent having to put RQE  */
	/* back PRQ to do RECV_DMA work if it is RQE_RX_DMA.  Else, if not   */
	/* RQE_RX_DMA pull the RQE off of the PRQ with queue_readl. 	     */
	if (( l_rqe.val = queue_previewl( p_rqe )) == (unsigned long)-1 )
	{
	    ReRoute [p_no] = FALSE;
	    UNSCHED_PRQ_WORK( p_pcb );
	    FOOTPRINT( 'p', 'f', 'A' );		/* pfA */
	    return;
	}

	if ( l_rqe.f.rtype == RQE_RX_DMA )
	{
/*
   The interface to the Rx side of Bus Master DMA is best understood by
   careful studying of intzero.c, where the channel is allocated between
   transmit and receive data requirements.  It is critical that we un-
   schedule ourselves because, if we ran again before the receive data
   we dispatch, we would re-send the same block.  The Bus Master terminal
   count interrupt service will reshedule PRQ_WORK if applicable.
*/
	    UNSCHED_PRQ_WORK( p_pcb );
	    ReRoute [p_no] = TRUE;
	    FOOTPRINT( 'p', 'f', 'r' );		/* pfr */
	    /* schedule bus master receive DMA work */
            disable();
            Dma_Count += 1;
            work_q [ RECV_DMA_WORK ] = B_TRUE;
            enable();
	    return;
	}

	/* It wasn't a RQE_RX_DMA RQE, so now pull the RQE off the PRQ  */
	/* to process it.						*/
	l_rqe.val = queue_readl( p_rqe );
	if ( SleepEn [p_no] )
	{
/*
   If a scheduler service level for a port is sleeping, i.e. handling all
   data requests, every RQE is sent to the sleeping function.  The sleeping
   function then dispatches driver-bound RQEs directly by interfacing
   queue_writel specifying arqueue, thus bypassing additional re-routing.
   Only a sleep_disable (or other method of resetting SleepEn) will terminate
   sleep-induced RQE rerouting.  Sleep_disable additionally reschedules us.
*/
	    FOOTPRINT( 'p', 'f', 's' );	/* pfs */
	    p_pcb->sleep_rqe = l_rqe;
	    SCHED_SLEEPWAKE( p_pcb );
	    UNSCHED_PRQ_WORK( p_pcb );
	    return;
	}
/*
   Everything is normal and the PRQ is just buffering RQEs, presumably those
   backlogged while waiting for Rx DMA to complete.  We will, slowly but
   surely, flush out the PRQ and turn off ReRoute.
*/
	FOOTPRINT( 'p', 'f', 'w' );		/* pfw */
	rc = queue_writel( &arqueue, l_rqe.val );
	if ( rc < 0 )
	    FOOTPRINT( 'p', 'f', 'F' );		/* pfF */
	else
	    host_intr( 0 );		/* Type 0 = ARQ went non-empty */

/*
   Notice that we leave PRQ_WORK scheduled on exit.  The next time we run
   for this port, we'll unschedule ourselves if the queue is empty.  This
   seems more efficient than calling Q_EMPTY now.  This is also impor-
   tant because the Rx ISR unconditionally reschedules PRQ_WORK, so this
   function is called to clean up afterwards.
*/
	FOOTPRINT( 'p', 'f', 'z' );		/* pfz */
	return;
}


t_pcb *
sleep( register volatile t_pcb    *p_pcb )
{
	extern volatile t_pcb	*ProcSleep ();

	FOOTPRINT( 'e', 'e', 's' );		/* ees */

	/* if wakeup already scheduled, don't go to sleep */
	if ( ( work_q[SLEEPWAKE] & p_pcb->mask_on)  )
	{
		FOOTPRINT( 'e', 'e', 'w' );		/* eew */
		return( p_pcb );
	}
	if ( !Q_EMPTY( p_pcb->rsp_q ) )
	    SCHED_PRQ_WORK( p_pcb );
	while (TRUE)
	{
	    p_pcb = ProcSleep( p_pcb->port_no, p_pcb );
	    UNSCHED_SLEEPWAKE( p_pcb );
	    if ( p_pcb->sleep_rqe.val != (unsigned long)-1 )
		break;
	}
	return ( p_pcb );
}

void  sleep_enable( volatile t_pcb    *p_pcb )
{
	FOOTPRINT( 'e', 'e', 'a' );		/* eea */
	if ( SleepEn [ p_pcb->port_no ] )
	{
	/*
	   This is an error, and fairly major.  A task on some scheduler level
	   is sleeping, i.e. using the stack frame for this port already.  Two
	   scheduler levels can't run re-routing state machines simultaneously.
	*/
	    FOOTPRINT( 'e', 'e', 'E' );		/* eeE */
	    return;
	}
	SleepEn [ p_pcb->port_no ] = SLEEPWAKE;
	return;
}

/*
   Because of their different priority levels, PRQ_WORK has to unschedule
   itself for a particular port when it reroutes and dispatches.  If RQEs
   remain in the port response queue, they must be flushed, so PRQ_WORK
   is scheduled after SleepEn is cleared.  This sequence should be preserved
   in case a pre-emptable scheduler is written (for obvious reasons).
*/

void  sleep_disable( register volatile t_pcb    *p_pcb )
{
	FOOTPRINT( 'e', 'D', 'a' );		/* eDa */
	SleepEn [ p_pcb->port_no ] = FALSE;
	UNSCHED_SLEEPWAKE( p_pcb );
	if ( !Q_EMPTY( p_pcb->rsp_q ))
	    SCHED_PRQ_WORK( p_pcb );
	return;
}


/****************************************************************************
  When operation of a port begins, which line changes are allowed to cause
  interrupts, i.e. generate RQEs (for either the driver or state machines),
  is decided and a modem interrupt mask is passed which contains config-
  uration and protocol specific data.  This mask is respected by the protocol
  specific error handlers, which may threshhold error counts and send RQEs
  selectively, if desired.
 ****************************************************************************/

/*
  The unsigned character parameter to "mi_enable" is composed of the following
  bit assignments:

  0x80 = CTS     0x40 = DSR     0x20 = RI     0x10 = CD
  0x08 = TXRX    0x04 = X21     0x02 = TXRDY  0x01 = RXRDY
*/

mi_enable( register volatile t_pcb	*p_pcb,
	   unsigned char		int_mask )
{
	ioptr			scc_io = p_pcb->scc_base;
	unsigned char		temp;
	unsigned char		int_en;		/* Copy of SCC IE register */

/*
   CTS and CD are checked together because they are controlled by a single
   interrupt enable bit on the DUSCC.  The active interrupt mask must be
   interrogated in the offlevel to see if the change was important.
*/

	int_en = 0;				/* No scc interrupts */
	if ( !( int_mask & MASK_X21 ) )
		if ( int_mask & ( MASK_CTS | MASK_CD ) )	/* SCC lines */
			int_en |= IE_DELTA;
/*
   The RI and DSR lines come in through the CIO.
*/
	if ( !( int_mask & MASK_X21 ) )
	{
		ioptr		p_cmd  = p_pcb->cio_base + P_A_CS;

		if ( ODD(p_pcb->port_no ))		/* Odd, CIO port B */
		{
			p_cmd += 2;			/* Command/Status B */
		}
		out08( p_cmd, CS_DIS_I );		/* Disable interrupts */

		if ( int_mask & ( MASK_RI | MASK_DSR ) )
		{
/*
   Wire ringing on the FOB and the associated noise made explicit enabling
   of the CIO lines RI and DSR mandatory.  Otherwise, noise at the clock
   frequency would interrupt constantly and prohibit normal adapter function.
*/
			unsigned char	r_ptran = 0;

			if ( int_mask & MASK_RI )
				r_ptran |= CIOD_RI;
			if ( int_mask & MASK_DSR )
				r_ptran |= CIOD_DSR;

			out08( p_pcb->alt_cio + P_PTRAN, r_ptran );

			out08( p_cmd, CS_CLR_I );
			out08( p_cmd, CS_EN_I | CS_ERR_I );
		}

		p_pcb->cio_last_data = in08( p_pcb->cio_data );
	}

	if ( int_mask & MASK_X21 )
	{
		/* Enable X21 in enable register */
		out08( ENREG, ( ( in08( ENREG ) ) & ENR_C_X21 ) );
	}

/*
  The Tx/Rx interrupt enable bit controls SCC interruption for changes in
  selected Receiver and Transmitter/Receiver Status registers.
*/

	if ( int_mask & MASK_TXRX )
	{
		extern unsigned char	trsMask[],rsMask[];
		unsigned char		port = p_pcb->port_no;

/* Tx/Rx Status: DMA Underrun, CTS Underrun, Frame Complete and SOM Ack */

		if ( trsMask [port] & TRS_IE )
			int_en |= IE_TRSR;

/* Rx Status: 7/6 for EOM detect, PAD and or Abort error */

		if ( rsMask [port] & RS_IE_76 )
			int_en |= IE_RSR_76;

/* Rx Status: 5/4 for DMA Overrun, Short Frame Detect */

		if ( rsMask [port] & RS_IE_54 )
			int_en |= IE_RSR_54;

/* Rx Status: 3/2 for Idle and SYN/Flag detect */

		if ( rsMask [port] & RS_IE_32 )
			int_en |= IE_RSR_32;

/* Rx Status: 1/0 for CRC, parity and other framing related errors */

		if ( rsMask [port] & RS_IE_10 )
			int_en |= IE_RSR_10;
	}

	if ( int_mask & MASK_TXRDY )		/* Transmitter Ready */
		int_en |= IE_TXRDY;
	if ( int_mask & MASK_RXRDY )		/* Receiver Ready */
		int_en |= IE_RXRDY;

	/* Clear any interrupts already pending */
	out08( scc_io + SCC_ICTS, in08( scc_io + SCC_ICTS ) );

	out08( scc_io + SCC_IE, int_en );

	p_pcb->modem_int = int_mask;
	return ( 0 );
}

/*
   Return the current modem line status.
*/

unsigned char   modem_flags( volatile t_pcb    *p_pcb )
{
	register unsigned char	output = 0;
	unsigned char		scc_data,cio_data;

	scc_data = in08( p_pcb->scc_base + SCC_ICTS );
	cio_data = in08( p_pcb->cio_data );

	if ( scc_data & ICS_DCD )
		output |= MASK_CD;
	if ( scc_data & ICS_CTS )
		output |= MASK_CTS;
	if ( !( cio_data & ~DATA_SET_READY ) )
		output |= MASK_DSR;
	if ( !( cio_data & ~RING_INDICATE ) )
		output |= MASK_RI;
	return ( output );
}

/*
   Get the current modem line status
*/

get_modem_stats( register cmd_blk		*p_cmd,
		 unsigned char far		*p_buf  )
{
	register volatile t_pcb		*p_pcb = &pcb [p_cmd->_port];
	register unsigned short		port;
	t_rqe				l_rqe;

	l_rqe.f.rtype    = RQE_STAT_SOL;
	l_rqe.f.port     = port = p_cmd->_port;
	l_rqe.f.status   = 0;
	l_rqe.f.sequence = modem_flags( ( p_pcb = &pcb [port] ) );

	add_rqe( p_pcb, l_rqe.val );
	return ( ~PCQ_LOCK );
}

/*
   When the Error/Status offlevel detects either a transmit frame failure
   or catastrophic loss of link, any active transmit must be aborted.  This
   also occurs if the transmit CTS failsafe timeout is reached.
   The channel indicated by txchan must be stopped, if running.  The next
   channel will be started by xmit_frame if already set up.

   Starting the next channel isn't particularly applicable if the link went
   down.  A method for aborting additional transmits queued to the board
   without starting them and having them time out must be devised.  The
   Error/Status offlevel must be able to communicate this information with
   either the port command queue work processor or, perhaps, TxCommon.
   Think about checking a link status in the PCB at command dequeue time.
*/

extern void	xmit_frame( volatile t_pcb * );

void tx_abort ( register volatile t_pcb	*p_pcb,
		unsigned char		r_type )
{
	ioptr		p_dma;
	register bufno	*p_last;
	t_rqe		qe;
	cmd_t		*p_cmd;

	if (ODD(p_pcb->txchan))		/* Odd (High) is in use */
	{
		p_dma  = p_pcb->tx_dma_1;
		p_last = &p_pcb->txbuf1;
	}
	else
	{
		p_dma  = p_pcb->tx_dma_0;
		p_last = &p_pcb->txbuf0;
	}

	if ( *p_last != NO_BUFNO )		/* Is a channel xmitting? */
	{
		p_cmd = (cmd_t *)&cmdblk [*p_last];

		/* Stop the operating channel immediately */
		out16 ( p_dma, ( in16( p_dma ) ) & ~PDMA_CCW_EN );

		qe.f.rtype    = RQE_TX_NAK;
		qe.f.port     = p_cmd->port;
		qe.f.status   = r_type;
		qe.f.sequence = p_cmd->sequence;
		if(r_type == STOP_PORT)
			qe.f.rtype    = RQE_CMD_ACK;
		add_rqe( p_pcb, qe.val );	/* Send, ReRoute? */

		p_cmd->flags &= ~TX_CMPL_ACK;	/* Don't let xmit_frame send */
						/* another RQE for the frame */
		SCHED_TX_WORK( p_pcb );

		/* if rx timer specified and not intermediate frame, start it */
		if ( ( p_pcb->rx_timeout ) && ( p_cmd->flags & TX_FINAL_BLK ) )
		{
		    start_duscc_timer( p_pcb, p_pcb->rx_timeout, RX_TIMER );
		}
	}
	return;
}
