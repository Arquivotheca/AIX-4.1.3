static char sccsid[] = "@(#)22	1.3  src/bos/usr/lib/asw/mpqp/adapdiag.c, ucodmpqp, bos411, 9428A410j 11/30/90 10:46:05";

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTION: set_scc_diag: Initializes a selected communication
 *           port on the adapter according to data provided in
 *           the SCC_DIAG command that is sent to adapter.
 *           Physical connections supported include RS-232-D,
 *           V.35, RS422-A, X.21. SDLC and Bisync and Async are
 *           the three protocols supported.  Other options
 *           include channel connection: normal, auto-echo,
 *           local loop or physical wrap.
 *
 *	     c_serial_setup: allows all duscc and cio registers to
 *	     set, read, or bits ANDed and ORed.
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
 *
 */

#include "mpqp.def"
#include "mpqp.typ"
#include "mpqp.ext"
#include "mpqp.pro"
#include "portcb.def"
#include "portcb.typ"
#include "mpqp.h"
#include "iface.def"

/* external declarations */
extern	int	xmit_frame();
extern	int	bsc_xmit_frame();
extern	int	tx_pre_sdlc();
extern	int	tx_pre_bsc();
extern	int	tx_pre_asy();
extern	int	tx_pre_x21();

typedef struct SCC_DIAG_CMD
{
unsigned char     cmd_typ;                 /* Command type                  */ 
unsigned char     cmd_port;                /* Port number                   */
unsigned short    cmd_sequence;            /* Command sequence number       */
unsigned char     cmd_empty [12];          /* Not used                      */
unsigned char     cmd_proto;	           /* Protocol                      */
unsigned char     cmd_s1;	           /* Syn 1 char                    */
unsigned char     cmd_s2;	           /* Syn 2 char                    */
unsigned char     cmd_a1;	           /* Address/Parity/ASCII-EBCDIC   */
unsigned char     cmd_char_len;		   /* Character length (asy only)   */
unsigned char     cmd_baud_rate;           /* Encoded baud rate             */
unsigned char     cmd_chan_conn;           /* Channel connect mode          */
unsigned char     cmd_intr_mask;           /* Modem Interrupt Mask          */
} t_scc_diag;

#define 	l_DUAL_ADDRESS	0XFF	/* DUAL OCTET ADDRESS - SDLC */
#define		l_SDLC		0x02
#define		l_BISYNC	0x01
#define 	l_ASYNC		0x08
#define 	l_X21		0x80
#define 	l_V35		0x40
#define 	l_232		0x10
#define		l_422		0x20
#define 	l_1_STOP	0x01
#define		l_1_5_STOP	0x02
#define		l_2_STOP	0x04
#define 	l_ASCII		0xFF
#define		l_FORCE_0	0x01
#define		l_FORCE_1	0x02
#define		l_EVEN_PAR	0x04
#define 	l_ODD_PAR	0x08
#define		l_NO_PAR	0x10
#define		l_INT_CLK	0xF0

/* Start of code */

set_scc_diag (register t_scc_diag	*p_cmd,
              unsigned char far		*p_tx_bfr )
{
register volatile t_pcb    *p_pcb;	/* port control blk pointer */
 
	/* Initialize PCB pointer */
	p_pcb = &pcb [p_cmd->cmd_port];

	if ((p_cmd->cmd_proto & 0xF0) == l_X21)
	{
		p_pcb->scc_reg = scc_def_x21;
		p_pcb->f_txserve = tx_pre_x21;
		p_pcb->f_txframe = xmit_frame;
	}

	else if ((p_cmd->cmd_proto & 0x0F) == l_SDLC)
	{
		/* set up default register settings in pcb */
		p_pcb->scc_reg = scc_def_sdlc;
		p_pcb->f_txserve = tx_pre_sdlc;
		p_pcb->f_txframe = xmit_frame;

		/* if address compare */
		if (p_cmd->cmd_s1 != 0)
		{
			/* load first address byte */
			p_pcb->scc_reg.s1 = p_cmd->cmd_s1;
			p_pcb->scc_reg.cm1 |= C1_SEC;

			/* if dual address mode */
			if (p_cmd->cmd_a1 == l_DUAL_ADDRESS)
			{
				/* load second adress byte */
				p_pcb->scc_reg.s2 = p_cmd->cmd_s2;
				p_pcb->scc_reg.cm1 |= C1_16BIT;
			} 
		}
	}

	/* if bisync */
	else if ((p_cmd->cmd_proto & 0x0f) == l_BISYNC)
	{

		/* set up bisync defaults */
		p_pcb->f_txserve = tx_pre_bsc; 
		p_pcb->f_txframe = bsc_xmit_frame; 

		if (p_cmd->cmd_a1 == l_ASCII)		/* ASCII/Odd parity */
			p_pcb->scc_reg = scc_def_bi_a;
		else					/* EBCDIC */
			p_pcb->scc_reg = scc_def_bi_e;

		/* set up syn characters */
		p_pcb->scc_reg.s1 = p_cmd->cmd_s1;
		p_pcb->scc_reg.s2 = p_cmd->cmd_s2;
	}

	/* if async mode */
	else if ((p_cmd->cmd_proto & 0x0f) == l_ASYNC)
	{
		/* set up async defaults */
		p_pcb->scc_reg = scc_def_asy;
		p_pcb->f_txserve = tx_pre_asy; 
		p_pcb->f_txframe = xmit_frame; 

		/* set character length */
		p_pcb->scc_reg.tp &= TP_5BITS;
		p_pcb->scc_reg.tp |= p_cmd->cmd_char_len - 5;
		p_pcb->scc_reg.rp &= RP_5BITS;
		p_pcb->scc_reg.rp |= p_cmd->cmd_char_len - 5;

		/*  set parity */
		if (p_cmd->cmd_a1 == l_NO_PAR)
			p_pcb->scc_reg.cm1 &= C1_NOPAR;
	 
		/* else even parity */
		else if (p_cmd->cmd_a1 == l_EVEN_PAR)
			p_pcb->scc_reg.cm1 &= ~C1_PODD;

		/* else force parity */
		else
		{
			p_pcb->scc_reg.cm1 |= C1_FRCPAR;
			if (p_cmd->cmd_a1 == l_EVEN_PAR)
			p_pcb->scc_reg.cm1 &= ~C1_PODD;
		}
		/* set stop bits */
		if (p_cmd->cmd_s2 == l_2_STOP)
		p_pcb->scc_reg.tp |= TP_A_STOP2;

		else if (p_cmd->cmd_s2 == l_1_5_STOP)
		{
			p_pcb->scc_reg.tp &= ~TP_A_STOP2;
			p_pcb->scc_reg.tp |= TP_A_STOP15;
		}
		else if ((p_cmd->cmd_s2==l_1_STOP) && (p_cmd->cmd_char_len==5))
			p_pcb->scc_reg.tp &= ~TP_A_STOP2;
	}

	/* set channel connection mode */
	p_pcb->scc_reg.cm2 |= p_cmd->cmd_chan_conn << 6;

	/* set baud rate if not external clocking */
	if (p_cmd->cmd_baud_rate != 0)
	{
		/* initialize transmit and receive timing registers */
		p_pcb->scc_reg.rt = 0;
		p_pcb->scc_reg.tt = 0;

		p_pcb->scc_reg.tt = TT_BRG;
		p_pcb->scc_reg.rt = RT_DPLL32B;

		/* if async, clear default baud rate */
		if ((p_cmd->cmd_proto & 0x0F) == l_ASYNC)
		{
			p_pcb->scc_reg.tt &= ~l_INT_CLK;
			p_pcb->scc_reg.rt = RT_A_BRG;
		}

		p_cmd->cmd_baud_rate &= 0x0F;
		p_pcb->scc_reg.tt |= p_cmd->cmd_baud_rate;
		p_pcb->scc_reg.rt |= p_cmd->cmd_baud_rate;

		/* init pin config register */
		p_pcb->scc_reg.pc &= 0xE0;
		p_pcb->scc_reg.pc |= PC_T_Tx1;
	}

	/* take off CTS controls for all protocols */
	if (!(p_cmd->cmd_proto & l_X21))
	{
		p_pcb->scc_reg.tp |= TP_1TXRTS;
		p_pcb->scc_reg.tp &= TP_0TXCTS;
	}

	/* turn off dcd enable for rx for all protocols */
	p_pcb->scc_reg.rp &= RP_0RXDCD;

	/* write pcb values to the DUSCC */
	init_scc(p_pcb);

	/* Set cio data reg in a neutral state and enable DCE clocking */
	p_pcb->cio_reg.data = DISABLE_DTR | DISABLE_HRS |
				DISABLE_232 | DISABLE_V35;

	/* Check physical link type for proper hardware initialization */

	if ((p_cmd->cmd_proto & 0xF0) & l_V35)		/* V.35 interface */
		p_pcb->cio_reg.data &= ENABLE_V35;

	else if ((p_cmd->cmd_proto & 0xF0) & l_422)	/* RS422 interface */
	/* Enable RS422 interface in IO register */
		p_pcb->cio_reg.data |= ENABLE_422;

	else if ((p_cmd->cmd_proto & 0x0F) & l_X21)	/* X.21 interface */
	/* Enable X21 in enable register */
		out08(ENREG,((in08(ENREG)) & ENR_C_X21));

	else						/* RS232 interface */
	{
		/* Enable RS232 interface in IO register */
		p_pcb->cio_reg.data &= ENABLE_232;

		if (p_cmd->cmd_baud_rate != 0)
		p_pcb->cio_reg.data |= ENABLE_DTE_CLK;
	}

	/* Set cio regs accordingly  */
	set_cio(p_pcb);

	/* free command blocks */
	mi_enable( p_pcb, p_cmd->cmd_intr_mask );
	return(0);				/* Succeeded */
}

/*
   The Serial Register Setup command allows virtually unlimited control
   of the DUSCC and CIO for the diagnostic program.
    First, the command block is queried to see where the parameter list
   really is, either the block itself or the associated buffer.  The
   routine processes register/value pairs serially until the list ends
   with reg=0xFF.  At this time, any read data is bubbled back to the
   driver through the edrr for the port.
*/

#define	NUM_SCC_REGS	34
#define NUM_CIO_REGS	18

int c_serial_setup ( cmd_blk  *cmd, unsigned char far  *buffer )
{
register int	reg;
register int	io_addr;

int		val;		/* next register candidate */
int		abs_reg;	/* absolute reg value      */

extern int	        scc_io_tab[2][NUM_SCC_REGS];
extern int		cio_io_tab[2][NUM_CIO_REGS];

int		scc_base,cio_base;
int		pid;

char		*p_edrr,*p_save;

char far	*p_pair;

/* Fetch the I/O Base addresses of the devices for the given port number.  */

	reg = cmd->_port;
	pid = ( reg >>= 1 );
	scc_base = pcb[reg].scc_base;
	cio_base = pcb[reg].cio_base;

/* Point into the proper port EDRR for register read requests. */

	p_save = &edrr [reg][0];
	p_edrr = p_save + 2;

/* Differentiate between command block and buffer resident register lists. */

	if ( cmd->p._sreg._loc )			/* in buffer */
		p_pair = buffer + cmd->p._sreg._off;
	else						/* in command */
		p_pair = &cmd->p._sreg._pairs [0];

/* while not end delimiter */

	while ( ( reg = *p_pair++ ) != 0xFF )
	{
		abs_reg = reg & 0x1F;
		val = *p_pair++;
	
/* Setup the I/O address of the operation so that common code can be used
   for the actual movement of data to and from the I/O address space. */

		if ( reg < 0x80 )			/* SCC Command */
		{
			if ( abs_reg > NUM_SCC_REGS )
				continue;
			if ( ( io_addr = scc_io_tab [pid][abs_reg] ) == -1 )
			{
				if ( ( reg & 0x60 ) == 0x60 )	/* Read */
					*p_edrr++ = 0xFF;
				continue;
			}
			io_addr += scc_base;
		}
		else					/* CIO Command */
		{
			if ( abs_reg > NUM_CIO_REGS )
				continue;
			if ( ( io_addr = cio_io_tab [pid][abs_reg] ) == -1 )
			{
				if ( ( reg & 0x60 ) == 0x60 )	/* Read */
					*p_edrr++ = 0xFF;
				continue;
			}
			io_addr += cio_base;
			reg &= 0x7F;
		}

/* Now, with the high bit of possible CIO commands cleared, the same ranges
   can be used for automatic handling of the different register functions. */

		if ( reg < 0x20 )			/* Write request */
			(void)out08 ( io_addr, val );

		else if ( reg < 0x40 )			/* OR Mask write */
			(void)out08 ( io_addr, in08 ( io_addr ) | val );

		else if ( reg < 0x60 )			/* !AND Mask write */
			(void)out08 ( io_addr, in08 ( io_addr ) & ~val );

		else					/* Read request */
			*p_edrr++ = in08 ( io_addr );

	}

/* If any reads were performed, place the significant data length in the
   first byte of the appropriate EDRR.  Then return indicating either that
   the EDRR is valid or not.  This saves the driver some data movement. */

	if ( ( reg = p_edrr - &edrr [cmd->_port][2] ) )
		*p_save = reg;				/* Save count */

	return( 0 );
}

typedef struct
{
	unsigned char		_type;
	unsigned char		_port;
	unsigned short		_sequence;
	unsigned char		_filler[12];
	unsigned short		_timeconst;
} t_fast;

data_blaster  ( register t_fast		*p_cmd,
		unsigned char far	*p_buf )
{
register volatile t_pcb		*p_pcb;
 
	p_pcb = &pcb [p_cmd->_port]; 
   
	if ( p_cmd->_timeconst < 2 )
		p_cmd->_timeconst = 2;

	set_FAST( p_pcb->scc_base, p_cmd->_timeconst );

	p_pcb->cio_reg.data |= ENABLE_DTE_CLK;
	set_cio( p_pcb );

	return( 0 );
}
