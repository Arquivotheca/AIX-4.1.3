static char sccsid[] = "@(#)23	1.5  src/bos/usr/lib/asw/mpqp/asyoffl.c, ucodmpqp, bos411, 9428A410j 7/20/93 13:12:27";

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS: asyoffl.c - Offlevel function handling for async protocol.
 *	      Prepares buffers for transmit, processes buffers for receive
 *	      and interprets duscc and cio errors.
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

/*
   The receive offlevel is only entered when Port DMA completes and
   schedules RX_WORK.  DMA will only complete for two reasons on Async:
   Character Match, when the frame is good, and Terminal Count, when
   the frame is likely OK as well.  Since the ISR starts the alternate
   channel, if prepared, buffer overflow may be non-critical since we
   should be able to start the alternate channel within four character
   times.  Only cases where the next Rx channel is not initialized are
   likely to cause data loss.
*/

extern unsigned short		rx_bsiz;	/* rx buffer size */

typedef struct
{	rx_buf_hdr	rbh;
	unsigned char	data[1];
} rb;

int rx_post_asy (register volatile t_pcb	*p_pcb, 
		  rx_buf_hdr far		*p_buf )
{
unsigned char	dma_stat;	/* Echo DMA status reg value 		*/
unsigned char	scc_stat;	/* DUSCC receiver status reg		*/
unsigned short	tc;		/* Echo DMA terminal count value	*/
int		rc;		/* retrun code 				*/
t_rqe		*p_rqe = &( p_pcb->error_rqe );

	FOOTPRINT( 'a', 'P', 'a' );		/* aPa */
	/* Read in DMA status, DUSCC status and TC from buffer */
	dma_stat = p_buf->offset >> 8;
	tc	 = p_buf->length;
	scc_stat = p_buf->type;

	/* init RQE fields */
	p_rqe->f.rtype    = RQE_RX_DMA;
	p_rqe->f.port     = p_pcb->port_no;
	p_rqe->f.status   = p_pcb->rx_last;
	p_rqe->f.sequence = 0;

	/* write receive buffer header */
	p_buf->length = p_pcb->rx_flgt - tc;
	p_buf->offset = p_pcb->rx_off;
	p_buf->type   = BUF_STAT_NOERR;

	/* if CM not set, buffer overflowed */
   	if ( ( dma_stat & DMA_ISR_CM ) == 0)
 	{	p_buf->type = BUF_STAT_OVFLW;
		p_rqe->f.sequence = BUF_STAT_OVFLW;
		FOOTPRINT( 'a', 'P', 'O' );		/* aPO */
	}

	rc = Ascii_Fr_Op( (char far *)p_buf + p_buf->offset, p_buf->length );
	if ( rc )
	{	p_rqe->f.sequence = RX_PARITY_ERR;
		FOOTPRINT( 'a', 'P', 'P' );		/* aPP */
	}
	if ((p_rqe->f.sequence == BUF_STAT_OVFLW) ||
	    (p_rqe->f.sequence == RX_PARITY_ERR) )
	{
	    FOOTPRINT( 'a', 'P', 'x' );		/* aPx */
	    p_rqe->f.rtype    = RQE_RX_NODMA;
	    
	    /* put on ARQ */
	    ReRoute [p_pcb->port_no] = FALSE;

	    /* send rqe to arq or sleeping process */
	    add_rqe( p_pcb, p_rqe->val );

	    /* discard frame */
	    queue_writeb( &rx_free, p_pcb->rx_last );	
	    p_pcb->rx_state = NULL;
	}
	else
	{
	    FOOTPRINT( 'a', 'P', 'y' );		/* aPy */
	    /* start RQE rerouting (put on PRQ) */
	    ReRoute [p_pcb->port_no] = TRUE;

	    /* Port data movement statistics */
	    p_pcb->rx_bytes += p_buf->length;
	    p_pcb->rx_frames++;
	
	    /* send rqe to arq or sleeping process */
	    add_rqe( p_pcb, p_rqe->val );
	}

	FOOTPRINT( 'a', 'P', 'z' );		/* aPz */
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
extern t_pdma_setup		pdma_cdt;	/* Channel Descriptor Table */

tx_pre_asy( register cmd_t	*p_cmd,
	    volatile t_pcb	*p_pcb )
{
	pdma_cdt.p._txla  = &( p_pcb->pdma_tx_llc[p_pcb->txchan & 0x01]);
	pdma_cdt._ccw = _CCW_TX_ASY;
	txcmd[p_pcb->port_no] = FT_A; /* no SOM command wanted */

	/* Port data movement statistics */
	p_pcb->tx_bytes  += p_cmd->length;
	p_pcb->tx_frames++;

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

void asy_decode ( volatile t_pcb	*p_pcb )
{
register eblock			*p_eblk = &( p_pcb->estat );
register unsigned short		q_temp;
t_rqe				*p_rqe = &( p_pcb->error_rqe );

	p_rqe->f.status = 0;
	p_rqe->f.sequence = 0;
	if ( p_eblk->e_type & ETYP_RXTX_STAT )
	{
		p_rqe->f.rtype = RQE_RX_NODMA;
/*
  Check both the Rx and Tx/Rx Status Register, as the interrupt is multiplexed.
*/
		q_temp = p_eblk->e_rsr;		/* Receiver Status Register */
		if ( q_temp & RS_A_PARITY )
		{
			p_rqe->f.sequence = RX_PARITY_ERR;
		}
		if ( q_temp & RS_A_FRAMING )
		{
			p_rqe->f.sequence = ASY_FRAMING_ERR;
		}
		if ( q_temp & RS_A_OVERRUN )
		{
			p_rqe->f.sequence = RX_OVERRUN;
		}
		if ( q_temp & RS_A_LOSTRTS )
		{
			p_rqe->f.sequence = ASY_LOST_RTS;
		}

		q_temp = p_eblk->e_trsr;	/* Tx/Rx Status Register */
		if ( q_temp & TRS_CTSUNDER )	/* CTS under ran */
		{
			tx_abort( p_pcb, CTS_UNDERRUN );
		}
		if ( q_temp & TRS_DPLL )	/* Phase Locked Loop: !syn */
		{
			p_rqe->f.sequence = LOST_SYNC;
		}

                if ( p_rqe->f.sequence != 0 )           /* a rqe to post? */
                {
                   disable_rx( p_pcb );                 /* disable receive */
                   add_rqe ( p_pcb, p_rqe->val );       /* post rqe to driver */
                }
	}

	if ( p_eblk->e_type & ETYP_ECT_STAT )	/* ECT_STAT eblocks */
	{
		scc_delta( p_pcb, p_eblk->e_ictsr );
	}					/* ECT_STAT eblocks */

	if ( p_eblk->e_type & ETYP_CIO_STAT )	/* CIO STAT eblocks */
	{
		cio_delta( p_pcb, p_eblk->e_ciodata );
	}					/* CIO STAT eblocks */

	return;
}
