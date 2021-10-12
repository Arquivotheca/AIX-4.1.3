static char sccsid[] = "@(#)34	1.5  src/bos/usr/lib/asw/mpqp/pcbinit.c, ucodmpqp, bos411, 9428A410j 7/20/93 13:15:27";

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS: 
 *	init_pcb	: Initialize a Port Control Block to defaults.
 *	init_scc	: Initialize an SCC channel from the scc_regs
 *			:  structure in the (passed) port control block.
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
   Initialize a port control block.
*/

#include "mpqp.def"
#include "mpqp.typ"
#include "mpqp.ext"
#include "mpqp.pro"
#include "portcb.typ"

extern unsigned short		*p_io_tab[4];

init_pcb( int		 port )
{
register volatile t_pcb		*p_pcb = &pcb [port];
register unsigned short		*pipio = p_io_tab [port];

	FOOTPRINT( 'P', 'i', 'a' );		/* Pia */
	p_pcb->port_no = port;
	p_pcb->cmd_q = &port_cq [port];
	p_pcb->rsp_q = &port_rq [port];
	p_pcb->mask_on = portbit( port );
	p_pcb->mask_off = ~portbit( port );

	p_pcb->rx_next = p_pcb->rx_curr = p_pcb->rx_last = NO_BUFNO;
	p_pcb->rxchan = 0x00;
	p_pcb->rx_cmb = 0x00;  /*Disable character matching initially 4/26/93*/
	p_pcb->txbuf0 = p_pcb->txbuf1 = NO_BUFNO;
	p_pcb->txchan = 0x00;

	p_pcb->rx_dma_0 = *pipio++; p_pcb->rx_dma_1 = *pipio++;
	p_pcb->tx_dma_0 = *pipio++; p_pcb->tx_dma_1 = *pipio++;
	p_pcb->scc_base = *pipio++; p_pcb->scc_data = *pipio++;
	p_pcb->cio_base = *pipio++; p_pcb->cio_data = *pipio++;
	p_pcb->alt_cio  = *pipio;
	p_pcb->cio_port = p_pcb->port_no & 1;	/* A=0, B=1 */

	p_pcb->f_txserve = (unsigned char (*)())0;
	p_pcb->f_txframe = (unsigned char (*)())0;
	p_pcb->f_rxserve = (unsigned char (*)())0;

	p_pcb->port_status = 0x00;

	return ( 0 );
}

/*
   Setup a DUSCC to the parameters given in the scc_reg section of the
   passed port control block.
*/

init_scc ( volatile t_pcb		*p_pcb )
{
register volatile t_scc_reg	*p_s = &p_pcb->scc_reg;
register ioptr			scc = p_pcb->scc_base;

	out08( scc + SCC_CM1, p_s->cm1 );
	out08( scc + SCC_CM2, p_s->cm2 );
	out08( scc + SCC_S1, p_s->s1 );
	out08( scc + SCC_S2, p_s->s2 );
	out08( scc + SCC_TP, p_s->tp );
	out08( scc + SCC_TT, p_s->tt );
	out08( scc + SCC_RT, p_s->rt );
	out08( scc + SCC_RP, p_s->rp );

	out08( scc + SCC_CTPH, p_s->ctph );				/*314*/
	out08( scc + SCC_CTPL, p_s->ctpl );				/*314*/
	out08( scc + SCC_CTC, p_s->ctc );

	out08( scc + SCC_OM, p_s->om );
	out08( scc + SCC_PC, p_s->pc );
	return ( 0 );
}
