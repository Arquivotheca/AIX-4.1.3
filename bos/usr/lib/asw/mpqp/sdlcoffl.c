static char sccsid[] = "@(#)38	1.14  src/bos/usr/lib/asw/mpqp/sdlcoffl.c, ucodmpqp, bos411, 9428A410j 7/20/93 12:33:35";

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS: 
 *	rx_post_sdlc		: SDLC receive post processor. Builds
 *				  buffer header, checks for DMA, DUSCC
 *				  errors.
 *	tx_pre_sdlc		: SDLC transmit preprocessor.  Sets up
 *				  echo DMA.
 *	bop_decode		: Checks for SCC and CIO errors
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
#include "mpqp.h"			/* External data objects */
#include "mpqp.pro"			/* Function parameter lists */
#include "portcb.typ"			/* blah, blah, blah... */
#include "portcb.def"			/*  */
#include "iface.def"			/* adapter cmd and rsp queue values */

extern void tx_abort ( volatile t_pcb  *, unsigned char );
extern void disable_rx ( volatile t_pcb * );

extern void scc_delta ( volatile t_pcb *, unsigned short );
extern void cio_delta ( volatile t_pcb *, unsigned short );

/*****************************************************************************
 **   ******    *******    *****    *******   *******   *     *   *******   **
 **   *     *   *         *     *   *            *      *     *   *         **
 **   *     *   *         *         *            *      *     *   *         **
 **   ******    ******    *         ******       *      *     *   ******    **
 **   *   *     *         *         *            *       *   *    *         **
 **   *    *    *         *     *   *            *        * *     *         **
 **   *     *   *******    *****    *******   ********     *      *******   **
 *****************************************************************************/

extern unsigned short		rx_bsiz;	/* rx buffer size */

typedef struct
{	rx_buf_hdr	rbh;
	unsigned char	data[1];
} rb;

int rx_post_sdlc (register volatile t_pcb	*p_pcb, 
		  rx_buf_hdr far		*p_buf)
{
unsigned short		tc;		/* DMA terminal count value	*/
unsigned char		dma_stat;	/* Echo DMA status reg value 	*/
unsigned char		scc_stat;	/* DUSCC recvr stat reg	*/
t_rqe			*p_rqe = &( p_pcb->error_rqe );
unsigned char 		rx_ctl;		/* pointer to data in rx buffer */
#define RQE_RX_POLL	RQE_CPS_ACK	/* intermediate rqe type for state
					* machine process.  If RX_DMA, the frame
					* is DMAed before state machine gets
					* to process it. */
 
	FOOTPRINT( 's', 'r', 'a' );		/* sra */
	/* Read in DMA status, DUSCC status and TC from buffer */
	dma_stat = p_buf->offset >> 8;
	tc	 = p_buf->length;
	scc_stat = p_buf->type;
	/* init RQE fields */
	p_rqe->f.port     = p_pcb->port_no;
	p_rqe->f.status   = p_pcb->rx_last;
	p_rqe->f.sequence = 0;
	p_rqe->f.rtype = RQE_RX_DMA;

/*	There are some errors that the DUSCC seems to flag erroneously.
	In the case of abort frames, sometimes aborts are detected when
	frames are good.  Therefore, if EOM and abort set simultaneously,
	reset abort indication.

	In the case of short frames, just check length manually.
	Frame must be at least 2 bytes (FCS not included).

	In addition, some status bits are set in the duscc after the
	DMA channel has read it, therefore, read the receive status
	again and or it into the status saved by the DMA channel */

	if ( ( scc_stat & RS_B_ABORT ) && ( scc_stat & RS_B_EOM ) )
		scc_stat &= ~RS_B_ABORT;

	p_buf->length = p_pcb->rx_flgt - tc;

	/* if no receive errors */
	if (( !( scc_stat & ~( RS_B_EOM | RS_B_IDLE | RS_B_FLAG ) ) ) &&
	    (dma_stat & DMA_ISR_EOM) )
	{
		/* write receive buffer header */
		p_buf->offset = p_pcb->rx_off;				/*260*/
		p_buf->type = BUF_STAT_NOERR;


		/* if poll frame and autoresp, change rqe type */
		if ( SleepEn[ p_pcb->port_no ] )
		{
			FOOTPRINT( 's', 'r', 'p' );		/* srp */
			if ( p_pcb->port_state == DATA_TRANSFER )
			{
				p_rqe->f.rtype = RQE_RX_POLL;
				p_buf->type = BUF_STAT_AR_D;
			}
		}
		else
		{
/*
   If the frame is going to the host system, start RQE rerouting or else the
   frame won't be dispatched to the receive DMA handler on level BMWORK.
*/
			FOOTPRINT( 's', 'r', 'r' );		/* srr */
			ReRoute [p_pcb->port_no] = 0xFF;
		}

		/* Port data movement statistics */
		p_pcb->rx_bytes += p_buf->length;			/*233*/
		p_pcb->rx_frames++;					/*233*/
	}
	else
	{
		/* Since a receive error occurred, throw the data away */
		p_rqe->f.rtype = RQE_RX_NODMA;

		/* put on ARQ */
		ReRoute [p_pcb->port_no] = FALSE;

		/* if EOM not set, buffer overflowed */
	   	if ( !(dma_stat & DMA_ISR_EOM) ) {
			FOOTPRINT( 's', 'r', 'O' );		/* srO */
	 		p_buf->type = BUF_STAT_OVFLW;
			p_rqe->f.sequence = BUF_STAT_OVFLW;
		}
		else if (scc_stat & RS_B_CRC) {
			FOOTPRINT( 's', 'r', 'C' );		/* srC */
			p_rqe->f.sequence = FRAME_CRC;
		} else if (scc_stat & RS_B_RCL) {
			FOOTPRINT( 's', 'r', 'R' );		/* srR */
			p_rqe->f.sequence = SDLC_RESIDUAL;
		} else if (scc_stat & RS_B_ABORT) {
			FOOTPRINT( 's', 'r', 'A' );		/* srA */
			p_rqe->f.sequence = SDLC_ABORT;
		} else if (scc_stat & RS_B_SHORT) {
			FOOTPRINT( 's', 'r', 'S' );		/* srS */
			p_rqe->f.sequence = SDLC_SHORT_FRAME;
		} 
		add_rqe( p_pcb, p_rqe->val );
		queue_writeb( &rx_free, p_pcb->rx_last );	
		p_pcb->rx_state = NULL;
		return(0);
	}

	/* send rqe to arq or sleeping process */
	add_rqe( p_pcb, p_rqe->val );

	rx_ctl = *( (unsigned char far *)p_buf + p_buf->offset + 1 );
	if ( rx_ctl & POLL_BIT )
	{
		if ( p_pcb->timer_type == RX_TIMER )
		{
			FOOTPRINT( 's', 'r', 't' );		/* srt */
			/* stop receive timer */
			stop_duscc_timer( p_pcb );
		}
	}
	else
	{
		FOOTPRINT( 's', 'r', 'u' );		/* sru */
		/* restart receive timer */
		if ( p_pcb->rx_timeout )
		{
			start_duscc_timer( p_pcb, p_pcb->rx_timeout, RX_TIMER );
		}

	}
	FOOTPRINT( 's', 'r', 'z' );			/* srz */
	return( 0 );
}

/******************************************************************************
 **  *******  ******      *     *     *   *****   *     *  *******  *******  **
 **     *     *     *    * *    **    *  *     *  **   **     *        *     **
 **     *     *     *   *   *   * *   *  *        * * * *     *        *     **
 **     *     ******   *******  *  *  *   *****   *  *  *     *        *     **
 **     *     *   *    *     *  *   * *        *  *     *     *        *     **
 **     *     *    *   *     *  *    **  *     *  *     *     *        *     **
 **     *     *     *  *     *  *     *   *****   *     *  *******     *     **
 ******************************************************************************/

extern unsigned char		txcmd[NUM_PORT];
extern unsigned short		txccw[NUM_PORT];

extern t_pdma_setup		pdma_cdt;	/* Channel Descriptor Table */

extern unsigned char		txftype[NUM_PORT];

/*
   SDLC transmit frame pre-processor.  This function does almost nothing, with
   the only exception being that the global transmit controls are setup.  The
   entry length must be returned, as the preprocessors are allowed to change
   the data frame as required.  The calling function puts our return value
   back into the Tx Compatibility command block, but if data changes are made
   which effect the location of the start of data in the buffer, the card_addr
   field must be updated directly in the command block.
*/

tx_pre_sdlc( register cmd_t	*p_cmd,
	     volatile t_pcb	*p_pcb )
{
	unsigned char			port = p_cmd->port;

	pdma_cdt.p._txla  = &( p_pcb->pdma_tx_llc[p_pcb->txchan & 0x01]);
	pdma_cdt._ccw = _CCW_TX_SDLC;
	txccw[port] = _CCW_TX_SDLC;
	txftype[port] = FT_S;		/* SDLC frame */

	FOOTPRINT( 's', 'T', 'a' );			/* sTa */

	if (  *(p_cmd->card_addr + 1) & POLL_BIT )
	{
		FOOTPRINT( 's', 'T', 'p' );		/* sTp */
		p_cmd->flags |= TX_FINAL_BLK;
		txftype[port] |= FT_FINAL;
	}

	/* Currently txcmd and txftype are not being used for SDLC, 
           but set them up anyway if they need to be used at a later time.
	*/
	txcmd[port] = txftype[port];

	/* Port data movement statistics */
	p_pcb->tx_bytes += p_cmd->length;		/*233*/
	p_pcb->tx_frames++;				/*233*/

	FOOTPRINT( 's', 'T', 'z' );			/* sTz */
	return ( p_cmd->length );
}

/*****************************************************************************
 **         *****    *******      *      *******   *     *    *****         **
 **        *     *      *        * *        *      *     *   *     *        **
 **        *            *       *   *       *      *     *   *              **
 **         *****       *      *******      *      *     *    *****         **
 **              *      *      *     *      *      *     *         *        **
 **        *     *      *      *     *      *      *     *   *     *        **
 **         *****       *      *     *      *       *****     *****         **
 *****************************************************************************/

void bop_decode ( volatile t_pcb	*p_pcb )
{
register eblock			*p_eblk = &( p_pcb->estat );
register unsigned short		q_temp;
t_rqe				*p_rqe = &( p_pcb->error_rqe );
bufno				buf;
rx_buf_hdr far			*p_rx_buf;

	FOOTPRINT( 's', 'S', 'a' );			/* sSa */
	p_rqe->f.sequence = 0;
	if ( p_eblk->e_type & ETYP_RXTX_STAT )
	{
		FOOTPRINT( 's', 'S', 'r' );		/* sSr */
		p_rqe->f.rtype = RQE_RX_NODMA;
/*
  Check both the Rx and Tx/Rx Status Register, as the interrupt is multiplexed.
*/
		q_temp = p_eblk->e_rsr;		/* Receiver Status Register */
		if ( q_temp & RS_B_ABORT )	/* ABORT line condition */
		{
			FOOTPRINT( 's', 'S', 'A' );	/* sSA */
			p_rqe->f.sequence = SDLC_ABORT;
		}
		else if ( q_temp & RS_B_OVERRUN )	/* Receiver Overrun */
		{
			FOOTPRINT( 's', 'S', 'O' );	/* sSO */
			p_rqe->f.sequence = RX_OVERRUN;
		}
		else if ( q_temp & RS_B_SHORT )	/* Short Frame detected   */
 		{
 			FOOTPRINT( 's', 'S', 'S' );	/* sSS */
 			p_rqe->f.sequence = SDLC_SHORT_FRAME;
 		}
		else if ( q_temp & RS_B_CRC )		/* Bad CRC */
		{
			FOOTPRINT( 's', 'S', 'R' );	/* sSR */
			p_rqe->f.sequence = FRAME_CRC;
		}

	        q_temp = p_eblk->e_trsr;	/* Tx/Rx Status Register */
		if ( q_temp & TRS_TXEMPTY )		/* DMA underran */
		{
			FOOTPRINT( 's', 'S', 'T' );	/* sST */
			tx_abort( p_pcb, TX_UNDERRUN );
		}
		else if ( q_temp & TRS_CTSUNDER )	/* CTS under ran */
		{
			FOOTPRINT( 's', 'S', 'C' );	/* sSC */
			tx_abort( p_pcb, CTS_UNDERRUN );
		}
		else if ( q_temp & TRS_DPLL )  /* Phase Locked Loop: !syn */
		{
			FOOTPRINT( 's', 'S', 'P' );	/* sSP */
			p_rqe->f.sequence = LOST_SYNC;
		}

		if ( p_rqe->f.sequence != 0 )		/* a rqe to post? */
		{
		   disable_rx( p_pcb );			/* disable receive */
		   add_rqe ( p_pcb, p_rqe->val );	/* post rqe to driver */
		}

	}					/* RXTX_STAT eblocks */

	if ( p_eblk->e_type & ETYP_ECT_STAT )	/* ECT_STAT eblocks */
	{
		FOOTPRINT( 's', 'S', 'e' );		/* sSe */
		scc_delta( p_pcb, p_eblk->e_ictsr );
	}					/* ECT_STAT eblocks */

	if ( p_eblk->e_type & ETYP_CIO_STAT )	/* CIO STAT eblocks */
	{
		FOOTPRINT( 's', 'S', 'c' );		/* sSc */
		cio_delta( p_pcb, p_eblk->e_ciodata );
	}					/* CIO STAT eblocks */

	FOOTPRINT( 's', 'S', 'z' );			/* sSz */
	return;
}
