static char sccsid[] = "@(#)39	1.11  src/bos/usr/lib/asw/mpqp/setparm.c, ucodmpqp, bos411, 9428A410j 2/11/94 22:40:43";

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS: 
 * setparm			: Saves parameters in port control block.
 *				  Initilizes the SCC. Turns on appropriate
 *				  drivers and receivers.  Used for both set parm
 *				  and change parm commands.
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
 *  FUNCTION: This procedure initializes a selected communication
 *            port on the adapter according to data provided in 
 *            the SET_PARM command that is sent to adapter.
 *            Physical connections supported include RS-232-D,
 *            V.35, RS422-A, X.21. SDLC, Bisync, X.21 and Async
 *            are the protocols supported.
 */
     
#include "mpqp.def"
#include "mpqp.typ"
#include "portcb.def"
#include "portcb.typ"
#include "mpqp.ext"
#include "mpqp.pro"
#include "mpqp.h"
#include "iface.def"
  
typedef struct SET_PARM_CMD
{
unsigned char	cmd_typ;		/* Command type			*/ 
unsigned char	cmd_port;		/* Port number			*/
unsigned short	cmd_sequence;		/* Command sequence number	*/
unsigned char	cmd_empty [12];		/* Not used			*/
unsigned char	cmd_field_sel;		/* Field selection		*/
unsigned char	cmd_mod_int;		/* Modem interrupt		*/
unsigned char	cmd_phys_link;		/* Physical link type		*/
unsigned char	cmd_poll_addr;		/* Poll address (bisync/sdlc)	*/
unsigned char	cmd_select_addr;	/* Select address (bisync only)	*/
unsigned char	cmd_baud_rate;		/* Encoded baud rate		*/
unsigned char	cmd_dial_proto;		/* Auto-dial protocol		*/
unsigned char	cmd_dial_flags;		/* Dial flags			*/
unsigned char	cmd_data_proto;		/* Data transfer protocol	*/
unsigned char	cmd_data_flags;		/* Protocol flags		*/
unsigned short	cmd_rcv_tmr;		/* Receive timer		*/
} t_set_parm;

extern	int	tx_pre_x21( cmd_t * );
extern	int	rx_post_x21( volatile t_pcb *, rx_buf_hdr far *);
extern	void	xmit_frame( volatile t_pcb * );
extern	void	xmt_cont( cmd_blk *, unsigned char, int );
extern  unsigned char	trsMask[], rsMask[];
extern	void	copy_parms( volatile t_pcb *, t_set_parm *, unsigned char );

/* PROGRAM STARTS HERE   */

set_parm (t_set_parm		*p_cmd,
	  unsigned char far	*p_buf)
{
register volatile t_pcb	*p_pcb;		/* port control blk pointer	*/
register unsigned char	fs;		/* field select			*/
t_rqe			q_rqe;		/* response queue element	*/
unsigned short		r_u;		/* return value			*/
unsigned short		i;
 
	FOOTPRINT( 'M', 's', 'a' );	/* Msa */
	p_pcb = &pcb [p_cmd->cmd_port];
	fs = p_pcb->field_select = p_cmd->cmd_field_sel;

	/* Copy all pertinent data from command block to PCB */
	/* Note: must check field select because of change parms to driver
	   uses set parms at the adapter level */

	copy_parms( p_pcb, p_cmd, fs );

	/* initialize cio regs to turn all bits off */
	p_pcb->cio_reg.data = DISABLE_DTR | DISABLE_HRS | 
			      DISABLE_V35 | DISABLE_232 | DISABLE_T232;

	/* Check physical link type for proper hardware initialization */

	if (p_pcb->phys_link & PCB_PL_V35)
		p_pcb->cio_reg.data &= ENABLE_V35;
      
	/* else if 422 */
	else if ( p_pcb->phys_link & PCB_PL_422 )
		p_pcb->cio_reg.data |= ENABLE_422;
      
	/* else if 232, hayes or V.25bis */
	else if (! ( p_pcb->phys_link & PCB_PL_X21 ) )
		p_pcb->cio_reg.data &= (ENABLE_232 & ENABLE_T232);

	set_cio(p_pcb);
		
	if ( !(p_pcb->phys_link & PCB_PL_X21) )
	{
		/* unblock Transmit Data */
		out08( CIO_1_BASE + P_B_DATA, ( in08( CIO_1_BASE +
	 	 P_B_DATA) & DISABLE_BLK ));
	}
	/* if x.21, done later during first PAL interrupt service */

	/* Not autodial or x21, set duscc regs for selected protocol */
	if (p_pcb->phys_link > PCB_PL_X21)
	{
		/* set up proto and flags */
		p_pcb->proto = p_pcb->data_proto;
		p_pcb->flags = p_pcb->data_flags;

		/* Set state to PROTOCOL_SET_UP */
		p_pcb->port_state = PROTOCOL_SET_UP;

		if /* Selected protocol is bisync */
		   (p_pcb->data_proto & PCB_PRO_BSC)
			set_bsc(p_pcb);

		else if /* Selected protocol is sdlc */
		   (p_pcb->data_proto & PCB_PRO_SDLC)
			set_sdlc(p_pcb);
	}

	else if (p_pcb->phys_link & PCB_PL_X21)		/* X.21 interface */
	{
		/* Initialize port for X.21 interface */
		p_pcb->scc_reg = scc_def_x21;

		/* set up proto and flags */
		p_pcb->proto = PCB_PRO_X21;
		p_pcb->flags = 0;

		/* Write duscc registers */
		p_pcb->scc_reg.om |= OM_RTS_ACTIVE;
		init_scc (p_pcb);

		/* init tx offlevel, rx ccw, char match bytes */
		p_pcb->f_txserve = tx_pre_x21;
		p_pcb->f_txframe = xmit_frame;
		p_pcb->f_rxserve = rx_post_x21;
		p_pcb->rx_ccw = _CCW_X21_CM;
		p_pcb->rx_cmb = PD_X21_CM;
        	for (i=0;i<2;i++) {
	    	    p_pcb->pdma_rx_llc[i]._ccw = p_pcb->rx_ccw;
	    	    p_pcb->pdma_tx_llc[i]._ccw = _CCW_TX;
        	}
		/* set up duscc interrupt mask */
		if (p_pcb->start_parm & PCB_SP_LEASED)
		    rsMask[0] = RS_B_OVERRUN | RS_B_ABORT | RS_B_CRC | 
			RS_B_SHORT; 
		else				/* switched */
		    rsMask[0] = RS_C_OVERRUN;
		trsMask[0] = 
		    TRS_TXEMPTY | TRS_CTSUNDER | TRS_EOFRAME | TRS_SENDACK;
		/* Make sure Control is off */
		out08( ENREG, ((ENREG_DISABLE) & ENR_C_X21) );
	}

	else /* V.25bis or Smart Modem */
	{
	    p_pcb->port_state = CALL_SET_UP;

	    /* set up proto and flags */

	    /* If dial protocol is not set, assume that it is same as 	*/
	    /* data protocol.						*/
	    if (p_pcb->dial_proto == 0)
		p_pcb->dial_proto = p_pcb->data_proto;
	    p_pcb->proto = p_pcb->dial_proto;
	    p_pcb->flags = p_pcb->dial_flags;
		
	    if (p_pcb->phys_link & PCB_PL_V25)	/* V.25bis interface */
	    {
		/* Auto-dial protocol is bisync */
		if (p_pcb->proto & PCB_PRO_BSC)
		{
		    /* If dial flags haven't been set, enable defaults */
		    if (p_pcb->flags == 0)
			p_pcb->flags = PCB_PF_ASCII | PCB_AF_ODDPAR |
			    PCB_AF_PAREN;
		    set_bsc(p_pcb);
		}

		/* Auto-dial protocol is sdlc */
		else if (p_pcb->proto & PCB_PRO_SDLC)
		    /* No need to check if dial flags weren't set, as 0	*/
		    /* is the default for SDLC dial flags.		*/

		    /* Set DUSCC regs for SDLC regs */
		    set_sdlc(p_pcb);

		else /* async */
		{
		    /* if defaults requested;7 bit char,1 stop bit,odd parity */
		    if (p_pcb->dial_flags == 0) 
		        p_pcb->dial_flags = p_pcb->flags = PCB_AF_PAREN |
			    PCB_AF_ODDPAR | PCB_AF_1STB | PCB_AF_7BCS;
		    set_asc(p_pcb);

		    /* Enable DTE clock source */
		    p_pcb->cio_reg.data |= ENABLE_DTE_CLK;
		    set_cio(p_pcb);
		}
	    }
	    else /* Auto-dial protocol is async for smart modem */
	    {
		if (p_pcb->phys_link != PCB_PL_HAYES)
		    p_pcb->phys_link = PCB_PL_HAYES;
		if (p_pcb->dial_proto != PCB_PRO_ASYNC )
		    p_pcb->dial_proto = p_pcb->proto = PCB_PRO_ASYNC;

		/* if defaults requested; 7 bit char, 1 stop bit, odd parity */
		if (p_pcb->dial_flags == 0) 
		    p_pcb->dial_flags = p_pcb->flags =
			PCB_AF_PAREN |PCB_AF_ODDPAR | PCB_AF_1STB | PCB_AF_7BCS;

		set_asc(p_pcb);
		/* Enable DTE clock source */
		p_pcb->cio_reg.data |= ENABLE_DTE_CLK;
		set_cio(p_pcb);
	    }
	}

	/* Set modem interrupt mask if not given by device driver */
	if ( (fs & PCB_FS_MDM_INT) == 0 )
	{
		/* if not x.21 */
		if (!(p_pcb->phys_link & PCB_PL_X21))
		{
			if (p_pcb->port_state != CALL_SET_UP )
				p_pcb->modem_int = PCB_MM_DS;
			else
				p_pcb->modem_int = PCB_MM_CD | PCB_MM_CT |
						   PCB_MM_DS;
		}
		else
			p_pcb->modem_int = PCB_MM_X2;
	}
	/* else use value given by device driver */
	else
		p_pcb->modem_int = p_pcb->modem_mask;

	p_pcb->modem_int |= MASK_TXRX;

	/* Build adapter response queue with successful SET PARM command */
	q_rqe.f.rtype    = RQE_CMD_ACK;
	q_rqe.f.port     = p_cmd->cmd_port;
	q_rqe.f.status   = p_cmd->cmd_typ;
	q_rqe.f.sequence = 0;

	/* Generate RQE */
	r_u = queue_writel( &arqueue, q_rqe.val );
	if ( r_u < 0 )
	    FOOTPRINT( 'M', 's', 'E' );	/* Msa */
	else
	    host_intr(0);

	/* Free command block */
	free_cmdblk ( (cmd_blk *)p_cmd - cmdblk);

	FOOTPRINT( 'M', 's', 'z' );	/* Msz */
	return(~PCQ_LOCK);
}

chg_parm ( t_set_parm		*p_cmd,
	   int			ignore )
{
register volatile t_pcb	*p_pcb;		/* port control blk pointer	*/
register unsigned char	fs;		/* field select			*/
t_rqe			q_rqe;		/* response queue element	*/
unsigned short		r_u;		/* return value			*/
 
	FOOTPRINT( 'M', 'c', 'a' );	/* Mca */
	p_pcb = &pcb [p_cmd->cmd_port];
	fs = p_cmd->cmd_field_sel;

	copy_parms( p_pcb, p_cmd, fs );

	/* if poll addr changed and running SDLC, change addr in DUSCC */
	if ( ( fs & PCB_FS_PA ) && ( p_pcb->proto & PCB_PRO_SDLC ) )
	{
		out08( p_pcb->scc_base + SCC_CC, RESET_RX );
		p_pcb->scc_reg.cm1 |= C1_SEC;
		p_pcb->scc_reg.s1 = p_pcb->poll_addr;
		out08( p_pcb->scc_base + SCC_CM1, p_pcb->scc_reg.cm1 );
		out08( p_pcb->scc_base + SCC_S1, p_pcb->poll_addr );
		out08( p_pcb->scc_base + SCC_CC, ENABLE_RX );
	}

	/* Free command block */
	free_cmdblk ( (cmd_blk *)p_cmd - cmdblk);

	FOOTPRINT( 'M', 'c', 'z' );	/* Mcz */
	return(~PCQ_LOCK);
}

void copy_parms( volatile t_pcb			*p_pcb,
			  t_set_parm		*p_cmd,
		 	  unsigned char		fs )
{
	if (fs & PCB_FS_PL)
		p_pcb->phys_link   = p_cmd->cmd_phys_link;
	if (fs & PCB_FS_MDM_INT)
		p_pcb->modem_mask   = p_cmd->cmd_mod_int;
	if (fs & PCB_FS_AP)
	{
		p_pcb->dial_proto  = p_cmd->cmd_dial_proto;
		p_pcb->dial_flags  = p_cmd->cmd_dial_flags;
	}
	if (fs & PCB_FS_DP)
	{
		p_pcb->data_proto  = p_cmd->cmd_data_proto;
		p_pcb->data_flags  = p_cmd->cmd_data_flags;
	}

	if (fs & PCB_FS_BR)
		/* make sure high nibble is 0 */
		p_pcb->baud_rate   = (p_cmd->cmd_baud_rate & 0x0F);
	if (fs & PCB_FS_PA)
		p_pcb->poll_addr   = p_cmd->cmd_poll_addr;
	if (fs & PCB_FS_SA)
		p_pcb->select_addr = p_cmd->cmd_select_addr;
	if (fs & PCB_FS_RT)
		p_pcb->rx_timeout  = p_cmd->cmd_rcv_tmr;

	return;
}
