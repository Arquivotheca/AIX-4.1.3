static char sccsid[] = "@(#)40	1.13  src/bos/usr/lib/asw/mpqp/setproto.c, ucodmpqp, bos411, 9428A410j 7/20/93 12:33:47";

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS: 
 * 	set_sdlc 		: Sets up DUSCC registers based on protocol.
 * 	set_bsc
 * 	set_asc
 * 	set_int_clk		: Sets appropriate DUSCC timing regs if internal
 *				  clocking selected.
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
 * FUNCTION: This procedure initializes a selected communication
 *           port on the adapter for sdlc protocol.
 */

#include "mpqp.def"
#include "mpqp.typ"
#include "portcb.def"
#include "portcb.typ"
#include "mpqp.ext"
#include "mpqp.pro"
#include "mpqp.h"

extern	void	xmit_frame();
extern	void	bsc_xmit_frame();
extern	int	tx_pre_sdlc();
extern	int	tx_pre_bsc();
extern	int	tx_pre_asy();
extern	int	rx_post_sdlc();
extern	int	rx_post_bsc();
extern	int	rx_post_asy();
extern  unsigned char	trsMask[], rsMask[];
extern	unsigned short	txinc[];
extern	unsigned short	bitrate[],byterate[];				/*309*/

set_int_clk(register volatile t_pcb	*p_pcb)
{
/* Note! for synchronous protocols only, DTE clock set for async
	 in default protocol setup */

	FOOTPRINT( 'S', 'i', 'b' );		/* Sib */
	p_pcb->scc_reg.rt = RT_DPLL32B;
	p_pcb->scc_reg.rt |= p_pcb->baud_rate;

	p_pcb->scc_reg.tt = TT_BRG;
	p_pcb->scc_reg.tt |= p_pcb->baud_rate;

	p_pcb->scc_reg.pc &= 0xE0;
	p_pcb->scc_reg.pc |= PC_T_Tx1;

	if (! (p_pcb->phys_link & ( PCB_PL_V35 | PCB_PL_422 )) )
	{
		p_pcb->cio_reg.data |= ENABLE_DTE_CLK;
	}
	set_cio(p_pcb);
}

set_sdlc (register volatile t_pcb   *p_pcb)
{
    unsigned short i;

	/* Copy SDLC default values of SCC registers from table to PCB */
	if (p_pcb->phys_link & PCB_PL_X21)
		p_pcb->scc_reg = scc_x21_sdlc;
	else
	{
		p_pcb->scc_reg = scc_def_sdlc;

		if /* continuous carrier */
		   (p_pcb->flags & PCB_AF_CC)
		   p_pcb->scc_reg.tp &= TP_0TXRTS;
	}

	/* SDLC NRZI is selected */
	if (p_pcb->flags & PCB_AF_NRZI)
	   p_pcb->scc_reg.cm1 |= C1_NRZI;

	/* poll address not zero (secondary station)  */
	if (p_pcb->poll_addr)
	{
	   p_pcb->scc_reg.cm1 |= C1_SEC;
	   p_pcb->scc_reg.s1 = p_pcb->poll_addr;
	}

	if (p_pcb->baud_rate != 0)	/* Internal clocking? */
		set_int_clk( p_pcb );

	init_scc(p_pcb);		/* Setup the SCC */

	if (p_pcb->baud_rate != 0)	/* Internal clocking? */
	{
		/* Set DPLL to NRZI, Enter Hunt manually */
		out08( p_pcb->scc_base + SCC_CC, CC_DPLL_NRZI );
		out08( p_pcb->scc_base + SCC_CC, CC_DPLL_SEARCH );
	}
	else				/* external clocking */
		/* turn off DPLL, since it may have been turned on */
		/* while dialing with internal clocking */
		out08( p_pcb->scc_base + SCC_CC, CC_DPLL_DISABLE );

	if (p_pcb->port_state == CALL_SET_UP)
	{
		/* Only modify if setting up for a call */
		if (!(p_pcb->flags & PCB_AF_TX_CTS) ) /* Transmit w/o CTS */
		   p_pcb->scc_reg.tp &= TP_0TXCTS;

		init_scc(p_pcb);		/* Setup the SCC */
	}

	/* set up transmit offlevel, rx ccw */
	p_pcb->f_txserve = tx_pre_sdlc;
	p_pcb->f_txframe = xmit_frame;
	p_pcb->f_rxserve = rx_post_sdlc;
	p_pcb->rx_ccw = _CCW_SDLC_RX;
	p_pcb->tx_pad = 0;
        for (i=0;i<2;i++) {
	    p_pcb->pdma_rx_llc[i]._ccw = p_pcb->rx_ccw;
	    p_pcb->pdma_tx_llc[i]._ccw = _CCW_TX_SDLC;
        }
	/* set up DUSCC interrupt mask */
	rsMask[p_pcb->port_no] = RS_B_OVERRUN | RS_B_ABORT | RS_B_CRC | RS_B_SHORT; 
	trsMask[p_pcb->port_no] = 
	    TRS_TXEMPTY | TRS_CTSUNDER | TRS_EOFRAME | TRS_SENDACK;
	return;
}

set_bsc (register volatile t_pcb  *p_pcb)
{
    unsigned short i;

	/* Copy BISYNC default values of SCC registers from table to PCB */

	if (p_pcb->phys_link & PCB_PL_X21)
	{
   		if ( p_pcb->flags & PCB_PF_ASCII )	/* BiSync ASCII */
		{
			p_pcb->scc_reg = scc_x21_bi_a;
		}
		else
		{
			p_pcb->scc_reg = scc_x21_bi_e;
		}
	}
	else					/* BiSync EBCDIC */
	{
   		if ( p_pcb->flags & PCB_PF_ASCII )	/* BiSync ASCII */
		{
			p_pcb->scc_reg = scc_def_bi_a;
		}
		else
		{
			p_pcb->scc_reg = scc_def_bi_e;
		}

   		if /* continuous carrier */
      		  (p_pcb->flags & PCB_PF_CC)
			p_pcb->scc_reg.tp &= TP_0TXRTS;
	}

        txinc[p_pcb->port_no] = 150;

	if /* data transfer state */
	  (p_pcb->port_state != CALL_SET_UP)
	{
		if ( p_pcb->baud_rate != 0 )	/* Internal clocking? */
		{
			set_int_clk( p_pcb );
		}
		
		init_scc(p_pcb);		/* Setup the SCC */

		if ( p_pcb->baud_rate != 0 )	/* Internal clocking? */
		{
			/* Set DPLL to NRZI, Enter Hunt manually */
			out08( p_pcb->scc_base + SCC_CC, CC_DPLL_NRZI );
			out08( p_pcb->scc_base + SCC_CC, CC_DPLL_SEARCH );
		}
	}
	else /* Setting up for call */
	{
		/* Only modify wait for CTS if setting up for a call */
		if (!(p_pcb->flags & PCB_AF_TX_CTS) )	/* Transmit w/o CTS */
			p_pcb->scc_reg.tp &= TP_0TXCTS;
		init_scc(p_pcb);		/* Setup the SCC */
	}

	/* set up transmit offlevel, rx ccw */
	p_pcb->f_txserve = tx_pre_bsc;
	p_pcb->f_txframe = bsc_xmit_frame;
	p_pcb->f_rxserve = rx_post_bsc;
	p_pcb->bsc_prev_tx = NULL;
	p_pcb->bsc_prev_rx = NULL;
	p_pcb->tx_state    = NULL;
	p_pcb->rx_state    = NULL;
	p_pcb->rx_ccw = _CCW_BSC_RX;
	p_pcb->tx_pad = 0x55;			/* EBCDIC and ASCII */
        for (i=0;i<2;i++) {
	    p_pcb->pdma_rx_llc[i]._ccw = p_pcb->rx_ccw;
	    p_pcb->pdma_tx_llc[i]._ccw = _CCW_TX_BSC;
        }
	/* set up DUSCC interrupt mask */
	rsMask[p_pcb->port_no] = RS_C_OVERRUN | RS_C_PAD | RS_C_CRC |
	                         RS_C_PARITY; 
	trsMask[p_pcb->port_no] = 
	    TRS_TXEMPTY | TRS_CTSUNDER | TRS_EOFRAME | TRS_SENDACK;
	return;
}


set_asc (register volatile t_pcb  *p_pcb)
{
    unsigned short i;

	/* Set async, 2400 bps, seven-bit data, odd parity, 1 stop bit */
	p_pcb->scc_reg = scc_def_asy;

	/* if V.25BIS auto-dial */ 
	if (p_pcb->phys_link & PCB_PL_V25)
		p_pcb->scc_reg.cm1 &= C1_PEVEN;

	/* if user setup desired */
	if (p_pcb->field_select & PCB_FS_AP)
	{
		/* if wait for cts on tx */
		if (p_pcb->flags & PCB_AF_TX_CTS)
			p_pcb->scc_reg.tp |= TP_1TXCTS;
		/* if continuous carrier */
		if (p_pcb->flags & PCB_AF_CC)
			p_pcb->scc_reg.tp &= TP_0TXRTS;
		/* if no parity selected */
  		if (!(p_pcb->flags & PCB_AF_PAREN) )
			p_pcb->scc_reg.cm1 &= C1_NOPAR;
		/* if even parity selected */
		else if (!(p_pcb->flags & PCB_AF_ODDPAR) )
			p_pcb->scc_reg.cm1 &= ~C1_PODD;
 
		/* if not 1 stop bit */
		if ((p_pcb->flags & PCB_AF_STOP_MSK) == PCB_AF_2STB)
			p_pcb->scc_reg.tp |= TP_A_STOP2;
		else if ((p_pcb->flags & PCB_AF_STOP_MSK) == PCB_AF_15STB)
		{
			p_pcb->scc_reg.tp &= ~TP_A_STOP2;
			p_pcb->scc_reg.tp |= TP_A_STOP15;
		}
		
		/* if not default length of 7 bits */
		if ((p_pcb->flags & PCB_AF_CHAR_MSK) != PCB_AF_7BCS)
		{
			p_pcb->scc_reg.tp &= TP_5BITS;
			p_pcb->scc_reg.tp |= p_pcb->dial_flags & TP_8BITS;
			p_pcb->scc_reg.rp &= RP_5BITS;
			p_pcb->scc_reg.rp |= p_pcb->dial_flags & RP_8BITS;
		}
	}
	/* Set clocking to talk to modem at desired speed */
	if ( p_pcb->baud_rate != 0 )	
	{
		p_pcb->scc_reg.rt = (RT_A_BRG | p_pcb->baud_rate);

		p_pcb->scc_reg.tt = (TT_BRG | p_pcb->baud_rate);
		set_cio(p_pcb);
	}
	/* Program the SCC chip with selected value */
	init_scc(p_pcb);  
	p_pcb->f_txserve = tx_pre_asy;
	p_pcb->f_txframe = xmit_frame;
	p_pcb->f_rxserve = rx_post_asy;
	p_pcb->rx_ccw = _CCW_ASY_RX;
	p_pcb->rx_cmb = PD_CM_ASY_O; /* character match on LF and XOFF
					 with no parity */
        for (i=0;i<2;i++) {
	    p_pcb->pdma_rx_llc[i]._ccw = p_pcb->rx_ccw;
	    p_pcb->pdma_tx_llc[i]._ccw = _CCW_TX_ASY;
        }

	/* set up DUSCC interrupt mask */
	rsMask[p_pcb->port_no] = RS_B_OVERRUN;
	trsMask[p_pcb->port_no] = TRS_TXEMPTY; 
	/* if CTS enable TX set, turn on CTS underrun status bit */
	if ( p_pcb->scc_reg.tp & TP_1TXCTS )
		trsMask[p_pcb->port_no] |= TRS_CTSUNDER;

	return;
}
