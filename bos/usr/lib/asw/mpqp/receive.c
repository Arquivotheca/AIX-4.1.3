static char sccsid[] = "@(#)36	1.10  src/bos/usr/lib/asw/mpqp/receive.c, ucodmpqp, bos411, 9428A410j 8/23/93 13:24:25";

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS: 
 *      get_rsr			: Get the RSR saved by the echo chip at the 
 *				  list address pointer pointed to by the CDT.
 *	enable_rx		: Set up echo CDT, but don't start it
 * 	disable_rx		:  Disable echo DMA
 * 	f_rxwork		: Swaps DMA channels, set up buffer header,
 *				  unschedule rx work, call rx post processor.
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
#include "portcb.def"			/* PCB definitions */
#include "iface.def"			/* adapter cmd and rsp queue values */

extern t_pdma_setup		pdma_cdt;	/* Channel Descriptor Table */
extern unsigned short		rx_bsiz;	/* rx buffer size */

/*
   get_rsr gets a copy of what was in the DUSCC Receiver Status Register 
   from the second byte of the second word in the List Address Pointer.
   The List Address Pointer's address is contained in the CDT.

   The hardware copies the RSR to this address and then clears the RSR.
*/

unsigned short
get_rsr ()
{
    return(pdma_cdt.p._rxla->_status);
}

/*
   enable_rx allocates a receive buffer, sets up the indicated Echo channel,
   and returns the buffer number for use by the caller.
*/

enable_rx ( register volatile t_pcb	*p_pcb,
	    ioptr			p_dma )
{
register unsigned char		port = p_pcb->port_no;
bufno				buf;

	FOOTPRINT( 'r', 'E', 'a' );			/* rEa */
	if (( buf = queue_readb ( &rx_free )) == (bufno)-1 )
	{
		FOOTPRINT( 'r', 'e', 'E' );		/* reE */
		return ( NO_BUFNO );
	}
/*
   Setup address, count, I/O address, channel control word, character match.
*/
	pdma_cdt._ca  = RECV_BUFFER( buf ) + p_pcb->rx_off;		/*260*/
	pdma_cdt._tc  = p_pcb->rx_flgt;					/*260*/
	pdma_cdt._ioa = p_pcb->scc_base + SCC_RXFIFO;
	pdma_cdt._ccw = p_pcb->rx_ccw;
	pdma_cdt._cmb = p_pcb->rx_cmb;
	pdma_cdt.p._rxla  = &( p_pcb->pdma_rx_llc[p_pcb->rxchan & 0x01]);

	SetPDMA( p_dma );
	FOOTPRINT( 'r', 'e', 'z' );			/* rez */
	return ( buf );
}

/*
     If an error is detected that means the currently in-progress receive
   cannot provide meaningful data, i.e. SDLC short frame, the channel must
   be stopped and the next/alternate enabled.  This operation is not
   entirely trivial because the old Rx buffer must be freed and potentially
   two new Rx buffers allocated.
     It is extremely important that the Port DMA channel which caused the
   disable_rx call still be running.  If the channel terminates naturally,
   this procedure will abort a possibly valid frame in progress.  The
   EOM of the subsequent frame to a short frame would cause a good Hunt
   mode and, possibly, a frame to be lost when disabled.
*/
disable_rx ( register volatile t_pcb	*p_pcb )
{
	ioptr		p_dma_c;	/* Current port DMA channel base */
	ioptr		p_dma_n;	/* Next port DMA channel base */

	FOOTPRINT( 'r', 'd', 'a' );			/* rda */
	if ( p_pcb->rxchan & 1 )
	{
		p_dma_c = p_pcb->rx_dma_0;
		p_dma_n = p_pcb->rx_dma_1;
	}
	else
	{
		p_dma_c = p_pcb->rx_dma_1;
		p_dma_n = p_pcb->rx_dma_0;
	}

	if ( !( in16( p_dma_c + PDMA_CCW ) & PDMA_CCW_EN ) )
	{
		/*  IX23652.  When bisync is specified and when the 
		    bisync data frame occur close together, the 
		    scheduler is unable to re-schedule the receive
	 	    port DMA.  This causes lost of port DMA frames
		    and shuts down the port activity because no 
		    receive port DMA's were scheduled.
		    PROBLEM:  Bisync code path is to long and thus 
		    preventing the schedule to schedule port DMA's. 
		    Error being detected are RX_OVERRUNS.
		    SOLUTION:  Allow the dma channel to be re-armed.  */
		/*  IX27425. This superscedes IX23652.
		    PROBLEM: When frames occur close together both channels
		    could already have fired before either has been processed
		    by f_rxwork. When this happens the duscc is still enabled
		    but when the next buffer is enabled by f_rxwork the next
		    port DMA may not have been armed.
		*/
		FOOTPRINT( 'r', 'd', 'D' );		/* rdD */
	}
	else
		out16( p_dma_c + PDMA_CCW, 0 );

	/* free old buffers plus get new ones */
	init_rcv(p_pcb);

	FOOTPRINT( 'r', 'd', 'z' );			/* rdz */
	return ( 0 );
}

extern unsigned short volatile	rxstat[NUM_PORT];

extern unsigned short rx_count[];

f_rxwork ( unsigned char 	port )
{
	register volatile t_pcb	*p_pcb = &pcb [port];
	rx_buf_hdr far		*p_buf;
	ioptr			curr_dma, next_dma;

	FOOTPRINT( 'r', 'f', 'a' );			/* rfa */
	
	if (ODD( p_pcb->rxchan ))			/* Even/Odd next? */
	{
		curr_dma = p_pcb->rx_dma_0;
		next_dma = p_pcb->rx_dma_1;
	} else {
		curr_dma = p_pcb->rx_dma_1;
		next_dma = p_pcb->rx_dma_0;
	}

	if ( p_pcb->rx_curr == NO_BUFNO )
	{
		FOOTPRINT( 'r', 'f', 'n' );		/* rfn */
		UNSCHED_RX_WORK( p_pcb );
		return;
	}

	p_buf = (rx_buf_hdr far *)RECV_BUFFER( p_pcb->rx_curr );
	p_buf->offset  = rxstat[port];
	p_buf->length  = p_pcb->pdma_rx_llc[p_pcb->rxchan & 0x01]._old_tc;
	p_buf->type    = p_pcb->pdma_rx_llc[p_pcb->rxchan & 0x01]._status;

	if ( p_pcb->rx_next == NO_BUFNO )
	{
		p_pcb->rx_next = enable_rx ( p_pcb, next_dma ); 
		if ( p_pcb->rx_next == NO_BUFNO )	/* Underrun DMA? */
		{
		    FOOTPRINT( 'r', 'f', 'E' );		/* rfE */
		    return;
		} else {
		    FOOTPRINT( 'r', 'f', 'd' );		/* rfd */
		    InitPDMA( next_dma );
		}
	}

	p_pcb->rx_last = p_pcb->rx_curr;
	p_pcb->rx_curr = p_pcb->rx_next;

	p_pcb->rx_next = enable_rx ( p_pcb, curr_dma );
	p_pcb->rxchan++;


	disable();
	if(--rx_count[port] == 0)
	{
		UNSCHED_RX_WORK( p_pcb );
	}
	else
	{
		InitPDMA( curr_dma );
	}
	enable();


	(*(p_pcb->f_rxserve) )( p_pcb, p_buf );

	FOOTPRINT( 'r', 'f', 'z' );			/* rfz */
	return;
}
