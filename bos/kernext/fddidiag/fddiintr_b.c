static char sccsid[] = "@(#)83	1.2  src/bos/kernext/fddidiag/fddiintr_b.c, diagddfddi, bos411, 9428A410j 11/8/93 09:51:27";
/*
 *   COMPONENT_NAME: DIAGDDFDDI
 *
 *   FUNCTIONS: fddi_cmd_handler
 *		fddi_cmd_to
 *		fddi_enter_diag_cmplt1597
 *		fddi_issue_cmd
 *		fddi_ls_bti_handler
 *		fddi_oflv
 *		fddi_slih
 *		fddi_uls_handler
 *		hsr_bti_handler
 *		issue_pri_cmd
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "fddiproto.h"
#include "fddi_comio_errids.h"
#include <sys/adspace.h>
#include <sys/iocc.h>
#include <sys/ioacc.h>
#include <sys/dma.h>

/* 
 * Declaration of FDDI control structure - controls all adapters on machine
 */
fddi_ctl_t fddi_ctl =
{
	FALSE, 		/* DD init flag */
	FALSE, 		/* DD first open flag */
	LOCK_AVAIL,	/* global lock */
	0,		/* Number of config'd adapters */
	0,		/* Number of opens */
	0,		/* next channel number to assign */
	0,		/* current size of open table */
	NULL,		/* the current open table */
	{		/* array of ACS pointers */
		NULL
	}
};

/*
 * initialize 	fdditrace.next = 0
 *		fdditrace.res1 = "FDDI"
 *		fdditrace.res2 = "TRAC"
 *		fdditrace.res3 = "ETBL"
 *		fdditrace.table[0] = "!!!!"
 *
 * In the low-level debugger, one would do a find for
 *
 *		FDDITRACETBL
 *
 * to find the starting location of the internal trace table.
 *
 */
fddi_trace_t	fdditrace = 
{ 
	0,		/* fdditrace.next */
	0x46444449, 	/* "FDDI" */
	0x54524143, 	/* "TRAC" */
	0x4554424c, 	/* "ETBL" */
	0x21212121 	/* "!!!!" */
};

/*
 * FDDI component dump table objects:
 */
struct cdt	*p_fddi_cdt = NULL;
int		l_fddi_cdt = 0;






/*
 * NAME: fddi_uls_handler
 *                                                                    
 * FUNCTION: handle all the meaningful events found in the link statistics
 *                                                                    
 * EXECUTION ENVIRONMENT: Interrupt environment.
 *
 *	This is the interrupt handler of a completed 
 *	Update Link Statistics (ULS) command.
 *                                                                   
 * NOTES: 
 *
 *	The link statistics consists of errors, events, and status bits.
 *	The adapter will clear or maintain error bits when read.
 *		- Cleared error bits require individual processing: 
 *		  notification and/or state change.
 *		- Maintained error bits are considered unrecoverable and cause
 *		  the driver to go to the DEAD state.
 *	Event bits are all cleared when read. Events require notification
 *	and/or state change.
 *	Status bits toggle so we to detect changes only. 
 *
 *	The following action can be taken in this routine:
 *
 *		Enter the DEAD state and halt processing
 *		Enter Network Recovery Mode
 *		Call a command specific completion function
 *		Async Notification of all open users
 *
 *	Process Limbo Entry Conditions (LEC) first.
 *	(We do not check the aci_code because that is set when we get
 *	an Adapter Check Interrupt (ACI) and will be processed in the
 *	ACI interrupt handler.)
 *	If there is a command waiting on the Link statistics then give
 *	that command's completion routine the opportunity to examine
 *	the LS next.
 *	Finally, process the Event Notifications.
 *
 *	Invalid SET COUNT event which is detected here will be 
 *	satisfied by retrying the command which is done automatically
 *	through the generic command handler.
 *
 *	All events indicated in the Link Statistics
 *	will be satisfied when this routine is completed.
 *		
 * RECOVERY OPERATION: 
 *
 * 	The ULS command never needs to be retried.
 *	If it fails it is not a problem inherent in the command but
 *	a more serious problem like an ADAPTER CHECK which is 
 *	an unrecoverable error.
 *
 * DATA STRUCTURES: 
 *
 *	The links structure in the work section of the acs is a copy
 *	of the link statistics on the adapter. The update link stats
 *	command will update the adapter's copy from various hardware 
 *	registers. Then we will read the adapter's copy into the acs
 *	work section. 
 *
 * RETURNS:  none
 */  	
 

int
fddi_uls_handler (
	fddi_acs_t	*p_acs,
	fddi_cmd_t	*p_cmd,
	int		bus)
{
	cio_stat_blk_t	sb; 		/* status blk */
	fddi_links_t	*p_ls;		/* link statistics */
	int		rc = 0;

	FDDI_TRACE("ouhB", p_acs, p_acs->dev.state, p_cmd->stat);
	/* 
	 * PIO the link statistics
	 */
	bus = BUSMEM_ATT(p_acs->dds.bus_id, (uint)p_acs->dds.bus_mem_addr);
	FDDI_GET_LINK_STATS (p_acs->ras.ls);
	BUSMEM_DET(bus);
	p_ls = &p_acs->ras.ls;
	/*
	 * Trace link statistics
	 *	Adapter check code printed out at ACI detection time
	 *	because it doesn't require an ULS command.
	 */
	FDDI_DBTRACE("ouhC", 
			p_acs, 
			*(uint *) &p_acs->ras.ls.smt_error_lo, 
			*(uint *) &p_acs->ras.ls.smt_event_lo); 
	FDDI_DBTRACE("ouhC", 
			p_acs, 
			*(uint *) &p_acs->ras.ls.cpv, 
			*(uint *) &p_acs->ras.ls.setcount_lo); 
	/* 
	 * if we have any BTI conditions indicated
	 * in the Link Statistics, handle them.
	 */
	if ( (p_ls->smt_event_lo & FDDI_BTI_SMT_EVNT_LO) ||
		( (p_ls->smt_event_lo & FDDI_SMT_EVNT_LO_PT_MSK) ==
	  		FDDI_SMT_EVNT_LO_PTF) )
	{
		rc = fddi_ls_bti_handler(p_acs, p_ls);
		return (rc); 
	}

	sb.code = CIO_ASYNC_STATUS;
	sb.option[0] = FDDI_RING_STATUS;

	sb.option[1] = FDDI_SMT_EVENT;
	sb.option[3] = (p_ls->smt_event_hi << 16) | p_ls->smt_event_lo;
	if ((p_ls->smt_event_lo & FDDI_SMT_EVNT_LO_LSS) == 0x0)
	{
		/*
		 * send status blk indicating that
		 * LLC services have been disabled.
		 */
		if (p_acs->dev.state == FDDI_OPEN)
		{
			fddi_logerr(p_acs, ERRID_FDDI_LLC_DISABLE,
				__LINE__, __FILE__);
			p_acs->dev.state = FDDI_LLC_DOWN;
			sb.option[2] = FDDI_LLC_DISABLE;
			fddi_async_status(p_acs, &sb);
		}
	}
	if ( p_ls->smt_event_lo & FDDI_SMT_EVNT_LO_LSS )
	{
		/* 
		 * send status blk indicating that 
		 * LLC services have been re-enabled
		 */
		if (p_acs->dev.state == FDDI_LLC_DOWN)
		{
			fddi_logerr(p_acs, ERRID_FDDI_LLC_ENABLE,
				__LINE__, __FILE__);
			p_acs->dev.state = FDDI_OPEN;
			sb.option[2] = FDDI_LLC_ENABLE;
			fddi_async_status(p_acs, &sb);
		}
	}
	if ( p_ls->smt_event_lo & FDDI_SMT_EVNT_HI_RGA )
	{
		/* 
		 * send status blk indicating that 
		 * an long address has been remotely added.
		 */
		sb.option[2] = FDDI_ADDR_RMVD;
		fddi_async_status(p_acs, &sb);
	}

	if ((p_ls->smt_event_lo & FDDI_SMT_EVNT_LO_ROS) != p_acs->dev.rop)
	{
		/* 
		 * send status blk indicating that 
		 * an long address has been remotely added.
		 */
		
		if ( p_ls->smt_event_lo & FDDI_SMT_EVNT_LO_ROS )
			sb.option[2] = FDDI_ROP;
		else 	sb.option[2] = FDDI_NO_ROP;
			
		p_acs->dev.rop = p_ls->smt_event_lo & FDDI_SMT_EVNT_LO_ROS;

		fddi_async_status(p_acs, &sb);
	}

	if ( p_ls->smt_event_lo & FDDI_SMT_EVNT_LO_AGA )
	{
		/* 
		 * send status blk indicating that 
		 * an long address has been remotely added.
		 * !!! add in threshold for reporting of this
		 */
		sb.option[2] = FDDI_ADDR_ADDED;
		fddi_async_status(p_acs, &sb);
	}

	/*
	 * The remainder of the status blocks do not need to 
	 * goto the normal users.  Only want to send these status
	 * blocks to the SMT netid user.
	 * 
	 * We do want to error log those errors that we need to.
	 */
	sb.option[1] = FDDI_SMT_ERROR;
	sb.option[3] = (p_ls->smt_error_hi << 16) | p_ls->smt_error_lo;

	if ( p_ls->smt_event_lo & FDDI_SMT_EVNT_LO_RTT )
	{
		if (p_acs->dev.thresh_rtt-- == 0)
		{
			/* 
		 	* send status blk indicating that 
		 	* a restricted token dialog has been terminated
		 	*/
			sb.option[2] = FDDI_RTT;
			if (p_acs->ctl.p_netids[FDDI_SMT_NETID] != NULL)
				fddi_report_status(
					p_acs, 
					p_acs->ctl.p_netids[FDDI_SMT_NETID], 
					&sb);
			p_acs->dev.thresh_rtt = FDDI_SMT_THRESH;
		}
	}
	if ( p_ls->smt_event_lo & FDDI_SMT_EVNT_LO_TRC )
	{
		if (p_acs->dev.thresh_trc-- == 0)
		{
			/* 
		 	 * send status blk indicating that 
		 	 * T-Req has been changed remotely
		 	 */
			sb.option[2] = FDDI_REMOTE_T_REQ;
			if (p_acs->ctl.p_netids[FDDI_SMT_NETID] != NULL)
				fddi_report_status(
					p_acs, 
					p_acs->ctl.p_netids[FDDI_SMT_NETID], 
					&sb);
			p_acs->dev.thresh_trc = FDDI_SMT_THRESH;
		}
	}
	if ( p_ls->smt_error_lo & FDDI_SMT_ERR_LO_STUCK )
	{
		if (p_acs->dev.thresh_stuck-- == 0)
		{
			/* 
		 	* send status blk indicating that a port 
		 	* is stuck.
		 	*/
			fddi_logerr(p_acs, ERRID_FDDI_PORT,
				__LINE__, __FILE__);
			sb.option[2] = FDDI_PORT_STUCK;
			if (p_acs->ctl.p_netids[FDDI_SMT_NETID] != NULL)
				fddi_report_status(p_acs, 
					p_acs->ctl.p_netids[FDDI_SMT_NETID], 
					&sb);
			p_acs->dev.thresh_stuck = FDDI_SMT_THRESH;
		}
	}
	if ( p_ls->smt_error_lo & FDDI_SMT_ERR_LO_TME )
	{
		if (p_acs->dev.thresh_tme-- == 0)
		{
			/* 
		 	* send status blk indicating that 
		 	* Trace Max has been exceeded.
		 	*/
			fddi_logerr(p_acs, ERRID_FDDI_TRACE,
				__LINE__, __FILE__);
			sb.option[2] = FDDI_TRACE_MAX;
			if (p_acs->ctl.p_netids[FDDI_SMT_NETID] != NULL)
				fddi_report_status(p_acs, 
					p_acs->ctl.p_netids[FDDI_SMT_NETID], 
					&sb);
			p_acs->dev.thresh_tme = FDDI_SMT_THRESH;
		}
	}
	if ( p_ls->smt_error_lo & FDDI_SMT_ERR_LO_SBF )
	{
		if (p_acs->dev.thresh_sbf-- == 0)
		{
			/* 
		 	* send status blk indicating that 
		 	* optical bypass switch is stuck
		 	*/
			fddi_logerr(p_acs, ERRID_FDDI_BYPASS,
				__LINE__, __FILE__);
			sb.option[2] = FDDI_BYPASS_STUCK;
			if (p_acs->ctl.p_netids[FDDI_SMT_NETID] != NULL)
				fddi_report_status(p_acs, 
					p_acs->ctl.p_netids[FDDI_SMT_NETID], 
					&sb);
			p_acs->dev.thresh_sbf = FDDI_SMT_THRESH;
		}
	}
	/*
	 * If ULS command done in response to an invalid set count
	 *	then put the new setcount in the current command block.
	 */
	if (p_acs->dev.cmd_blk.stat == FDDI_HCR_INV_SETCOUNT)
	{
		p_acs->dev.cmd_blk.cpb[0] = p_acs->ras.ls.setcount_lo;
		p_acs->dev.cmd_blk.cpb[1] = p_acs->ras.ls.setcount_hi;
	}

	/* ok */
	FDDI_TRACE("ouhE", p_acs, p_acs->dev.state, p_cmd->stat);
	return (rc);
}
/*
 * NAME: hsr_bti_handler
 *                                                                    
 * FUNCTION: 
 *	This routine is called in response to a BTI being detected in
 *	HSR.  This routine will determine if the error is recoverable.
 *	If the error is recoverable, the error will be taken care of
 *	and processing can then proceed as normal.  If not, the device
 *	will go into limbo.
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt thread (SLIH)
 *                                                                   
 * NOTES: 
 *	We first check for a MC error.  If there is one, we initiated
 *	recovery logic to clear the error on the channel.
 *	We then check for an Adapter Check.  If an adapter check has
 *	occured, we will log the error then enter limbo.
 *
 *
 * RECOVERY OPERATION:  
 *	The limbo recovery operation may be initiated.  This is dependent
 *	on the BTI that is indicated in the HSR.
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *	0		- continue processing other interrupts
 *	ENETUNREACH 	- don't continue processing other interrupts
 *		  	(as in unrecoverable errors)
 */  
void 
hsr_bti_handler (
	fddi_acs_t	*p_acs,
	int		bus,
	int		ioa)
{
	int		iocc;
	fddi_icr_cmd_t	icr;
	ushort		hsr;
	uchar		tmp_pos;

	FDDI_TRACE("obtB", p_acs, p_acs->dev.state, p_acs->dev.oflv_copy);
	/*
	 * Check for a MC error
	 *	NSF - No card select feedback return
	 *	CCR - Channel check on Read
	 *	CCW - Channel check on write
	 *	DPR - Data parity on read
	 *
	 * initiate recovery logic if one of those error occured.
	 */
	if (p_acs->dev.oflv_copy & ( FDDI_HSR_NSF | FDDI_HSR_CCR | 
		FDDI_HSR_CCW | FDDI_HSR_DPR) )
	{
		/*
		 * The adapter got an MC error during a BUS master 
		 * operation.
		 */
		/* Set D/D bit in POS reg to get access to the ICRs */
		iocc = IOCC_ATT (p_acs->dds.bus_id, 
				(IO_IOCC + (p_acs->dds.slot<<16)));

		PIO_GETCX( iocc + FDDI_POS_REG2, &tmp_pos);
		PIO_PUTCX ((iocc + FDDI_POS_REG2), 
				p_acs->dev.pos2 | FDDI_POS2_CEN | FDDI_POS2_DD);

		/* Read the ICR and obtain DMA address */

		PIO_GETSRX(bus+0x0, &icr.local_addr);
		PIO_GETSRX(bus+0x2, &icr.len3);
		PIO_GETSRX(bus+0x4, &icr.hi_addr3);
		PIO_GETSRX(bus+0x6, &icr.lo_addr3);
		PIO_GETSRX(bus+0x8, &icr.len2);
		PIO_GETSRX(bus+0xa, &icr.hi_addr2);
		PIO_GETSRX(bus+0xc, &icr.lo_addr2);
		PIO_GETSRX(bus+0xe, &icr.len1);
		PIO_GETSRX(bus+0x10, &icr.hi_addr1);
		PIO_GETSRX(bus+0x12, &icr.lo_addr1);
		PIO_GETSRX(bus+0x14, &icr.cmd);


		/* trace contents of ICRs */
		FDDI_TRACE("obt1", icr.local_addr, icr.cmd, 
			p_acs->dev.oflv_copy);
		FDDI_TRACE("obtC", icr.len1, icr.hi_addr1, icr.lo_addr1);
		FDDI_TRACE("obtC", icr.len2, icr.hi_addr2, icr.lo_addr2);
		FDDI_TRACE("obtC", icr.len3, icr.hi_addr3, icr.lo_addr3);


		/* Turn off D/D bit */
		PIO_PUTCX ((iocc + FDDI_POS_REG2), 
				(tmp_pos & ~(FDDI_POS2_AR)));

		IOCC_DET(iocc); 
		/*
		 * issue a d_complete to clear the error on the channel.
		 * save the error. log error.
		 * re-d_master the TX small frame cache.
		 */
		p_acs->dev.mcerr = d_complete(p_acs->dev.dma_channel, 
					DMA_WRITE_ONLY,
					p_acs->tx.p_sf_cache, 
					(size_t) FDDI_SF_BUFSIZ, 
					&p_acs->tx.xmd, 
					(char *)p_acs->tx.p_d_sf);

		
		FDDI_TRACE("obt2", p_acs, p_acs->dev.oflv_copy, 
			p_acs->dev.mcerr);
		fddi_logerr(p_acs, ERRID_FDDI_MC_ERR,
			__LINE__, __FILE__);
		d_master(p_acs->dev.dma_channel, DMA_WRITE_ONLY,
			p_acs->tx.p_sf_cache, (size_t)FDDI_SF_BUFSIZ,
			&p_acs->tx.xmd, (char *)p_acs->tx.p_d_sf);

		/*
		 * Tell the adapter to go!!!
		 */
		PIO_PUTSRX(ioa + FDDI_NS1_REG, (FDDI_NS1_LCA | FDDI_NS1_HCC));
	}
	if (p_acs->dev.oflv_copy & FDDI_HSR_ACI)
	{
		/* 
		 * Grab the LS from the adapter.
		 * and put in the LS structure in the ACS.
		 */
		FDDI_GET_LINK_STATS (p_acs->ras.ls);

		FDDI_TRACE("obt3", p_acs, p_acs->ras.ls.aci_code, 
			p_acs->dev.oflv_copy);
		fddi_logerr(p_acs, ERRID_FDDI_ADAP_CHECK,
			__LINE__, __FILE__);

		if ((p_acs->dev.state == FDDI_INIT) && 
				(p_acs->ras.ls.aci_code & FDDI_ACI_CMD_HDLR))
		{
			fddi_conn_done(p_acs,CIO_HARD_FAIL);
			fddi_bugout(p_acs, FDDI_ADAP_CHECK, 
				p_acs->ras.ls.aci_code, 0);
		}
		else	fddi_enter_limbo (p_acs, FDDI_ADAP_CHECK, 
				(ulong)p_acs->ras.ls.aci_code);
		/*
		 * Don't do anymore processing 
		 */
	}
	if (p_acs->dev.oflv_copy & FDDI_HSR_AWS)
	{
		/* 
		 * Grab the LS from the adapter.
		 * and put in the LS structure in the ACS.
		 */
		FDDI_GET_LINK_STATS (p_acs->ras.ls);

		FDDI_TRACE("obt4", p_acs, p_acs->ras.ls.aci_code, 
			p_acs->dev.oflv_copy);
		fddi_logerr(p_acs, ERRID_FDDI_ADAP_CHECK,
			__LINE__, __FILE__);

		fddi_enter_limbo (p_acs, FDDI_ADAP_CHECK, 
				(ulong)p_acs->ras.ls.aci_code);
		/*
		 * Don't do anymore processing 
		 */
	}

	/*
	 * As errors are handled the appropriate bits are turned 
	 *	off in the hsr which is passed back to the offlevel handler
	 */
	FDDI_TRACE("obtE", p_acs, p_acs->dev.state, p_acs->dev.oflv_copy);
	return;
} /* end hsr_bti_handler() */
/*
 * NAME: fddi_issue_cmd
 *                                                                    
 * FUNCTION: issue a command to the adapter 
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *
 *	Process or interrupt environment.
 *	Interrupts must be disabled to the level of the SLIH before 
 *	calling this routine.
 *                                                                   
 * NOTES: 
 *
 *	Issue the command passed in. Set the 'command in progress' pointer,
 *	start the watchdog timer, get the set count from shared memory and
 *	issue the command.
 *	If a command is in progress then this must be a priority command
 *	trying to go out when a regular command is yet to finish. This is
 *	not a problem because all priority commands are 'queued' in the
 *	pri_que member of the acs.dev struct.
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: none
 */  

void
fddi_issue_cmd (
	fddi_acs_t	*p_acs,
	fddi_cmd_t	*p_cmd, 
	int		bus)
{
	int 	ioa;

	FDDI_DBTRACE("PicB", p_acs, p_cmd, p_acs->dev.state);
	FDDI_DBTRACE("PicC", p_cmd->cmd_code, p_cmd->ctl, p_cmd->pri);

	ASSERT((bus != NULL))
	ASSERT(p_acs->dev.p_cmd_prog != &(p_acs->dev.cmd_blk));

	if (p_acs->dev.p_cmd_prog)
	{
		/* not yet: command in progress */
		FDDI_DBTRACE("Pic1", p_acs, p_cmd, p_acs->dev.p_cmd_prog);
		return;
	}
	/* initialize */
	p_cmd->stat = CIO_OK;
	if (p_cmd->cmd_code != FDDI_HCR_READ_ADDR)
	{
		p_cmd->cpb[0] = p_acs->ras.ls.setcount_lo;
		p_cmd->cpb[1] = p_acs->ras.ls.setcount_hi;
	}

	p_acs->dev.p_cmd_prog = p_cmd;

	ioa = BUSIO_ATT(p_acs->dds.bus_id, p_acs->dds.bus_io_addr);

	/* start watchdog timer */
	w_start (&(p_acs->dev.cmd_wdt));

	/* 
	 * string move parameters to shared memory 
	 *	(len includes setcount)
	 */
	PIO_PUTSTRX(bus + FDDI_CPB_SHARED_RAM, &p_cmd->cpb[0], p_cmd->cpb_len);

	/* issue the current command to the adapter */
	PIO_PUTSRX(ioa + FDDI_HCR_REG, p_cmd->cmd_code);

	BUSIO_DET(ioa);
	FDDI_DBTRACE("PicE", p_acs, p_cmd, p_acs->dev.state);

	/* ok */
	return;
}
/*
 * NAME: issue_pri_cmd
 *                                                                    
 * FUNCTION:  issue a priority command
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt thread only
 *                                                                   
 * NOTES: 
 *
 *	Called by: fddi_cmd_handler()
 *		   fddi_slih () on an RSI interrupt
 *
 *	Setup the priority command structure and issue the command.
 *
 * RECOVERY OPERATION:  none
 *
 * DATA STRUCTURES: 
 *
 *	The 'pri_que' member of the ACS is a bitwise OR of
 *	all the priority commands pending. Each time a priority
 *	command is issued it's bit will be turned off.
 *	Assume that 'pri_cmd_to_issue' is in the 'p_acs->dev.pri_que'
 *	so that if there is a command in progress when 'issue_cmd'
 *	is called it will not be forgotten.
 *
 * RETURNS: none
 */  
static int
issue_pri_cmd (
	fddi_acs_t	*p_acs,
	int		pri_cmd_to_issue,
	int		bus)
{
	fddi_cmd_t	*p_cmd;

	p_cmd = &(p_acs->dev.pri_blk);

	/* common priority command setup */
	p_cmd->ctl = 0;
	p_cmd->cpb_len = 0; 

	FDDI_DBTRACE("PpcB", p_acs, p_acs->dev.state, pri_cmd_to_issue );
	/*
	 * 'pri_cmd_to_issue' can indicate more than one command but only one
	 *	command will be issued at a time. The order of the
	 *	if statement prioritizes the commands when more
	 *	than one command is indicated.
	 */
	if (pri_cmd_to_issue & FDDI_PRI_CMD_ULS)
	{
		/* setup command block for Update Links Statistics */
		p_cmd->cmd_code = FDDI_HCR_ULS;
		p_cmd->cmplt = (int(*)()) fddi_uls_handler;
		p_cmd->cpb_len = 0;

		/* 
		 * turn off priority command indicator 
		 *	to make sure command doesn't run more than once
		 */
		p_acs->dev.pri_que &= (~FDDI_PRI_CMD_ULS);
		FDDI_DBTRACE("Ppc1", p_acs, p_acs->dev.pri_que, 0 );
	}
	else
	{
		/* add logic for other priority commands here ... */

		/* internal programming error */
		FDDI_DBTRACE("Ppc2", p_acs, p_acs->dev.pri_que, 0 );
		ASSERT (0);
	}

	/* issue the command in the pri_blk */
	fddi_issue_cmd (p_acs, p_cmd, bus);

	/* ok */
	FDDI_DBTRACE("PpcE", p_acs, p_acs->dev.state, pri_cmd_to_issue );
	return ;
}

/*
 * NAME: fddi_cmd_handler
 *                                                                    
 * FUNCTION: command completion handler
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment (SLIH)
 *                                                                   
 * NOTES: 
 *
 *	Called by: fddi_slih(), 	SLIH int handler
 *
 *	This is the generic command completion handler. It calls a
 *	command specific handler (if specified) in the 'command
 *	in progress'.
 *
 *	The command specific routine will determine what command will
 *	run next. If the specific routine wants to run another command
 *	it will set up the command structure in the ACS and that command
 *	will run at the next available chance. If no other commands are
 *	needed then the specific completion routine will set the 'cmd'
 *	to NULL.
 *
 * RECOVERY OPERATION: 
 *
 *	Specific command failures are managed by the command specific
 *	handlers - except, in the case of an invalid setcount. This
 *	error is handled generically since it can happen independent of
 *	the command type and and since it can be corrected independent
 *	of the command type.

 *	To correct an invalid setcount error one, a ULS command is issued
 *	out of the priority command block (maintaining the values in the
 *	common command block). Second, when the ULS command completes it's
 *	completion handler (fddi_uls_handler()) is called  and the setcount in
 *	the common command block is updated. Finally, the command in the
 *	common command block will be rerun with the new setcount.
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: none
 */  

void
fddi_cmd_handler (
	fddi_acs_t	*p_acs,
	uint		bus) 
{
	fddi_cmd_t	*p_cmd;
	extern int 	fddi_hcr_cmd_cmplt();

	FDDI_DBTRACE("ochB", p_acs, p_acs->dev.state, p_acs->dev.p_cmd_prog);

	/* turn off the watchdog timer */
	w_stop (&(p_acs->dev.cmd_wdt));

	/* initialize and set no command in progress */
	p_cmd = p_acs->dev.p_cmd_prog;
	p_acs->dev.p_cmd_prog = NULL;

	/* get status from shared memory
	 * and place in low order 2 bytes of the stat field
	 */
	if (p_cmd->stat == 0)
		PIO_GETSRX (bus + FDDI_CPB_RES, ((uchar *)&p_cmd->stat+2));

	/* sanity check for command specific completion function */
	ASSERT (p_cmd->cmplt)

	/*
	 * Handle some generic HCR errors
	 */
	if (p_cmd->stat == FDDI_HCR_INV_SETCOUNT)
	{
		FDDI_DBTRACE("och1", p_acs, p_cmd->cpb[FDDI_CPB_ERR_IDX], 0);
		/*
		 * Probably one already set...but this
		 *	covers a small window.
		 */
		p_acs->dev.pri_que |= FDDI_PRI_CMD_ULS;
	}
	else
	{
		/* 
		 * Call command specific completion function:
		 *	Completion routines will fill in the cmd structure
		 *	with the next command to go out if one is needed.
		 *	Completion routines report status and make state
		 *	changes depending on the status of the command.
		 *	Also, completion routines initiate LIMBO and bugout.
		 */
		if (p_cmd->cmplt == fddi_hcr_cmd_cmplt)
			p_acs->dev.hcr_cmd.hsr_val = 
					(short) (p_acs->dev.oflv_copy & 0xffff);
		(void) (*p_cmd->cmplt)(p_acs, p_cmd, bus);
	}

	FDDI_DBTRACE("och2", p_acs, p_acs->dev.pri_que, 0);
	if (p_acs->dev.limbo_to == TRUE)
	{
		FDDI_TRACE("och2", p_acs, p_acs->dev.state,p_acs->dev.limbo_to);
		p_acs->dev.oflv_copy = 0;
		return;
	}

	/* issue next command */
	if (p_acs->dev.pri_que)
	{
		/*
		 * Issue a priority command from the priority queue
		 */
		issue_pri_cmd (p_acs, p_acs->dev.pri_que, bus);
	}
	else if (p_acs->dev.cmd_blk.cmd_code != NULL)
	{
		FDDI_DBTRACE("och3", p_acs, p_acs->dev.cmd_blk.cmd_code, 0);
		/*
		 * Issue a command (not a priority command)
		 *	(This may be a rerun of a command that failed)
		 */
		fddi_issue_cmd (p_acs, &(p_acs->dev.cmd_blk), bus);
	}

	/* ok */
	FDDI_DBTRACE("ochE", p_acs, p_acs->dev.state, 0);
	return ;
}

/*
 * NAME: fddi_cmd_to
 *                                                                    
 * FUNCTION: watchdog time expiration for HCR command
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment 
 *
 *	This Interrupt level is not the same as our SLIH
 *	so serialization with the SLIH is required.
 *                                                                   
 * NOTES: 
 *
 *	We just had a watchdog timer expire. Assume that we lost
 *	communication with our adapter. Purge all queues, rcv, tx,
 *	and command, and go to the LIMBO state.
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS:  none
 */  
void
fddi_cmd_to (
	struct	watchdog	*p_wdt)
{
	fddi_acs_t	*p_acs;
	int		ipri;
	int		bus;


	/* go to offlevel interrupt priority */
	ipri = i_disable ( INTCLASS2 );
	FDDI_DBTRACE("tctB", p_wdt, 0, 0);

	/* 
	 * Get ACS ptr from wdt ptr
	 */
	p_acs = (fddi_acs_t *) ((uint) p_wdt - 
		((uint) offsetof (fddi_acs_t, dev) + 
		(uint) offsetof (fddi_acs_dev_t, cmd_wdt)));


	p_acs->dev.oflv_events |= FDDI_CMD_WDT_IO;
	if (p_acs->dev.oflv_running == FALSE)
	{
		/*
		 * schedule offlevel process and set running flag.
		 *	oflv_running is set to FALSE after the last
		 *	check of the hsr i fddi_oflv();
		 */
		i_sched (&p_acs->dev.ihs);
		p_acs->dev.oflv_running = TRUE;
	}
	/* return to priority at this functions' invocation */
	i_enable (ipri);

	/* trace this event */
	FDDI_DBTRACE("tctE", p_acs, p_acs->dev.state, 0);

	/* ok */
	return ;
}

/*
 * NAME: fddi_ls_bti_handler()
 *                                                                    
 * FUNCTION: 
 *	This routine handles the Link Statistics BTIs.  This routine
 *	determines the exact BTI and takes appropriate action.
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment
 *                                                                   
 * NOTES: 
 *	This routine is called in response to the LS_BTI_HANDLER() macro
 *	indicating that there is a BTI condition to handle.  This routine
 *	will either enter limbo (via fddi_enter_limbo()) or will initiate
 *	the shutdown of the device (via fddi_bug_out()).
 *	
 * RECOVERY OPERATION: 
 *	If a recoverable BTI is detected, the recovery operation initiated
 *	is through limbo (fddi_enter_limbo()).
 *
 * DATA STRUCTURES: 
 *	This routine works with the link statistics structure that
 *	is in the ACS (p_acs->ras.ls) which is of type fddi_links_t.
 *
 * RETURNS:  none
 */  
int
fddi_ls_bti_handler(
	fddi_acs_t 	*p_acs,
	fddi_links_t	*p_ls)
{
	int		rc;
	ulong 		rc1,ac;
	ulong		cond=CIO_NET_RCVRY_ENTER;
	cio_stat_blk_t	sb;

	FDDI_DBTRACE("olbB", p_acs, p_ls->smt_event_lo, p_ls->smt_error_lo);
	/* 
	 * The BTIs that are in the Link Statistics are:
	 *
	 *	SMT Event Word (low):
	 *		LSC	- LLC service change 
	 *			  (LLC disable indicated)  (LEC)
	 *		PTF	- Path Test Field (failure indicated) (LEC)
	 *		TRC	- T-Req changed remotely (LEC)
	 *		STR	- Self test required (LEC)
	 *		MD	- MAC Disconnected (fatal)
	 *		RDF	- Remote Disconnect Flag (fatal)
	 *
	 */

	cond = CIO_OK;


	ac = (ulong)( (p_ls->smt_event_hi << 16) || 
			p_ls->smt_event_lo);
	
	sb.code = CIO_ASYNC_STATUS;
	sb.option[2] = ac;
	sb.option[3] = (ulong)( (p_ls->smt_error_hi << 16) || 
			p_ls->smt_error_lo);

	/* determine the exact BTI.  Take appropriate
	 * action for the BTI.
	 */
	
	if ( p_ls->smt_event_lo & FDDI_SMT_EVNT_LO_RDF )
	{
		fddi_logerr(p_acs, ERRID_FDDI_RMV_ADAP,
			__LINE__, __FILE__);
		rc1 = FDDI_REMOTE_DISCONNECT;
		cond = CIO_HARD_FAIL;
	}
	else if ( p_ls->smt_event_lo & FDDI_SMT_EVNT_LO_MD )
	{
		fddi_logerr(p_acs, ERRID_FDDI_RMV_ADAP,
			__LINE__, __FILE__);
		rc1 = FDDI_MAC_DISCONNECT;
		cond = CIO_HARD_FAIL;
	}
	else if ( p_ls->smt_event_lo & FDDI_SMT_EVNT_LO_STR )
	{
		fddi_logerr(p_acs, ERRID_FDDI_SELF_TEST,
			__LINE__, __FILE__);
		rc1 = FDDI_REMOTE_SELF_TEST;
		cond = CIO_NET_RCVRY_MODE;
	}
	else if ( (p_ls->smt_event_lo & FDDI_SMT_EVNT_LO_PT_MSK)
		== (FDDI_SMT_EVNT_LO_PTF) )
	{
		fddi_logerr(p_acs, ERRID_FDDI_PATH_ERR,
			__LINE__, __FILE__);
		rc1 = FDDI_PATH_TEST;
		cond = CIO_NET_RCVRY_MODE;
	}




	if (cond == CIO_HARD_FAIL) 
		fddi_bugout(p_acs, rc1, 0, ac);
	else
		fddi_enter_limbo(p_acs, rc1, ac);


	FDDI_DBTRACE("olbE", p_acs, p_ls->smt_event_lo, p_ls->smt_error_lo);
	return(0);
} /* end fddi_ls_bti_handler() */

/*
 * NAME:     fddi_slih
 *
 * FUNCTION: fddi SLIH 
 *
 * EXECUTION ENVIRONMENT: interrupt thread
 *
 * NOTES:
 *
 *	Multiple bits in the HSR (Host Status Register) can be set 
 *	per interrupt indicating multiple events that need to be handled.
 *
 * RECOVERY OPERATION:
 *
 *	When we read the HSR and get an exception we must go into 
 *	LIMBO. At this point we have no idea what interrupts are pending
 *	and hence no idea what state the adapter is in. The only recovery
 *	is to go back to a know state. This will happen by calling
 *	the enter LIMBO routine.
 *
 * DATA STRUCTURES:
 *
 *	Bit(s) are set indicating the type of interrupt(s) and then the
 *	interrupt line is pulled causing an interrupt on the host. 
 *
 * RETURNS:   
 *		INTR_SUCC 	- we handled this interrupt
 *		INTR_FAIL	- we did not handle this interrupt
 *				  it was not our interrupt
 */

int
fddi_slih (
	fddi_acs_t	*p_acs)
{
	int		ioa;
	ushort		hsr;
	uchar		tmp_pos;
	int 		rc;
	int		iocc;

	FDDI_PTRACE("sliB");
	FDDI_DBTRACE("SlhB", p_acs, p_acs->dev.state, p_acs->dev.oflv_events);

	if (p_acs->dev.limbo_to == TRUE)
	{
		FDDI_DBTRACE("Slh1", p_acs->dev.state, p_acs->dev.limbo_to,0);
		return (INTR_SUCC);
	}

	iocc = IOCC_ATT(p_acs->dds.bus_id, IO_IOCC + (p_acs->dds.slot << 16));
	PIO_GETCX( iocc + FDDI_POS_REG2, &tmp_pos);
	IOCC_DET(iocc);

	if (!(tmp_pos & FDDI_POS2_CEN))
	{
		/* 
		 * Not our interrupt
		 */
		return (INTR_FAIL);
	}

	/*
	 * Get HSR register value from adapter. BUS_GETSRX sets up
	 *	an exception handler and returns nonzero if an exception
	 *	occurred.
	 */
	ioa = BUSIO_ATT(p_acs->dds.bus_id, p_acs->dds.bus_io_addr);
	rc = BUS_GETSRX (ioa + FDDI_HSR_ADDR, &hsr);
	BUSIO_DET(ioa);

	if (rc)
	{
		FDDI_TRACE("Slh1", p_acs, p_acs->dev.state, rc);
		FDDI_TRACE("SlhC", ioa, p_acs->dds.bus_id, 
			p_acs->dds.bus_io_addr);
		/* 
		 * exception: we do not know the state of our adapter
		 *	Clear all bits except the EXCEPTION bit.
		 *	The offlevel will put the state machine into
		 *	LIMBO.
		 */
		p_acs->dev.oflv_events = FDDI_EXCEPT_IO;
		fddi_logerr(p_acs, ERRID_FDDI_PIO, __LINE__, __FILE__);
		fddi_run_stest(p_acs, 0, 0);
	}
	else if (hsr == 0) 
	{
		FDDI_DBTRACE("Slh2", p_acs, p_acs->dev.state, 0);
		/* 
		 * Not our interrupt
		 */
		return (INTR_FAIL);
	}
	else
	{
		FDDI_DBTRACE("Slh3", p_acs, p_acs->dev.state, hsr);
		/*
		 * OR into master copy the new interrupts
		 */
		p_acs->dev.oflv_events |= hsr;
	}

	if (p_acs->dev.oflv_running == FALSE)
	{
		/*
		 * schedule offlevel process and set running flag.
		 *	oflv_running is set to FALSE after the last
		 *	check of the hsr in fddi_oflv();
		 */
		i_sched (&p_acs->dev.ihs);
		p_acs->dev.oflv_running = TRUE;
	}
	/* 
	 * acknowledge interrupt to card and to the system 
	 */
	i_reset (p_acs);

	FDDI_DBTRACE("SlhE", p_acs, p_acs->dev.state, p_acs->dev.oflv_events);
	FDDI_PTRACE("sliE");
	return (INTR_SUCC);
}

/*
 * NAME:     fddi_oflv
 *
 * FUNCTION: fddi offlevel processing 
 *
 * EXECUTION ENVIRONMENT: interrupt thread INTOFFL1
 *
 * NOTES:
 *
 *	
 * RECOVERY OPERATION:
 *
 *	Error recovery is taken when one of the following 
 *	errors are indicated in the HSR:
 *
 * 		NSF - No CD SFDBK (MC error)
 *		CCR - Channel Check on read (MC error)
 *		CCW - Channel Check on write (MC error)
 *		DPR - Data parity on read (MC error)
 *		ACI - Adapter Check interrupt
 *		AWS - Adapter Warm Start
 *	
 *
 * DATA STRUCTURES:
 *	
 *	Coordination between the SLIH and this routine is achieved with
 *	two data objects: 
 *	p_acs->dev.oflv_events  - master copy of the pending interrupts
 *	p_acs->dev.oflv_running - indicates the whether this routine
 *				is running or not
 *	(In this context if the offlevel is scheduled it is the same as if
 *	it were running.)
 *
 *	Both these objects are maintained with exclusive access by disabling
 *	interrupts. These are the only two data objects where exclusive access
 *	is needed between the SLIH and the offlevel threads.
 *
 *	For each type of interrupt there is a function to perform.  There
 *	are two types of interrupts: errors and non-errors.  Errors must
 *	be handled immeditately.  
 *	Non-errors can be prioritized as to which interrupt is serviced first.   *	The non-error interrupt	priorities are as follows (most favored 
 *	to least):
 *	
 *		Receive
 *		Transmit
 *		Command completion
 *		Ring status 
 *
 * RETURNS:    0 - successful (always returns zero)
 */

/*
 * If we get this many spurious interrupts in one execution thread
 *	then we kill our card...
 */

int
fddi_oflv (
	struct	ihs	*p_ihs)
{
	fddi_acs_t	*p_acs;
	int		rc;
	int		ipri;
	int		bus;
	int		ioa;
	int		iocc;
	int		test_num;
	int		spurious = 0;	/* spurious interrupts/thread */
	extern int	dnld_handler();

	/* get the acs ptr from offlevel ihs ptr */
	p_acs = (fddi_acs_t *) ((uint) p_ihs - offsetof (fddi_acs_t, dev));

	FDDI_PTRACE("oflB");

	FDDI_DBTRACE("offB", p_acs, p_acs->dev.oflv_events, p_acs->dev.state);

	/*
	 * Attach to bus and pass value to all the 
	 *	handlers that need bus access
	 */
	bus = BUSMEM_ATT(p_acs->dds.bus_id, (uint) p_acs->dds.bus_mem_addr);
	ioa = BUSIO_ATT (p_acs->dds.bus_id, p_acs->dds.bus_io_addr);

	while (TRUE)
	{
		/*
		 * Get a snapshot of the oflv_events from the master copy, 
		 *	zero out master copy 
		 *	if oflv_events is zero then this thread is completed
		 *	so reset the oflv_running indicator.
		 */
		ipri = i_disable (INTCLASS2);		
		p_acs->dev.oflv_copy = p_acs->dev.oflv_events;
		p_acs->dev.oflv_events = 0;	
		if (p_acs->dev.oflv_copy == 0)
		{
			/* 
			 * Finished processing this thread
			 * 	N.B. THIS IS THE ONLY BREAK FROM THIS
			 *	'while (TRUE)' loop
			 */
			p_acs->dev.oflv_running = FALSE;
			i_enable (ipri);
			break;
		}
		i_enable (ipri);

		/*
		 * Any non HSR errors
		 */
		if (p_acs->dev.oflv_copy & FDDI_EXCEPT_IO) 
		{
			fddi_enter_limbo(p_acs, FDDI_PIO_FAIL, 
				p_acs->dev.oflv_copy);
		}
		if (p_acs->dev.oflv_copy & FDDI_DEAD_IO) 
		{
			/*
			 * Fatal error, bug out and clear the
			 * oflv_events var to stop from processing
			 * any more work.
			 */
			fddi_bugout(p_acs, FDDI_PIO_FAIL, 0, 0);
		}
		/* 
		 * error HSR processing 
		 */
		if ( p_acs->dev.oflv_copy & FDDI_HSR_ERRORS )
		{
			FDDI_DBTRACE("off1", 
					p_acs, 
					p_acs->dev.oflv_copy, 
					p_acs->dev.state);
			/* 
			 * Call error handler:
			 */
			hsr_bti_handler (p_acs, bus, ioa);
		}
 		/* 
		 * normal HSR processing 
		 */
 		if ( p_acs->dev.oflv_copy & FDDI_HSR_RIR ) 
 		{
 			/* 
			 * frame rcv'd 
			 */
			p_acs->ras.ds.rcv_intr_cnt++;
 			rcv_handler (p_acs,bus);
 		} 
 		if ( p_acs->dev.oflv_copy & FDDI_HSR_RCI ) 
 		{
 			/* 
			 * rcv command complete:
			 * 	send another rcv command to the adapter 
			 */
			p_acs->ras.ds.rcv_cmd_cmplt++;
			FDDI_TRACE("off3", p_acs->dev.oflv_copy, p_acs, 
				p_acs->ras.ds.rcv_cmd_cmplt);
			PIO_PUTSRX(ioa+FDDI_NS1_REG, FDDI_NS1_RCV);
 		}
 		if (p_acs->dev.oflv_copy & FDDI_HSR_TIA)
 		{
 			/* 
			 * transmit frame complete 
			 */
			p_acs->ras.ds.tx_intr_cnt++;
 			tx_handler (p_acs, bus);
 		} 
 		if (p_acs->dev.oflv_copy & FDDI_CMD_WDT_IO)
 		{
			if (p_acs->dev.p_cmd_prog)
			{
				/*
		 	 	 * Kill command
		 		 */
				/* trace this event */
				FDDI_DBTRACE("oct1", 
						p_acs, 
						p_acs->dev.state, 
					p_acs->dev.p_cmd_prog->cmd_code);

				p_acs->dev.p_cmd_prog->stat = FDDI_CIO_TIMEOUT;

				fddi_cmd_handler (p_acs, bus);
			}
			else
			{
				/* 
		 		* No command to timeout on
		 		*/
				/* trace this event */
				FDDI_DBTRACE("oct2", 
						p_acs, 
						p_acs->dev.state, 
						0);
			}
 		} 
 		if ( p_acs->dev.oflv_copy & FDDI_HSR_CCI )
		{
			/* command complete */
			FDDI_DBTRACE("off2", 
					p_acs->dev.p_cmd_prog,
					p_acs->dev.oflv_copy, 
					p_acs->dev.state);
			if (p_acs->dev.p_cmd_prog)
			{
				/* process command completion */
				fddi_cmd_handler (p_acs,bus);
			}
			else
			{
				/* spurious interrupt: no command in progress */
				FDDI_DBTRACE("off3", 
					p_acs->dev.p_cmd_prog,
					p_acs->dev.oflv_copy, 
					p_acs->dev.state);
				spurious++;
			}
		} 
		/* Ring Status Interrupt */
		if ( p_acs->dev.oflv_copy & FDDI_HSR_RSI )
		{
			/* 
			 * Process RSI by requesting a ULS command.
			 *	All we do know is issue the ULS command.
			 *	The RSI events in the link statistics will
			 *	be handled by the ULS completion routine.
			 */
			p_acs->dev.pri_que |= FDDI_PRI_CMD_ULS;

			/* Check to see if a command is in progress */
			if (p_acs->dev.p_cmd_prog == NULL)
			{
				/* 
				 * no command in progress: 
				 * 	issue uls command 
				 */
				issue_pri_cmd (p_acs, FDDI_PRI_CMD_ULS, bus);
			}
		} 
		/* Download/diagnostic complete */
		if ( p_acs->dev.oflv_copy & FDDI_HSR_DDC ) 
		{
			FDDI_DBTRACE("off4", 
					p_acs->dev.p_cmd_prog,
					p_acs->dev.oflv_copy, 
					p_acs->dev.state);
			/* this can only happen when diagnostic/downloading */
			if (p_acs->dev.state == FDDI_DWNLDING)
			{
				/* handle the diag/download completion */
				dnld_handler (p_acs, bus);
			}
			else if ((p_acs->dev.state == FDDI_DWNLD) && 
				((p_acs->ctl.mode == 'D') ||
					(p_acs->ctl.mode == 'C')))
			{
				/*
				 * Doing Diagnostic/Dma transfers:
				 *	wakeup ioctl...
				 */
				p_acs->dev.ioctl_status = CIO_OK;
				e_wakeup(&p_acs->dev.ioctl_event);
			}
			else 
			{
				/*
				 * Trace spurious interrupt
				 */
				spurious++;
			}
		}
		/* Download/diagnostic ABORT */
		if ( p_acs->dev.oflv_copy & FDDI_HSR_DDA )
		{
			FDDI_TRACE("off5", 
					p_acs->dev.p_cmd_prog,
					p_acs->dev.oflv_copy, 
					p_acs->dev.state);
			if (p_acs->dev.state == FDDI_DWNLDING)
			{
				/* 
				 * Wakeup the process that initiated this 
				 *	DOWNLOAD. The state will reflect 
				 *	whether the DOWNLOAD worked or not.
				 */
				e_wakeup(&p_acs->dev.ioctl_event);
			}
			else 
			{
				/*
				 * Trace spurious interrupt
				 */
				spurious++;
			}
		}

 		if (p_acs->dev.oflv_copy & FDDI_LIMBO_WDT_IO)
 		{
			int 		i;
			int		error;
			ushort		hsr;
			ushort		badrc;
			fddi_cmd_t	*p_cmd;

			iocc = IOCC_ATT (p_acs->dds.bus_id, 
				(IO_IOCC + (p_acs->dds.slot<<16)));
			/*
	 		* set the pos regs after the reset then
	 		* re-map sharred ram by turning on the
	 		* Download/Diagnostic (DD) bit
	 		*/
			PIO_PUTCX( (iocc + FDDI_POS_REG6), p_acs->dev.pos6 );
			PIO_PUTCX( (iocc + FDDI_POS_REG7), p_acs->dev.pos7 );
			PIO_PUTCX( (iocc + FDDI_POS_REG5), p_acs->dev.pos5 );
			PIO_PUTCX( (iocc + FDDI_POS_REG4), p_acs->dev.pos4 );
			PIO_PUTCX( (iocc + FDDI_POS_REG3), p_acs->dev.pos3 );
			PIO_PUTCX( (iocc+FDDI_POS_REG2), 
				(p_acs->dev.pos2 | FDDI_POS2_CEN) );
			IOCC_DET(iocc);

			p_acs->dev.limbo_to = FALSE;
			/*
	 		* Read all the individual Self Test results.
	 		* Each test result is 2 bytes long.
	 		*/
			for ( i=0; i<FDDI_STEST_CNT;++i)
			{
				PIO_GETSRX(bus+(i<<1),&(p_acs->dev.stestrc[i]));
				FDDI_TRACE("Nlt1", i, 
					(ulong)p_acs->dev.stestrc[i], 0);
			}

			error=FALSE;
			for (i=0;i<8;++i)
			{
				if (p_acs->dev.stestrc[i] !=0)
				{
					error=TRUE;
					badrc = p_acs->dev.stestrc[i];
					test_num = i;
				}
			}
			if ( (p_acs->dev.stestrc[8] != 0x0000) &&
				(p_acs->dev.stestrc[8] != 0x80ea) )
			{
				error=TRUE;
				badrc = p_acs->dev.stestrc[8];
				test_num = i;
			}
	
			if ( (p_acs->dev.stestrc[9] != 0x0000) &&
				(p_acs->dev.stestrc[9] != 0x9001) )
			{
				error=TRUE;
				badrc = p_acs->dev.stestrc[9];
				test_num = i;
			}

			if ((p_acs->ctl.card_type == FDDI_SC) && 
 				(p_acs->dev.stestrc[10] != 0x0000) &&
				(p_acs->dev.stestrc[10] != 0xA0EA) )
			{
				error=TRUE;
				badrc = p_acs->dev.stestrc[10];
				test_num = i;
			}

			if ((p_acs->dev.stestrc[10] == 0x0000) &&
				(p_acs->dev.attach_class == FDDI_ACLASS_SAS))
			{
				error = TRUE;
				badrc = p_acs->dev.stestrc[10];
				test_num = i;
			}

			if ((p_acs->dev.stestrc[10] == 0xA0EA) &&
				(p_acs->dev.attach_class == FDDI_ACLASS_DAS))
			{
				error = TRUE;
				badrc = p_acs->dev.stestrc[10];
				test_num = i;
			}
		
			if (error)
			{
				fddi_logerr (p_acs, ERRID_FDDI_SELFT_ERR,
					__LINE__, __FILE__);
				p_acs->dev.stest_cover = TRUE;
				fddi_bugout(p_acs, FDDI_SELF_TEST, 
					(ulong)badrc, 
					test_num);
				break;
			} /* self tests failed */

			p_acs->dev.stest_cover = FALSE;
			/*
	 		* Read in the hsr to clear it of the misc. interrupts 
			* the card generates when first enabled.
	 		*/
			PIO_GETSRX(ioa, &hsr);
			/*
	 		* Set the interrupt mask to the Base interrupts.
	 		*/

			PIO_PUTSRX (ioa+FDDI_HMR_REG, FDDI_HSR_BASE_INTS);	

			p_cmd = &(p_acs->dev.cmd_blk);
			p_cmd->pri =0;
			p_cmd->cmplt = (int(*)()) fddi_limbo_cmplt;
			p_cmd->cmd_code = FDDI_HCR_START_MCODE;
			(void)fddi_issue_cmd(p_acs, p_cmd, bus);
 		} 

 		if (p_acs->dev.oflv_copy & FDDI_TX_WDT_IO)
 		{
			/* 
	 		* go into LIMBO: start limbo timer which will try to
	 		*	reconnect upon expiration
	 		*/
			fddi_logerr(p_acs,ERRID_FDDI_TX_ERR,__LINE__,__FILE__);
			fddi_enter_limbo (p_acs, FDDI_TX_ERROR, 0);
 		} 

 		if (p_acs->dev.oflv_copy & FDDI_DNLD_WDT_IO)
 		{
			if ((p_acs->dev.cmd_blk.cmd_code == 0) &&
				(p_acs->dev.state == FDDI_DWNLDING))
			{
				p_acs->dev.ioctl_status = CIO_TIMEOUT;
				p_acs->dev.state = FDDI_NULL;
			}

			PIO_PUTSRX (ioa + FDDI_HMR_REG, 0x807f);

			iocc = IOCC_ATT(p_acs->dds.bus_id, IO_IOCC + 
				(p_acs->dds.slot << 16));
			PIO_PUTCX( iocc + FDDI_POS_REG2, 
				(p_acs->dev.pos2 | FDDI_POS2_CEN) & 
					~FDDI_POS2_DD);
			IOCC_DET(iocc);

			e_wakeup(&p_acs->dev.ioctl_event);
 		} 

 		if (p_acs->dev.oflv_copy & FDDI_CLOSE_WDT_IO)
 		{
			e_wakeup (&(p_acs->dev.close_event));
 		} 

		if (spurious > FDDI_SPURIOUS_THRESHOLD)
		{
			FDDI_DBTRACE("off6", 
					p_acs,
					p_acs->dev.oflv_copy, 
					p_acs->dev.state);
			/*
			 * Enter Network recovery mode
			 */
			fddi_enter_limbo(p_acs, FDDI_CMD_FAIL, 0);
		}
			
	} /* end while (TRUE) */


	BUSIO_DET (ioa);
	BUSMEM_DET(bus);

	FDDI_DBTRACE("offE", p_acs, p_acs->dev.oflv_copy, p_acs->dev.state);
	FDDI_PTRACE("oflE");
	return (0);

} /* end fddi_oflv() */

/*
 * NAME: fddi_enter_diag_cmplt
 *                                                                    
 * FUNCTION: completes and interprets the results of a set addr command
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment (during DDC interrupt)
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: none
 */  


int
fddi_enter_diag_cmplt(
	fddi_acs_t 	*p_acs, 
	fddi_cmd_t	*p_cmd,
	int		bus)
{

	FDDI_DBTRACE("IedA", p_acs, p_acs->dev.state, p_cmd->stat);

	e_wakeup(&p_acs->dev.ioctl_event);
	p_cmd->cmd_code = NULL;

	/* err path return */
	return;
}
	


