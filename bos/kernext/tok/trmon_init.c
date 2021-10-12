static char sccsid[] = "@(#)19	1.15  src/bos/kernext/tok/trmon_init.c, sysxtok, bos41B, 9504A 1/17/95 11:01:25";
/*
 *   COMPONENT_NAME: SYSXTOK
 *
 *   FUNCTIONS: bringup_timer
 *		cfg_adap_parms
 *		close_adap
 *		ds_act
 *		init_adap
 *		open_adap
 *		open_adap_pend
 *		open_timeout
 *		reset_adap
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

/*
 *
 * FUNCTION NAME = ds_act
 *
 * FUNCTION =  Initiate the Activation the Token-Ring Adapter.
 *
 * NOTES:
 *
 *       This function will initiate the bringup of the Token-Ring adapter.
 *
 *       This function assumes that:
 *               1. The Adapter Control Area (ACA) has been successfully setup.
 *               2. The Receive Chain is fully populated with pinned and
 *                       d_master() PAGESIZE mbufs.
 *               3. The Transmit and Receive Chain index pointers are
 *                       setup to point to
 *                       the first List element in the respective Chains.
 *               4. The SLIH and OFF-LEVEL intr structures have been setup.
 *               5. The SLIH has been successfully registered.
 *
 * RETURN CODES:
 *      0   - Good return code.
 *
 */

void
ds_act(dds_t *p_dds)
{
    TRACE_DBG(MON_OTHER, "actB", (int)p_dds, 0, 0);
    
    /*
     *  Set the bringup state to Phase 0 of the reset adapter
     *  sequence. 
     */
    WRK.reset_spin = 0;
    WRK.adap_init_spin = 0;
    WRK.bringup = RESET_PHASE0;

    /*
     *  Start the reset process
     */
    reset_adap(p_dds);
    
    TRACE_DBG(MON_OTHER, "actE", (int)p_dds, 0, 0);
    return;
    
} /* end function ds_act */

/*
 *  FUNCTION:   bringup_timer()
 *
 *  INPUT:      p_wdt - pointer to watchdog structure in the DDS
 *
 */
void
bringup_timer(struct watchdog *p_wdt)
{
    dds_t	*p_dds = *(dds_t **) ((ulong)p_wdt + sizeof(struct watchdog));
    int		slihpri;
    
    TRACE_DBG(MON_OTHER, "SL_L", (int)p_dds, 0, 0);
    slihpri = disable_lock(PL_IMP, &SLIH_LOCK);
    
    /*
     *   Determine what phase of the adapter activation sequence or what
     *   control operation is being done.  Call the appropriate routine.
     */
    TRACE_BOTH(MON_OTHER, "oflB", (int)p_dds, WRK.bu_wdt_cmd, 0);
    switch(WRK.bu_wdt_cmd)
    {
    case INTR_CYCLE_LIMBO:
	ds_act(p_dds);
	break;
    case INTR_ADAP_RESET:
	reset_adap(p_dds);
	break;
    case INTR_ADAP_INIT:
	init_adap(p_dds);
	break;
    case INTR_ADAP_OPEN:
	open_timeout(p_dds);
	break;
    case INTR_ADAP_CLOSE:
	e_wakeup(&(WRK.close_event));
	WRK.footprint = ACT_INTR_BUP_0;
	break;
    case INTR_PROBATION:
	/*
	 *   Our probation timer has popped
	 *   We made it through the probationary period
	 *   successfully. Start up the exodus of limbo.
	 *
	 *   If wrk.limbo != PROBATION, probably got a
	 *   ring status interrupt == wire fault just
	 *   before the timer popped
	 */
	if (WRK.limbo == PROBATION) {
	    egress_limbo(p_dds);
	}
	else
	{
	    TRACE_BOTH(MON_OTHER, "Foot", (int)p_dds, (int)ACT_INTR_BUP_2, 0);
	}
	break;
    case INTR_FUNC_TIMEOUT:
	/*
	 *   The setting of the functional address
	 *   timed out.  Wakeup the ioctl routine.
	 */
	if (WRK.event_wait & FUNCT_WAIT) {
	    TRACE_BOTH(MON_OTHER, "FooT", ACT_INTR_BUP_3, 0, 0);
	    WRK.event_wait &= ~FUNCT_WAIT;
	    if (WRK.funct_event != EVENT_NULL) {
		e_wakeup( &(WRK.funct_event));
	    }
	}
	break;
    case INTR_GROUP_TIMEOUT:
	/*
	 *   The setting of the group address timed out.
	 *   Wakeup the ioctl routine.
	 */
	if (WRK.event_wait & GROUP_WAIT) {
	    TRACE_BOTH(MON_OTHER, "FooT", ACT_INTR_BUP_4, 0, 0);
	    WRK.event_wait &= ~GROUP_WAIT;
	    if (WRK.group_event != EVENT_NULL) {
		e_wakeup( &(WRK.group_event));
	    }
	}
	break;
    case INTR_ELOG_TIMEOUT:
	/*
	 *   The reading of the adapter error log timed out.
	 *   Wakeup the ioctl routine.
	 */
	if (WRK.event_wait & ELOG_WAIT) {
	    TRACE_BOTH(MON_OTHER, "FooT", ACT_INTR_BUP_5, 0, 0);
	    WRK.event_wait &= ~ELOG_WAIT;
	    if (WRK.elog_event != EVENT_NULL) {
		e_wakeup( &(WRK.elog_event));
	    }
	}
	break;
    case INTR_READ_ADAP_TIMEOUT:
	/*
	 *   The reading of the adapter ring info timed out.
	 *   Wakeup the ioctl routine.
	 */
	if (WRK.event_wait & READ_ADAP_WAIT) {
	    TRACE_BOTH(MON_OTHER, "FooT", ACT_INTR_BUP_6, 0, 0);
	    WRK.event_wait &= ~READ_ADAP_WAIT;
	    if (WRK.read_adap_event != EVENT_NULL) {
		e_wakeup( &(WRK.read_adap_event));
	    }
	}
	break;
    }   /* end switch */

    unlock_enable(slihpri, &SLIH_LOCK);
    TRACE_DBG(MON_OTHER, "SL_U", (int)p_dds, 0, 0);
    
    TRACE_BOTH(MON_OTHER, "oflE", (int)p_dds,0, 0);
    return;
    
}  /* end function bringup_timer */

/*--------------------------------------------------------------------*/
/***************        Reset Adapter                   ***************/
/*--------------------------------------------------------------------*/

/*
 *  
 *  This routine will issue a reset then an enable to the Token-Ring
 *  adapter.  After successful completion of this function, the adapter will
 *  be in a state awaiting the initialization parameters.
 *  
 *  RETURN CODES:
 *  0 - Successful completion
 *  ENETUNREACH - Reset of adapter failed.
 *  
 *  If an unrecoverable hardware error occured, the failure reason will be
 *  stored in WRK.afoot.
 *  
 *  ROUTINES CALLED:
 *  
 *  
 */

void
reset_adap(dds_t *p_dds)
{
    int			ioa, iocc, rc;
    ushort		reset_rc;
    uchar		pos4;
    
    TRACE_BOTH(MON_OTHER, "rset", (int)p_dds, WRK.adap_state, WRK.bringup);

    switch(WRK.bringup) {
    case RESET_PHASE0:
	/*
	 *  First time through just hit the reset register to start off
	 *  the reset procedure.
	 */
	hwreset( p_dds );

	if (WRK.pio_rc) {
		bug_out( p_dds, ERRID_CTOK_DEVICE_ERR, NDD_PIO_FAIL, 0 );
		return;
	}

	/*
	 *  Do statistics stuff
	 */
	WRK.dev_stime = lbolt;
	bzero(&TOKSTATS, sizeof(TOKSTATS) );
	bzero(&DEVSTATS, sizeof(DEVSTATS) );
	TOKSTATS.device_type = TOK_MON;
	COPY_NADR(WRK.adap_open_opts.node_addr, TOKSTATS.tok_nadr);

	/*
	 *  Start a system timer to wait 2 second and then return to
	 *  check the reset status.  This is done so that the rest of
	 *  the bringup will be run on the system interrupt thread
	 *  instead of the process thread.
	 */
	BUWDT.count = 0;
	BUWDT.restart = RESET_TIME;
	WRK.bu_wdt_cmd = INTR_ADAP_RESET;
	WRK.bringup = RESET_PHASE1;
	w_start(&BUWDT);
	break;
	
    case RESET_PHASE1:
	/*
	 *  Check the status of the reset.   This will be retried
	 *  RESET_SPIN_COUNT number of times if there was no status.  
	 */
	if (WRK.reset_spin < RESET_SPIN_COUNT) {
	    
            ioa = BUSIO_ATT( DDI.bus_id, DDI.io_port );
	    TOK_GETSRX( ioa + STATUS_REG, &reset_rc);
    	    TRACE_BOTH(MON_OTHER, "rs01", p_dds, reset_rc, ioa);
	    BUSIO_DET( ioa );
	    
	    /*
	     *  adapter reset worked!
	     */
	    if ((reset_rc & 0x00f0) == RESET_OK) {
		/*
		 *   Reenable adapter interrupts
		 *
		 */
		if (WRK.mask_int) {
		    ioa = BUSIO_ATT( DDI.bus_id, DDI.io_port );
		    TOK_PUTCX( ioa + IMASK_ENABLE, 0x00);
		    BUSIO_DET( ioa );
		    WRK.mask_int = FALSE;
		}
		
		/*
		 *   Get the current POS 4 Setting.
		 *   Turn On Parity.
		 */
        	iocc = (int)IOCC_ATT( (ulong)DDI.bus_id,
			(ulong)(IO_IOCC + (DDI.slot << 16)));
		TOK_GETPOS( iocc + POS_REG_4, &pos4 );
		pos4 = pos4 | MC_PARITY_ON;
		TOK_PUTPOS( iocc + POS_REG_4, pos4 );
		IOCC_DET( iocc );

		if (WRK.pio_rc) {
		    bug_out( p_dds, ERRID_CTOK_DEVICE_ERR, NDD_PIO_FAIL, 0 );
		    return;
		}

		/*
		 *   The next step in the adapter bringup is the
		 *   adapter initialization
		 *   Change WRK.bringup to ADAP_INIT_PHASE0 and
		 *   call init_adap() routine.
		 */
		WRK.bringup = ADAP_INIT_PHASE0;
		rc = init_adap(p_dds);
		break;
	    } 
	    
	    /*
	     *  Adapter Reset failed!
	     */
	    if ( (reset_rc & 0x00f0) == RESET_FAIL) {
		WRK.footprint = ACT_RESET_PHS2_0;
		WRK.afoot = reset_rc;
		/*
		 * If in Ring Recovery mode, kick off another retry cycle.
		 * Otherwise, this is in response to the 1st activation
		 * enter limbo
		 */
		TRACE_BOTH(MON_OTHER, "FooT", (int)p_dds, ACT_RESET_PHS2_0,
				(int)reset_rc);

		if (WRK.limbo != PARADISE) {
		    cycle_limbo(p_dds, TOK_ADAP_INIT, 0);
		    break;
		} else {
		    WRK.adap_state = OPEN_STATE;

		    enter_limbo(p_dds, TOK_ADAP_INIT, 0, reset_rc, FALSE);
		    break;
		}
	    }
	    
	    ++WRK.reset_spin;
	    
	    /*
	     *  Set the timer to delay and then come back and check the
	     *  status once again.
	     */
	    BUWDT.count = 0;
	    BUWDT.restart = RESET_TIME;
	    WRK.bu_wdt_cmd = INTR_ADAP_RESET;
	    WRK.bringup = RESET_PHASE1;
	    w_start(&BUWDT);
	    break;
	}
	
	/*
	 *  Never received status from the adapter within the retry period
	 */
	WRK.footprint = ACT_RESET_PHS2_2;
	WRK.afoot = reset_rc;
	TRACE_BOTH(MON_OTHER, "FooT", (int)p_dds, ACT_RESET_PHS2_2,
		(int)reset_rc);

	if (WRK.limbo != PARADISE) {
	    cycle_limbo(p_dds, TOK_ADAP_INIT, 0);
	} else {
	    WRK.adap_state = OPEN_STATE;

	    enter_limbo(p_dds, TOK_ADAP_INIT, 0, 0, FALSE);
	    break;
	}
    } /* End of switch */
    
}  /* end reset_adap() function */

/*--------------------------------------------------------------------*/
/***************        Initialize Adapter              ***************/
/*--------------------------------------------------------------------*/

/*
 *  This routine will give the Token-Ring adapter the initialization
 *  parameters. This routine assumes that the adapter has just been reset.
 * 
 *  After successful completion of this routine, the adapter will be in a state
 *  awaiting the open adapter command and the adapter open options.
 * 
 *  RETURN CODES:
 *  0 - Successful completion
 *  ENETUNREACH - Adapter initialization failed
 *  
 * 
 *  ROUTINES CALLED:
 *  - reset_adap()
 * 
 * 
 *  RETRY LOGIC:
 *  
 *  This function will retry the adapter initialization sequence 3 times.
 */

int
init_adap(dds_t *p_dds)
{
    
    int		i, j, error;
    int		ioa;
    ushort	*p_adap_i_parms, t1, init_rc;
    uint	rc=0;
    
    TRACE_DBG(MON_OTHER, "inaB", (int)p_dds, WRK.adap_state, WRK.bringup);

    switch(WRK.bringup) {
    case ADAP_INIT_PHASE0:
	/*
	 *  Write initialization parameters to adapter.  Try 3 times to get
	 *  them written correctly.
	 */
        ioa = BUSIO_ATT( DDI.bus_id, DDI.io_port );
	for (i=0;i<3;i++) {
	    
	    /*
	     * set up pointer to the adapter registers
	     */
	    p_adap_i_parms = (unsigned short *)
		&(WRK.adap_iparms.init_options);
	    
	    j=11;
	    TOK_PUTSRX( ioa + ADDRESS_REG, 0x0200 );
	    while (j--) {    /* Load adapter init. paramters */
		TOK_PUTSRX( ioa + AUTOINCR_REG, *p_adap_i_parms );
		++p_adap_i_parms;
	    }
	    
	    /*
	     * read back the init parameters to verify they got there ok.
	     * set up pointer to the adapter registers
	     */
	    p_adap_i_parms = (unsigned short *)
		&(WRK.adap_iparms.init_options);
	    j=11;
	    error = FALSE;
	    TOK_PUTSRX( ioa + ADDRESS_REG, 0x0200 );
	    while ( (j--) && !error) {
		TOK_GETSRX( ioa + AUTOINCR_REG, &t1);
		if (t1 !=  *p_adap_i_parms)
		{
		    /*
		     *  init. parms did not get there ok
		     */
		    error = TRUE;
		} else {
		    ++p_adap_i_parms;
		}
	    }
	    
	    /*
	     *  If the init parameters were written correctly.
	     */
	    if (!error) {
		/*
		 *  issue WRITE INTERRUPT Instruction (Execute) to adapter
		 */
		TOK_PUTSRX( ioa + COMMAND_REG, EXECUTE );
		break;
	    }
	}
	BUSIO_DET( ioa );

	if (WRK.pio_rc) {
	    bug_out( p_dds, ERRID_CTOK_DEVICE_ERR, NDD_PIO_FAIL, 0 );
	    return(EIO);
	}
	
	/*
	 *  Couldn't get the parameters written to the adapter correctly.
	 */
	if (i == 3) {
	    TRACE_BOTH(MON_OTHER, "FooT", (int)p_dds, ACT_IPHS0_0, (ulong)t1);
	    WRK.footprint = ACT_IPHS0_0;
	    WRK.afoot = t1;
	    
	    TRACE_BOTH(MON_OTHER, "FooT", (int)p_dds, ACT_IPHS0_1, 0);

	    /*
	     *  Enter (cycle) limbo to retry the activation of card
	     */
	    WRK.adap_state = OPEN_STATE;

	    if (WRK.limbo != PARADISE) {
		cycle_limbo(p_dds, TOK_ADAP_INIT, 0);
		break;
	    } else {
		enter_limbo(p_dds, TOK_ADAP_INIT, 0, 0, FALSE);
	    }
	    rc = ENETUNREACH;
	} else {
	    /*
	     *  Set the timer to delay and then come back and check the
	     *  status once again.
	     */
	    BUWDT.count = 0;
	    BUWDT.restart = INIT_TIME;
	    WRK.bu_wdt_cmd = INTR_ADAP_INIT;
	    WRK.bringup = ADAP_INIT_PHASE1;
	    w_start(&BUWDT);
	    rc = 0;
	}
	break;
	
    case ADAP_INIT_PHASE1:
	/*
	 *  Check the status of the init command.
	 */
	while (WRK.adap_init_spin < ADAP_INIT_SPIN_COUNT) {
	    /*
	     *  Read the status of the init parameters
	     */
            ioa = BUSIO_ATT( DDI.bus_id, DDI.io_port );
	    TOK_GETSRX( ioa + STATUS_REG, &init_rc);
	    BUSIO_DET( ioa );
	    
	    if ((init_rc & 0x0070) == INIT_ADAP_OK) {
		/*
		 *   Get the adapter's pointers
		 */
		get_adap_point(p_dds);

		/*
		 *   Set next adap state to OPEN_PHASE0
		 */
		WRK.bringup = OPEN_PHASE0;
		rc = open_adap(p_dds);
		break;
		
	    }
	    
	    if ( (init_rc & INIT_ADAP_FAIL) == INIT_ADAP_FAIL) {
		/*
		 *   Initialization of adapter failed
		 *   Check if in Ring Recovery mode, if so set adap state
		 *   to RESET_PHASE0 and start all over.
		 *
		 */
		
		WRK.footprint = ACT_IPHS1_0;
		WRK.afoot = init_rc;
		TRACE_BOTH(MON_OTHER, "FooT", (int)p_dds, ACT_IPHS1_0,
			init_rc);

		/*
		 *  Enter (cycle) limbo to retry the activation of card
		 */
		WRK.adap_state = OPEN_STATE;
		if (WRK.limbo != PARADISE) {
		    cycle_limbo(p_dds, TOK_ADAP_INIT, 0);
		    break;
		} else {
		    enter_limbo(p_dds, TOK_ADAP_INIT, 0, init_rc, FALSE);
		}
		rc = ENETUNREACH;
		break;
	    }

	    ++WRK.adap_init_spin;

	    /*
	     *  Set the timer to delay and then come back and check the
	     *  status once again.
	     */
	    BUWDT.count = 0;
	    BUWDT.restart = INIT_TIME;
	    WRK.bu_wdt_cmd = INTR_ADAP_INIT;
	    WRK.bringup = ADAP_INIT_PHASE1;
	    w_start(&BUWDT);
	    rc = 0;
	    break;
	    
	} /* End of while (adap_init_spin < ADAP_INIT_SPIN_COUNT) */
	
	/*
	 *  Never got adapter status within the retry period so adapter
	 *  must be dead.  
	 */
	if (WRK.adap_init_spin >= ADAP_INIT_SPIN_COUNT) {
	    TRACE_BOTH(MON_OTHER, "FooT", (int)p_dds, ACT_IPHS1_3, 0);
	    WRK.footprint = ACT_IPHS1_3;
	    WRK.afoot = init_rc;

	    /*
	     *  Enter (cycle) limbo to retry the activation of card
	     */
	    WRK.adap_state = OPEN_STATE;
	    if (WRK.limbo != PARADISE) {
		cycle_limbo(p_dds, TOK_ADAP_INIT, 0);
		break;
	    } else {
		enter_limbo(p_dds, TOK_ADAP_INIT, 0, init_rc, FALSE);
	    }
	    rc = ENETUNREACH;
	}
	
    } /* End of switch() */
	
    TRACE_DBG(MON_OTHER, "inaE", (int)p_dds, WRK.adap_state, WRK.bringup);
    return(rc);

}  /* end function init_adap() */

/*---------------------------------------------------------------------*/
/*                              OPEN ADAP                              */
/*---------------------------------------------------------------------*/

/*
 *
 *  FUNCTION:   open_adap()
 *      This routine will issue an Open command to the adapter.  This will
 *      cause the adapter to insert onto the Token-Ring.  The adapter open
 *      command usually  takes from 15 to 30 seconds to complete.
 * 
 *
 *  INPUT:  p_dds - pointer to DDS
 *
 */
int
open_adap(dds_t *p_dds)
{
    int rc;

    TRACE_DBG(MON_OTHER, "opnB", (int)p_dds, 0, 0);
    /*
     *   Move the adapter open options to the ACA
     */
    if (WRK.do_dkmove) {
        d_kmove (&(WRK.adap_open_opts),
		  WRK.p_d_open_opts,
		  sizeof(tok_adap_open_options_t),
		  WRK.dma_chnl_id,
		  DDI.bus_id,
		  DMA_WRITE_ONLY);
    } else {
	bcopy(&(WRK.adap_open_opts),
              WRK.p_open_opts,
              sizeof(tok_adap_open_options_t));
    }
    
    /*
     *  Issue the adapter open command to the adapter with the open
     *  options in the DDS.
     */
    issue_scb_command(p_dds, ADAP_OPEN_CMD, WRK.p_d_open_opts);

    BUWDT.count = 0;
    BUWDT.restart = OPEN_TIMEOUT;
    WRK.bu_wdt_cmd = INTR_ADAP_OPEN;
    w_start(&BUWDT);
    
    TRACE_DBG(MON_OTHER, "opnE", (int)p_dds, 0, 0);
    return(0);
} /* end open_adap() */

/*---------------------------------------------------------------------*/
/*                         OPEN ADAP PEND                              */
/*---------------------------------------------------------------------*/
/*
 *  FUNCTION: open_adap_pend()
 *
 *  INPUT:  p_dds - pointer to DDS
 *          p_iwe - pointer to INTR work element
 *
 *
 */

void
open_adap_pend(dds_t *p_dds, intr_elem_t *p_iwe)
{
    ndd_statblk_t   sb;
    int		    errid, limbo_reason;
    
    TRACE_DBG(MON_OTHER, "oapB", (int)p_dds, (ulong)p_iwe->stat0, WRK.limbo);
    /*
     *   Cancel the open timeout timer
     */
    w_stop(&BUWDT);

    /*
     *  Open command failed so send async status and enter limbo.
     */
    if (p_iwe->stat0 != 0x8000) {
	TRACE_DBG(MON_OTHER, "FooT", (int)p_dds, ACT_OA_PEND_1,
		(ulong)p_iwe->stat0);
	/*
	 * save the status 1/2 word that was returned
	 * by the adapter
	 */
	WRK.open_fail_code = p_iwe->stat0;
	WRK.afoot = p_iwe->stat0;
	WRK.footprint = ACT_OA_PEND_1;

	/*
	 * decode open status to determine reason for failure
	 */
	switch (p_iwe->stat0 & 0xFF) {
	case 0x11:	/* wire fault */
	    errid = ERRID_CTOK_WIRE_FAULT;
	    limbo_reason = TOK_WIRE_FAULT;
	    break;

	case 0x17:	/* ring beaconing (not defined, but I have seen it */
	case 0x27:  /* expected ring beaconing */
	    errid = ERRID_CTOK_CONFIG;
	    limbo_reason = TOK_RING_SPEED;
	    break;

	case 0x2A:	/* remove received */
	case 0x3A:
	case 0x4A:
	case 0x5A:
	    errid = NULL;	/* error is logged if recovery fails */
	    limbo_reason = TOK_RMV_ADAP;
	    break;

	case 0x38:	/* duplicate address, log error & go to dead state  */
	    bug_out(p_dds, ERRID_CTOK_DUP_ADDR, TOK_RECOVERY_THRESH,
			TOK_DUP_ADDR);
	    TRACE_DBG(MON_OTHER, "oapE", (int)p_dds, 0, 0);
	    return;

	default:
	    errid = ERRID_CTOK_ADAP_OPEN;
	    limbo_reason = TOK_ADAP_OPEN;
	}
	
	if (WRK.limbo != PARADISE) {
	    /*
	     *   Almost! But no cigar...back into Limbo
	     */
	    cycle_limbo(p_dds, limbo_reason, errid);
	} else {       /* Send Back good ack to caller */
	    WRK.adap_state = OPEN_STATE;

	    if (errid != NULL) {
		logerr(p_dds, errid, __LINE__, __FILE__);
	    }

	    enter_limbo(p_dds, limbo_reason, errid, p_iwe->stat0, FALSE);
	}
    } else {
	/*
	 * Check for set group/functional address command received during
	 * the last stages of opening the adapter, if so go thru limbo once
	 * to set the address.
	 */
	if (WRK.command_to_do) {
	    WRK.command_to_do = FALSE;
	    if (WRK.limbo != PARADISE) {
		cycle_limbo(p_dds, 0, 0);
	    } else {
		WRK.adap_state = OPEN_STATE;
		enter_limbo(p_dds, 0, 0, 0, FALSE);
	    }
	}

	if (WRK.limbo==CHAOS) {
	    /*
	     *   We're Open for business.
	     *   Start the exodus from limbo mode.
	     */
	    TRACE_DBG(MON_OTHER, "FooT", (int)p_dds, ACT_OA_PEND_0, 0);
	    egress_limbo(p_dds);
	} else {
	    /*
	     *  Set state machine
	     */
	    WRK.adap_state = OPEN_STATE;
	    NDD.ndd_flags |= NDD_RUNNING;
	
	    /*
	     *  build the Connected status block.
	     */

	    WRK.connect_sb_sent = TRUE;
	    bzero (&sb, sizeof(sb));
	    sb.code = NDD_CONNECTED;
	    TRACE_BOTH(MON_OTHER, "stat", (int)p_dds, sb.code, 0);
	    TRACE_BOTH(MON_OTHER, "sta2", NDD.ndd_flags, 0, 0);
	    NDD.nd_status( &NDD, &sb );
	    
	    /*
	     *  start adapter recv
	     */
	    TRACE_DBG(MON_OTHER, "strv", (int)p_dds, 0, 0);
	    WRK.recv_mode = TRUE;    /* change state */
	    issue_scb_command(p_dds, ADAP_RCV_CMD, WRK.recv_list[0]);
    
	}   /* end if not comming out of limbo mode */
    }
    
    TRACE_DBG(MON_OTHER, "oapE", (int)p_dds, 0, 0);

}  /* end function open_adap_pend() */

/*---------------------------------------------------------------------*/
/*                         OPEN TIMEOUT                                */
/*---------------------------------------------------------------------*/
/*
 *  FUNCTION:   open_timeout()
 *
 *  INPUT:      p_dds - pointer to DDS
 *
 */

void
open_timeout( dds_t *p_dds )
{
	ndd_statblk_t	sb;

	WRK.footprint = ACT_OA_TO_0;
	
	TRACE_DBG(MON_OTHER, "FooT", (int)p_dds, ACT_OA_TO_0, 0);
	if (WRK.limbo != PARADISE) {
	    /*
	     *   Not quite...try again.
	     */
	    cycle_limbo(p_dds, TOK_ADAP_OPEN, 0);
	} else {
	    /*
	     *  Log Permanent hardware error and return
	     */
	    logerr(p_dds, ERRID_CTOK_ADAP_OPEN, __LINE__, __FILE__);

	    /*
	     * change state from OPEN_PENDING to OPEN_STATE so that the
	     * enter_limbo code works right
	     */
	    WRK.adap_state = OPEN_STATE;
	    enter_limbo(p_dds, TOK_ADAP_OPEN, ERRID_CTOK_ADAP_OPEN, 0, FALSE);
	}
} /* end function open_timeout() */


/*--------------------------------------------------------------------*/
/***************        Close  Adapter                  ***************/
/*--------------------------------------------------------------------*/

void
close_adap(dds_t *p_dds)
{
    /*
     *   NOTE:
     *       The system close call CANNOT FAIL.
     *       - stop the adapter bringup timer
     *       - close the adapter
     *       - do a hardware reset to ensure that it is closed
     */
    
    w_stop(&BUWDT);
    
    issue_scb_command(p_dds, ADAP_CLOSE_CMD, NULL);
    
    BUWDT.count = 0;
    BUWDT.restart = CLOSE_TIMEOUT;
    WRK.bu_wdt_cmd = INTR_ADAP_CLOSE;
    WRK.close_event = EVENT_NULL;
    w_start(&BUWDT);
    
    e_sleep(&WRK.close_event, EVENT_SHORT);

    hwreset(p_dds);
    
    WRK.adap_state = CLOSED_STATE;

}  /* end function close adapter */

/*
 * NAME: cfg_adap_parms
 *
 * FUNCTION:
 *
 *  Resets all adapter parameters to the configuration values.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes on the process thread.
 *
 * NOTES:
 *      This functions will reset the work variables for the adapter
 *      initialization and open options to the configuration settings (for
 *      the options that are configurable) and the non-configurable items
 *      to the default settings.
 *
 *      This function assumes that the VPD has already been succuessfully
 *      read from the adapter.  It will access the VPD's burned-in-address.
 *
 * RECOVERY OPERATION:
 *      None.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS:
 *      None.
 */
void
cfg_adap_parms( dds_t  *p_dds )
{
    /*
     *   Set the Work area adapter initialization and open options to
     *   what was passed in at config time.
     */
    WRK.adap_iparms.init_options = DF_ADAP_INIT_OPTIONS;
    WRK.adap_iparms.cmd = NULL;
    WRK.adap_iparms.xmit = NULL;
    WRK.adap_iparms.rcv = NULL;
    WRK.adap_iparms.ring = NULL;
    WRK.adap_iparms.scb_clear = NULL;
    WRK.adap_iparms.adap_chk = NULL;
    WRK.adap_iparms.rcv_burst_size = DF_RCV_BURST_SIZE;
    WRK.adap_iparms.xmit_burst_size = DF_TX_BURST_SIZE;
    WRK.adap_iparms.dma_abort_thresh = DF_DMA_ABORT_THRESH;
    WRK.adap_iparms.scb_add1 = NULL;
    WRK.adap_iparms.scb_add2 = NULL;
    WRK.adap_iparms.ssb_add1 = NULL;
    WRK.adap_iparms.ssb_add2 = NULL;
    
    /*
     * default open options are
     * - disable soft error interrupts
     * - pass adapter MAC frames
     * - adapter contends for active monitor
     */
    WRK.adap_open_opts.options= DF_ADAP_OPEN_OPTIONS;
    if (DDI.attn_mac) {
	NDD.ndd_flags |= TOK_ATTENTION_MAC;
	WRK.adap_open_opts.options |= (0x01<<11);
    }

    if (DDI.beacon_mac) {
	NDD.ndd_flags |= TOK_BEACON_MAC;
	WRK.adap_open_opts.options |= (0x01<<7);
    }

    WRK.adap_open_opts.buf_size = ADAP_BUF_SIZE_1K;
    
    NDD.ndd_mintu = CTOK_MIN_PACKET;

    if (DDI.ring_speed) {
	/*
	 *   Set the MAX packet size for 16 Mbps data rate
	 */
	NDD.ndd_mtu = CTOK_16M_MAX_PACKET;
	
	WRK.adap_open_opts.xmit_buf_min_cnt =
	    ADAP_16M_TX_BUF_MIN_CNT_1K;
	WRK.adap_open_opts.xmit_buf_max_cnt =
	    ADAP_16M_TX_BUF_MAX_CNT_1K;
    } else {
	/*
	 *   Set the MAX packet size for 4 Mbps data rate
	 */
	NDD.ndd_mtu = CTOK_4M_MAX_PACKET;
	
	WRK.adap_open_opts.xmit_buf_min_cnt =
	    ADAP_4M_TX_BUF_MIN_CNT_1K;
	WRK.adap_open_opts.xmit_buf_max_cnt =
	    ADAP_4M_TX_BUF_MAX_CNT_1K;
    }
    
    *((ulong *)(WRK.adap_open_opts.grp_addr)) = 0;
    *((ulong *)(WRK.adap_open_opts.func_addr)) = 0;
    WRK.adap_open_opts.rcv_list_size = 0x0e;
    WRK.adap_open_opts.xmit_list_size = 0x1a;
    WRK.adap_open_opts.res1 =  NULL;
    WRK.adap_open_opts.res2 = NULL;
    WRK.adap_open_opts.prod_id_addr1 = NULL;
    WRK.adap_open_opts.prod_id_addr2 = NULL;
    
    /*
     *   Check if we need to use the alternate network address.
     *   If so, set the open options accordingly.
     *   If not, plug in the burned in address from the VPD.
     */
    if (DDI.use_alt_addr) {
	COPY_NADR(DDI.alt_addr, WRK.adap_open_opts.node_addr);
    } else {
	COPY_NADR(WRK.tok_vpd_addr, WRK.adap_open_opts.node_addr);
    }
    /*
     *  Set our local hardware address
     */
    NDD.ndd_physaddr = (caddr_t)(&WRK.adap_open_opts.node_addr);
    
}  /* end cfg_adap_parms */
