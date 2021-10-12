static char sccsid[] = "@(#)16	1.14  src/bos/kernext/tok/trmon_limbo.c, sysxtok, bos411, 9428A410j 5/27/94 11:19:30";
/*
 *   COMPONENT_NAME: SYSXTOK
 *
 *   FUNCTIONS: bug_out
 *		cycle_limbo
 *		egress_limbo
 *		enter_limbo
 *		kill_limbo
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
 * NAME: enter_limbo
 *
 * FUNCTION:
 *      Prepares the Token-Ring device driver for entering Network
 *      recovery mode (limbo for short).  This routine initiates
 *      the cleanup of the TX and RCV chains, logs the error,
 *      notifies all attached users via a asynchronous status
 *      block, and then initiates the re-cycling of the adapter.
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *     This routine executes on the interrupt.
 *
 * NOTES:
 *	The enter_limbo() function assumes that the SLIH_LOCK has been
 *	acquired prior to calling this routine.
 *
 *     The enter_limbo() function assumes that the limbo state variable
 *     has been checked prior to calling.  This routine should never
 *     be called if we are in some form of limbo.  It should only be
 *
 *     called under the following conditions:
 *         1. adap_state = OPEN_STATE &&
 *            limbo = PARADISE
 *         2. A valid enter limbo condition has occurred.
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *      software error recovery.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */

void
enter_limbo(
    dds_t	*p_dds,		/* pointer to DDS */
    uint	reason,		/* reason for entering limbo */
    uint	errid,		/* errid that was logged before enter_limbo */
    ushort	ac,		/* Adapter code */
    int		got_tx_lock )	/* TX_LOCK already acquired */
{
    int rc;
    intr_elem_t     iwe;
    int txpri;
    int ndx;
    int timer_delay;
    ndd_statblk_t	sb;
    
    
    TRACE_BOTH(MON_OTHER, "elmB", (ulong)p_dds, reason, (ulong)ac);

    if (!got_tx_lock) {
	TRACE_DBG(MON_OTHER, "TX_L", (int)p_dds, 0, 0);
	txpri = disable_lock(PL_IMP, &TX_LOCK);
    }
    
    /*
     *   NOTE:
     *   This check is key.  We may be in the process of
     *   shutting down the adapter in response to a
     *   close... OR we may be in the middle of the 1st activation
     *   request
     *
     *   In either situation we want to make sure that if
     *   we get a limbo entry condition that we DO NOT
     *   enter limbo.
     */

    if ((WRK.adap_state == CLOSE_PENDING) ||
	(WRK.adap_state == OPEN_PENDING))
    {
	/*
	 *   We have a Limbo entry condition,
	 *   BUT we have a close pending OR 
	 *   we are in the middle of activating the adapter for the first time.
	 *   Do nothing.
	 *
	 *   If the adapter close command never
	 *   comes, the close failsafe timer will pop and let us
	 *   exit gracefully.
	 *
	 *   If in the middle of activating the adapter for the 1st time,
	 *   our timers will catch us and let us contiue/exit gracefully.
	 */
	TRACE_BOTH(MON_OTHER, "lSQZ", p_dds, WRK.adap_state, WRK.limbo );
    } else if (WRK.adap_state != LIMBO_STATE) {
	TRACE_BOTH(MON_OTHER, "lSAF", p_dds, WRK.adap_state, WRK.limbo );
	/*
	 *   NOTE:
	 *       At this point we have a go for
	 *       Limbo Mode entry.  Set the limbo
	 *       state machine for chaos.
	 */
	TRACE_BOTH(MON_OTHER, "lGO!", p_dds, WRK.adap_state, WRK.limbo);
	
	WRK.limbo = CHAOS;      /* We are now in Limbo! */
	WRK.rr_entry = reason;   /* save entry reason */
	WRK.rr_errid = errid;    /* save entry errid */
	WRK.limcycle = 1;           /* limbo cycle counter to 1 */
	/* 
	 *  Set the state machine and the NDD flags.
	 */
	NDD.ndd_flags &= ~NDD_RUNNING;
	NDD.ndd_flags |= NDD_LIMBO;
	WRK.adap_state = LIMBO_STATE;
	
        /*
	 *       Build the asynchronous status block
	 */

	bzero (&sb, sizeof(sb));
	sb.code = NDD_LIMBO_ENTER;
	sb.option[0] = reason;
        if ( reason == NDD_ADAP_CHECK ) {
            bcopy( &(WRK.ac_blk), 
		&(sb.option[1]),
		sizeof(adap_check_blk_t) );
        } else {
	    sb.option[1] = ac;
        }
	TRACE_BOTH(MON_OTHER, "stat", (int)p_dds, sb.code, sb.option[0]);
	TRACE_BOTH(MON_OTHER, "sta2", NDD.ndd_flags, 0, 0);
	NDD.nd_status( &NDD, &sb );

	w_stop(&XMITWDT);

	/*
	 *       clean out the TX chain.
	 *       also flush the TX Queue.
	 */
        rc = clean_tx(p_dds);
	
	/*
	 *	clean out the receive chain
	 */
        WRK.recv_mode = FALSE;
        clear_recv_chain( p_dds, FALSE );
	
	/*
	 *	If any user processes were waiting for adapter responses
	 *	- cancel their failsafe timers
	 *	- wake them up
	 */
	
	w_stop(&BUWDT);

	WRK.event_wait &= ~FUNCT_WAIT;
	if (WRK.funct_event != EVENT_NULL) {
	    e_wakeup( &WRK.funct_event );
	}
	
	WRK.event_wait &= ~GROUP_WAIT;
	if (WRK.group_event != EVENT_NULL) {
	    e_wakeup( &WRK.group_event );
	}
	
	WRK.event_wait &= ~ELOG_WAIT;
	if (WRK.elog_event != EVENT_NULL) {
	    e_wakeup( &WRK.elog_event );
	}
	
	WRK.event_wait &= ~READ_ADAP_WAIT;
	if (WRK.read_adap_event != EVENT_NULL) {
	    e_wakeup( &WRK.read_adap_event );
	}
	
	/*
	 *	For some errors, we delay before we reset the adapter
	 *	and try to recover from the error.  In those cases start
	 *	timer and do the ds_act when the timer pops.
	 */
        if ( reason == TOK_ADAP_OPEN  ||
	     reason == TOK_RING_SPEED ||
	     reason == TOK_RMV_ADAP ) {
	    switch (reason) {
	    case TOK_ADAP_OPEN:
		timer_delay = OPEN_ERROR_TIME;
		break;
	    case TOK_RING_SPEED:
		timer_delay = RING_SPEED_TIME;
		break;
	    case TOK_RMV_ADAP:
		timer_delay = RMV_RECEIVED_TIME;
	    }
	    BUWDT.count = 0;
	    BUWDT.restart = timer_delay;
	    WRK.bu_wdt_cmd = INTR_CYCLE_LIMBO;
	    w_start(&BUWDT);
	} else {
            ds_act(p_dds);
	}
	
	TRACE_BOTH(MON_OTHER, "elmE", (int)p_dds, 0, 0 );
    }

    if (!got_tx_lock) {
	unlock_enable(txpri, &TX_LOCK);
	TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);
    }

    return;

} /* end enter_limbo() */

/*
 * NAME: kill_limbo
 *
 * FUNCTION:
 *      This function terminates the Token-Ring Ring Recovery Scheme.
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine only executes on a process thread.
 *
 * NOTES:
 *     This rountine can only be if we are in some form of
 *     limbo mode AND we have received the very last close.
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *      software error recovery.
 *
 * DATA STRUCTURES: 
 *	Changes the limbo state to LIMBO_KILL_PEND to indicate that 
 * 	we are in the process of killing limbo.
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */

void
kill_limbo(dds_t *p_dds)
{
    int limbo;
    
    TRACE_BOTH(MON_OTHER, "klmB", (int)p_dds, (ulong)WRK.adap_state,
	    (ulong)WRK.limbo);
    
    /* save the current state of limbo.
     * set the limbo state to indicate that we
     * are in the process of killing limbo.
     */
    
    limbo = WRK.limbo;
    WRK.limbo = LIMBO_KILL_PEND;
    
    /*
     *  what phase of limbo are we in?
     */
    switch (limbo)
    {
    case CHAOS:
	w_stop(&BUWDT);

	hwreset(p_dds);
	TRACE_BOTH(MON_OTHER, "FooT", (int)p_dds, LMB_KILL_0, 0);
	break;
    case PROBATION:
	/*
	 *   We are still in the penalty box.
	 *   Stop the probation timer, then close the adapter
	 */
	w_stop(&BUWDT);
	TRACE_BOTH(MON_OTHER, "FooT", (int)p_dds, LMB_KILL_1, 0);
	close_adap( p_dds );
	break;
    } /* endswitch */

    WRK.limbo = PARADISE;
    
    TRACE_BOTH(MON_OTHER, "klmE", (int)p_dds, (ulong)WRK.adap_state,
	    (ulong)WRK.limbo);
} /* end kill_limbo() */

/*
 * NAME: cycle_limbo
 *
 * FUNCTION:
 *      This function continues the Limbo recovery sequence.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes on the interrupt level.
 *
 * NOTES:
 *     This routine may be called for one of the following reasons.
 *         1. During the cycling of limbo, the activation
 *            sequence failed.
 *         2. Double Jeopardy!  A limbo entry condition occurred
 *            during the activation sequence during the cycling
 *            of limbo.
 *         3. Red Zone.  A limbo entry condition occurred during
 *            the Egress of Limbo.
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *      software error recovery.
 *
 * DATA STRUCTURES:
 *
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */
void
cycle_limbo( dds_t *p_dds,
	     uint  reason,
	     uint  errid)
{
    int	timer_delay;

    TRACE_BOTH(MON_OTHER, "clmB", (int)p_dds, WRK.adap_state, WRK.limbo);
    TRACE_BOTH(MON_OTHER, "clm2", reason, errid, 0);
    /*
     * died during activation of adapter
     */
    if (WRK.limbo == CHAOS) {
	/*
	 *  cancel the bringup timer
	 */
	w_stop(&BUWDT);
    } else if ((WRK.limbo != PARADISE) &&
	       (WRK.limbo != NO_OP_STATE))
    {
	/*
	 *   We have a failure during the Red Zone (egress of limbo).
	 *   Stop the bringup timer.
	 */
	w_stop(&BUWDT);
	
	WRK.limbo = CHAOS;
	
    } else {
	/*
	 * we have either bugged out while we were in 
	 * limbo and this call to cycle_limbo() is a left
	 * over one (i.e. a timer pop) OR
	 * we've gotten so confused as to call cycle_limbo()
	 * while we were not in limbo to begin with.
	 * 
	 * This latter case should not happen.
	 */
	return;

    }   /* end NULL else */

    /*
     * The reason that we are cycling limbo may be different than the
     * reason that we entered limbo.  If they are different, set a new
     * limbo reason and set the number of limbo cycles to one.  This is
     * to handle sequences like WIRE_FAULT, ADAP_INIT, WIRE_FAULT, and
     * ADAP_INIT causing a bug_out because resetting the adapter didn't work.
     * It also handles cases like entering limbo because of WIRE_FAULT and
     * then getting a RING_SPEED error after the cable is connected.
     */
    if (WRK.rr_entry != reason) {
	WRK.rr_entry = reason;
	WRK.limcycle = 1;

	/*
	 * Adapter initialization errors are not logged until just before
	 * bugging out so cycle_limbo will be called with an errid of 0 in
	 * that case.  That takes care the problem with adapter initialization
	 * frequently taking more than one cycle thru limbo.
	 *
	 * However, we still need to handle entering limbo because of a wire
	 * fault and then getting another error when the cable is connected.
	 */
	if ( (errid != NULL) && (errid != WRK.rr_errid) ) {
	    WRK.rr_errid = errid;
	    logerr(p_dds, errid, __LINE__, __FILE__);
	}
    } else {
	WRK.limcycle++;
    }

    /*
     * retry failure to reset device or initialize adapter three times
     */
    if ( ( reason == TOK_ADAP_INIT ) && ( WRK.limcycle == 4 ) ) {
	bug_out( p_dds, ERRID_CTOK_PERM_HW, TOK_RECOVERY_THRESH,
		TOK_PERM_HW_ERR );
        TRACE_BOTH(MON_OTHER, "clmE", (int)p_dds, (ulong)WRK.limcycle, 0);
        return;
    }

    /*
     * retry what seems to be the wrong ring speed twice
     */
    if ( ( reason == TOK_RING_SPEED ) && ( WRK.limcycle == 3 ) ) {
	bug_out( p_dds, NULL, TOK_RECOVERY_THRESH, TOK_RING_SPEED );
        TRACE_BOTH(MON_OTHER, "clmE", (int)p_dds, (ulong)WRK.limcycle, 0);
        return;
    }

    /*
     * retry being asked to leave the ring twice
     */
    if ( ( reason == TOK_RMV_ADAP ) && ( WRK.limcycle == 3 ) ) {
	bug_out( p_dds, ERRID_CTOK_RMV_ADAP, TOK_RECOVERY_THRESH,
		TOK_RMV_ADAP );
        TRACE_BOTH(MON_OTHER, "clmE", (int)p_dds, (ulong)WRK.limcycle, 0);
        return;
    }

    /*
     *	For some errors, we delay before we reset the adapter
     *	and try to recover from the error.  In those cases start
     *	timer and do the ds_act when the timer pops.
     */
    if ( reason == TOK_ADAP_OPEN  ||
	 reason == TOK_RING_SPEED ||
	 reason == TOK_RMV_ADAP ) {
	switch (reason) {
	case TOK_ADAP_OPEN:
	    timer_delay = OPEN_ERROR_TIME;
	    break;
	case TOK_RING_SPEED:
	    timer_delay = RING_SPEED_TIME;
	    break;
	case TOK_RMV_ADAP:
	    timer_delay = RMV_RECEIVED_TIME;
	}
	BUWDT.count = 0;
	BUWDT.restart = timer_delay;
	WRK.bu_wdt_cmd = INTR_CYCLE_LIMBO;
	w_start(&BUWDT);
    } else {
        ds_act(p_dds);
    }
    
    TRACE_BOTH(MON_OTHER, "clmE", (int)p_dds, (ulong)WRK.limcycle, 0);
    return;

} /* end cycle_limbo */

/*
 * NAME: egress_limbo
 *
 * FUNCTION:
 *      This function brings the device driver out of limbo.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes on the interrupt level.
 *
 * NOTES:
 *     This routine will walk the device driver out of limbo mode.
 *     The path taken is:
 *         - Through the probation period
 *         - kicking off the receiving of data.
 *         - setting the functional address
 *         - then setting the group address
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *      software error recovery.
 *
 * DATA STRUCTURES:
 *
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */

void 
egress_limbo( dds_t *p_dds)
{
    
    /* local vars */
    int rc;
    ndd_statblk_t	sb;
    
    TRACE_BOTH(MON_OTHER, "eglB", (ulong)p_dds, (ulong)WRK.adap_state,
	    (ulong)WRK.limbo); /* egress_limbo begin */
    
    switch(WRK.limbo)
    {
    case PARADISE:
    case NO_OP_STATE:
	/*
	 * we have either bugged out while we were in 
	 * limbo and this call to egress_limbo() is a left
	 * over one (i.e. a timer pop) OR
	 * we've gotten so confused as to call egress_limbo()
	 * while we were not in limbo to begin with.
	 * 
	 * This latter case should not happen.
	 */
	TRACE_BOTH(MON_OTHER, "FooT", LMB_EGRESS_0, WRK.bugout, 0);
	break;
	
    case CHAOS:
	BUWDT.count = 0;
	BUWDT.restart = PENALTY_BOX_TIME;
	WRK.bu_wdt_cmd = INTR_PROBATION;
	WRK.bringup = RESET_PHASE1;
	WRK.limbo = PROBATION;
	w_start(&BUWDT);

	break;
    case PROBATION:
	/*
	 *  Our probation timer has expired.
	 *  Start the backing out of Limbo Mode.
	 *
	 *  We must issue the Receive SCB command
	 *  immediately after the OPEN SCB comand completes
	 */
	w_stop(&BUWDT);
	
	WRK.recv_mode = TRUE;
	
	load_recv_chain(p_dds);
	
	TRACE_DBG(MON_OTHER, "strv", (int)p_dds, 0, 0);
	/*
	 *  start adapter receive
	 */
	issue_scb_command(p_dds, ADAP_RCV_CMD, WRK.recv_list[0]);
	
	/*
	 * Get Transmit started up again.
	 */
	tx_limbo_startup(p_dds);
	
	/*
	 *   We're out of Limbo! We've made it to PARADISE!  Set the
	 *   state machine and NDD flags.
	 */
	WRK.rr_entry = 0;
	WRK.limbo = PARADISE;
	WRK.adap_state = OPEN_STATE;
	NDD.ndd_flags &= ~NDD_LIMBO;
	NDD.ndd_flags |= NDD_RUNNING;

	/*
	 * only log exiting limbo if we logged entering limbo
	 */
	if (WRK.rr_errid) {
		logerr(p_dds, ERRID_CTOK_RCVRY_EXIT, __LINE__, __FILE__);
	}

	bzero (&sb, sizeof(sb));
	sb.code = NDD_LIMBO_EXIT;
	TRACE_BOTH(MON_OTHER, "stat", (int)p_dds, sb.code, 0);
	TRACE_BOTH(MON_OTHER, "sta2", NDD.ndd_flags, 0, 0);
	NDD.nd_status( &NDD, &sb );

	if (!WRK.connect_sb_sent) {
	    /*
	     *  the Connected status block was never sent
	     */
	    WRK.connect_sb_sent = TRUE;
	    bzero (&sb, sizeof(sb));
	    sb.code = NDD_CONNECTED;
	    TRACE_BOTH(MON_OTHER, "stat", (int)p_dds, sb.code, 0);
	    TRACE_BOTH(MON_OTHER, "sta2", NDD.ndd_flags, 0, 0);
	    NDD.nd_status( &NDD, &sb );
	}
	
	break;
    } /* end switch */
    
    TRACE_BOTH(MON_OTHER, "eglE", (int)p_dds, (ulong)WRK.limcycle, 0);

} /* end egress_limbo() */


/*
 * NAME: bug_out
 *
 * FUNCTION:
 *     This function moves the device handler into the
 *     dead state.  A fatal error has just been detected.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes on the interrupt level or process thread.
 *
 * NOTES:
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *      software error recovery.
 *
 * DATA STRUCTURES:
 *
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */
void
bug_out(
    dds_t	*p_dds,
    uint	errid,
    uint	reason,
    uint	subcode)
{
    
    int rc, txpri;
    struct ndd_statblk sb;
    int iocc;
    uchar pos2;
    
    TRACE_DBG(MON_OTHER, "TX_L", (int)p_dds, 0, 0);
    txpri = disable_lock(PL_IMP, &TX_LOCK);
    
    TRACE_BOTH(MON_OTHER, "bubB", (int)p_dds, WRK.adap_state, WRK.limbo);
    TRACE_BOTH(MON_OTHER, "bub1", errid, reason, subcode);
    
    /*
     *       Build the asynchronous status block
     */

    bzero (&sb, sizeof(sb));
    sb.code = NDD_HARD_FAIL;
    sb.option[0] = reason;
    if ( reason == NDD_ADAP_CHECK ) {
	bcopy( &(WRK.ac_blk), &(sb.option[1]), sizeof(adap_check_blk_t) );
    } else {
	sb.option[1] = subcode;
    }
    TRACE_BOTH(MON_OTHER, "stat", (int)p_dds, sb.code, sb.option[0]);
    TRACE_BOTH(MON_OTHER, "sta2", NDD.ndd_flags, 0, 0);
    NDD.nd_status( &NDD, &sb );
    
    /*
     * Log the error
     */
    if (errid) {
	logerr(p_dds, errid, __LINE__, __FILE__);
    }
    
    /* 
     * clean out the TX chain.
     * also flush the TX Queue.
     */
    rc = clean_tx(p_dds);
    
    /* 
     * clean out the receive chain
     */
    clear_recv_chain( p_dds, FALSE );
    
    WRK.adap_state = DEAD_STATE;
    NDD.ndd_flags &= ~(NDD_RUNNING | NDD_LIMBO);
    NDD.ndd_flags |= NDD_DEAD;
    WRK.limbo = NO_OP_STATE;
    
    /* 
     * Disable the card.  We don't
     * want to give the card the chance to 
     * misbehave anymore.
     *
     * Get access to pos registers.
     * Get the current setting of pos reg 2.
     * Turn off the card enable bit in pos.
     *
     */
    iocc = (int)IOCC_ATT( (ulong)DDI.bus_id,
	(ulong)(IO_IOCC + (DDI.slot << 16)));
    TOK_GETPOS( iocc + POS_REG_2, &pos2 );
    TOK_PUTPOS( iocc + POS_REG_2, (pos2 & (~CARD_ENABLE)) );
    IOCC_DET( iocc );

    TRACE_BOTH(MON_OTHER, "bugE", (int)p_dds, (ulong)WRK.limcycle, 
    		(ulong)WRK.bugout);
    
    unlock_enable(txpri, &TX_LOCK);
    TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);

}  /* end function bug_out() */
