static char sccsid[] = "@(#)45	1.15  src/bos/usr/lib/asw/mpqp/transmit.c, ucodmpqp, bos41J, 9519A_all 5/5/95 10:33:11";
/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS: 
 *	txCommon		: Preprocess one frame ahead.
 *	xmit_frame		: When previous frame completes,
 *				  start next frame if ready.
 *	start_frame		: Start DMA to transmit frame.
 *	f_txwork		: Unschedule transmit work and call xmit_frame.
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

#include "mpqp.def"			/* Constant/Macro definitions */
#include "mpqp.typ"			/* Type declarations */
#include "mpqp.ext"			/* External data objects */
#include "mpqp.pro"			/* Function parameter lists */
#include "mpqp.h"			/* Adapter Hardware Definitions */
#include "portcb.typ"			/* PCB structure definition */
#include "portcb.def"			/* PCB definitions */
#include "iface.def"			/* RQE definitions, etc. */

extern unsigned char		txcmd[NUM_PORT];
extern unsigned char		txftype[NUM_PORT];

extern unsigned short		txccw[NUM_PORT];
extern unsigned short		txlgt[NUM_PORT];
extern unsigned short		txflgt[NUM_PORT];

extern unsigned short		txioa[NUM_PORT];
extern unsigned short		txstat[NUM_PORT];

extern unsigned short		tscc_val[NUM_PORT];
extern unsigned short		tscc_sval[NUM_PORT];
extern unsigned short		tscc_typ[NUM_PORT];
extern unsigned short		tscc_styp[NUM_PORT];


#define MAJOR( x )     ((( x ) >> 4) & 0xF)

extern t_pdma_setup		pdma_cdt;	/* Channel Descriptor Table */
extern unsigned short		InitPDMA();

extern void	xmit_frame(),start_frame();
extern unsigned short           rx_size[];

txCommon ( cmd_t			*p_cmd,
	   unsigned char 		q_cmd  )
{
register volatile t_pcb		*p_pcb = &pcb [p_cmd->port];
register unsigned		temp;
ioptr				p_dma,txfifo;
bufno				*p_last,*p_next;

/*
   If both txbuf fields are in use, we can't accept the command now.
   If either channel is running, it is "last".  If "last" is running,
   "next" can be preprocessed but DMA cannot be initiated.  In this
   case, "next" is set and the port is unscheduled.  The command queue
   is not backed up, as the command has been handled.
   If, on entry, a "next" has already been processed, the port command
   queue is backed up and the port unscheduled.
   In both cases where the PCQ_WORK level for a port is cleared, it will
   be the f_txwork offlevel that restarts port command processing.
*/
	FOOTPRINT( 'T', 't', 'a' );		/* Tta */
	if (ODD( p_pcb->txchan ))		/* Odd (High) is in use */
	{
		p_last = &p_pcb->txbuf1;
		p_next = &p_pcb->txbuf0;
		p_dma  = p_pcb->tx_dma_0;
	}
	else
	{
		p_last = &p_pcb->txbuf0;
		p_next = &p_pcb->txbuf1;
		p_dma  = p_pcb->tx_dma_1;
	}

/* 
    If p_next is not equal to NO_BUFNO, the next transmit has already been 
    set up.  Insert this at the beginning of the port command queue and 
    wait until the transmit sending the current p_last completes.  When the 
    current p_last completes, p_last gets freed (set to NO_BUFNO) and becomes 
    p_next, p_next becomes p_last.
*/
	if ( *p_next != NO_BUFNO )
	{
		FOOTPRINT( 'T', 't', 'b' );	/* Ttb */
		queue_insertb( p_pcb->cmd_q, q_cmd );
		return ( PCQ_LOCK );
	}

	/*  Where is data?  If TX Short, the transmit data is in the	*/
	/*  data portion of the command block, else it is in the	*/
	/*  transmit buffer linked to this command block.		*/

	if ( p_cmd->type == TX_SHORT )
	    p_cmd->card_addr = &( p_cmd->cs.data [0] );

/*
   Invoke the protocol-specific transmit data preprocessor.  What happens
   there is irrelevant except that the updated length is returned.  A
   length of zero will terminate the command immediately, exactly as if it
   ended normally.  A return value of -1 means that, during processing,
   the buffer exceeded its allowable maximum size.  An error will be
   sent to the driver.
*/

	temp = ( *(p_pcb->f_txserve) )( p_cmd, p_pcb );
	if ( temp == 0 )
	{
		/* No data length.  See notes above.  */
		FOOTPRINT( 'T', 't', 'L' );	/* TtL */
		return ( ~PCQ_LOCK );
	}
	if ( temp == (unsigned short)-1 )
	{
		/* Buffer size exceeded.  See notes above.  */
		FOOTPRINT( 'T', 't', 'B' );	/* TtB */
		return ( ~PCQ_LOCK );
	}

	p_cmd->length = temp;

	/* Dispatch the work */

	pdma_cdt._ioa = p_pcb->scc_base + SCC_TXFIFO;
	pdma_cdt._ca  = p_cmd->card_addr;
	pdma_cdt._tc  = p_cmd->length;
	rx_size[p_cmd->port]=p_cmd->length;
	SetPDMA( p_dma );

	*p_next = (cmd_blk *)p_cmd - &cmdblk[0];
/*
   The exit path below should (emphasis) serialize transmits so that a Port
   DMA channel which has terminal counted and performed EOM handshaking with
   the SCC but *hasn't really emptied the FIFO and sent the applicable FCS*
   can't ever have another DMA channel started on top of it.
*/
	if ( *p_last != NO_BUFNO )
	{
		FOOTPRINT( 'T', 't', 'N' );	/* TtN */
		return ( PCQ_LOCK );
	}

	(*(p_pcb->f_txframe))( p_pcb );

	FOOTPRINT( 'T', 't', 'z' );		/* Ttz */
	return ( ~PCQ_LOCK );
}

tx_ack ( t_pcb		*p_pcb,
	 bufno		*p_last,
	 bufno		*p_next ) 
{
	register cmd_t		*p_cmd;

    	FOOTPRINT( 'T', 'a', 'E' );	/* TaE */
	p_cmd = (cmd_t *)&cmdblk[*p_last];

	if ( p_cmd->flags & TX_CMPL_ACK )
	{
	t_rqe				qe;

		qe.f.rtype    = RQE_TX_ACK;
		qe.f.port     = p_cmd->port;
		qe.f.status   = 0;
		qe.f.sequence = p_cmd->sequence;

		/* if tx timer popped, return NAK status */
		if ( p_pcb->port_status == TX_FS_TIMER )
		{
    			FOOTPRINT( 'T', 'a', 'p' );	/* Tap */
			qe.f.rtype = RQE_TX_NAK;
			qe.f.status = TX_FS_TIMER;
			p_pcb->port_status = 0; /* clear port_status since    */
						/* failsafe timer was reported*/
		}
		add_rqe( p_pcb, qe.val );	/* Send, ReRoute? */
	}

	/* if not in data transfer state, don't start rx timer */
	if ( p_pcb->port_state == DATA_TRANSFER)
	{
		/* Start rx timer if specified and not intermediate frame */
		if (( p_pcb->rx_timeout ) && ( p_cmd->flags & TX_FINAL_BLK ))
		{
    			FOOTPRINT( 'T', 'a', 's' );		/* Tas */
			start_duscc_timer( p_pcb, p_pcb->rx_timeout, RX_TIMER );
		}
		/* Only turn off timer if it is the transmit failsafe timer */
		/* that is running and if next transmit has not started.    */
		else if ( ( p_pcb->timer_type == TX_FS_TIMER ) &&
			  ( *p_next == NO_BUFNO ) )
		{
    			FOOTPRINT( 'T', 'a', 'r' );		/* Tar */
			/* turn off transmit failsafe timer */
			stop_duscc_timer( p_pcb );
		}

	}
        p_pcb->bsc_prev_tx = p_pcb->tx_state;        /* shift states */
/*
  Free the completed command block.
  Schedule PCQWORK, command processing may be waiting for a TxBuf to free.
*/
	if ( !( p_cmd->flags & TX_NO_FREE ) )
	{
    		FOOTPRINT( 'T', 'a', 'f' );		/* Taf */
		free_cmdblk( *p_last );
	}
	/* Even if the completed command block does not get freed, p_last
	   should be freed, so another transmit can be accepted.  The
	   code that set the flag TX_NO_FREE will free the command block.
	*/
	*p_last = NO_BUFNO;			/* Free used channel */
	work_q [PCQ_WORK] |= p_pcb->mask_on;
	return;
}

void xmit_frame( 
	register volatile t_pcb	*p_pcb )
{
	unsigned char		port = p_pcb->port_no;
	ioptr			p_dma;
	bufno			*p_last,*p_next;
	int			bak2bak = 0;
	register ioptr		p_scc = p_pcb->scc_base;

	FOOTPRINT( 'T', 'x', 'a' );		/* Txa */
	if ( p_pcb->txchan & 1 )		/* Odd (High) is in use */
	{
		p_last = &p_pcb->txbuf1;
		p_next = &p_pcb->txbuf0;
		p_dma  = p_pcb->tx_dma_0;
	}
	else
	{
		p_last = &p_pcb->txbuf0;
		p_next = &p_pcb->txbuf1;
		p_dma  = p_pcb->tx_dma_1;
	}
/*
   The availability of a frame to transmit is detected by checking the next
   (not in-use) txbuf field.
*/

	/* was there a failure on the last transmit? */
	if ( *p_last != NO_BUFNO )
	{

		if ( p_pcb->port_status == TX_FS_TIMER &&
			p_pcb->start_parm & PCB_SP_LEASED )
		{
			cmd_blk 	*p_cmd;
			unsigned char	q_cmd;
		
			/* turn on port work for this port */
			work_q [PCQ_WORK] |= p_pcb->mask_on;
			
			/* if we have transmits queued up get rid of them */
			while(1)
			{
				/* Check the first queue element on the     */
				/* command queue.  If not empty, or element */
				/* is not PCQ_LOCK, then read the element   */
				/* from the queue.  This will prevent doing */
				/* unnecessary queue_insertb's.		    */
				q_cmd = queue_previewb( p_pcb->cmd_q );

				/* stop if the queue is empty */
				if ( q_cmd == (unsigned char)0xFF ) {
				    FOOTPRINT( 'T', 'x', 'b' );	 /* Txb */
	    			    break;
				}
				
				/* stop if there is a lock */
				if ( q_cmd == PCQ_LOCK ) {
				    FOOTPRINT( 'T', 'x', 'c' );	 /* Txc */
	    			    break;
				}
				
        			q_cmd = queue_readb( p_pcb->cmd_q );

				/* is this a transmit? */
				p_cmd = &cmdblk[q_cmd];
				if ( (p_cmd->_type == TX_SHORT) ||
				     (p_cmd->_type == TX_LONG) ||
				     (p_cmd->_type == TX_GATHER) ) {
					/* free it and get the next one */
				        FOOTPRINT( 'T', 'x', 'd' ); /* Txd */
					free_cmdblk( q_cmd );
					continue;
				}
				
				/* stop because it's not a transmit */
				FOOTPRINT( 'T', 'x', 'e' ); /* Txe */
	    			queue_insertb( p_pcb->cmd_q, q_cmd );
				break;
			}
			
			/* if the next transmit is already set up free it */
			if ( *p_next != NO_BUFNO )
			{
				free_cmdblk( *p_next );
				*p_next=NO_BUFNO;
			}
		}
	}

	if ( *p_next != NO_BUFNO )
	{
	    ++p_pcb->txchan;			/* Increment channel in use */
	    start_frame( p_pcb, p_dma, port, bak2bak );
	}

	if ( *p_last != NO_BUFNO )		/* Clean up from last tx */
	{
		bak2bak = 1; 			/* frame just finished */
		tx_ack( p_pcb, p_last, p_next);		/* clean up last tx */

		if ( *p_next == NO_BUFNO )		/* Next is not ready? */
		{
			if (p_pcb->proto & PCB_PRO_SDLC_HD)
			{
			    out08( p_scc + SCC_CC,  DISABLE_TX );
	    		    FOOTPRINT( 'T', 'x', 'o' );		/* Txo */
			}
			else
	    		    FOOTPRINT( 'T', 'x', 'n' );		/* Txn */
		}
	}

	FOOTPRINT( 'T', 'x', 'z' );				/* Txz */
	return;
}

/*
   The global txcmd field is composed of up to two commands which should
   be sent to the port command register before initiating Echo DMA, i.e.
   starting the data movement.  The Tx Enable command is always issued,
   followed by some type of SOM and, potentially, another.  The actual
   DMA is started by the SCC ISR upon completion of the SOM operation.

   In the case of async transmits, SOM is not used, because there are no
   leading flags or sync characters.  Therefore, for async the transmitter
   is enabled, and then the DMA is started.  Transmission will start when
   the tx characters are loaded into the TX FIFO.
*/

void start_frame( register volatile t_pcb	*p_pcb,
		  ioptr				 p_dma,
		  unsigned char			 port, 
		  int				 bak2bak )
{
	register ioptr		p_scc = p_pcb->scc_base;
	unsigned short		temp_ccw;

	FOOTPRINT( 'T', 's', 'a' );			/* Tsa */
	txioa[port]   = p_dma;
	txlgt[port]   = txflgt[port];
	txftype[port] = txcmd[port];

	/* Enable the SCC transmitter */
	out08( p_scc + SCC_CC, CC_ENABLE_TX );

	/* Drive RTS */
      	out08( p_scc + SCC_OM, ( p_pcb->scc_reg.om |= OM_RTS_ACTIVE ) );

	if ( ( txftype[port] & FT_A ) || ( bak2bak) )		/* Async? */
	{
	    FOOTPRINT( 'T', 's', 'A' );		/* TsA */
	    out16( p_dma, (temp_ccw = in16( p_dma ) | PDMA_CCW_EN ) );
	}
	else
	{
	    /*  Start the TX Failsafe timer -- the timeout is	*/
	    /*  based on the length of the frame in bytes (L), by 	*/
	    /*  the following:  the maximum timeout occurs for the	*/
	    /*  minimum baud rate of 300, so the maximum frame 	*/
	    /*  time is then:					*/
	    /*							*/
	    /*                 8L                                   */
	    /*           T  =  ---	       			        */
	    /*                 300                                  */
	    /*                                                      */
	    /*  Since the timer hardware uses tenths of seconds,	*/
	    /*  the actual timeout value (number of ticks) is:	*/
	    /*                                                      */
	    /* 	            8L	             4L		        */
	    /*           T  =  ---- * 10   =    ----                */
	    /*                  300              15                 */
	    /*                                                      */
	    /*  This is precisely the number of ticks it takes 	*/
	    /*  for the frame to complete transmission at 300 	*/
	    /*  baud; a more realistic value would have a 50%	*/
	    /*  margin (longer timeout) to accomodate timing 	*/
	    /*  errors or processing overhead:			*/
	    /*							*/
	    /*                  4L             8L         L         */
	    /*	     T  =  ---- * 2   =   ----   ~   ---        */
	    /*                  15             15         2         */
	    /*                                                      */
	    /*  This brings the timeout value to roughly one half	*/
	    /*  the number of bytes in the frame, so a simple	*/
	    /*  shift right is performed to derive the value.	*/

	    /* Set failsafe timer if in DATA_TRANSFER state */
	    if ( ((p_pcb->port_state != V25_CALL_DIALING) &&
		  (p_pcb->port_state != WAIT_FOR_CONNECT) ) &&
		  (!((p_pcb->phys_link & PCB_PL_X21) && 
		  (p_pcb->port_state != DATA_TRANSFER))) )
	    {
	        p_pcb->timer_type = TX_FS_TIMER;
		tscc_typ[port] = TX_FS_TIMER;
		tscc_val[port] = MAX( 300, in16( p_dma + PDMA_TC ));

		start_duscc_timer( p_pcb, tscc_val[port], tscc_typ[port] );
	    }
	    /* Idle FLAGs after intermediate SDLC frames (final bit off) */
	    /* and MARKs after the final SDLC frame and in half-duplex.  */
	    if ( txftype[port] & FT_S )  { 		 /* If SDLC frame.... */
	        if ( ( txftype[port] & FT_FINAL) ||     /* and final frame*/
		  (p_pcb->proto & PCB_PRO_SDLC_HD) ) /* or half-duplx */
                    SET_TPR( TPR & TP_B_IMARK );        /* idle MARKs */
                else                                    /* otherwise, */
                    SET_TPR( TPR | TP_B_IFLAG );        /* idle FLAGs */
	    }

	    if (( (p_pcb->phys_link & PCB_PL_X21) && 
		  (p_pcb->x21_state != DATA_XFER) ) ||  /* If X.21 call phase*/
	        ( (p_pcb->baud_rate != 0) &&            /* or internal clock */
                  ( rx_size[port]<= 16) &&               /* with small frame  */
		  ( p_pcb->proto & PCB_PRO_SDLC_HD) ) ) /* in half_duplex,   */
	    {			           /* wait for SOM ack to enable DMA.*/
		FOOTPRINT( 'T', 's', 'i' );             /* Tsi */
                out08( p_scc + SCC_TXFIFO, p_pcb->tx_pad );
                out08( p_scc + SCC_TXFIFO, p_pcb->tx_pad );
                out08( p_scc + SCC_CC, CC_TX_SOMP );    /* Enable TX */
	    } 
	    else					/* all other cases */
	    {
		FOOTPRINT( 'T', 's', 'e' );		/* Tse */
		out08( p_scc + SCC_CC, CC_TX_SOM );
		out16(p_dma, (txccw[port] = in16(p_dma) | PDMA_CCW_EN));
		txccw[port] = 0;	/* clear ccw for SOM ack ISR */
	    }
	}
	FOOTPRINT( 'T', 's', 'z' );				/* Tsz */
	return;
}

/*
   On entry to the tx_work processor off-level, no transmit may be in progress.
   This is a luxury, as pertinent data can be removed from volatile regions and
   then the alternate channel started, if already prepared.
*/

void f_txwork( unsigned char		port )
{
	register volatile t_pcb		*p_pcb = &pcb [port];

	FOOTPRINT( 'T', 'f', 'a' );			/* Tfa */
	UNSCHED_TX_WORK( p_pcb );
	
	/*  Call the protocol-specific start-frame routine:	*/

	(*(p_pcb->f_txframe))( p_pcb );
	FOOTPRINT( 'T', 'f', 'z' );			/* Tfz */
	return;
}
