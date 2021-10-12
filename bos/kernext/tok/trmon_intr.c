static char sccsid[] = "@(#)18	1.12  src/bos/kernext/tok/trmon_intr.c, sysxtok, bos411, 9428A410j 5/26/94 18:18:22";
/*
 *   COMPONENT_NAME: SYSXTOK
 *
 *   FUNCTIONS: adap_chk
 *		cmd_pro
 *		ioctl_to
 *		mc_err
 *		ring_stat_pro
 *		tok_cdt_func
 *		tokslih
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

#include "tokpro.h"

extern dd_ctrl_t   mon_dd_ctrl;

/*--------------------------  T O K S L I H  ---------------------------*/
/*
 *  NAME: tokslih
 *
 *  FUNCTION:
 *      Token Ring device driver Second Level Interrupt Handler.
 *
 *  EXECUTION ENVIRONMENT:
 *      Runs strictly at interrupt level.
 *
 *  RETURNS:
 *      INTR_SUCC       The interrupt was accepted and processed.
 *      INTR_FAIL       The interrupt was not accepted.
 *
 *  EXTERNAL PROCEDURES CALLED: d_complete, d_master,
 *                              xmattach, xmdetach, d_kmove,
 *                              enter_limbo, logerr, bug_out
 */
/*----------------------------------------------------------------------*/

int
tokslih ( struct intr *ihsptr )
{
    register dds_t  *p_dds = (dds_t *)ihsptr;
    intr_elem_t     iwe;            /* interrupt work element */
    ushort          reason;         /* interrupt reason */
    unsigned int    ioa;
    int             rc;
    int		    slihpri;
    
    TRACE_DBG(MON_OTHER, "intB", (int)p_dds, 0, 0);

    TRACE_DBG(MON_OTHER, "SL_L", (int)p_dds, 0, 0);
    slihpri = disable_lock(PL_IMP, &SLIH_LOCK);

    /* 
     *  Get type of int from status reg
     */
    ioa = BUSIO_ATT(DDI.bus_id, DDI.io_port);
    TOK_GETSRX( ioa + STATUS_REG, &reason);
	/*
	{
	    BUSIO_DET(ioa);
	    unlock_enable(slihpri, &SLIH_LOCK);
	    TRACE_DBG(MON_OTHER, "SL_U", (int)p_dds, 0, 0);
	    return(INTR_FAIL);
	} */
    if (!(reason & TOK_INT_SYSTEM)) /* is this ours? */
    {
	TRACE_BOTH(MON_OTHER, "iNOT", (int)p_dds, reason, 0);
	BUSIO_DET(ioa);
	unlock_enable(slihpri, &SLIH_LOCK);
	TRACE_DBG(MON_OTHER, "SL_U", (int)p_dds, 0, 0);
	return(INTR_FAIL);
    }
    
    /*
     *  Process the micro-channel interrupts
     */
    if (reason & MC_ERR) {
	rc = mc_err(p_dds, ioa, reason);
	BUSIO_DET(ioa);
	unlock_enable(slihpri, &SLIH_LOCK);
	TRACE_DBG(MON_OTHER, "SL_U", (int)p_dds, 0, 0);
	return(INTR_SUCC);
    }
    
    /*
     *  If we are not in a dead, null, or downloading microcode
     *  state, read the SSB into command portion of the intterupt
     *  work element.
     */
    if ((WRK.adap_state != DEAD_STATE) &&
	(WRK.adap_state != CLOSED_STATE))
    {
	switch (reason & TOK_IC)        /* dispatch on interrupt code */
	{
	case TRANSMIT_STATUS:
	    if (WRK.do_dkmove) {
		d_kmove (&(WRK.tx_iwe.cmd),
			  WRK.p_d_ssb,
			  sizeof(t_ssb),
			  WRK.dma_chnl_id,
			  DDI.bus_id,
			  DMA_READ);
	    } else {
		WRK.tx_iwe.cmd   = WRK.p_ssb->c_type;
		WRK.tx_iwe.stat0 = WRK.p_ssb->stat0;
		WRK.tx_iwe.stat1 = WRK.p_ssb->stat1;
		WRK.tx_iwe.stat2 = WRK.p_ssb->stat2;
	    }
	    
	    /*
	     *  Call transmit done handler
	     */
	    tx_done(p_dds, &(WRK.tx_iwe) );
	    break;
	    
	case RECEIVE_STATUS:
	    if (WRK.do_dkmove) {
		d_kmove (&(WRK.recv_iwe.cmd),
			  WRK.p_d_ssb,
			  sizeof(t_ssb),
			  WRK.dma_chnl_id,
			  DDI.bus_id,
			  DMA_READ);
	    } else {
		WRK.recv_iwe.cmd   = WRK.p_ssb->c_type;
		WRK.recv_iwe.stat0 = WRK.p_ssb->stat0;
		WRK.recv_iwe.stat1 = WRK.p_ssb->stat1;
		WRK.recv_iwe.stat2 = WRK.p_ssb->stat2;
	    }
	    
	    /*
	     *  Call receive handler
	     */
	    tok_receive(p_dds, &(WRK.recv_iwe));
	    break;
	    
	case ADAPTER_CHK:           /* ADAPTER CHECK */
	    TOK_PUTSRX( ioa + ADDRESS_REG, ADAPTER_CHK_ADDR);
	    TOK_GETSRX( ioa + AUTOINCR_REG, &WRK.ac_blk.code);
	    TOK_GETSRX( ioa + AUTOINCR_REG, &WRK.ac_blk.parm0);
	    TOK_GETSRX( ioa + AUTOINCR_REG, &WRK.ac_blk.parm1);
	    TOK_GETSRX( ioa + AUTOINCR_REG, &WRK.ac_blk.parm2);
	    adap_chk(p_dds, &iwe);
	    TRACE_BOTH(MON_OTHER, "ADCK", (int)p_dds, reason, 0);
	    break;
	    
	default:                    /* other valid interrupt */
	    if (WRK.do_dkmove) {
		d_kmove (&iwe.cmd,
			  WRK.p_d_ssb,
			  sizeof(t_ssb),
			  WRK.dma_chnl_id,
			  DDI.bus_id,
			  DMA_READ);
	    } else {
		iwe.cmd   = WRK.p_ssb->c_type;
		iwe.stat0 = WRK.p_ssb->stat0;
		iwe.stat1 = WRK.p_ssb->stat1;
		iwe.stat2 = WRK.p_ssb->stat2;
	    }
	    
	    switch (reason & TOK_IC) {
	    case RING_STATUS:
		ring_stat_pro(p_dds, &iwe);
		break;
	    case CMD_STATUS:
		cmd_pro(p_dds, &iwe);
		break;
	    }
	    TRACE_DBG(MON_OTHER, "DEFL", (int)p_dds, reason, 0);
	    break;
	} /* end switch (reason) */
	
    }

    /*
     * acknowledge the interrupt
     */
    TOK_PUTSRX(ioa + COMMAND_REG, ACK_INT);
    
    /*
     *  reset interrupt hardware
     */
    BUSIO_DET(ioa);
    TRACE_DBG(MON_OTHER, "intE", (int)p_dds, reason, 0);

    unlock_enable(slihpri, &SLIH_LOCK);
    TRACE_DBG(MON_OTHER, "SL_U", (int)p_dds, 0, 0);
    return(INTR_SUCC);

} /* end function tokslih */

int
mc_err(p_dds, bus, reason)
    register dds_t  *p_dds;
    unsigned int    bus;
    ushort          reason;         /* interrupt reason */
{
    /*
     * NOTE:
     * Since the adapter has detected a Micro Channel
     * error, we must clear the error via d_complete().
     * We can only do this if the ACA is available to
     * do a d_complete() on.  Doing the d_complete()
     * will clear the error on our DMA channel, thus
     * allowing DMA to continue on our DMA channel.
     */
    TRACE_BOTH(MON_OTHER, "FooT", SLIH_0, reason, 0);
    
    if ( (WRK.adap_state != DEAD_STATE) &&
	(WRK.adap_state != NULL_STATE) )
    {
	/*
	 *  We're not in the null or dead state, clear
	 *  the error by d_completing and re-d_mastering the ACA.
	 */
	WRK.mcerr = d_complete(WRK.dma_chnl_id,
			       DMA_READ, WRK.p_mem_block, PAGESIZE,
			       &WRK.mem_block_xmd,
			       WRK.p_d_mem_block);
	
	d_master(WRK.dma_chnl_id, DMA_READ|DMA_NOHIDE,
		 WRK.p_mem_block, PAGESIZE,
		 &WRK.mem_block_xmd, WRK.p_d_mem_block);
	
	TRACE_BOTH(MON_OTHER, "FooT", SLIH_2, WRK.mcerr, 0);
	logerr( p_dds, ERRID_CTOK_DEVICE_ERR, __LINE__, __FILE__);

	/*
	 *  Enter limbo
	 */
	if ( WRK.limbo == PARADISE )
	    enter_limbo( p_dds, NDD_BUS_ERROR, ERRID_CTOK_DEVICE_ERR, reason,
			FALSE );
	else if ( !(WRK.limbo == NO_OP_STATE) )
	    cycle_limbo(p_dds, NDD_BUS_ERROR, ERRID_CTOK_DEVICE_ERR);
    }
    else
    {   /* acknowledge the interrupt */
	TRACE_BOTH(MON_OTHER, "FooT", SLIH_1, 0, 0);
	TOK_PUTSRX(bus + COMMAND_REG, ACK_INT);
    }   /* end if IOCC buffer failed succeeded */
    return(0);
} /* End of mc_err() */

/*****************************************************************************/
/*
 * NAME:     tok_cdt_func
 *
 * FUNCTION: process component dump table interrupt
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  pointer to this driver's component dump table
 *
 */
/*****************************************************************************/
trmon_cdt_t *tok_cdt_func (void)
{
    return (&mon_dd_ctrl.cdt);
}

/*
 * NAME: ring_stat_pro
 * 	Ring Status Processing 
 *
 * FUNCTION:
 *
 *      Process Ring Status adapter Interrupts.
 *
 * EXECUTION ENVIRONMENT:
 *
 *       This routine executes on the interrupt.
 *
 * NOTES:
 *
 *      This function determines if the ring status interrupt will require
 *      the device driver to go into limbo mode.  If so, the enter_limbo() OR
 *      cycle_limbo() function will be called to initiate the Network Recovery
 *      scheme.
 *
 *      If not, the function determines if an adapter counter overflowed and if
 *      one has, the Read adapter Error log command is issued to the adapter
 *      via the issue_scb_command() primitive.
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *       software error recovery.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS:
 *      None.
 */
void
ring_stat_pro( dds_t *p_dds,
	  intr_elem_t *p_iwe)

{
    ndd_statblk_t sb;
    int rc, reason, txpri;
    unsigned long errid;
    
    TRACE_DBG(MON_OTHER, "RING", p_iwe->stat0, 0, 0);
    /*
     *   Determine if we need to log
     *   the Alert.
     *
     *   NOTE:
     *       We will not log the error if we
     *       are cycling limbo.
     */
    
    WRK.ring_status = p_iwe->stat0;	/* save the ring status */

    /*
     * nothing to do if close is in progress
     */
    if (WRK.adap_state == CLOSE_PENDING) {
	return;
    }

    if (WRK.ring_status & RS_SINGLE_STATION) {
	TOKSTATS.singles++;
    }
    if (WRK.ring_status & RS_TX_BEACON) {
	TOKSTATS.tx_beacons++;
    }
    
    errid = FALSE;
    if ( p_iwe->stat0 & RS_SIGNAL_LOSS ) {
	TOKSTATS.signal_loss++;
	errid = ERRID_CTOK_WIRE_FAULT;
    } else {
	if ( p_iwe->stat0 & RS_LOBE_WIRE_FAULT ) {
	    TOKSTATS.lobewires++;
	    errid = ERRID_CTOK_WIRE_FAULT;
	} else {
	    if ( p_iwe->stat0 & RS_AUTO_REMOVE_1 ) {
		errid = ERRID_CTOK_AUTO_RMV;
	    } else {
		if ( p_iwe->stat0 & RS_REMOVE_RCVD ) {
		    TOKSTATS.removes++;
		    errid = ERRID_CTOK_RMV_ADAP;
		} else {
		    if ( p_iwe->stat0 & RS_HARD_ERROR ) {
			TOKSTATS.hard_errs++;
		    } else {
			if ( p_iwe->stat0 & RS_SOFT_ERROR ) {
			    TOKSTATS.soft_errs++;
			}
		    }
		}
	    }
	}
    }
    
    if (p_iwe->stat0 & RS_ENTER_LIMBO) {
	/*
	 *   Get the reason for entering limbo
	 */
	switch (p_iwe->stat0 & RS_ENTER_LIMBO)
	{
	case RS_LOBE_WIRE_FAULT:
	    reason=  TOK_WIRE_FAULT;
	    break;
		
	case RS_AUTO_REMOVE_1:
	    reason = NDD_AUTO_RMV;
	    break;
		
	case RS_REMOVE_RCVD:
	    reason = TOK_RMV_ADAP;
		
	}  /* end switch */
	    
	/*
	 *   We have an enter limbo condition.
	 *   Check the state of limbo.
	 */
	if ( (WRK.limbo != PARADISE) &&
	    (WRK.limbo != NO_OP_STATE) )
	{   /*
	     *   Double Jeopardy!
	     *   We have a valid limbo entry condition, BUT
	     *   we have not yet exited limbo.
	     *   Call cycle limbo to continue w/ recovery scheme
	     */
	    TRACE_BOTH(MON_OTHER, "FooT", INTR_RSP_0, p_iwe->stat0, 0);
	    cycle_limbo(p_dds, reason, errid);
	} else if (WRK.limbo == PARADISE) {
	    WRK.limbo_iwe = *p_iwe;
	    WRK.footprint = INTR_RSP_2;
	    if (errid != FALSE) {
		logerr(p_dds, errid, __LINE__, __FILE__);
	    }
	    
	    TRACE_BOTH(MON_OTHER, "FooT", INTR_RSP_2, p_iwe->stat0, 0);
	    enter_limbo(p_dds, reason, errid, p_iwe->stat0, FALSE);
	    
	} /* end else enter limbo */
	
    } else {
	/*
	 *  we have some error to log
	 */
	if (errid != FALSE) {
	    logerr(p_dds, errid, __LINE__, __FILE__);
	} /* end if we have an error to log */
	
	
	if (p_iwe->stat0 & RS_COUNTER_OVERFLW) {
	    /*  Read the Adapter Error Log */
	    TRACE_BOTH(MON_OTHER, "FooT", INTR_RSP_1, p_iwe->stat0, 0);

	    TRACE_DBG(MON_OTHER, "TX_L", (int)p_dds, 0, 0);
	    txpri = disable_lock(PL_IMP, &TX_LOCK);

	    issue_scb_command (p_dds, ADAP_ELOG_CMD, WRK.p_d_errlog);

	    TOKSTATS.line_errs += WRK.adap_err_log.line_err_count;
	    TOKSTATS.burst_errs += WRK.adap_err_log.burst_err_count;
	    TOKSTATS.abort_errs += WRK.adap_err_log.abort_del_err_count;
	    TOKSTATS.int_errs += WRK.adap_err_log.internal_err_count;
	    TOKSTATS.lostframes += WRK.adap_err_log.lost_frame_err_count;
	    TOKSTATS.rx_congestion += WRK.adap_err_log.rec_cong_err_count;
	    NDD.ndd_genstats.ndd_ierrors +=
		WRK.adap_err_log.rec_cong_err_count;
	    TOKSTATS.framecopies += WRK.adap_err_log.frame_cpy_err_count;
	    TOKSTATS.token_errs += WRK.adap_err_log.token_err_count;
	    DEVSTATS.ARI_FCI_errors += WRK.adap_err_log.ari_fci_err_count;
	    DEVSTATS.DMA_bus_errors += WRK.adap_err_log.dma_bus_err_count;
	    DEVSTATS.DMA_parity_errors +=
		WRK.adap_err_log.dma_parity_err_count;

	    unlock_enable(txpri, &TX_LOCK);
	    TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);

	} /* end if counter overflow */
	
	/* Determine if we need to build an Async. Status block 
	 */
	
	if (((p_iwe->stat0 & 0xd820) ==0)  &&
	    (WRK.bcon_sb_sent == TRUE))
	{
	    TOKSTATS.recoverys++;

	    bzero (&sb, sizeof(sb));
	    sb.code = NDD_STATUS;
	    sb.option[0] = TOK_BEACONING;
	    sb.option[1] = p_iwe->stat0;
	    TRACE_BOTH(MON_OTHER, "stat", (int)p_dds, sb.code, sb.option[0]);
	    TRACE_BOTH(MON_OTHER, "sta2", NDD.ndd_flags, 0, 0);
	    NDD.nd_status( &NDD, &sb );
	    
	    /* set the bcon_sb_sent flag to FALSE.
	     * this will allow us to send a status block
	     * the next time we get a ring beaconing condition 
	     */
	    
	    WRK.bcon_sb_sent = FALSE;
	    
	} else if ((p_iwe->stat0 & RS_HARD_ERROR) &&
		   !(WRK.bcon_sb_sent))
	{
	    /* Build the TOK_RING_BEACONING status block
	     * send status block to all attached users 
	     */

	    bzero (&sb, sizeof(sb));
	    sb.code = NDD_STATUS;
	    sb.option[0] = TOK_BEACONING;
	    sb.option[1] = p_iwe->stat0;
	    TRACE_BOTH(MON_OTHER, "stat", (int)p_dds, sb.code, sb.option[0]);
	    TRACE_BOTH(MON_OTHER, "sta2", NDD.ndd_flags, 0, 0);
	    NDD.nd_status( &NDD, &sb );
	    
	    /* set the bcon_sb_sent flag to TRUE so we will
	     * not send another status block until we get the
	     * Ring Recovered status from the adapter. 
	     */
	    
	    WRK.bcon_sb_sent = TRUE;
	    
	}
	
    }  /* end if NOT limbo entry condition */
    
} /* end function ring_stat_pro */

/*--------------------------------------------------------------------*/
/*                                                                    */
/*********************          COMMAND PRO          ******************/
/*                                                                    */
/*--------------------------------------------------------------------*/
void
cmd_pro(
    dds_t	*p_dds,
    intr_elem_t *p_iwe)
{
    int 		rc, txpri;
    volatile t_scb	scb;
    volatile t_ssb	ssb;
    
    TRACE_DBG(MON_OTHER, "CMDP", (int)p_dds, p_iwe->cmd, 0);
    switch (p_iwe->cmd)          /* Type of command being acknowledged */
    {
    case ADAP_OPEN_CMD:                 /* OPEN command (0x0003) */
	/*  Check OPEN status */
	open_adap_pend(p_dds, p_iwe);
	break;
	
    case ADAP_CLOSE_CMD:                 /* CLOSE command (0x0007) */
	/* As A Result Of Detach */
	/* From Manager */
	w_stop(&BUWDT);
	e_wakeup(&(WRK.close_event));
	break;
	
    case ADAP_GROUP_CMD:                 /* Set Group Address */
	/*
	 *   in response to a Set Goup Addr ioctl need to
	 *   cancel the Group address failsafe timer.
	 *   Wakeup the sleeping process.
	 */
	w_stop(&BUWDT);

	TRACE_DBG(MON_OTHER, "TX_L", (int)p_dds, 0, 0);
	txpri = disable_lock(PL_IMP, &TX_LOCK);

	WRK.event_wait &= ~GROUP_WAIT;
	if (WRK.group_event != EVENT_NULL) {
	    e_wakeup( &WRK.group_event );
	}

	unlock_enable( txpri, &TX_LOCK );
	TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);
	break;
	
    case ADAP_FUNCT_CMD:                 /* Set Functional Address */
	/*
	 *   In response to Set Func. Addr ioctl need to
	 *   cancel the Functional address failsafe timer.
	 *   Wakeup the sleeping process
	 */
	w_stop(&BUWDT);

	TRACE_DBG(MON_OTHER, "TX_L", (int)p_dds, 0, 0);
	txpri = disable_lock(PL_IMP, &TX_LOCK);

	WRK.event_wait &= ~FUNCT_WAIT;
	if (WRK.funct_event != EVENT_NULL) {
	    e_wakeup( &WRK.funct_event );
	}

	unlock_enable( txpri, &TX_LOCK );
	TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);
	break;
	
    case ADAP_ELOG_CMD:          /* Read Error Log command */
	/*
	 * the error counters are read because of a control operation or
	 * because an error counter is about to overflow.  If the command
	 * was issued because of the error counter overflow, the watchdog
	 * timer is not set, and it may be running to time "probation" so
	 * we don't want to stop it.
	 */
	if (WRK.bu_wdt_cmd != INTR_PROBATION) {
		w_stop(&BUWDT);
	}
	/*
	 *  move/copy the adapter error log counters to the
	 *  WRK section of the DDS
	 */
	if (WRK.do_dkmove) {
	    d_kmove (&(WRK.adap_err_log),
		      WRK.p_d_errlog,
		      sizeof(tok_adap_error_log_t),
		      WRK.dma_chnl_id,
		      DDI.bus_id,
		      DMA_READ);
	} else {
	    bcopy (WRK.p_errlog,
		   &(WRK.adap_err_log),
		   sizeof(tok_adap_error_log_t));
	}
	
	TRACE_DBG(MON_OTHER, "TX_L", (int)p_dds, 0, 0);
	txpri = disable_lock(PL_IMP, &TX_LOCK);

	WRK.event_wait &= ~ELOG_WAIT;
	if (WRK.elog_event != EVENT_NULL) {
	    e_wakeup( &WRK.elog_event );
	}

	unlock_enable( txpri, &TX_LOCK );
	TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);
	break;
	
    case ADAP_READ_CMD:          /* Read Adapter command */
	w_stop(&BUWDT);

#ifdef TOK_ADAP_ADDR
	/*
	 * if debug code is being generated to read the addresses from the
	 * adapter, check what the read adapter command read
	 */
	if (WRK.event_wait & READ_ADAP_ADDR) {
		/*
		 *  move/copy the adapter addresses to the
		 *  WRK section of the DDS
		 */
		if (WRK.do_dkmove) {
		    d_kmove (&(WRK.adap_addr),
			      WRK.p_d_adap_addr,
			      sizeof(tok_adap_addr_t),
			      WRK.dma_chnl_id,
			      DDI.bus_id,
			      DMA_READ);
		} else {
		    bcopy (WRK.p_adap_addr,
			   &(WRK.adap_addr),
			   sizeof(tok_adap_addr_t));
		}
	} else {
#endif

	/*
	 *  move/copy the adapter parameters to the
	 *  WRK section of the DDS
	 */
	if (WRK.do_dkmove) {
	    d_kmove (&(WRK.ring_info),
		      WRK.p_d_ring_info,
		      sizeof(tok_ring_info_t),
		      WRK.dma_chnl_id,
		      DDI.bus_id,
		      DMA_READ);
	} else {
	    bcopy (WRK.p_ring_info,
		   &(WRK.ring_info),
		   sizeof(tok_ring_info_t));
	}

#ifdef TOK_ADAP_ADDR
	}
#endif
	
	TRACE_DBG(MON_OTHER, "TX_L", (int)p_dds, 0, 0);
	txpri = disable_lock(PL_IMP, &TX_LOCK);

	WRK.event_wait &= ~READ_ADAP_WAIT;
	if (WRK.read_adap_event != EVENT_NULL) {
	    e_wakeup( &WRK.read_adap_event );
	}

	unlock_enable( txpri, &TX_LOCK );
	TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);
	break;
	
    case REJECT_CMD:                 /* REJECT command */
	/*
	 *  The CRS_ENTER_LIMBO defines for what Command Reject
	 *  statuses do we enter limbo mode on.  They are:
	 *      - Illegal Command
	 *      - Address Error
	 *      - Adapter Closed
	 */
	
	TRACE_BOTH(MON_OTHER, "FooT", INTR_CMD_0, p_iwe->stat0, p_iwe->stat1);
	WRK.footprint = INTR_CMD_0;
	WRK.afoot = (unsigned short) p_iwe->cmd;
	TRACE_BOTH(MON_OTHER, "REJT", REJECT_CMD, p_iwe->cmd, 0);
	
	if (WRK.do_dkmove) {
	    d_kmove (&scb, WRK.p_d_scb, sizeof(scb),
		      WRK.dma_chnl_id, DDI.bus_id, DMA_READ);
	} else {
	    bcopy (WRK.p_scb, &scb, sizeof(scb));
	}

	if (WRK.do_dkmove) {
	    d_kmove (&ssb, WRK.p_d_ssb, sizeof(ssb),
		      WRK.dma_chnl_id, DDI.bus_id, DMA_READ);
	} else {
	    bcopy (WRK.p_ssb, &ssb, sizeof(ssb));
	}
	
	TRACE_BOTH(MON_OTHER, "SCB1", scb.adap_cmd, scb.addr_field1,
		scb.addr_field2);
	TRACE_BOTH(MON_OTHER, "SSB1", ssb.c_type, ssb.stat0, ssb.stat1);
	TRACE_BOTH(MON_OTHER, "SSB2", ssb.stat2, 0, 0);
	
	/*
	 *  We have a Limbo entry condition
	 */
	if (p_iwe->stat0 & CRS_ENTER_LIMBO) {
	    
	    if (WRK.limbo == PARADISE ) {
		/*
		 *   Enter limbo
		 */
		WRK.limbo_iwe = *p_iwe;
		logerr( p_dds, ERRID_CTOK_DEVICE_ERR, __LINE__, __FILE__);
		enter_limbo(p_dds, NDD_CMD_FAIL, ERRID_CTOK_DEVICE_ERR,
			p_iwe->stat0, FALSE);
	    } else if (WRK.limbo != NO_OP_STATE) {
		/*
		 *   Double Jeopardy!
		 *   We have a valid limbo entry condition
		 *   BUT we are not out of limbo yet.  Cycle the
		 *   recovery scheme.
		 */
		cycle_limbo(p_dds, NDD_CMD_FAIL, ERRID_CTOK_DEVICE_ERR);
	    }
	    
	}   /* end if go for limbo */
	
	break;                        /* End Command reject */
	
    default:                             /* Unexpected or illegal */
	TRACE_BOTH(MON_OTHER, "FooT", INTR_CMD_1, p_iwe->cmd,
		p_iwe->stat0);
	break;                              /* command acknowledgment */
	/* Do nothing and get out. */
    }  /* End Switch */

} /* end function cmd_pro */

/*--------------------------------------------------------------------*/
/*                                                                    */
/*********************          Adapter Check        ******************/
/*                                                                    */
/*--------------------------------------------------------------------*/
int
adap_chk(p_dds, p_iwe)     /* process adapter check */
    dds_t		*p_dds;
    intr_elem_t		*p_iwe;
{
    
    int rc=0;
    
    TRACE_BOTH(MON_OTHER, "FooT", INTR_ADAP_CHK, 0, 0);
    WRK.footprint = INTR_ADAP_CHK;
    /*
     *  We're off to Limboland!
     */
    if (WRK.limbo == PARADISE) {
	WRK.limbo_iwe = *p_iwe;
	logerr(p_dds, ERRID_CTOK_ADAP_CHECK, __LINE__, __FILE__);
	enter_limbo(p_dds, NDD_ADAP_CHECK, ERRID_CTOK_ADAP_CHECK,
		WRK.ac_blk.code, FALSE);
    }   /* end if enter limbo */
    else if (WRK.limbo != NO_OP_STATE) {
	cycle_limbo(p_dds, NDD_ADAP_CHECK, ERRID_CTOK_ADAP_CHECK);
    }
    
    return(rc);
} /* end function adapter check */
