static char sccsid[] = "@(#)21	1.17  src/bos/kernext/tok/trmon_dd.c, sysxtok, bos411, 9428A410j 5/28/94 11:05:27";
/*
 *   COMPONENT_NAME: SYSXTOK
 *
 *   FUNCTIONS:	cfg_pos_regs
 *		config_init
 *		config_term
 *		tok_gen_crc
 *		tokconfig
 *		tokdsgetvpd
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
 * Fixed storage area for the global device driver control structure.
 * Initialize the lockl lock (mon_cfg_lock) in the beginning of the structure
 * to LOCK_AVAIL for synchronizing the config commands.
 */
dd_ctrl_t mon_dd_ctrl = {LOCK_AVAIL};

/*
 * global flag for mon_dd_ctrl initialization control
 */
int	dd_init_state = FALSE;

/*
 * MIB status table - this table defines the MIB variable status returned
 * on NDD_MIB_QUERY operation
 */
token_ring_all_mib_t mon_mib_status = {
/* Generic Interface Extension Table */
	MIB_READ_ONLY,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* ifExtnsChipSet */
	MIB_NOT_SUPPORTED,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* ifExtnsRevWare */
	MIB_READ_ONLY,			/* ifExtnsMulticastsTransmittedOks */
	MIB_READ_ONLY,			/* ifExtnsBroadcastsTransmittedOks */
	MIB_READ_ONLY,			/* ifExtnsMulticastsReceivedOks */
	MIB_READ_ONLY,			/* ifExtnsBroadcastsReceivedOks */
	MIB_NOT_SUPPORTED,		/* ifExtnsPromiscuous */
/* Generic Interface Test Table */
	MIB_NOT_SUPPORTED,		/* ifEXtnsTestCommunity */
	MIB_NOT_SUPPORTED,		/* ifEXtnsTestRequestId */
	MIB_NOT_SUPPORTED,		/* ifEXtnsTestType */
	MIB_NOT_SUPPORTED,		/* ifEXtnsTestResult */
	MIB_NOT_SUPPORTED,		/* ifEXtnsTestCode */
/* Generic Receive Address Table */
	MIB_READ_ONLY,			/* RcvAddrTable */
/* Token-Ring status and parameter values group */
	MIB_NOT_SUPPORTED,		/* dot5Commands */ 
	MIB_READ_ONLY,			/* dot5RingStatus */
	MIB_READ_ONLY,			/* dot5RingState */
	MIB_READ_ONLY,			/* dot5RingOpenStatus */
	MIB_READ_ONLY,			/* dot5RingSpeed */
	MIB_READ_ONLY,0,0,0,0,0,	/* dot5UpStream */
	MIB_READ_ONLY,			/* dot5ActMonParticipate */
	MIB_READ_ONLY,0,0,0,0,0,	/* dot5Functional */
/* Token-Ring Statistics group */
	MIB_READ_ONLY,			/* dot5StatsLineErrors */
	MIB_READ_ONLY,			/* dot5StatsBurstErrors */
	MIB_NOT_SUPPORTED,		/* dot5StatsACErrors */
	MIB_READ_ONLY,			/* dot5StatsAbortTransErrors */
	MIB_READ_ONLY,			/* dot5StatsInternalErrors */
	MIB_READ_ONLY,			/* dot5StatsLostFrameErrors */
	MIB_READ_ONLY,			/* dot5StatsReceiveCongestions */
	MIB_READ_ONLY,			/* dot5StatsFrameCopiedErrors */
	MIB_READ_ONLY,			/* dot5StatsTokenErrors */
	MIB_READ_ONLY,			/* dot5StatsSoftErrors */
	MIB_READ_ONLY,			/* dot5StatsHardErrors */
	MIB_READ_ONLY,			/* dot5StatsSingalLoss */
	MIB_READ_ONLY,			/* dot5StatsTransmitBeacons */
	MIB_READ_ONLY,			/* dot5StatsRecoverys */
	MIB_READ_ONLY,			/* dot5StatsLobeWires */
	MIB_READ_ONLY,			/* dot5StatsRemoves */
	MIB_READ_ONLY,			/* dot5StatsSingles */
	MIB_NOT_SUPPORTED,		/* dot5StatsFreqErrors */
/* Token-Ring Timer group */
	MIB_NOT_SUPPORTED,		/* dot5TimerReturnRepeat */
	MIB_NOT_SUPPORTED,
	MIB_NOT_SUPPORTED,
	MIB_NOT_SUPPORTED,
	MIB_NOT_SUPPORTED,
	MIB_NOT_SUPPORTED,
	MIB_NOT_SUPPORTED,
	MIB_NOT_SUPPORTED,
	MIB_NOT_SUPPORTED,
	MIB_NOT_SUPPORTED,
};


/*****************************************************************************/
/*
 * NAME:     tokconfig
 *
 * FUNCTION: tokconfig entry point from kernel
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *
 */
/*****************************************************************************/
int
tokconfig (
    int         cmd,    /* operation desired (INIT, TERM, QVPD, DWNLD) */
    struct uio *p_uio)   /* pointer to uio structure */
{
    int rc;
    ndd_config_t	ndd_config;     /* config information           */
    dds_t		*p_dds;         /* device control structure     */
    dds_t		*index_ptr;
    tr_mon_dds_t	*p_ddi;
    tr_mon_dds_t	tempddi;
    int			dds_size;
    int			ddpri;

    /*
     * pincode to avoid problem with disable_lock in trace routine
     */
    if (rc = pincode(tokconfig)) {
	return (rc);
    }

    if (rc = lockl(&CFG_LOCK, LOCK_SHORT)) {
	unpincode(tokconfig);
	return (rc);
    }

    /*
     *  Copy in the ndd_config_t structure.
     */
    if (rc = uiomove((caddr_t) &ndd_config, sizeof(ndd_config_t), UIO_WRITE,
			p_uio)) {
	unlockl( &CFG_LOCK );
	unpincode(tokconfig);
	return (rc);
    }

    if (!dd_init_state) {
	lock_alloc(&DD_LOCK, LOCK_ALLOC_PIN, MON_DD_LOCK, -1);
	simple_lock_init(&DD_LOCK);
	lock_alloc(&TRACE_LOCK, LOCK_ALLOC_PIN, MON_TRACE_LOCK, -1);
	simple_lock_init(&TRACE_LOCK);

	mon_dd_ctrl.num_devs = 0;
	mon_dd_ctrl.num_opens = 0;
	mon_dd_ctrl.p_dds_head = NULL;

	mon_dd_ctrl.trace.next_entry = 0;

	mon_dd_ctrl.cdt.header._cdt_magic = DMP_MAGIC;
	strcpy (mon_dd_ctrl.cdt.header._cdt_name, DD_NAME_STR);
	mon_dd_ctrl.cdt.header._cdt_len = sizeof(struct cdt_head);
	add_cdt ("dd_ctrl", (char *)(&mon_dd_ctrl),
			 (int)sizeof(mon_dd_ctrl));

	dd_init_state = TRUE;
    }
    
    /*
     *  Look for the dds structure
     */
    p_dds = mon_dd_ctrl.p_dds_head;
    while (p_dds) {
        if (p_dds->seq_number == ndd_config.seq_number) {
	    break;
        }
        p_dds = p_dds->next;
    }

    TRACE_BOTH(MON_OTHER, "CFGb", (int)p_dds, ndd_config.seq_number,
		cmd); 

    switch (cmd) {
    case CFG_INIT:
	/* does the device already exist */
	if (p_dds) {
	    TRACE_BOTH(MON_OTHER, "CFI1", (ulong)EBUSY, 0, 0);
	    rc = EBUSY;
	    break;
	}

	/*
	 * make sure that we don't try to configure too many adapters
	 */
	if (mon_dd_ctrl.num_devs >= TOK_MAX_ADAPTERS) {
	    TRACE_BOTH(MON_OTHER, "CFI2", (ulong)EBUSY, 0, 0);
	    rc = EBUSY;
	    break;
	}
	
	/*
	 * get temporary copy of ddi from user space
	 */
	p_ddi = (tr_mon_dds_t *) ndd_config.dds;
	if (copyin(p_ddi, &tempddi, sizeof(tr_mon_dds_t))) {
	    TRACE_BOTH(MON_OTHER, "CFI3", (ulong)EINVAL, 0, 0);
	    rc = EINVAL;
	    break;
	}
	
	/*
	 * compute size needed for entire dds (depends on ddi contents)
	 * NOTE: 1 is added to the transmit queue size (see config_init)
	 */
	dds_size = sizeof(dds_t);
	dds_size += (tempddi.xmt_que_size +1) * sizeof(xmt_elem_t);
	
	/*
	 * get memory for dds
	 */
	if ((p_dds = (dds_t *) xmalloc ( dds_size, 0, pinned_heap )) == NULL) {
	    /* error not logged because DDS is not set up */
	    TRACE_BOTH(MON_OTHER, "CFI4", (ulong)ENOMEM, 0, 0);
	    rc = ENOMEM;
	    break;
	}
	bzero (p_dds, dds_size);
        p_dds->seq_number = ndd_config.seq_number;

	/*
	 *  copy input attributes into the DDS
	 */
	bcopy (&tempddi, &(DDI), sizeof(DDI));
	WRK.alloc_size = dds_size;
	TRACE_BOTH(MON_OTHER, "DDI1", DDI.bus_type, DDI.bus_id,
		DDI.intr_level);
	TRACE_BOTH(MON_OTHER, "DDI2", DDI.intr_priority, DDI.tcw_bus_mem_addr,
		DDI.dma_arbit_lvl);
	TRACE_BOTH(MON_OTHER, "DDI3", DDI.io_port, DDI.slot, DDI.xmt_que_size);
	TRACE_BOTH(MON_OTHER, "DDI4", DDI.ring_speed, DDI.use_alt_addr,
		DDI.attn_mac);
	TRACE_BOTH(MON_OTHER, "DDI5", DDI.beacon_mac, 0, 0);

	/*
	 * initialize the locks in the dds
	 */
  	lock_alloc(&TX_LOCK, LOCK_ALLOC_PIN, MON_TX_LOCK, p_dds->seq_number);
  	lock_alloc(&SLIH_LOCK, LOCK_ALLOC_PIN, MON_SLIH_LOCK, 
		p_dds->seq_number);
	TRACE_BOTH(MON_OTHER, "SL_A", &TX_LOCK, 0, 0);
	TRACE_BOTH(MON_OTHER, "SL_A", &SLIH_LOCK, 0, 0);
	simple_lock_init(&TX_LOCK);
	simple_lock_init(&SLIH_LOCK);
	
	/*
	 *  Initialize the DDS
	 */
	if (rc = config_init(p_dds)) {
		TRACE_BOTH(MON_OTHER, "CFI5", rc, 0, 0);
		config_term(p_dds);
	}
	break;

    case CFG_UCODE: /* Download microcode to adapter */
	/*
	 *  make sure device exists
	 */
	if (!p_dds) {
	    TRACE_BOTH(MON_OTHER, "CFU1", (ulong)ENODEV, 0, 0);
	    rc = ENODEV;
	    break;
	}

	/*
	 *  Check to make sure that the device is not open before
	 *  downloading microcode.
	 */
	if (WRK.adap_state != NULL_STATE) {
	    TRACE_BOTH(MON_OTHER, "CFU2", (ulong)EBUSY, 0, 0);
	    rc = EBUSY;
	    break;
	}

	/*
	 *  Go ahead and download microcode
	 */
	if (tokdnld(p_dds, &ndd_config)) {
	    TRACE_BOTH(MON_OTHER, "CFU3", (ulong)EIO, 0, 0);
	    rc = EIO;
	    break;
	}

	/*
	 *  Update the state machine and issue an ns_attach() which may
	 *  return EEXIST if the device is already in the chain
	 */
	WRK.adap_state = CLOSED_STATE;
	if (rc = ns_attach(&NDD)) {
	    TRACE_BOTH(MON_OTHER, "CFU4", rc, 0, 0);
	    break;
        }
	
	break;
	
    case CFG_TERM:
	/*
	 *  make sure device exists
	 */
	if (!p_dds) {
	    TRACE_BOTH(MON_OTHER, "CFT1", (ulong)ENODEV, 0, 0);
	    rc = ENODEV;
	    break;
	}

	/*
	 *  Check if device can be removed and then call ns_detach
	 */
	if ((WRK.adap_state != NULL_STATE &&
	     WRK.adap_state != CLOSED_STATE) ||
	     ns_detach(&NDD)) {
	    TRACE_BOTH(MON_OTHER, "CFT2", (ulong)EBUSY, WRK.adap_state, 0);
	    rc = EBUSY;
	    break;
	}

	/*
	 *  Remove the DDS from the mon_dd_ctrl list and free the resources
	 */
	config_term (p_dds);
	break;
	
    case CFG_QVPD:
	/*
	 *  make sure device exists
	 */
	if (!p_dds) {
	    TRACE_BOTH(MON_OTHER, "CFQ1", (ulong)ENODEV, 0, 0);
	    rc = ENODEV;
	    break;
	}

	if (copyout(&VPD, ndd_config.p_vpd, 
		MIN(ndd_config.l_vpd, sizeof(VPD)))) {
	    TRACE_BOTH(MON_OTHER, "CFQ2", (ulong)EIO, 0, 0);
	    rc = EIO;
	}
	break;
	
    default:
	TRACE_BOTH(MON_OTHER, "CFUN", (ulong)EINVAL, 0, 0);
	rc = EINVAL;

    } /* end switch (cmd) */

    TRACE_BOTH(MON_OTHER, "CFGe", (int)p_dds, rc, 0);
    if (!mon_dd_ctrl.num_devs) {
	del_cdt ("dd_ctrl", (char *)(&mon_dd_ctrl),
			 (int)sizeof(mon_dd_ctrl));
	TRACE_BOTH(MON_OTHER, "SL_F", &DD_LOCK, 0, 0);
	TRACE_BOTH(MON_OTHER, "SL_F", &TRACE_LOCK, 0, 0);
	lock_free (&TRACE_LOCK);
	lock_free (&DD_LOCK);
	dd_init_state = FALSE;
    }

    unlockl( &CFG_LOCK );

    unpincode(tokconfig);

    return (rc);

} /* end of tokconfig() */

/*****************************************************************************/
/*
 * NAME:     config_init
 *
 * FUNCTION: process tokconfig entry with cmd of INIT
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: statistics fields must be initialized in tokopen since
 *	  tokopen zeros the area
 *
 * RETURNS:  0 or errno
 *
 */
/*****************************************************************************/
int
config_init (p_dds)
    dds_t	*p_dds;
{
    int		rc;

    TRACE_DBG(MON_OTHER, "CFIb", (int)p_dds, 0, 0);

    /*
     * Add 1 to the user's request to allow for the circular queue not
     * using one entry (so the head and tail pointer are equal only when
     * the queue is empty).
     */
    DDI.xmt_que_size++;

    /*
     * the device driver uses PL_IMP to serialize with network services
     */
    ASSERT (DDI.intr_priority == PL_IMP);
    if (DDI.intr_priority != PL_IMP) {
	TRACE_BOTH(MON_OTHER, "CFIe", EINVAL, 0, 0);
	mon_dd_ctrl.num_devs++;  /* config_term will decrement this count */
	return( EINVAL );
    }
 
    /*
     * set up the interrupt control structure section
     */
    IHS.next = (struct intr *) NULL;
    IHS.handler = tokslih;
    IHS.bus_type = DDI.bus_type;
    IHS.flags = INTR_NOT_SHARED + INTR_MPSAFE;
    IHS.level = DDI.intr_level;
    IHS.priority = DDI.intr_priority;
    IHS.bid = DDI.bus_id;

    /*
     * Set up the NDD structure for this DDS
     */
    NDD.ndd_name = DDI.lname;
    NDD.ndd_alias = DDI.alias;
    NDD.ndd_flags = NDD_BROADCAST;
    #ifdef DEBUG
	NDD.ndd_flags |= NDD_DEBUG;
    #endif
    NDD.ndd_correlator = (caddr_t)p_dds;
    NDD.ndd_open = tokopen;
    NDD.ndd_close = tokclose;
    NDD.ndd_output = tok_output;
    NDD.ndd_ctl = tokioctl;
    if (DDI.ring_speed) {
	NDD.ndd_mtu = CTOK_16M_MAX_PACKET;
	NDD.ndd_flags |= TOK_RING_SPEED_16;
    } else {
	NDD.ndd_mtu = CTOK_4M_MAX_PACKET;
	NDD.ndd_flags |= TOK_RING_SPEED_4;
    }
    NDD.ndd_mintu = CTOK_MIN_PACKET;
    NDD.ndd_type = NDD_ISO88025;
    NDD.ndd_addrlen = CTOK_NADR_LENGTH;
    NDD.ndd_hdrlen = TOK_NETID_RECV_OFFSET;	/* AC, FC, dest, src, routing*/
    NDD.ndd_specstats = (caddr_t)&(TOKSTATS);
    NDD.ndd_speclen = sizeof(TOKSTATS);

    p_dds->dds_correlator = (int)&mon_dd_ctrl;
    WRK.xmit_queue = (xmt_elem_t *)((uint)p_dds + sizeof(dds_t));
    WRK.adap_state = NULL_STATE;
    WRK.limbo = PARADISE;
    
    WRK.group_event = EVENT_NULL;
    WRK.funct_event = EVENT_NULL;
    WRK.elog_event = EVENT_NULL;
    WRK.read_adap_event = EVENT_NULL;
    
    if (rc = cfg_pos_regs(p_dds)) {
	TRACE_BOTH(MON_OTHER, "indE", rc, 0, 0);
	mon_dd_ctrl.num_devs++;  /* config_term will decrement this count */
	return( rc );
    }
    
    /*
     *   Get the vital product data from the adapter.
     */
    if (rc = tokdsgetvpd(p_dds)) {
	TRACE_BOTH(MON_OTHER, "indE", rc, 0, 0);
	mon_dd_ctrl.num_devs++;  /* config_term will decrement this count */
	return( rc );
    }

    /*
     *  Set up adapter open options
     */
    cfg_adap_parms(p_dds);
    
    /*
     * update device driver controls
     */
    p_dds->next = mon_dd_ctrl.p_dds_head;
    mon_dd_ctrl.p_dds_head = p_dds;
    mon_dd_ctrl.num_devs++;

    /*
     * add dds to component dump table
     */
    add_cdt ("DDS", (char *)p_dds, WRK.alloc_size);

    TRACE_DBG(MON_OTHER, "CFIe", 0, 0, 0);
    return (0);
} /* end config_init */

/*
 * NAME:     config_term
 *
 * FUNCTION: process tokconfig entry with cmd of TERM
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  void
 *
 */
void
config_term (dds_t *p_dds)
{
    dds_t	*prev_ptr;
    dds_t	*index_ptr;
    
    /*
     *  config_term begin
     */
    TRACE_DBG(MON_OTHER, "CFTb", (ulong)p_dds, 0, 0);
    
    mon_dd_ctrl.num_devs--;

    /*
     *  Remove this adapter structure from the adapter chain.
     */
    prev_ptr = index_ptr = mon_dd_ctrl.p_dds_head;
    while (index_ptr != NULL) {
	if (index_ptr == p_dds) {
	    if (mon_dd_ctrl.p_dds_head == index_ptr) {
		mon_dd_ctrl.p_dds_head = mon_dd_ctrl.p_dds_head->next;
	    } else {
		prev_ptr->next = index_ptr->next;
	    }
	    break;
	}
	prev_ptr = index_ptr;
	index_ptr = index_ptr->next;
    }
    
    /*
     *  delete dds from component dump table (if present)
     */
    del_cdt ("DDS", (char *)p_dds, WRK.alloc_size);

    /*
     * free the locks in the dds area
     */
    lock_free(&TX_LOCK);
    lock_free(&SLIH_LOCK);
    TRACE_BOTH(MON_OTHER, "SL_F", &TX_LOCK, 0, 0);
    TRACE_BOTH(MON_OTHER, "SL_F", &SLIH_LOCK, 0, 0);
    
    /*
     *  give back the dds memory
     */
    xmfree( p_dds, pinned_heap );
    
    TRACE_DBG(MON_OTHER, "CFTe", 0, 0, 0);
    return;
} /* end config_term */
    
/*
 * NAME: cfg_pos_regs
 *
 * FUNCTION:
 *
 *      Configures the POS registers for the Token-Ring adapter.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes on the process thread.
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *      None.
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *      None.
 */

int
cfg_pos_regs(dds_t *p_dds)
{
	ushort	pio_addr;	/* PIO address */
	ushort	pos_intr;	/* POS interrupt level setting */
	uchar	adtp4, adtp5;	/* temp POS4 and POS5 values */
	int	iocc;
    
        iocc = (int)IOCC_ATT( (ulong)DDI.bus_id,
			(ulong)(IO_IOCC + (DDI.slot << 16)));
    
	TOK_GETPOS( iocc + POS_REG_0, &WRK.cfg_pos[0] );
	TOK_GETPOS( iocc + POS_REG_1, &WRK.cfg_pos[1] );
	if ((WRK.cfg_pos[0] != TOKEN_L) || (WRK.cfg_pos[1] != TOKEN_H)) {
		IOCC_DET( iocc );
		return(EIO);
	}
	
	/*
	 * set PIO addr, interrupt level, ring speed and card
	 * enable by setting value in POS2
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 * |user |      PIO        |      INTR       |card |
	 * |def. |      addr       |      level      |enabl|
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 *    7     6     5     4     3     2     1     0
	 */
	switch( (unsigned int)DDI.io_port)
	{   /* get the PIO registers base address */
	case PIO_86A0:
	    pio_addr = 0;
	    break;
	case PIO_96A0:
	    pio_addr = 1;
	    break;
	case PIO_A6A0:
	    pio_addr = 2;
	    break;
	case PIO_B6A0:
	    pio_addr = 3;
	    break;
	case PIO_C6A0:
	    pio_addr = 4;
	    break;
	case PIO_D6A0:
	    pio_addr = 5;
	    break;
	case PIO_E6A0:
	    pio_addr = 6;
	    break;
	case PIO_F6A0:
	    pio_addr = 7;
	    break;
	}   /* end switch */
	
	switch(DDI.intr_level)
	{   /* get the bus interrupt level */
	case POS_INT_9:             /*  Interrupt Level 9 */
	    pos_intr = 0;
	    break;
	case POS_INT_3:             /*  Interrupt Level 3 */
	    pos_intr = 1;
	    break;
	case POS_INT_4:             /*  Interrupt Level 4 */
	    pos_intr = 2;
	    break;
	case POS_INT_5:             /*  Interrupt Level 5 */
	    pos_intr = 3;
	    break;
	case POS_INT_7:             /*  Interrupt Level 7 */
	    pos_intr = 4;
	    break;
	case POS_INT_10:            /*  Interrupt Level 10 */
	    pos_intr = 5;
	    break;
	case POS_INT_11:            /*  Interrupt Level 11 */
	    pos_intr = 6;
	    break;
	case POS_INT_12:            /*  Interrupt Level 12 */
	    pos_intr = 7;
	    break;
	}   /* end switch */
	
	/*
	 * we turn off the card enable bit in pos reg 2.
	 * The card will be enabled on the first open to the device.
	 */
	
	TOK_PUTPOS( iocc + POS_REG_2,
		  ((pio_addr << 4) |
		   ( (pos_intr << 1) & ~(CARD_ENABLE) ) ));
	
	TOK_GETPOS( iocc + POS_REG_2, &WRK.cfg_pos[2] );
	
	/*
	 *      For POS 3, we can set the Ring Speed (4/16 Mb), the
	 *      Disable Early Token Release, and Include System
	 *      in DMA Recovery.
	 *
	 *      Ring Speed:
	 *              0 = 4Mb
	 *              1 = 16Mb
	 *      Disable Early Token Release:
	 *              0 = ETR at 16Mb
	 *              1 = no ETR
	 *
	 *      DMA Recovery
	 *              0 = Do not participate
	 *              1 = participate
	 *
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 * |          Reserved           |DMA  |Ring |Disab|
	 * |                             |Rcvry|Speed|Etr  |
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 *    7     6     5     4     3     2     1     0
	 */

	/*
	 * set POS 6 & 7 to zero, so that VPD area is not read
	 */
	TOK_PUTPOS( iocc + POS_REG_6, 0);
	TOK_GETPOS( iocc + POS_REG_6, &WRK.cfg_pos[6] );
	TOK_PUTPOS( iocc + POS_REG_7, 0);
	TOK_GETPOS( iocc + POS_REG_7, &WRK.cfg_pos[7] );

	if (DDI.ring_speed) {
		TOK_PUTPOS( iocc + POS_REG_3,
		  ( (DMA_RCVRY) | (RING_SPEED_16) ) );
	} else {
		TOK_PUTPOS( iocc + POS_REG_3, DMA_RCVRY );
	}
	
	TOK_GETPOS( iocc + POS_REG_3, &WRK.cfg_pos[3] );
	
	/*
	 * set arbitration level, fairness enable, time to
	 * free MC after preempt, MC Parity enable,
	 * SFDBKRTN (1=Monitor SFDBKRTN)
	 *  note: fairness is enabled if bit is 0
	 *  +---------+-----+-----+-----+-----+-----+-----+-----+
	 *  |Monitor  |Parit|MC fr|Fairn|     MC Bus Arbit-     |
	 *  |SFDBKRTN |enabl|delay|disab|      ration level     |
	 *  +---------+-----+-----+-----+-----+-----+-----+-----+
	 *      7        6     5     4     3     2     1     0
	 */
	
	adtp4 = ((0x0f & (DDI.dma_arbit_lvl)) |
		 (SFDBKRTN | MC_PARITY_ON | MC_PREEMPT_TIME));
	
	TOK_PUTPOS( iocc + POS_REG_4, adtp4 );
	TOK_GETPOS( iocc + POS_REG_4, &WRK.cfg_pos[4] );
	
	/*
	 *   POS5:
	 *  note: DMA arbitration is allowed if bit is 0
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 * |Chan check |Strea|Block|   Reserved for        |
	 * |& stat. bit|data |Arbit|      future use       |
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 *    7     6     5     4     3     2     1     0
	 *
	 */
	
	adtp5 = (STREAM_DATA | ~(MC_ARBITRATION) );
	
	TOK_PUTPOS( iocc + POS_REG_5, adtp5 );
	TOK_GETPOS( iocc + POS_REG_5, &WRK.cfg_pos[5] );
    
	IOCC_DET( iocc );

	return(WRK.pio_rc);
}  /* end cfg_pos_regs */

/****************************************************************************
 *
 * ROUTINE NAME: tokdsgetvpd
 *
 * DESCRIPTIVE NAME: Device Specific adapter get adapter's vital
 *                   product data
 *
 * FUNCTION: Read and store the adapter's Vital Product Data via POS registers
 *
 * INPUT: DDS pointer - tells which adapter
 *
 * OUTPUT: Vital Product Data stored in the DDS work area along with the
 *         status of the VPD.
 *
 * RETURN:  N/A
 *
 * CALLED FROM: config_init
 *
 * CALLS TO: N/A
 *
 *****************************************************************************
 */

int tokdsgetvpd (dds_t *p_dds)
{
	int    index;
	int    index2;
	int    vpd_found;   /* Vital Product Data header found flag         */
	int    na_found;    /* Network Address found flag                   */
	int    rl_found;    /* ROS Level found flag                         */
	int    crc_valid;   /* CRC valid flag                               */
	int    iocc;
	ushort cal_crc;     /* Calculated value of CRC                      */
	ushort vpd_crc;     /* Actual VPD value of CRC                      */
    
	TRACE_DBG(MON_OTHER, "vpdB", (int)p_dds, 0, 0);
    
	/* Get access to the IOCC to access POS registers                   */
        iocc = (int)IOCC_ATT( (ulong)DDI.bus_id,
			(ulong)(IO_IOCC + (DDI.slot << 16)));
    
	TOK_PUTPOS( iocc + POS_REG_7, 0);
	
	/********************************************************************/
	/* NOTE: "for" loop does not handle VPD greater than 256 bytes      */
	/********************************************************************/
	
	/*
	 *  Get VPD from adapter for the default length
	 */
	for (index=0; index < TOK_VPD_LENGTH; index++) {
	    /*
	     *  Set up the correct address for the VPD read byte
	     */
	    TOK_PUTPOS( iocc + POS_REG_6, (index + 1));
	    TOK_GETPOS( iocc + POS_REG_3, &VPD.vpd[index] );
	}
	
	/*
	 *  Reset POS registers and release access to the IOCC
	 */
	TOK_PUTPOS( iocc + POS_REG_6, 0);
	TOK_PUTPOS( iocc + POS_REG_3, WRK.cfg_pos[3] );
	IOCC_DET( iocc );
	
	if (WRK.pio_rc) {
	    return(WRK.pio_rc);
	}

	/*
	 *  Test some of the Fields of Vital Product Data
	 */
	if ((VPD.vpd[0] == 'V') &&
	    (VPD.vpd[1] == 'P') &&
	    (VPD.vpd[2] == 'D') &&
	    (VPD.vpd[7] == '*'))
	{
	    
	    /*
	     *  Update the Vital Product Data Status - Valid Data
	     */
	    vpd_found = TRUE;
	    na_found  = FALSE;
	    rl_found  = FALSE;
	    crc_valid = FALSE;
	    
	    /*
	     *  Update the Vital Product Data length
	     */
	    VPD.l_vpd = ((2 * ((VPD.vpd[3] << 8) | VPD.vpd[4])) + 7);
	    
	    /*
	     *  Test for which length will be saved - save the smaller
	     */
	    if (VPD.l_vpd > TOK_VPD_LENGTH) {
		
		/* 
		 *  Mismatch on the length - can not test crc - assume 
		 *  crc is good
		 */
		VPD.l_vpd = TOK_VPD_LENGTH;
		crc_valid = TRUE;
		
	    } else {
		
		/*
		 *  Put together the CRC value from the adapter VPD
		 */
		vpd_crc = ((VPD.vpd[5] << 8) | VPD.vpd[6]);
		
		/* 
		 *  One can only verify CRC if one had enough space to save
		 *  it all Verify that the CRC is valid
		 */
		cal_crc = tok_gen_crc(&VPD.vpd[7], (VPD.l_vpd - 7));
		
		/*
		 *  Test if the CRC compares
		 */
		if (vpd_crc == cal_crc) {
		    crc_valid = TRUE;
		}
		
	    }
	    
	    /* 
	     *  Get Network Address and ROS Level
	     */
	    for (index=0; index < VPD.l_vpd; index++) {
		/* 
		 *  Test for the Network Address Header
		 */
		if ((VPD.vpd[(index + 0)] == '*') &&
		    (VPD.vpd[(index + 1)] == 'N') &&
		    (VPD.vpd[(index + 2)] == 'A') &&
		    (VPD.vpd[(index + 3)] ==  5 ))
		{
		    
		    
		    /* 
		     *  Set the Network Address found flag
		     */
		    na_found = TRUE;
		    
		    /* 
		     *  Save Network Address in DDS work section
		     */
		    for (index2 = 0; index2 < CTOK_NADR_LENGTH; index2++) {
			/*
			 *  Store the indexed byte in the DDS Work Section
			 */
			WRK.tok_vpd_addr[index2] =
			    VPD.vpd[(index + 4 + index2)];
		    }
		}
		
		/*
		 *  Test for the ROS Level Header
		 */
		if ((VPD.vpd[(index + 0)] == '*') &&
		    (VPD.vpd[(index + 1)] == 'R') &&
		    (VPD.vpd[(index + 2)] == 'L'))
		{
		    /*
		     *  Set the ROS Level found flag
		     */
		    rl_found = TRUE;
		    
		    /* 
		     *  Save ROS Level in DDS work section
		     */
		    for (index2 = 0; index2 < ROS_LEVEL_SIZE; index2++) {
			/*
			 *  Store the indexed byte in the DDS Work Section
			 */
			WRK.tok_vpd_rosl[index2]
			    = VPD.vpd[(index + 4 + index2)];
			
		    }
		}
	    }
	    
	    /*
	     *  Test the appropriate flags to verify everything is valid
	     */
	    if ((vpd_found == TRUE) &&        /* VPD Header found */
		(na_found  == TRUE) &&        /* Network Address found */
		(rl_found  == TRUE) &&     /* ROS Level found */
		(crc_valid == TRUE))       /*  CRC value is valid */
	    {
		
		/*
		 *  VPD is valid based on the test we know to check
		 */
		VPD.status = TOK_VPD_VALID;

	    } else {
		/*
		 *  VPD failed the test - set the status
		 */
		VPD.status = TOK_VPD_INVALID;
		
	    } /* endif for test if VPD is valid */
	} else {
	    /* 
	     *  Bad Vital Product Data
	     *  Update the Vital Product Data Status
	     */
	    VPD.status = TOK_VPD_INVALID;
	    
	} /* endif test of some of the VPD fields */
    
	TRACE_DBG(MON_OTHER, "vpdE", p_dds, 0, 0);
	return(0);
} /* end tokdsgetvpd */

/*****************************************************************************/
/*
 * NAME:     tok_gen_crc
 *
 * FUNCTION: generate a 16-bit crc value (used to check VPD)
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  16-bit crc value per specs for MCA bus VPD area crc
 *
 */
/*****************************************************************************/
ushort tok_gen_crc (
   register uchar *buf,  /* area with data whose crc value is to be computed */
   register int   len)   /* number of bytes is data area */
{
   register uchar work_msb;
   register uchar work_lsb;
   register uchar value_msb;
   register uchar value_lsb;
   ushort tempshort;

   TRACE_DBG(MON_OTHER, "CRCb", buf, len, 0);

   /* step through the caller's buffer */
   for (value_msb = 0xFF, value_lsb = 0xFF; len > 0; len--)
   {
      value_lsb ^= *buf++;
      value_lsb ^= (value_lsb << 4);

      work_msb = value_lsb >> 1;
      work_lsb = (value_lsb << 7) ^ value_lsb;

      work_lsb = (work_msb << 4) | (work_lsb >> 4);
      work_msb = ((work_msb >> 4) & 0x07) ^ value_lsb;

      value_lsb = work_lsb ^ value_msb;
      value_msb = work_msb;

   } /* end loop to step through the caller's buffer */

   tempshort = ((ushort)value_msb << 8) | value_lsb;
   TRACE_DBG(MON_OTHER, "CRCe", tempshort, 0, 0);
   return(tempshort);
} /* end tok_gen_crc */
