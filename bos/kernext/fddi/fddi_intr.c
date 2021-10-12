static char sccsid[] = "@(#)70	1.9  src/bos/kernext/fddi/fddi_intr.c, sysxfddi, bos41J, 9520A_a 5/16/95 11:38:52";
/*
 *   COMPONENT_NAME: SYSXFDDI
 *
 *   FUNCTIONS: fddi_slih
 *		hcr_uls_cmplt
 *		hsr_error_handler
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
#include <sys/dma.h>

static int rci_count=0;
/* 
 * Declaration of FDDI control structure - controls all adapters on machine
 */
extern fddi_tbl_t fddi_tbl;


/*
 * NAME:     fddi_slih
 *
 * FUNCTION: interrupt handler for the driver
 *
 * EXECUTION ENVIRONMENT: interrupt thread
 *
 * NOTES:
 *
 *	Get the Host Status Register from the adapter and handle the events
 *	set on it.  Loop until the HSR is empty.
 *
 *
 * RETURNS:   
 *		INTR_SUCC 	- we handled this interrupt
 *		INTR_FAIL	- we did not handle this interrupt
 *				  it was not our interrupt
 */

int
fddi_slih (struct intr *ihs)
{
	fddi_acs_t	*p_acs;
	int		ioa;
	uchar		tmp_pos;
	int 		rc;
	int 		saved_rc;
	int		iocc;
	int		bus;
	int		ipri;

	p_acs = (fddi_acs_t *)((uint)ihs - (uint)offsetof(fddi_acs_t, ihs));

	if (p_acs->ndd.ndd_output == fddi_dump_output)
	{
		ushort hsr,hmr;
		ioa = BUSIO_ATT(p_acs->dds.bus_id, p_acs->dds.bus_io_addr);
		BUS_GETSRX(ioa+FDDI_HSR_ADDR, &hsr);
		BUS_GETSRX(ioa+FDDI_HMR_REG, &hmr);
		BUSIO_DET (ioa);
		FDDI_TRACE("Sfs-",p_acs,hsr, hmr);
		return(INTR_FAIL);
	}
	ipri = disable_lock(CFDDI_OPLEVEL, &p_acs->dev.slih_lock);

	FDDI_TRACE("SfsB",p_acs,0,0);

	saved_rc = INTR_FAIL;

	iocc = IOCC_ATT(p_acs->dds.bus_id, IO_IOCC + (p_acs->dds.slot << 16));
	PIO_GETPOS2( iocc + FDDI_POS_REG2, &tmp_pos);
	IOCC_DET(iocc);

	if (p_acs->dev.pio_rc)
	{
		FDDI_ETRACE("Sfs2",p_acs,p_acs->dev.pio_rc, 0);
		fddi_logerr(p_acs, ERRID_CFDDI_PIO, __LINE__, __FILE__,
				0, 0, 0);
		bugout(p_acs,NDD_PIO_FAIL,0, 0, TRUE);
		unlock_enable(ipri,&p_acs->dev.slih_lock);
		return(INTR_FAIL);
	}

	if (!(tmp_pos & FDDI_POS2_CEN))
	{
		/* Not our interrupt */
		FDDI_ETRACE("Sfs3",p_acs,tmp_pos,0);
		unlock_enable(ipri,&p_acs->dev.slih_lock);
		return (INTR_FAIL);
	}

	/*
	 * Attach to bus and pass value to all the 
	 *	handlers that need bus access
	 */
	ioa = BUSIO_ATT(p_acs->dds.bus_id, p_acs->dds.bus_io_addr);
	bus = BUSMEM_ATT(p_acs->dds.bus_id, (uint) p_acs->dds.bus_mem_addr);
	
	while (TRUE)
	{
		/*
	 	 * Get HSR register value from adapter. BUS_GETSRX sets up
	 	 *	an exception handler and returns nonzero if an exception
	 	 *	occurred.
	 	 */
		rc = BUS_GETSRX (ioa + FDDI_HSR_ADDR, &p_acs->dev.hsr_events);

		if (rc)
		{
			/* 
		 	 * exception: we do not know the state of our adapter
		 	 *	Clear all bits except the EXCEPTION bit.
		 	 *	The offlevel will put the state machine into
		 	 *	LIMBO.
		 	 */
			FDDI_ETRACE("Sfs4",p_acs,rc,0);
			fddi_logerr(p_acs, ERRID_CFDDI_PIO, __LINE__, __FILE__,
				p_acs->dev.hsr_events, rc, 0);
			enter_limbo (p_acs, NDD_PIO_FAIL, rc, TRUE);
			break;
		}
		
		if (p_acs->dev.hsr_events == 0)
			break;

		saved_rc = INTR_SUCC;

		/* 
		 * error HSR processing 
		 */
		if ( p_acs->dev.hsr_events & FDDI_HSR_ERRORS )
		{
			/* 
			 * Call hsr error handler:
			 */
			if (hsr_error_handler (p_acs, bus, ioa))
			{
				FDDI_ETRACE("Sfs5",p_acs,0,0);
				fddi_logerr(p_acs, ERRID_CFDDI_PIO, 
					__LINE__, __FILE__,
					0, 0, 0);
				bugout(p_acs,NDD_PIO_FAIL,0,
					p_acs->dev.hsr_events, TRUE);
				break;
			}
		}

 		/* 
		 * normal HSR processing 
		 */
 		if ( p_acs->dev.hsr_events & FDDI_HSR_RIR ) 
 		{
 			/* 
			 * frame rcv'd 
			 */
                        rci_count = 0;
 			rx_handler (p_acs,bus);
 		} 

 		if ( p_acs->dev.hsr_events & FDDI_HSR_RCI ) 
 		{
 			/* 
			 * rcv command complete:
			 * 	send another rcv command to the adapter 
			 */
                        rci_count++;
                        if (rci_count > 20000) {
                                fddi_logerr(p_acs,ERRID_CFDDI_RCV_ERR,
					__LINE__,__FILE__,0,0,0);
                                enter_limbo (p_acs, CFDDI_PORT_STUCK, 0, TRUE);
                                rci_count = 0;
                        }
			else {
				PIO_PUTSRX(ioa+FDDI_NS1_REG, FDDI_NS1_RX);
				if (p_acs->dev.pio_rc)
				{
					FDDI_ETRACE("Sfs6",p_acs,0,0);
					fddi_logerr(p_acs, ERRID_CFDDI_PIO, 
						__LINE__, __FILE__,
						0, 0, 0);
					bugout(p_acs,NDD_PIO_FAIL,0,
						p_acs->dev.hsr_events, TRUE);
					break;
				}
			}
 		}

 		if (p_acs->dev.hsr_events & FDDI_HSR_TIA)
 		{
 			/* 
			 * transmit frame complete 
			 */
 			tx_handler (p_acs, bus);
 		} 

 		if ( p_acs->dev.hsr_events & FDDI_HSR_CCI )
		{
			/* 
			 * Call the completion routine if we issued a command
			 */
			if (p_acs->dev.p_cmd_prog)
			{
				/* process command completion */
				cmd_handler (p_acs,bus);
			}
		} 

		/* Ring Status Interrupt */
		if ( p_acs->dev.hsr_events & FDDI_HSR_RSI )
		{
			/* 
			 * Process RSI by requesting a ULS command.
			 *	All we do now is issue the ULS command.
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
				issue_pri_cmd (p_acs, FDDI_PRI_CMD_ULS, bus, 
					FALSE);
			}
		} 

		/* Download/diagnostic complete */
		if ( p_acs->dev.hsr_events & FDDI_HSR_DDC ) 
		{
			/* this can only happen when diagnostic/downloading */
			if (p_acs->dev.state == DNLDING_STATE)
			{
				/* handle the diag/download completion */
				dnld_handler (p_acs, bus);
			}
			else
			{
				e_wakeup(&p_acs->dev.cmd_event);
			}
		}

		/* Download/diagnostic ABORT */
		if ( p_acs->dev.hsr_events & FDDI_HSR_DDA )
		{
			if (p_acs->dev.state == DNLDING_STATE)
			{
				p_acs->dev.cmd_status = NDD_CMD_FAIL;
				e_wakeup(&p_acs->dev.cmd_event);
			}
		}

		if (p_acs->dev.hsr_events == 0)
			break;

	} /* end while (TRUE) */


	BUSMEM_DET(bus);
	BUSIO_DET (ioa);

	FDDI_TRACE("SfsE",p_acs,saved_rc,INTR_SUCC);
	unlock_enable(ipri,&p_acs->dev.slih_lock);
	return (saved_rc);
}

/*
 * NAME: hcr_uls_cmplt
 *                                                                    
 * FUNCTION: handle all the meaningful events found in the link statistics
 *                                                                    
 * EXECUTION ENVIRONMENT: Interrupt environment.
 *
 *	This is the cmd completion routine of a completed 
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
 * RETURNS:  none
 */  	
 

void
hcr_uls_cmplt (
	fddi_acs_t	*p_acs,
	fddi_cmd_t	*p_cmd,
	int		bus,
	int		ipri)
{
	ndd_statblk_t	sb; 		/* status blk */
	fddi_spec_stats_t	*p_ls;		/* link statistics */
	ulong 		rc1,ac;

	FDDI_TRACE("SucB",p_acs,p_cmd->cmd_code,p_cmd->stat);

	/* 
	 * PIO the link statistics: swap the values (except setcount).
	 */
	FDDI_GET_LINK_STATS (p_acs->ls);

	if (p_acs->dev.pio_rc)
	{
		FDDI_ETRACE("Suc1",p_acs,p_cmd->cmd_code,p_cmd->stat);
		fddi_logerr(p_acs, ERRID_CFDDI_PIO, __LINE__, __FILE__,
				0, 0, 0);
		unlock_enable(ipri, &p_acs->dev.cmd_lock);
		bugout(p_acs, NDD_PIO_FAIL, 0, ac, TRUE);
		ipri = disable_lock(CFDDI_OPLEVEL, &p_acs->dev.cmd_lock);
		return;
	}

	p_ls = &p_acs->ls;

	ac = (ulong)( (p_ls->smt_event_hi << 16) || 
			p_ls->smt_event_lo);

	/* 
	 * Check the statistics
	 */
	
	if ( p_ls->smt_event_lo & FDDI_SMT_EVNT_LO_RDF )
	{
		FDDI_ETRACE("Suc2",p_acs,ac,0);
		fddi_logerr(p_acs, ERRID_CFDDI_RMV_ADAP, __LINE__, __FILE__,
			ac, 0, 0);

		unlock_enable(ipri, &p_acs->dev.cmd_lock);
		bugout(p_acs,CFDDI_REMOTE_DISCONNECT,0, ac, TRUE);
		ipri = disable_lock(CFDDI_OPLEVEL, &p_acs->dev.cmd_lock);
		return;
	}
	else if ( p_ls->smt_event_lo & FDDI_SMT_EVNT_LO_STR )
	{
		FDDI_ETRACE("Suc3",p_acs,ac,0);
		fddi_logerr(p_acs, ERRID_CFDDI_SELF_TEST, __LINE__, __FILE__,
			ac, 0, 0);
		unlock_enable(ipri, &p_acs->dev.cmd_lock);
		enter_limbo(p_acs, CFDDI_REMOTE_SELF_TEST, 0, ac, TRUE);
		ipri = disable_lock(CFDDI_OPLEVEL, &p_acs->dev.cmd_lock);
		return;
	}
	else if ( (p_ls->smt_event_lo & FDDI_SMT_EVNT_LO_PT_MSK)
		== (FDDI_SMT_EVNT_LO_PTF) )
	{
		FDDI_ETRACE("Suc4",p_acs,ac,0);
		fddi_logerr(p_acs, ERRID_CFDDI_PATH_ERR, __LINE__, __FILE__,
			ac, 0, 0);
		unlock_enable(ipri, &p_acs->dev.cmd_lock);
		enter_limbo(p_acs, CFDDI_PATH_TEST, 0, ac, TRUE);
		ipri = disable_lock(CFDDI_OPLEVEL, &p_acs->dev.cmd_lock);
		return;
	}

	sb.code = NDD_STATUS;
	sb.option[0] = CFDDI_SMT_EVENT;
	sb.option[2] = (p_ls->smt_event_hi << 16) | p_ls->smt_event_lo;

	if ((p_ls->smt_event_lo & FDDI_SMT_EVNT_LO_LSS) == 0x0)
	{
		/*
		 * send status blk indicating that
		 * LLC services have been disabled.
		 * only if the state is different from what we think it is
		 */
		if (!(p_acs->ndd.ndd_flags & CFDDI_NDD_LLC_DOWN))
		{
			p_acs->ndd.ndd_flags |= CFDDI_NDD_LLC_DOWN;
			sb.option[1] = CFDDI_LLC_DISABLE;
			p_acs->ndd.nd_status(p_acs, &sb);
		}
	}
	if ( p_ls->smt_event_lo & FDDI_SMT_EVNT_LO_LSS )
	{
		/* 
		 * send status blk indicating that 
		 * LLC services have been re-enabled
		 * only if it is different from what we think it is
		 */
		if (p_acs->ndd.ndd_flags & CFDDI_NDD_LLC_DOWN)
		{
			p_acs->ndd.ndd_flags &= ~(CFDDI_NDD_LLC_DOWN);
			sb.option[1] = CFDDI_LLC_ENABLE;
			p_acs->ndd.nd_status(p_acs, &sb);
		}
	}

	if ((p_ls->smt_event_lo & FDDI_SMT_EVNT_LO_ROS) != p_acs->dev.rop)
	{
		/* 
		 * Set the NDD running flag bassed on what the ring op is
		 * set to.
		 */
		
		if ( p_ls->smt_event_lo & FDDI_SMT_EVNT_LO_ROS )
		{
			sb.option[1] = CFDDI_ROP;
		}
		else 	
		{
			sb.option[1] = CFDDI_NO_ROP;
		}
			
		p_acs->dev.rop = p_ls->smt_event_lo & FDDI_SMT_EVNT_LO_ROS;
		p_acs->ndd.nd_status(p_acs, &sb);
	}

	sb.option[0] = CFDDI_SMT_ERROR;
	sb.option[2] = (p_ls->smt_error_hi << 16) | p_ls->smt_error_lo;

	if ( p_ls->smt_event_lo & FDDI_SMT_EVNT_LO_RTT )
	{
		if (p_acs->dev.thresh_rtt-- == 0)
		{
			/* 
		 	* send status blk indicating that 
		 	* a restricted token dialog has been terminated
		 	*/
			sb.option[1] = CFDDI_RTT;
			p_acs->ndd.nd_status(p_acs, &sb);
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
			sb.option[1] = CFDDI_REMOTE_T_REQ;
			p_acs->ndd.nd_status(p_acs, &sb);
			p_acs->dev.thresh_trc = FDDI_SMT_THRESH;
		}
	}

	if ((p_ls->smt_error_lo & FDDI_SMT_ERR_LO_STUCK ) != p_acs->dev.stuck)
	{
		/* 
		 * send status blk indicating that a port 
		 * is stuck.
		 */
		fddi_logerr(p_acs, ERRID_CFDDI_PORT, __LINE__, __FILE__,
			sb.option[2], 0, 0);
		sb.option[1] = CFDDI_PORT_STUCK;
		p_acs->dev.stuck = p_ls->smt_error_lo & FDDI_SMT_ERR_LO_STUCK;
		if (p_acs->dev.stuck != 0)
			p_acs->ndd.nd_status(p_acs, &sb);
	}

	if ( p_ls->smt_error_lo & FDDI_SMT_ERR_LO_SBF )
	{
		if (p_acs->dev.thresh_sbf-- == 0)
		{
			/* 
		 	* send status blk indicating that 
		 	* optical bypass switch is stuck
		 	*/
			fddi_logerr(p_acs,ERRID_CFDDI_BYPASS, __LINE__,__FILE__,
				sb.option[2], 0, 0);
			sb.option[1] = CFDDI_BYPASS_STUCK;
			p_acs->ndd.nd_status(p_acs, &sb);
			p_acs->dev.thresh_sbf = FDDI_SMT_THRESH;
		}
	}

	/*
	 * Set the new setcount into the user's command block.  If the ULS 
	 * was due to invalid setcount, the command is waiting in the command
	 * block, and will be re-issued automaticly by the cmd_handler when this
	 * completion routine completes provided no other priority commands have
	 * been requested.
	 */
	p_acs->dev.cmd_blk.cpb[0] = p_acs->ls.setcount_lo;
	p_acs->dev.cmd_blk.cpb[1] = p_acs->ls.setcount_hi;

	FDDI_TRACE("SucE",p_acs,0,0);
	return;
}

/*
 * NAME: hsr_error_handler
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
 * RETURNS: 
 *	Boolean set to whether or not there has been a pio error
 */  
int 
hsr_error_handler (
	fddi_acs_t	*p_acs,
	int		bus,
	int		ioa)
{
	ushort		hsr;
	uchar		tmp_pos;

	FDDI_TRACE("SehB",p_acs,p_acs->dev.hsr_events,p_acs->dev.pio_rc);
	/*
	 * Check for a MC error
	 *	NSF - No card select feedback return
	 *	CCR - Channel check on Read
	 *	CCW - Channel check on write
	 *	DPR - Data parity on read
	 *
	 * initiate recovery logic if one of those error occured.
	 */
	if (p_acs->dev.hsr_events & ( FDDI_HSR_NSF | FDDI_HSR_CCR | 
		FDDI_HSR_CCW | FDDI_HSR_DPR) )
	{
		/*
		 * The adapter got an MC error during a BUS master 
		 * operation.
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

		
		fddi_logerr(p_acs, ERRID_CFDDI_MC_ERR, __LINE__, __FILE__,
			p_acs->dev.hsr_events,0,0);
		d_master(p_acs->dev.dma_channel, DMA_WRITE_ONLY,
			p_acs->tx.p_sf_cache, (size_t)FDDI_SF_BUFSIZ,
			&p_acs->tx.xmd, (char *)p_acs->tx.p_d_sf);

		/*
		 * Tell the adapter to go!!!
		 */
		PIO_PUTSRX(ioa + FDDI_NS1_REG, (FDDI_NS1_LCA | FDDI_NS1_HCC));
	}
	if (p_acs->dev.hsr_events & FDDI_HSR_ACI)
	{
		/* 
		 * Grab the LS from the adapter.
		 * and put in the LS structure in the ACS.
		 */
		FDDI_ETRACE("Seh1",p_acs,p_acs->dev.hsr_events, 0);
		FDDI_GET_LINK_STATS (p_acs->ls);

		fddi_logerr(p_acs, ERRID_CFDDI_ADAP_CHECK, __LINE__, __FILE__,
			p_acs->dev.hsr_events,0,0);

		enter_limbo (p_acs, NDD_ADAP_CHECK, 
			(ulong)p_acs->ls.aci_code, TRUE);
		/*
		 * Don't do anymore processing 
		 */
	}
	if (p_acs->dev.hsr_events & FDDI_HSR_AWS)
	{
		/* 
		 * Grab the LS from the adapter.
		 * and put in the LS structure in the ACS.
		 */
		FDDI_ETRACE("Seh2",p_acs,p_acs->dev.hsr_events, 0);
		FDDI_GET_LINK_STATS (p_acs->ls);

		fddi_logerr(p_acs, ERRID_CFDDI_ADAP_CHECK, __LINE__, __FILE__,
			p_acs->dev.hsr_events, 0, 0);

		enter_limbo (p_acs, NDD_ADAP_CHECK, 
			(ulong)p_acs->ls.aci_code, TRUE);
	}

	FDDI_TRACE("SehE",p_acs,p_acs->dev.hsr_events,p_acs->dev.pio_rc);
	return (p_acs->dev.pio_rc);
} /* end hsr_error_handler() */


