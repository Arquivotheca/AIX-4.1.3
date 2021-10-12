static char sccsid[] = "@(#)71	1.7  src/bos/kernext/fddi/fddi_limbo.c, sysxfddi, bos411, 9428A410j 6/18/94 18:00:17";
/*
 *   COMPONENT_NAME: SYSXFDDI
 *
 *   FUNCTIONS: bugout
 *		enter_limbo
 *		fddi_reset_to
 *		get_stest
 *		hcr_limbo_cmplt
 *		run_stest
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include "fddiproto.h"
#include <sys/sleep.h>
/* 
 * get access to the global device driver control block
 */
extern fddi_tbl_t	fddi_tbl;

/*
 * NAME: run_stest()
 *                                                                    
 * FUNCTION: 
 *	This routine will initiate the running of adapter
 *	self tests.
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment
 *                                                                   
 * NOTES: 
 *	The adapter will be reset via POS reg 2.  The limbo watch
 *	dog timer will be started.
 *
 *	NOTE: The limbo_to routine (called when the timer has gone off)
 *	will initiate the re-activation of the adapter.  There isn't any
 *	way to tell when the adapter is done, so the timer is set to 
 *	a time known to be sufficient for the restart of the microcode.
 *
 * RETURNS:  none
 */  
void
run_stest(	fddi_acs_t	*p_acs)
{
	int	iocc;
	uchar	pos;

	/*
	 * Write out pos 2 with the Adapter Reset (AR) and 
	 * Card Enable (CEN) bits set.
	 */

	iocc = IOCC_ATT(p_acs->dds.bus_id, (IO_IOCC+(p_acs->dds.slot<<16)) );
	PIO_PUTPOS2((iocc+FDDI_POS_REG2), (p_acs->dev.pos[2] | FDDI_POS2_AR  
					| FDDI_POS2_CEN) );
	IOCC_DET(iocc);

	w_start( &(p_acs->dev.limbo_wdt) );

	return;
} /* end run_stest() */

/*
 * NAME: enter_limbo()
 *                                                                    
 * FUNCTION: 
 *	This routine handles error recovery for errors which can be cured by
 * 	the driver automaticly.
 *
 * EXECUTION ENVIRONMENT: interrupt environment
 *                                                                   
 * NOTES: 
 *	Lock the slih, tx, and cmd locks to synchronize with the interrupt 
 * 			handler and runtime top-half entry points.
 *	Clean up the driver
 * 		Stop the timers
 *		If (state != LIMBO)   {cycling limbo, don't repeat status}
 *			send status and error log
 *			drain the transmit queue
 *			wakeup sleeping users  
 *		
 *		If (state != CLOSING)
 *			reset the adapter and begin the limbo timer
 *
 *	Unlock the locks taken
 *			
 * RETURNS:  none
 */  
void
enter_limbo(
	fddi_acs_t	*p_acs,		/* ACS pointer */
	ulong		rc1,		/* primary reason code */
	ulong		ac,		/* specific adapter code */
	uint		slih_flag)
{
	uint		state;		/* save old state */
	uint		iocc;
	uint		ipri1;
	uint		ipri2;
	uint		ipri3;
	ndd_statblk_t	sb;

	if (!slih_flag)
		ipri1 = disable_lock(CFDDI_OPLEVEL,&p_acs->dev.slih_lock);

	ipri2 = disable_lock(CFDDI_OPLEVEL,&p_acs->tx.lock);

	ipri3 = disable_lock(CFDDI_OPLEVEL,&p_acs->dev.cmd_lock);

	FDDI_TRACE("NelB",p_acs, rc1, p_acs->dev.state);

	if (p_acs->dev.state == DEAD_STATE)
	{
		unlock_enable(ipri3,&p_acs->dev.cmd_lock);
		
		unlock_enable(ipri2,&p_acs->tx.lock);
	
		if (!slih_flag)
			unlock_enable(ipri1,&p_acs->dev.slih_lock);
		
		FDDI_TRACE("NelD",p_acs, rc1, p_acs->dev.state);
		return;
	}

        w_stop( &(p_acs->dev.cmd_wdt) );
        w_stop( &(p_acs->dev.limbo_wdt) );
        w_stop( &(p_acs->dev.dnld_wdt) );
        w_stop( &(p_acs->tx.wdt) );

	if (p_acs->dev.state != LIMBO_STATE && 
		p_acs->dev.state != DNLDING_STATE &&
		p_acs->dev.state != LIMBO_RECOVERY_STATE)
	{
		sb.code = NDD_LIMBO_ENTER;
		sb.option[0] = rc1;
		sb.option[1] = ac;
		p_acs->ndd.nd_status(p_acs,&sb);

		fddi_logerr(p_acs, ERRID_CFDDI_RCVRY_ENTER, __LINE__, __FILE__,
				rc1, ac, 0);

		undo_tx(p_acs);
	}


	if (p_acs->dev.state != CLOSING_STATE)
	{
		p_acs->dev.state = LIMBO_STATE;
	}

	p_acs->dev.cmd_status = EINVAL;
	if (p_acs->dev.cmd_event != EVENT_NULL) 
	{
		e_wakeup(&p_acs->dev.cmd_event);
	}

	if (p_acs->dev.cmd_wait_event != EVENT_NULL)
	{
		e_wakeup(&p_acs->dev.cmd_wait_event);
	}

	/* Clear the commands */
	p_acs->dev.p_cmd_prog = (fddi_cmd_t *)0;
	p_acs->dev.cmd_blk.cmd_code = 0;

	/* Reset the receive queue to sync with adapter */
	p_acs->rx.nxt_rx = 0;
	
	/* Clear the commands list to prevent any further slih processing */
	p_acs->dev.hsr_events = 0;

	if ((p_acs->dev.state != CLOSING_STATE) && 
		(p_acs->dev.state != DNLDING_STATE))
	{
		run_stest(p_acs);	

		p_acs->ndd.ndd_flags &= ~(CFDDI_NDD_LLC_DOWN | NDD_RUNNING );
		p_acs->ndd.ndd_flags |= NDD_LIMBO;
	}

	FDDI_TRACE("NelE",p_acs, rc1, p_acs->dev.state);
	unlock_enable(ipri3,&p_acs->dev.cmd_lock);
	
	unlock_enable(ipri2,&p_acs->tx.lock);

	if (!slih_flag)
		unlock_enable(ipri1,&p_acs->dev.slih_lock);
		
	return;

} /* end enter_limbo() */

/*
 * NAME: get_stest()
 *                                                                    
 * FUNCTION: 
 *	This routine will obtain the results of the adapter's self tests
 *                                                                    
 * EXECUTION ENVIRONMENT: process or interrupt environment
 *                                                                   
 * NOTES: 
 *	Assumes that the adapter has just run the self tests (usually via reset)
 *	Turns the card on (via POS2) temporarily to get the self test results
 *	then returns the card to its previous state 
 *
 *	Copies the results into the acs structure.
 *
 * RETURNS:  none
 */  
int
get_stest(fddi_acs_t	*p_acs)
{
	int	iocc;
	int	i;
	int	bus;
	uchar	pos;
	int		ipri;

	ipri = disable_lock(CFDDI_OPLEVEL, &p_acs->dev.slih_lock);
	FDDI_TRACE("NgsB",p_acs, p_acs->dev.state,0);

	iocc = IOCC_ATT(p_acs->dds.bus_id, (IO_IOCC+(p_acs->dds.slot<<16)) );
	PIO_GETPOS2((iocc + FDDI_POS_REG2), &pos);
	pos = pos & ~(FDDI_POS2_AR);
	PIO_PUTPOS2((iocc+FDDI_POS_REG2), (pos | FDDI_POS2_CEN) );

	/*
	* Read all the individual Self Test results.
	* Each test result is 2 bytes long.
	*/
	bus = BUSMEM_ATT(p_acs->dds.bus_id, (ulong)p_acs->dds.bus_mem_addr);
	for ( i=0; i<FDDI_STEST_CNT;++i)
	{
		PIO_GETSRX(bus+(i<<1),&(p_acs->dev.stest[i]));
	}
	BUSMEM_DET(bus);

	PIO_PUTPOS2((iocc + FDDI_POS_REG2), pos);
	IOCC_DET(iocc);
	unlock_enable(ipri, &p_acs->dev.slih_lock);
	if (p_acs->dev.pio_rc)
	{
		FDDI_TRACE("Ngs1",p_acs, p_acs->dev.state,0);
		return (NDD_PIO_FAIL);
	}

	FDDI_TRACE("NgsE",p_acs, p_acs->dev.state,0);
	return (0);
} /* end */

/*
 * NAME: bugout()
 *                                                                    
 * FUNCTION: 
 *	This routine is called in response to a fatal error being detected.
 *	The device will no longer be functional when this
 *	routine returns.  The device state will be DEAD_STATE.
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment
 *                                                                   
 * NOTES: 
 *	Clean up the driver and turn off the adapter
 *
 * RETURNS:  none
 */  
void 
bugout (
		fddi_acs_t	*p_acs,	/* ACS ptr */
		ulong		rc1,	/* primary reason */
		ulong		rc2,	/* secondary reason */
		ulong		ac,	/* specific adapter code */
		uint		slih_flag)
{
	uint		state;		/* save old state */
	uint		iocc;
	uint		ipri1;
	uint		ipri2;
	uint		ipri3;
	ndd_statblk_t	sb;

	if (!slih_flag)
		ipri1 = disable_lock(CFDDI_OPLEVEL,&p_acs->dev.slih_lock);

	ipri2 = disable_lock(CFDDI_OPLEVEL,&p_acs->tx.lock);

	ipri3 = disable_lock(CFDDI_OPLEVEL,&p_acs->dev.cmd_lock);
	FDDI_TRACE("NboB",p_acs, rc1, p_acs->dev.state);

	/* already in the dead state, nothing to do */
	if (p_acs->dev.state == DEAD_STATE)
	{
		unlock_enable(ipri3,&p_acs->dev.cmd_lock);
		
		unlock_enable(ipri2,&p_acs->tx.lock);
	
		if (!slih_flag)
			unlock_enable(ipri1,&p_acs->dev.slih_lock);
		
		FDDI_TRACE("NboD",p_acs, rc1, p_acs->dev.state);
		return;
	}

        w_stop( &(p_acs->dev.cmd_wdt) );
        w_stop( &(p_acs->dev.limbo_wdt) );
        w_stop( &(p_acs->dev.dnld_wdt) );
        w_stop( &(p_acs->tx.wdt) );

	if (p_acs->dev.state != DNLDING_STATE)
	{
		/* send status to users */
	
		sb.code = NDD_HARD_FAIL;
		sb.option[0] = rc1;
		sb.option[1] = ac;
		p_acs->ndd.nd_status(p_acs,&sb);

		/* Drain the transmit queues */
		undo_tx(p_acs);
	}

	fddi_logerr(p_acs, ERRID_CFDDI_DOWN, __LINE__, __FILE__,
				rc1, rc2, ac);


	/* We are closing anyway, don't change the state */
	if (p_acs->dev.state != CLOSING_STATE)
	{
		p_acs->dev.state = DEAD_STATE;
		p_acs->ndd.ndd_flags = NDD_DEAD | NDD_UP | NDD_SIMPLEX | 
			(p_acs->ndd.ndd_flags & CFDDI_NDD_DAC) | NDD_BROADCAST;
	}

	p_acs->dev.cmd_status = EINVAL;
	if (p_acs->dev.cmd_event != EVENT_NULL)
	{
		e_wakeup(&p_acs->dev.cmd_event);
	}

	if (p_acs->dev.cmd_wait_event != EVENT_NULL)
	{
		e_wakeup(&p_acs->dev.cmd_wait_event);
	}

	/* Clear the commands */
	p_acs->dev.p_cmd_prog = (fddi_cmd_t *)0;
	p_acs->dev.cmd_blk.cmd_code = 0;

	p_acs->rx.nxt_rx = 0;
	
	p_acs->dev.hsr_events = 0;

	p_acs->dev.pio_rc = FALSE;

	/* turn off the CEN bit in POS2 */
	disable_card(p_acs);

	FDDI_TRACE("NgsE",p_acs, p_acs->dev.state,0);

	unlock_enable(ipri3,&p_acs->dev.cmd_lock);
	
	unlock_enable(ipri2,&p_acs->tx.lock);

	if (!slih_flag)
		unlock_enable(ipri1,&p_acs->dev.slih_lock);
		
	return;
} /* end bugout() */

/*
 * NAME: hcr_limbo_cmplt()
 *                                                                    
 * FUNCTION: 
 *	This routine will take the driver from a successful check
 *	of the self tests results to the beginning of the adapter
 *	activation sequence (FDDI_WR_USR_DATA).
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment
 *                                                                   
 * NOTES: 
 *	If an error is detected, this routine will enter limbo.
 *	This routine is called by the cmd_handler().  This routine
 *	determines if the Exit Diagnostic mode cmd succeeded.  If so,
 *	it will set the addresses that have been set by the users (if
 *	there are any).
 *
 *	When all the addresses have been set, this routine will initiate
 *	the connection sequence by issueing the Write User Data cmd.  The
 *	hcr_act_cmplt() routine will then take over the egress of limbo.  We
 *	will be in the FDDI_OPEN state when the connection sequence 
 *	successfully completes.
 *
 * RETURNS:  none
 */  
void
hcr_limbo_cmplt(fddi_acs_t	*p_acs,
		fddi_cmd_t	*p_cmd,
		uint		bus,
		int		ipri)
{
	uint	iocc;
	uchar	*p_addr;
	int 	i;


	FDDI_TRACE("NlcB",p_acs, p_cmd->cmd_code,p_cmd->stat);
	if (p_cmd->stat != FDDI_HCR_SUCCESS)
	{
		FDDI_ETRACE("Nlc1",p_acs, p_cmd->cmd_code,p_cmd->stat);
		p_cmd->cmd_code = 0;
		unlock_enable(ipri, &p_acs->dev.cmd_lock);
		fddi_logerr(p_acs, ERRID_CFDDI_CMD_FAIL, __LINE__, __FILE__,
				0, 0, 0);
		enter_limbo(p_acs, NDD_CMD_FAIL, 0, TRUE);
		ipri = disable_lock(CFDDI_OPLEVEL, &p_acs->dev.cmd_lock);
		return;
	}

	i = p_acs->dev.addr_index;
	while (i<FDDI_MAX_HDW_ADDRS)
	{
		if (p_acs->addrs.hdw_addrs[i].addr_cnt != 0)
			break;
		i++;
	}

	if (i<FDDI_MAX_HDW_ADDRS)
	{
		p_acs->dev.addr_index = i + 1;
		/*
		* Reset command structure each time
		*/
		p_cmd->pri = 0;
		p_cmd->cmplt = (int(*)()) hcr_limbo_cmplt;
		p_cmd->cmd_code = FDDI_HCR_WRITE_ADDR;
		p_cmd->cpb_len = 14;
		p_cmd->cpb[2] = SWAPSHORT(FDDI_HCR_ALD_SKY);
		p_cmd->cpb[3] = SWAPSHORT (FDDI_HCR_ATD_SKY);

		/*
	 	* Put address in proper format for the
		* adapter
	 	*/
		p_addr = (char *) &(p_cmd->cpb[4]);
		*(p_addr++)=p_acs->addrs.hdw_addrs[i].addr[5];
		*(p_addr++)=p_acs->addrs.hdw_addrs[i].addr[4];
		*(p_addr++)=p_acs->addrs.hdw_addrs[i].addr[3];
		*(p_addr++)=p_acs->addrs.hdw_addrs[i].addr[2];
		*(p_addr++)=p_acs->addrs.hdw_addrs[i].addr[1];
		*(p_addr++)=p_acs->addrs.hdw_addrs[i].addr[0];

		FDDI_TRACE("NlcD",p_acs, p_cmd->cmd_code,p_acs->dev.addr_index);
		return;
	}
	p_acs->dev.addr_index = 0;

	/*
	 * start the connection process. the
	 * hcr_act_cmplt() routine will take us 
	 * to the FDDI_OPEN state.
	 */

	p_cmd = &(p_acs->dev.cmd_blk);
	p_cmd->stat = 0;
	p_cmd->pri = 0;
	p_cmd->cmd_code = FDDI_HCR_WR_USR_DATA;
	p_cmd->cmplt = (int(*)()) hcr_act_cmplt;


	/* 
	 * Here we need to put the 32 byte User Data
	 * into the cpb.  We also need to swap the 
	 * User data.
	 *
	 * We will go thru the User Data char array
	 * by multiples of 2, assign that element and the next 
	 * to the CPB (after byte swapping).
	 */

	for (i=0; i<(FDDI_USR_DATA_LEN >> 1); i++)
	{
		/* 
		 * we add 2 here to account for the
		 * offset into the CPB where the 
		 * User Data will start.
		 */
		p_cmd->cpb[2+i] = SWAPSHORT((ushort)
			p_acs->dds.user_data[i<<1]);
	}
	p_cmd->cpb_len = FDDI_USR_DATA_LEN + 4; 

	FDDI_TRACE("NlcE",p_acs, p_cmd->cmd_code,p_cmd->stat);
	return;
} /* end limbo_cmplt() */
			

/*
 * NAME: fddi_reset_to()
 *                                                                    
 * FUNCTION: This routine begins the reactivation of the adapter after a reset
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment
 *                                                                   
 * NOTES:  
 *	Config the pos registers
 *	Check the self tests
 *	Start the cards reactivation
 *
 *	NOTE : if a problem occurs, bugout : the self tests problem probably
 *		indicates a hardware problem
 *
 * RETURNS:  none
 */  
void
fddi_reset_to(struct watchdog	*p_wdt)
{
	fddi_acs_t	*p_acs;
	int		ipri;
	uint		iocc, bus, ioa;
	ushort		hsr;
	int		i;
	fddi_cmd_t	*p_cmd;
	int		error;
	ushort		badrc;



	/* get ACS */
	p_acs = (fddi_acs_t *) ((uint) p_wdt - 
		((uint) offsetof (fddi_acs_dev_t, limbo_wdt) + 
		 (uint) offsetof (fddi_acs_t, dev)));


	FDDI_TRACE("NrtB",p_acs, 0,0);

	cfg_pos_regs(p_acs);

	if (get_stest(p_acs) == NDD_PIO_FAIL)
	{
		FDDI_ETRACE("Nrt1",p_acs, 0,0);
		fddi_logerr(p_acs, ERRID_CFDDI_PIO, __LINE__, __FILE__,
				0, 0, 0);
		bugout(p_acs,NDD_PIO_FAIL,0,0,FALSE);
		return;
	}

	if (p_acs->dev.state == DEAD_STATE)
	{
		FDDI_TRACE("NrtD",p_acs, 0,0);
		return;
	}

	/*  
	 * enable the card 
	 */
	iocc = IOCC_ATT(p_acs->dds.bus_id, IO_IOCC + (p_acs->dds.slot << 16));
	PIO_PUTPOS2(iocc+FDDI_POS_REG2, (p_acs->dev.pos[2] | FDDI_POS2_CEN));
	IOCC_DET(iocc);

	if (p_acs->dev.pio_rc)
	{
		FDDI_ETRACE("Nrt2",p_acs, 0,0);
		fddi_logerr(p_acs, ERRID_CFDDI_PIO, __LINE__, __FILE__,
				0, 0, 0);
		bugout(p_acs,NDD_PIO_FAIL,0,0,FALSE);
		return;
	}

	/*
	* Read all the individual Self Test results.
	* Each test result is 2 bytes long.
	*/

	badrc = 0;
	for (i=0;i<8;++i)
	{
		if (p_acs->dev.stest[i] !=0)
		{
			badrc = p_acs->dev.stest[i];
		}
	}
	if (p_acs->dev.stest[8] == 0x0000) 
	{
		p_acs->dev.attach_class = FDDI_ACLASS_DAS;
	}
	else if (p_acs->dev.stest[8] == 0x80ea) 
	{
		p_acs->dev.attach_class = FDDI_ACLASS_SAS;
	}
	else
	{
		badrc = p_acs->dev.stest[8];
	}

	if ( (p_acs->dev.stest[9] != 0x0000) &&
 		(p_acs->dev.stest[9] != 0x9001) )
	{
		badrc = p_acs->dev.stest[9];
	}

	if ((p_acs->dev.card_type == FDDI_SC) && 
		(p_acs->dev.stest[10] != 0x0000)  &&
		(p_acs->dev.stest[10] != 0xA0EA))
	{
		badrc = p_acs->dev.stest[10];
	} /* self tests failed */

	if (badrc != 0)
	{
		FDDI_ETRACE("Nrt3",p_acs, 0,0);
		fddi_logerr(p_acs, ERRID_CFDDI_SELFT_ERR, __LINE__,__FILE__,
			badrc, 0, 0);
		bugout(p_acs,CFDDI_SELF_TEST,0,badrc,FALSE);
		return;
	} /* self tests failed */

        ioa = BUSIO_ATT(p_acs->dds.bus_id, p_acs->dds.bus_io_addr);

	PIO_GETSRX(ioa, &hsr);
	/*
	* Set the interrupt mask to the Base interrupts.
	*/

	PIO_PUTSRX (ioa+FDDI_HMR_REG, FDDI_HMR_RX_TX);
	BUSIO_DET(ioa);
	if (p_acs->dev.pio_rc)
	{
		FDDI_ETRACE("Nrt4",p_acs, 0,0);
		fddi_logerr(p_acs, ERRID_CFDDI_PIO, __LINE__, __FILE__,
				0, 0, 0);
		bugout(p_acs,NDD_PIO_FAIL,0,0,FALSE);
		return;
	}

	bus = BUSMEM_ATT(p_acs->dds.bus_id, (ulong)p_acs->dds.bus_mem_addr);
	p_cmd = &(p_acs->dev.cmd_blk);
	p_cmd->pri =0;
	p_cmd->cmplt = (int(*)()) hcr_limbo_cmplt;
	p_cmd->cmd_code = FDDI_HCR_START_MCODE;
	p_cmd->cpb_len = 0;
	if (issue_cmd(p_acs, p_cmd, bus, FALSE))
	{
		FDDI_ETRACE("Nrt5",p_acs, 0,0);
		BUSMEM_DET(bus);
		fddi_logerr(p_acs, ERRID_CFDDI_PIO, __LINE__, __FILE__,
				0, 0, 0);
		bugout(p_acs,NDD_PIO_FAIL,0,0,FALSE);
		return;
	}

	BUSMEM_DET(bus);
	p_acs->dev.state = LIMBO_RECOVERY_STATE;

	FDDI_TRACE("NrtE",p_acs, 0,0);
	return;
} /* end fddi_reset_to() */

			
