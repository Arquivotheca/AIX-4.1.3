static char sccsid[] = "@(#)47	1.7  src/bos/usr/lib/asw/mpqp/x21offl.c, ucodmpqp, bos411, 9428A410j 7/20/93 12:33:54";
/*
 *COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS:
 *	rx_post_x21		: Interprets receive data for state changes,
 *				  or network data.
 *	tx_pre_x21		: Sets up buffer for transmit.
 *	x21_decode		: Interprets line changes, initiates clear if
 *				  no state machine is running.
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
#include "portcb.typ"
#include "portcb.def"
#include "iface.def"

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
extern int	Ascii_Fr_Op( unsigned char far *, int );   

typedef struct
{	rx_buf_hdr	rbh;
	unsigned char	data[1];
} rb;

#define	X21_CHAR_BEL	(unsigned char)0x07
#define	X21_CHAR_PLUS	(unsigned char)0xAB

int rx_post_x21 ( register volatile t_pcb	*p_pcb,
		  rx_buf_hdr far		*p_buf)
{
unsigned char		dma_stat;	/* Echo DMA status reg value	*/
unsigned char		scc_stat;	/* DUSCC receiver status reg	*/
unsigned short		tc;		/* Echo terminal count value	*/
t_rqe			rqe;		/* Response queue element	*/
unsigned char far	*p_start;	/* pointer, first data byte	*/
unsigned char far	*p_end;		/* pointer, last data byte	*/
int			length;		/* Corrected frame data length	*/
	FOOTPRINT( 'X', 'r', 'e' );			/* Xre */

	/* Read in DMA status, DUSCC status and TC from buffer */
	dma_stat = p_buf->offset >> 8;
	tc	 = p_buf->length;
	scc_stat = p_buf->type;

	/* init RQE fields */
	rqe.f.rtype = RQE_RX_NODMA;
	rqe.f.port = p_pcb->port_no;
	rqe.f.status = p_pcb->rx_last;
	rqe.f.sequence = 0;

	p_start = (unsigned char far *)p_buf + p_pcb->rx_off;		/*260*/
	length = p_pcb->rx_flgt - tc;					/*260*/
	p_end = p_start + length - 1;

	/* if no receive errors */
	if ( !(scc_stat & RS_C_OVERRUN ) )
	{
		/* Check for X.21 patterns imbedded in the frame */
	   	if ( dma_stat & DMA_ISR_CM )
		{
			if ( p_pcb->x21_state <= READY )
			{
				if ( *p_end == X21_CHAR_PLUS )
				{
					FOOTPRINT( 'X', 'r', 'p' ); /* Xrp */
					rqe.f.rtype = RQE_STAT_UNSOL;
					rqe.f.sequence = X21_PLUS;
					rqe.f.status = 0;
				}
				else if ( *p_end == X21_CHAR_BEL )
				{
					FOOTPRINT( 'X', 'r', 'b' ); /* Xrb */
					rqe.f.rtype = RQE_STAT_UNSOL;
				 	rqe.f.sequence = X21_BEL;
					rqe.f.status = 0;
				}
			}
			else
			{

				/* Check for valid CPS or DPI */
				/* write receive buffer header */
				p_buf->length = p_pcb->rx_flgt - tc;
				p_buf->offset = p_pcb->rx_off; 
				p_buf->type = BUF_STAT_NOERR;
				rqe.f.sequence = BUF_STAT_NOERR;
		
				/* strip off parity */
				if (!(Ascii_Fr_Op( p_start, length)) )
				{
					rqe.f.rtype = RQE_CPS_ACK;
				}
				else /* parity error */
				{
					rqe.f.rtype = RQE_RX_NODMA;
					rqe.f.sequence = INVALID_CPS;
				}
			}
		}
		else	/* DMA error occurred */
		{
			rqe.f.rtype = RQE_RX_NODMA;
			rqe.f.sequence = INVALID_CPS;
		}

	}
	else	/* Error indicated in the Rx status register */
	{
		rqe.f.rtype = RQE_RX_NODMA;
		rqe.f.sequence = INVALID_CPS;
	}

	/* free buffer if not rx data */
	if ( rqe.f.rtype != RQE_CPS_ACK )
		queue_writeb( &rx_free, p_pcb->rx_last );
	if (! ( ( rqe.f.rtype == RQE_STAT_UNSOL) && ( !rqe.f.sequence ) ) )
		add_rqe( p_pcb, rqe.val );

	return(0);
	FOOTPRINT( 'X', 'r', 'x' );				/* Xrx */

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

extern t_pdma_setup		pdma_cdt;	/* Channel Descriptor Table */
extern unsigned char		txcmd[NUM_PORT];
extern unsigned short		txccw[NUM_PORT];
extern unsigned char		txftype[ NUM_PORT ];	/* xmit frame type */

tx_pre_x21( register cmd_t	*p_cmd )
{
unsigned char far		*p_data = p_cmd->card_addr;
register unsigned short		length = p_cmd->length;
unsigned char			port = p_cmd->port;
t_pcb				*p_pcb;

	FOOTPRINT( 'X', 'p', 'r' );				/* Xpr */
	p_pcb = &pcb[port];
	pdma_cdt.p._txla  = &( p_pcb->pdma_tx_llc[p_pcb->txchan & 0x01]);
	pdma_cdt._ccw = _CCW_TX;
	txccw[port] = _CCW_TX;
	txcmd[port] = FT_X;
	return ( length );
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

void tx_abort ( volatile t_pcb	*, unsigned char );

void x21_decode ( volatile t_pcb	*p_pcb )
{
extern unsigned char		x21_Stat;
register eblock			*p_eblk = &p_pcb->estat;
register unsigned short		q_temp;
t_rqe				stat_rqe;

	FOOTPRINT('X', 'd', 'e');				/* Xde */
	stat_rqe.f.rtype    = RQE_STAT_UNSOL;
	stat_rqe.f.status = 0;
	stat_rqe.f.sequence = 0;
	if ( p_eblk->e_type & ETYP_X21_STAT )
	{
		q_temp = p_eblk->e_x21pal;
		if ( q_temp & CIO_X21_IA )	/* Indicate High */
		{
			FOOTPRINT('X', 'd', 'i');		/* Xdi */
			stat_rqe.f.sequence = X21_INDICATE;
		}
		else
		{
			q_temp &= CIO_X21_PAT;
			switch ( q_temp )
			{
				case 0x0: stat_rqe.f.sequence = X21_ZERO;
					FOOTPRINT('X','d','z');		/* Xdz*/
					break;
				case 0x1:
				case 0x2: stat_rqe.f.sequence = X21_ALT;
					FOOTPRINT('X','d','a');		/* Xda*/
				  	break;
				case 0x3: stat_rqe.f.sequence = X21_ONE;
					FOOTPRINT('X','d','o');		/* Xdo*/
			}
		}
		x21_Stat = q_temp;
		
		/* if no state machine currently running */
		if ( !SleepEn[p_pcb->port_no] )
		{
			if ( stat_rqe.f.sequence == X21_ZERO )
			{
				if ( p_pcb->start_parm & PCB_SP_LEASED )
				{
			  		tx_abort( p_pcb, X21_ZERO );
					/* turn off control */
					OUT08(ENREG,IN08(ENREG) & ENR_C_X21_CA);
					
				}
				else	
				{
			  		tx_abort( p_pcb, X21_ZERO );
					sleep_enable( p_pcb );
					dte_clear_confirm( p_pcb );
					call_term( p_pcb, NO_BUFNO );
					sleep_disable( p_pcb );
				}
				add_rqe( p_pcb, stat_rqe.val );
			}
		}
		else add_rqe( p_pcb, stat_rqe.val );
	}
/*
	If RSR, TRSR, ECT or other magical status, handle those conditions.
*/
	FOOTPRINT('X', 'd', 'x');				/* Xdx */
	return;
}

