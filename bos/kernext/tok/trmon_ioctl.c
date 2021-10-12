static char sccsid[] = "@(#)17	1.16  src/bos/kernext/tok/trmon_ioctl.c, sysxtok, bos411, 9428A410j 5/27/94 11:29:27";
/*
 *   COMPONENT_NAME: SYSXTOK
 *
 *   FUNCTIONS: disable_address
 *		enable_address
 *		get_addr
 *		get_mibs
 *		get_stats
 *		tokioctl
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

extern token_ring_all_mib_t mon_mib_status;

/*****************************************************************************/
/*
 * NAME:     tokioctl
 *
 * FUNCTION: ioctl entry point from kernel
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES: Several routines are conditionally compiled:
 *	  get_adap_addr & get_addr2 if TOK_ADAP_ADDR is defined
 *
 * RETURNS:  0 or errno
 *
 */
/*****************************************************************************/
int tokioctl (
	ndd_t	*p_ndd,		/* NDD structure for this device.    */
	int	cmd,		/* ioctl operation desired */
	caddr_t	arg,		/* arg for this cmd (usually a struct ptr) */
	int	length)		/* length of argument for this command  */
{
	register dds_t	*p_dds;
	int		rc=0;
	int		txpri;

	/*
	 *  Get adapter structure
	 */
	p_dds = (dds_t *) (((unsigned long) p_ndd) - offsetof(dds_t, ndd));
    
	TRACE_BOTH(MON_OTHER, "IOCb", (int)p_dds, cmd, arg);
	TRACE_BOTH(MON_OTHER, "IOC2", (int)p_dds, length, WRK.adap_state);

	TRACE_DBG(MON_OTHER, "TX_L", (int)p_dds, 0, 0);
	txpri = disable_lock(PL_IMP, &TX_LOCK);

	/* handle standard ioctl's */
	switch (cmd)
	{
	case NDD_CLEAR_STATS:		/* Clear all of the statistics */
		WRK.ndd_stime = WRK.dev_stime = lbolt;
		/*
		 * read error log from adapter to clear it
		 * - if the device is down, error recovery will clear it
		 */
		if (WRK.adap_state == OPEN_STATE) {
			WRK.event_wait |= ELOG_WAIT;
			WRK.elog_event = EVENT_NULL;
			issue_scb_command (p_dds, ADAP_ELOG_CMD,
				WRK.p_d_errlog);

			BUWDT.count = 0;
			BUWDT.restart = ELOG_CMD_TIMEOUT;
			WRK.bu_wdt_cmd = INTR_ELOG_TIMEOUT;
			w_start(&(BUWDT));
	
			while (WRK.event_wait & ELOG_WAIT) {
				TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);
				e_sleep_thread( &WRK.elog_event, &TX_LOCK,
					LOCK_HANDLER );
				TRACE_DBG(MON_OTHER, "TX_L", (int)p_dds, 0, 0);
			}
		}
		bzero(&NDD.ndd_genstats, sizeof(ndd_genstats_t));
		bzero(&TOKSTATS, sizeof(TOKSTATS) );
		bzero(&DEVSTATS, sizeof(DEVSTATS) );
		TOKSTATS.device_type = TOK_MON;
		COPY_NADR(WRK.adap_open_opts.node_addr, TOKSTATS.tok_nadr);
		break;

	case NDD_DISABLE_ADDRESS:	/* Disable functional address */
		rc = disable_address( p_dds, arg );
		break;

	case NDD_DUMP_ADDR:		/* Return remote dump routine addr */
		ASSERT (arg != 0);
		if (arg == 0) {
			rc = EINVAL;
			break;
		}
		*(uint *)arg = tokdump;
		break;

	case NDD_ENABLE_ADDRESS:	/* Enable functional address */
		rc = enable_address( p_dds, arg );
		break;

	case NDD_GET_ALL_STATS:		/* Get all the device statistics */
					/* that is the mon_all_stats  */
		ASSERT (arg != 0);
		if (arg == 0) {
			rc = EINVAL;
			break;
		}
		ASSERT (length == sizeof(mon_all_stats_t));
		if (length != sizeof(mon_all_stats_t)) {
			rc = EINVAL;
			break;
		}

		/*
		 * set general statistics and get statistics from device
		 */
		get_stats( p_dds );

		/* copy statistics to user's buffer */
		bcopy (&NDD.ndd_genstats, arg, sizeof(ndd_genstats_t));
		bcopy (&(TOKSTATS), arg + sizeof(ndd_genstats_t),
			sizeof(tok_genstats_t) + sizeof(tr_mon_stats_t));
		break;

	case NDD_GET_STATS:		/* Get the generic statistics */
					/* that is the tok_ndd_stats  */
		ASSERT (arg != 0);
		if (arg == 0) {
			rc = EINVAL;
			break;
		}
		ASSERT (length == sizeof(tok_ndd_stats_t));
		if (length != sizeof(tok_ndd_stats_t)) {
			rc = EINVAL;
			break;
		}

		/*
		 * set general statistics and get statistics from device
		 */
		get_stats( p_dds );

		/* copy statistics to user's buffer */
		bcopy (&NDD.ndd_genstats, arg, sizeof(ndd_genstats_t));
		bcopy (&(TOKSTATS), arg + sizeof(ndd_genstats_t),
			sizeof(tok_genstats_t));
		break;

	case NDD_MIB_ADDR:		/* Get the receive addresses for */
					/* this device: physical, broadcast */
					/* plus optional functional, group */
	{
#ifdef TOK_ADAP_ADDR
/*
 * For debug purposes, read the receive addresses from the adapter rather
 * than use the program values
 */
		rc = get_addr2( p_dds, arg, length );
#else
		rc = get_addr( p_dds, arg, length );
#endif
		break;
	}

	case NDD_MIB_GET:		/* Get all the MIB values */
		ASSERT (arg != 0);
		if (arg == 0) {
			rc = EINVAL;
			break;
		}
		ASSERT (length == sizeof(token_ring_all_mib_t));
		if (length != sizeof(token_ring_all_mib_t)) {
			rc = EINVAL;
			break;
		}

		get_mibs( p_dds, arg );

		break;

	case NDD_MIB_QUERY:		/* Query MIB support status */
		ASSERT (arg != 0);
		if (arg == 0) {
			rc = EINVAL;
			break;
		}
		ASSERT (length == sizeof(token_ring_all_mib_t));
		if (length != sizeof(token_ring_all_mib_t)) {
			rc = EINVAL;
			break;
		}
		/* copy status to user's buffer */
		bcopy (&mon_mib_status, arg, sizeof(token_ring_all_mib_t));
		break;
	
#ifdef TOK_FREEZE_DUMP
		/*
		 * For debug purposes, define a command to do a freeze dump
		 * and one to read the freeze dump
		 */
	case TOK_DO_FREEZE_DUMP:	/* Cause a freeze dump */
		freeze_dump( p_dds );
		break;

	case TOK_READ_FREEZE_DUMP:	/* Read the freeze dump */
		if (arg == 0) {
			rc = EINVAL;
			break;
		}
		if (length < TOK_FREEZE_DUMP_SIZE) {
			TRACE_DBG(MON_OTHER, "ILNG", p_dds, length,
				TOK_FREEZE_DUMP_SIZE);
			rc = EINVAL;
			break;
		}
		/* copy data to user's buffer */
		bcopy (WRK.freeze_data, arg, TOK_FREEZE_DUMP_SIZE);
		break;
#endif

	default:            /* Invalid (unsupported) op */
		rc = EOPNOTSUPP;
	} /* end switch (cmd) */
   
	unlock_enable( txpri, &TX_LOCK );
	TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);
    
	TRACE_BOTH(MON_OTHER, "IOCe", (int)p_dds, rc, NDD.ndd_flags);
	return (rc);
    
} /* end tokioctl */

/*****************************************************************************/
/*
 * NAME:     disable_address
 *
 * FUNCTION: disables an alternate (function or group) address
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * CALLED FROM: tokioctl
 *
 * INPUT:
 *	p_dds	- pointer to the device control area
 *	arg	- address to disable
 *
 * NOTES:
 *
 * RETURNS:  0
 *
 */
/*****************************************************************************/
int
disable_address(dds_t	*p_dds,
	caddr_t		arg)
{
	int		rc=0, i, j, temp_func;

	TRACE_BOTH(MON_OTHER, "dsb@", (int)p_dds, *((uint *) (arg + 2)), 0);
	ASSERT (arg != 0);
	if (arg == 0) {
		return(EINVAL);
	}
	if (*(uchar *)(arg + 2) & GROUP_ADR_MASK) {	/* group address */
		/*
		 * can only disable the address which was enabled
		 */
		ASSERT (*(uint *)(WRK.adap_open_opts.grp_addr) ==
			*((uint *) (arg + 2)));
		if (*(uint *)(WRK.adap_open_opts.grp_addr) !=
			*((uint *) (arg + 2))) {
			return(EINVAL);
		}

		/*
		 * set group address to zero
		 */
		*(uint *)(WRK.adap_open_opts.grp_addr) = 0;

		/*
		 * Not receiving data for a group address anymore.
		 * If not receiving data for any functional addresses,
		 * then not receiving data for any alternate addresses.
		 */
		NDD.ndd_flags &= ~TOK_RECEIVE_GROUP;
		if (! (*(uint *)(WRK.adap_open_opts.func_addr)) ) {
			NDD.ndd_flags &= ~NDD_ALTADDRS;
		}

		/*
		 * Programs like TCP/IP don't wait for the adapter to be
		 * up before issuing functional/group addresses.  To do so
		 * would slow down IPL.  If the adapter open hasn't been
		 * issued yet, it will get the address as part of the adapter
		 * open and thus don't do anything else.  If the adapter is
		 * not open (but not dead), set a flag to have the command
		 * processed when the adapter is open.
		 */
		if (WRK.adap_state != OPEN_STATE) {
			switch (WRK.adap_state)
			{
			case DEAD_STATE:
			case NULL_STATE:
			case CLOSED_STATE:
			case CLOSE_PENDING:
				TRACE_BOTH(MON_OTHER, "dsb2",
					WRK.adap_state, WRK.bringup,
					WRK.command_to_do);
				return(0);
			default: /* OPEN_PENDING or LIMBO_STATE */
				/*
				 * check both states to eliminate any
				 * timing window
				 */
				if ((WRK.bringup == ADAP_INIT_PHASE1) ||
				    (WRK.bringup == OPEN_PHASE0)) {
					WRK.command_to_do = TRUE;
				}
				TRACE_BOTH(MON_OTHER, "dsb2",
					WRK.adap_state, WRK.bringup,
					WRK.command_to_do);
				return(0);
			}
		}

		WRK.event_wait |= GROUP_WAIT;
		WRK.group_event = EVENT_NULL;
		issue_scb_command(p_dds, ADAP_GROUP_CMD,
			*(uint *)(WRK.adap_open_opts.grp_addr));

		BUWDT.count = 0;
		BUWDT.restart = GROUP_ADDR_TIMEOUT;
		WRK.bu_wdt_cmd = INTR_GROUP_TIMEOUT;
		w_start(&(BUWDT));
	
		while (WRK.event_wait & GROUP_WAIT) {
			TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);
			e_sleep_thread( &WRK.group_event, &TX_LOCK,
				LOCK_HANDLER );
			TRACE_DBG(MON_OTHER, "TX_L", (int)p_dds, 0, 0);
		}
	} else {	/* functional address */
		temp_func = *(uint *)(WRK.adap_open_opts.func_addr);

		/*
		 * can only disable an address which is currently enabled
		 */
		if ( ( *((uint *) (arg + 2)) & 
			*(uint *)(WRK.adap_open_opts.func_addr) ) !=
			 *((uint *) (arg + 2)) ) {
			return(EINVAL);
		}
		/*
		 * keep a reference count on each of the bits in the address
		 * and only turn off the bits whose reference count is 0
		 */
		for (i = 0, j = 1; i < 31; i++, j *= 2) {
			if ( *((uint *) (arg + 2)) & j) {
			    WRK.func_addr_ref[i]--;
			    if (! WRK.func_addr_ref[i]) {
				*(uint *)(WRK.adap_open_opts.func_addr) &= ~j;
			    }
			}
		}
		TRACE_BOTH(MON_OTHER, "fct@", (int)p_dds,
			*((ulong *)(WRK.adap_open_opts.func_addr)), 0);

		/*
		 * check if it is necessary to issue the set addr command
		 */
		if ( *(uint *)(WRK.adap_open_opts.func_addr) == temp_func ) {
			TRACE_BOTH(MON_OTHER, "dsb2", WRK.adap_state,
				WRK.bringup, WRK.command_to_do);
			return(0);
		}
	
		/*
		 * If not receiving data for any functional addresses
		 * and not receiving data for a group address
		 * then not receiving data for any alternate addresses.
		 */
		if (! (*(uint *)(WRK.adap_open_opts.func_addr)) ) {
			NDD.ndd_flags &= ~TOK_RECEIVE_FUNC;
			if (! (*(uint *)(WRK.adap_open_opts.grp_addr)) ) {
				NDD.ndd_flags &= ~NDD_ALTADDRS;
			}
		}

		/*
		 * Programs like TCP/IP don't wait for the adapter to be
		 * up before issuing functional/group addresses.  To do so
		 * would slow down IPL.  If the adapter open hasn't been
		 * issued yet, it will get the address as part of the adapter
		 * open and thus don't do anything else.  If the adapter is
		 * not open (but not dead), set a flag to have the command
		 * processed when the adapter is open.
		 */
		if (WRK.adap_state != OPEN_STATE) {
			switch (WRK.adap_state)
			{
			case DEAD_STATE:
			case NULL_STATE:
			case CLOSED_STATE:
			case CLOSE_PENDING:
				TRACE_BOTH(MON_OTHER, "dsb2",
					WRK.adap_state, WRK.bringup,
					WRK.command_to_do);
				return(0);
			default: /* OPEN_PENDING or LIMBO_STATE */
				/*
				 * check both states to eliminate any
				 * timing window
				 */
				if ((WRK.bringup == ADAP_INIT_PHASE1) ||
				    (WRK.bringup == OPEN_PHASE0)) {
					WRK.command_to_do = TRUE;
				}
				TRACE_BOTH(MON_OTHER, "dsb2",
					WRK.adap_state, WRK.bringup,
					WRK.command_to_do);
				return(0);
			}
		}

		WRK.event_wait |= FUNCT_WAIT;
		WRK.funct_event = EVENT_NULL;
		issue_scb_command(p_dds, ADAP_FUNCT_CMD,
			*(uint *)(WRK.adap_open_opts.func_addr));

		BUWDT.count = 0;
		BUWDT.restart = FUNC_ADDR_TIMEOUT;
		WRK.bu_wdt_cmd = INTR_FUNC_TIMEOUT;
		w_start(&(BUWDT));
	
		while (WRK.event_wait & FUNCT_WAIT) {
			TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);
			e_sleep_thread( &WRK.funct_event, &TX_LOCK,
				LOCK_HANDLER );
			TRACE_DBG(MON_OTHER, "TX_L", (int)p_dds, 0, 0);
		}
	}
	TRACE_BOTH(MON_OTHER, "dsb2", WRK.adap_state, WRK.bringup, 0);
	return(rc);

} /* end disable_address */

/*****************************************************************************/
/*
 * NAME:     enable_address
 *
 * FUNCTION: enables an alternate (function or group) address
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * CALLED FROM: tokioctl
 *
 * INPUT:
 *	p_dds	- pointer to the device control area
 *	arg	- address to enable
 *
 * NOTES:
 *
 * RETURNS:  0
 *
 */
/*****************************************************************************/
enable_address(dds_t	*p_dds,
	caddr_t		arg)
{
	int		rc=0, i, j, temp_func;

	TRACE_BOTH(MON_OTHER, "enb@", (int)p_dds, *((uint *) (arg + 2)), 0);
	ASSERT (arg != 0);
	if (arg == 0) {
		return(EINVAL);
	}
	if (*(uchar *)(arg+2) & GROUP_ADR_MASK) {	/* group address */
		/*
		 * only 1 group address is allowed
		 */
		if (*(uint *)(WRK.adap_open_opts.grp_addr)) {
			return(EINVAL);
		}

		/*
		 * set new group address
		 */
		*(uint *)(WRK.adap_open_opts.grp_addr) = *((uint *) (arg + 2));

		NDD.ndd_flags |= NDD_ALTADDRS+TOK_RECEIVE_GROUP;

		/*
		 * Programs like TCP/IP don't wait for the adapter to be
		 * up before issuing functional/group addresses.  To do so
		 * would slow down IPL.  If the adapter open hasn't been
		 * issued yet, it will get the address as part of the adapter
		 * open and thus don't do anything else.  If the adapter is
		 * not open (but not dead), set a flag to have the command
		 * processed when the adapter is open.
		 */
		if (WRK.adap_state != OPEN_STATE) {
			switch (WRK.adap_state)
			{
			case DEAD_STATE:
			case NULL_STATE:
			case CLOSED_STATE:
			case CLOSE_PENDING:
				TRACE_BOTH(MON_OTHER, "enb2",
					WRK.adap_state, WRK.bringup,
					WRK.command_to_do);
				return(0);
			default: /* OPEN_PENDING or LIMBO_STATE */
				/*
				 * check both states to eliminate any
				 * timing window
				 */
				if ((WRK.bringup == ADAP_INIT_PHASE1) ||
				    (WRK.bringup == OPEN_PHASE0)) {
					WRK.command_to_do = TRUE;
				}
				TRACE_BOTH(MON_OTHER, "enb2",
					WRK.adap_state, WRK.bringup,
					WRK.command_to_do);
				return(0);
			}
		}

		WRK.event_wait |= GROUP_WAIT;
		WRK.group_event = EVENT_NULL;
		issue_scb_command(p_dds, ADAP_GROUP_CMD,
			*(uint *)(WRK.adap_open_opts.grp_addr));

		BUWDT.count = 0;
		BUWDT.restart = GROUP_ADDR_TIMEOUT;
		WRK.bu_wdt_cmd = INTR_GROUP_TIMEOUT;
		w_start(&(BUWDT));
	
		while (WRK.event_wait & GROUP_WAIT) {
			TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);
			e_sleep_thread( &WRK.group_event, &TX_LOCK,
				LOCK_HANDLER );
			TRACE_DBG(MON_OTHER, "TX_L", (int)p_dds, 0, 0);
		}
	} else {	/* functional address */
		temp_func = *(uint *)(WRK.adap_open_opts.func_addr);

		/*
		 * add this address to the functional address mask
		 */
		*(uint *)(WRK.adap_open_opts.func_addr) |=
			*((uint *) (arg + 2));
		TRACE_BOTH(MON_OTHER, "fct@", (int)p_dds,
			*((ulong *)(WRK.adap_open_opts.func_addr)), 0);
	
		NDD.ndd_flags |= NDD_ALTADDRS+TOK_RECEIVE_FUNC;

		/*
		 * keep a reference count on each of the bits in the address
		 */
		for (i = 0, j = 1; i < 31; i++, j *= 2) {
			if ( *((uint *) (arg + 2)) & j) {
				WRK.func_addr_ref[i]++;
			}
		}

		/*
		 * check if it is necessary to issue the set addr command
		 */
		if ( *(uint *)(WRK.adap_open_opts.func_addr) == temp_func ) {
			TRACE_BOTH(MON_OTHER, "enb2", WRK.adap_state,
				WRK.bringup, WRK.command_to_do);
			return(0);
		}

		/*
		 * Programs like TCP/IP don't wait for the adapter to be
		 * up before issuing functional/group addresses.  To do so
		 * would slow down IPL.  If the adapter open hasn't been
		 * issued yet, it will get the address as part of the adapter
		 * open and thus don't do anything else.  If the adapter is
		 * not open (but not dead), set a flag to have the command
		 * processed when the adapter is open.
		 */
		if (WRK.adap_state != OPEN_STATE) {
			switch (WRK.adap_state)
			{
			case DEAD_STATE:
			case NULL_STATE:
			case CLOSED_STATE:
			case CLOSE_PENDING:
				TRACE_BOTH(MON_OTHER, "enb2",
					WRK.adap_state, WRK.bringup,
					WRK.command_to_do);
				return(0);
			default: /* OPEN_PENDING or LIMBO_STATE */
				/*
				 * check both states to eliminate any
				 * timing window
				 */
				if ((WRK.bringup == ADAP_INIT_PHASE1) ||
				    (WRK.bringup == OPEN_PHASE0)) {
					WRK.command_to_do = TRUE;
				}
				TRACE_BOTH(MON_OTHER, "enb2",
					WRK.adap_state, WRK.bringup,
					WRK.command_to_do);
				return(0);
			}
		}
	
		WRK.event_wait |= FUNCT_WAIT;
		WRK.funct_event = EVENT_NULL;
		issue_scb_command(p_dds, ADAP_FUNCT_CMD,
			*(uint *)(WRK.adap_open_opts.func_addr));

		BUWDT.count = 0;
		BUWDT.restart = FUNC_ADDR_TIMEOUT;
		WRK.bu_wdt_cmd = INTR_FUNC_TIMEOUT;
		w_start(&(BUWDT));
	
		while (WRK.event_wait & FUNCT_WAIT) {
			TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);
			e_sleep_thread( &WRK.funct_event, &TX_LOCK,
				LOCK_HANDLER );
			TRACE_DBG(MON_OTHER, "TX_L", (int)p_dds, 0, 0);
		}
	}
	TRACE_BOTH(MON_OTHER, "enb2", WRK.adap_state, WRK.bringup,
		WRK.command_to_do);
	return(rc);

} /* end enable_address */

#ifdef TOK_FREEZE_DUMP
/*
 *  do a freeze dump of the adapter.
 */
/*****************************************************************************/
/*
 * NAME:     freeze_dump
 *
 * FUNCTION: "freezes" the adapter and then dumps its memory
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * CALLED FROM: tokioctl
 *
 * INPUT:
 *	p_dds	- pointer to the device control area
 *
 * NOTES: THIS IS CONDITIONALLY COMPILED DEBUG CODE
 *
 * RETURNS:  none
 */
/*****************************************************************************/
void
freeze_dump(
	dds_t	*p_dds)
{
	ushort *buffer;
	int    len, ioa;

	/*
	 * only do a freeze_dump once
	*/
	if (WRK.freeze_dump)
		return;
	WRK.freeze_dump = 1;

	NDD.ndd_flags &= ~(NDD_RUNNING | NDD_LIMBO);
	NDD.ndd_flags |= NDD_DEAD;
	WRK.adap_state = DEAD_STATE;

	/*
	 * "freeze" the adapter
	 */
        ioa = BUSIO_ATT( DDI.bus_id, DDI.io_port );
	TOK_PUTSRX( ioa + ADDRESS_REG, 0x00AA);
	BUSIO_DET( ioa );

	hwreset(p_dds);

        ioa = BUSIO_ATT( DDI.bus_id, DDI.io_port );

	TOK_PUTSRX( ioa + ADDRESS_REG, 0);

	buffer = WRK.freeze_data;
	len = TOK_FREEZE_DUMP_SIZE;
	while (len > 0)
	{
		TOK_GETSRX( ioa + AUTOINCR_REG, buffer);
		len -= 2;
		++buffer;
	}
	BUSIO_DET( ioa );

} /* End freeze_dump block */
#endif

#ifdef TOK_ADAP_ADDR
/*****************************************************************************/
/*
 * NAME:     get_adap_addr
 *
 * FUNCTION: reads the adapter memory to get the adapter addresses
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * CALLED FROM: tokioctl
 *
 * INPUT:
 *	p_dds	- pointer to the device control area
 *
 * NOTES: THIS IS CONDITIONALLY COMPILED DEBUG CODE
 *
 * RETURNS:  0 - adapter is up and running OK
 *	     ENETDOWN - adapter is dead
 *	     ENETUNREACH - adapter is in any other state
 */
/*****************************************************************************/
int
get_adap_addr(dds_t	*p_dds)
{
	tok_adap_addr_t	tmpaddr;
	int	rc;

	TRACE_BOTH(MON_OTHER, "getA", (int)p_dds, 0, 0);
	if (WRK.adap_state != OPEN_STATE) {
		if (WRK.adap_state == DEAD_STATE) {
			return(ENETDOWN);
		} else {
			return(ENETUNREACH);
		}
	}

	tmpaddr.adap_node_addr[0] = sizeof(tok_adap_addr_t);
	tmpaddr.adap_node_addr[1] = WRK.p_a_addrs;

	if (WRK.do_dkmove) {
		d_kmove (&tmpaddr,
			WRK.p_d_adap_addr,
			sizeof(tok_adap_addr_t),
			WRK.dma_chnl_id,
			DDI.bus_id,
			DMA_WRITE_ONLY);
	} else {
		bcopy (&tmpaddr, WRK.p_adap_addr, sizeof(tok_adap_addr_t));
	}


	WRK.event_wait |= READ_ADAP_WAIT + READ_ADAP_ADDR;
	WRK.read_adap_event = EVENT_NULL;
	issue_scb_command (p_dds, ADAP_READ_CMD, WRK.p_d_adap_addr);

	BUWDT.count = 0;
	BUWDT.restart = READ_ADAP_TIMEOUT;
	WRK.bu_wdt_cmd = INTR_READ_ADAP_TIMEOUT;
	w_start(&(BUWDT));
	
	while (WRK.event_wait & READ_ADAP_WAIT) {
		TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);
		e_sleep_thread( &WRK.read_adap_event, &TX_LOCK, LOCK_HANDLER );
		TRACE_DBG(MON_OTHER, "TX_L", (int)p_dds, 0, 0);
	}
	WRK.event_wait &= ~READ_ADAP_ADDR;
	return(0);

} /* end get_adap_addr */
#endif

/*****************************************************************************/
/*
 * NAME:     get_addr
 *
 * FUNCTION: returns the receive addresses for this device
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * CALLED FROM: tokioctl
 *
 * INPUT:
 *	p_dds	- pointer to the device control area
 *	arg	- where to write the data
 *	length	- length of the area
 *
 * NOTES:
 *
 * RETURNS:  0
 */
/*****************************************************************************/
int
get_addr(dds_t	*p_dds, caddr_t	arg, int length)
{
	ndd_mib_addr_t  *p_table = (ndd_mib_addr_t *)arg;
	ndd_mib_addr_elem_t  *p_elem;
	int elem_len = 6 + CTOK_NADR_LENGTH;
	int count = 0, rc = 0;
	uchar broad_adr1[CTOK_NADR_LENGTH] =
		{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	uchar broad_adr2[CTOK_NADR_LENGTH] = {0xC0, 0, 0xFF, 0xFF, 0xFF, 0xFF};

	ASSERT (arg != 0);
	if (arg == 0) {
		return(EINVAL);
	}
	if (length < sizeof(ndd_mib_addr_t)) {
		return(EINVAL);
	}

	length -= sizeof(u_int);   /* room for count field */
	arg += sizeof(u_int);

	/* copy the specific network address in use first */
	if (length >= elem_len) {
		p_elem = (ndd_mib_addr_elem_t *)arg;
		p_elem->status = NDD_MIB_VOLATILE;
		p_elem->addresslen = CTOK_NADR_LENGTH;
		COPY_NADR(WRK.adap_open_opts.node_addr, p_elem->address);
		length -= elem_len;
		arg += elem_len;
		count++;
	} else {
		rc = E2BIG;
	}

	/* copy the first broadcast address */
	if (length >= elem_len) {
		p_elem = (ndd_mib_addr_elem_t *)arg;
		p_elem->status = NDD_MIB_NONVOLATILE;
		p_elem->addresslen = CTOK_NADR_LENGTH;
		COPY_NADR(broad_adr1, p_elem->address);
		length -= elem_len;
		arg += elem_len;
		count++;
	} else {
		rc = E2BIG;
	}

	/* copy the second broadcast address */
	if (length >= elem_len) {
		p_elem = (ndd_mib_addr_elem_t *)arg;
		p_elem->status = NDD_MIB_NONVOLATILE;
		p_elem->addresslen = CTOK_NADR_LENGTH;
		COPY_NADR(broad_adr2, p_elem->address);
		length -= elem_len;
		arg += elem_len;
		count++;
	} else {
		rc = E2BIG;
	}

	if ( NDD.ndd_flags & TOK_RECEIVE_FUNC) {
		/* copy the functional address */
		if (length >= elem_len) {
			p_elem = (ndd_mib_addr_elem_t *)arg;
			p_elem->status = NDD_MIB_VOLATILE;
			p_elem->addresslen = CTOK_NADR_LENGTH;
			*(char *)(p_elem->address) = 0xC0;
			*(char *)(p_elem->address + 1) = 0x00;
			*(ulong *)((char *)(p_elem->address + 2)) = 
			   *((ulong *)(WRK.adap_open_opts.func_addr));
			length -= elem_len;
			arg += elem_len;
			count++;
		} else {
			rc = E2BIG;
		}
	}

	if ( NDD.ndd_flags & TOK_RECEIVE_GROUP) {
		/* copy the group address */
		if (length >= elem_len) {
			p_elem = (ndd_mib_addr_elem_t *)arg;
			p_elem->status = NDD_MIB_VOLATILE;
			p_elem->addresslen = CTOK_NADR_LENGTH;
			*(char *)(p_elem->address) = 0xC0;
			*(char *)(p_elem->address + 1) = 0x00;
			*(ulong *)((char *)(p_elem->address + 2)) = 
			   *((ulong *)(WRK.adap_open_opts.grp_addr));
			length -= elem_len;
			arg += elem_len;
			count++;
		} else {
			rc = E2BIG;
		}
	}
		
        /* put the final count into the buffer */
        p_table->count = count;
	return( rc );
} /* end get_addr */

#ifdef TOK_ADAP_ADDR
/*****************************************************************************/
/*
 * NAME:     get_addr2
 *
 * FUNCTION: returns the receive addresses for this device
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * CALLED FROM: tokioctl
 *
 * INPUT:
 *	p_dds	- pointer to the device control area
 *	arg	- where to write the data
 *	length	- length of the area
 *
 * NOTES: THIS IS CONDITIONALLY COMPILED DEBUG CODE
 *
 * RETURNS:  0
 */
/*****************************************************************************/
int
get_addr2(dds_t	*p_dds, caddr_t	arg, int length)
{
	ndd_mib_addr_t  *p_table = (ndd_mib_addr_t *)arg;
	ndd_mib_addr_elem_t  *p_elem;
	int elem_len = 6 + CTOK_NADR_LENGTH;
	int rc, count = 0;
	uchar broad_adr1[CTOK_NADR_LENGTH] =
		{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	uchar broad_adr2[CTOK_NADR_LENGTH] = {0xC0, 0, 0xFF, 0xFF, 0xFF, 0xFF};

	if (arg == 0) {
		return(EINVAL);
	}
	if (length < sizeof(ndd_mib_addr_t)) {
		return(EINVAL);
	}

	/*
	 * read addresses from adapter
	 */
	if (rc = get_adap_addr( p_dds )) {
		return(rc);
	}

	length -= sizeof(u_int);   /* room for count field */
	arg += sizeof(u_int);

	/* copy the specific network address in use first */
	if (length >= elem_len) {
		p_elem = (ndd_mib_addr_elem_t *)arg;
		p_elem->status = NDD_MIB_VOLATILE;
		p_elem->addresslen = CTOK_NADR_LENGTH;
		bcopy(WRK.adap_addr.adap_node_addr, p_elem->address, 6);
		length -= elem_len;
		arg += elem_len;
		count++;
	}

	/* copy the first broadcast address */
	if (length >= elem_len) {
		p_elem = (ndd_mib_addr_elem_t *)arg;
		p_elem->status = NDD_MIB_NONVOLATILE;
		p_elem->addresslen = CTOK_NADR_LENGTH;
		COPY_NADR(broad_adr1, p_elem->address);
		length -= elem_len;
		arg += elem_len;
		count++;
	}

	/* copy the second broadcast address */
	if (length >= elem_len) {
		p_elem = (ndd_mib_addr_elem_t *)arg;
		p_elem->status = NDD_MIB_NONVOLATILE;
		p_elem->addresslen = CTOK_NADR_LENGTH;
		COPY_NADR(broad_adr2, p_elem->address);
		length -= elem_len;
		arg += elem_len;
		count++;
	}

	/* copy the functional address */
	if (length >= elem_len) {
		p_elem = (ndd_mib_addr_elem_t *)arg;
		p_elem->status = NDD_MIB_VOLATILE;
		p_elem->addresslen = CTOK_NADR_LENGTH;
		*(char *)(p_elem->address) = 0xC0;
		*(char *)(p_elem->address + 1) = 0x00;
		bcopy(WRK.adap_addr.adap_funct_addr, p_elem->address +2, 4);
		length -= elem_len;
		arg += elem_len;
		count++;
	}

	/* copy the group address */
	if (length >= elem_len) {
		p_elem = (ndd_mib_addr_elem_t *)arg;
		p_elem->status = NDD_MIB_VOLATILE;
		p_elem->addresslen = CTOK_NADR_LENGTH;
		*(char *)(p_elem->address) = 0xC0;
		*(char *)(p_elem->address + 1) = 0x00;
		bcopy(WRK.adap_addr.adap_group_addr, p_elem->address +2, 4);
		length -= elem_len;
		arg += elem_len;
		count++;
	}
		
        /* put the final count into the buffer */
        p_table->count = count;
	return(0);
} /* end get_addr2 */
#endif

/*****************************************************************************/
/*
 * NAME:     get_mibs
 *
 * FUNCTION: gathers all the mib variables and puts them in the MIB struct
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * CALLED FROM: tokioctl
 *
 * INPUT:
 *	p_dds	- pointer to the device control area
 *
 * NOTES:
 *
 * RETURNS:  void
 *
 */
/*****************************************************************************/
void
get_mibs(dds_t	*p_dds, token_ring_all_mib_t *arg)
{
	/*
	 * read error log from adapter to get statistics
	 */
	get_stats( p_dds );

	bzero (arg, sizeof(token_ring_all_mib_t));

	bcopy(TR_MIB_IBM16, arg->Generic_mib.ifExtnsEntry.chipset,
		CHIPSETLENGTH);
   	arg->Generic_mib.ifExtnsEntry.mcast_tx_ok = TOKSTATS.mcast_xmit;
   	arg->Generic_mib.ifExtnsEntry.bcast_tx_ok = TOKSTATS.bcast_xmit;
   	arg->Generic_mib.ifExtnsEntry.mcast_rx_ok = TOKSTATS.mcast_recv;
   	arg->Generic_mib.ifExtnsEntry.bcast_rx_ok = TOKSTATS.bcast_recv;
	arg->Generic_mib.ifExtnsEntry.promiscuous = PROMFALSE;

	arg->Generic_mib.RcvAddrTable = 3;
	if ( NDD.ndd_flags & TOK_RECEIVE_FUNC) {
		arg->Generic_mib.RcvAddrTable++;
	}
	if ( NDD.ndd_flags & TOK_RECEIVE_GROUP) {
		arg->Generic_mib.RcvAddrTable++;
	}

	arg->Token_ring_mib.Dot5Entry.ring_status = TR_MIB_NOPROBLEM;
	arg->Token_ring_mib.Dot5Entry.ring_state = TR_MIB_OPENED;
	arg->Token_ring_mib.Dot5Entry.ring_ostatus = TR_MIB_LASTOPEN;

	if (DDI.ring_speed) {
		arg->Token_ring_mib.Dot5Entry.ring_speed =
			TR_MIB_SIXTEENMEGABIT;
	} else {
		arg->Token_ring_mib.Dot5Entry.ring_speed = TR_MIB_FOURMEGABIT;
	}

	/*
	 * get the upstream neighbor's address (NAUN)
	 */
	if (get_ring_info( p_dds )) {
		bzero( arg->Token_ring_mib.Dot5Entry.upstream,
			CTOK_NADR_LENGTH);
	} else {
		COPY_NADR(WRK.ring_info.upstream_node_addr,
			arg->Token_ring_mib.Dot5Entry.upstream);
	}

	arg->Token_ring_mib.Dot5Entry.participate = TR_MIB_TRUE;

	arg->Token_ring_mib.Dot5Entry.functional[0] = 0xC0;
	arg->Token_ring_mib.Dot5Entry.functional[1] = 0x00;
	arg->Token_ring_mib.Dot5Entry.functional[2] = 
		WRK.adap_open_opts.func_addr[0];
	arg->Token_ring_mib.Dot5Entry.functional[3] = 
		WRK.adap_open_opts.func_addr[1];
	arg->Token_ring_mib.Dot5Entry.functional[4] = 
		WRK.adap_open_opts.func_addr[2];
	arg->Token_ring_mib.Dot5Entry.functional[5] = 
		WRK.adap_open_opts.func_addr[3];

	arg->Token_ring_mib.Dot5StatsEntry.line_errs = TOKSTATS.line_errs;
	arg->Token_ring_mib.Dot5StatsEntry.burst_errs = TOKSTATS.burst_errs;
	arg->Token_ring_mib.Dot5StatsEntry.ac_errs = TOKSTATS.ac_errs;
	arg->Token_ring_mib.Dot5StatsEntry.abort_errs = TOKSTATS.abort_errs;
	arg->Token_ring_mib.Dot5StatsEntry.int_errs = TOKSTATS.int_errs;
	arg->Token_ring_mib.Dot5StatsEntry.lostframes = TOKSTATS.lostframes;
	arg->Token_ring_mib.Dot5StatsEntry.rx_congestion =
		TOKSTATS.rx_congestion;
	arg->Token_ring_mib.Dot5StatsEntry.framecopies = TOKSTATS.framecopies;
	arg->Token_ring_mib.Dot5StatsEntry.token_errs = TOKSTATS.token_errs;
	arg->Token_ring_mib.Dot5StatsEntry.soft_errs = TOKSTATS.soft_errs;
	arg->Token_ring_mib.Dot5StatsEntry.hard_errs = TOKSTATS.hard_errs;
	arg->Token_ring_mib.Dot5StatsEntry.signal_loss = TOKSTATS.signal_loss;
	arg->Token_ring_mib.Dot5StatsEntry.tx_beacons = TOKSTATS.tx_beacons;
	arg->Token_ring_mib.Dot5StatsEntry.recoverys = TOKSTATS.recoverys;
	arg->Token_ring_mib.Dot5StatsEntry.lobewires = TOKSTATS.lobewires;
	arg->Token_ring_mib.Dot5StatsEntry.removes = TOKSTATS.removes;
	arg->Token_ring_mib.Dot5StatsEntry.singles = TOKSTATS.singles;
}

/*****************************************************************************/
/*
 * NAME:     get_ring_info
 *
 * FUNCTION: reads the adapter memory to get ring information
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * CALLED FROM: tokioctl
 *
 * INPUT:
 *	p_dds	- pointer to the device control area
 *
 * NOTES:
 *
 * RETURNS:  0 - adapter is up and running OK
 *	     ENETUNREACH - adapter is in any other state
 */
/*****************************************************************************/
int
get_ring_info(dds_t	*p_dds)
{
	tok_ring_info_t	tmpinfo;
	int	rc;

	if (WRK.adap_state != OPEN_STATE) {
		return(ENETUNREACH);
	}

	tmpinfo.adap_phys_addr[0] = sizeof(tok_ring_info_t);
	tmpinfo.adap_phys_addr[1] = WRK.p_a_parms;

	if (WRK.do_dkmove) {
		d_kmove (&tmpinfo,
			WRK.p_d_ring_info,
			sizeof(tok_ring_info_t),
			WRK.dma_chnl_id,
			DDI.bus_id,
			DMA_WRITE_ONLY);
	} else {
		bcopy (&tmpinfo, WRK.p_ring_info, sizeof(tok_ring_info_t));
	}

	WRK.event_wait |= READ_ADAP_WAIT;
	WRK.read_adap_event = EVENT_NULL;
	issue_scb_command (p_dds, ADAP_READ_CMD, WRK.p_d_ring_info);

	BUWDT.count = 0;
	BUWDT.restart = READ_ADAP_TIMEOUT;
	WRK.bu_wdt_cmd = INTR_READ_ADAP_TIMEOUT;
	w_start(&(BUWDT));
	
	while (WRK.event_wait & READ_ADAP_WAIT) {
		TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);
		e_sleep_thread( &WRK.read_adap_event, &TX_LOCK, LOCK_HANDLER );
		TRACE_DBG(MON_OTHER, "TX_L", (int)p_dds, 0, 0);
	}
	return(0);

} /* end get_ring_info */

/*****************************************************************************/
/*
 * NAME:     get_stats
 *
 * FUNCTION: reads the error log and adapter memory to get statistics
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * CALLED FROM: tokioctl
 *
 * INPUT:
 *	p_dds	- pointer to the device control area
 *
 * NOTES:
 *
 * RETURNS:  void
 */
/*****************************************************************************/
void
get_stats(dds_t	*p_dds)
{
	TRACE_BOTH(MON_OTHER, "getS", (int)p_dds, 0, 0);

	NDD.ndd_genstats.ndd_elapsed_time =
		NDD_ELAPSED_TIME(WRK.ndd_stime);
	TOKSTATS.dev_elapsed_time = NDD_ELAPSED_TIME(WRK.dev_stime);
	TOKSTATS.ndd_flags = NDD.ndd_flags;
	TOKSTATS.sw_txq_len = WRK.xmits_queued;
	TOKSTATS.hw_txq_len = WRK.xmits_adapter;
	NDD.ndd_genstats.ndd_xmitque_cur =
		TOKSTATS.sw_txq_len + TOKSTATS.hw_txq_len;

	if (WRK.adap_state != OPEN_STATE) {
		return;
	}

	WRK.event_wait |= ELOG_WAIT;
	WRK.elog_event = EVENT_NULL;
	issue_scb_command (p_dds, ADAP_ELOG_CMD, WRK.p_d_errlog);

	BUWDT.count = 0;
	BUWDT.restart = ELOG_CMD_TIMEOUT;
	WRK.bu_wdt_cmd = INTR_ELOG_TIMEOUT;
	w_start(&(BUWDT));
	
	while (WRK.event_wait & ELOG_WAIT) {
		TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);
		e_sleep_thread( &WRK.elog_event, &TX_LOCK, LOCK_HANDLER );
		TRACE_DBG(MON_OTHER, "TX_L", (int)p_dds, 0, 0);
	}

	TOKSTATS.line_errs += WRK.adap_err_log.line_err_count;
	TOKSTATS.burst_errs += WRK.adap_err_log.burst_err_count;
	TOKSTATS.abort_errs += WRK.adap_err_log.abort_del_err_count;
	TOKSTATS.int_errs += WRK.adap_err_log.internal_err_count;
	TOKSTATS.lostframes += WRK.adap_err_log.lost_frame_err_count;
	TOKSTATS.rx_congestion += WRK.adap_err_log.rec_cong_err_count;
	NDD.ndd_genstats.ndd_ierrors += WRK.adap_err_log.rec_cong_err_count;
	TOKSTATS.framecopies += WRK.adap_err_log.frame_cpy_err_count;
	TOKSTATS.token_errs += WRK.adap_err_log.token_err_count;
	DEVSTATS.ARI_FCI_errors += WRK.adap_err_log.ari_fci_err_count;
	DEVSTATS.DMA_bus_errors += WRK.adap_err_log.dma_bus_err_count;
	DEVSTATS.DMA_parity_errors += WRK.adap_err_log.dma_parity_err_count;
} /* end get_stats */
