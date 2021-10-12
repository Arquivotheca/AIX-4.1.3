static char sccsid[] = "@(#)15	1.15  src/bos/kernext/tok/trmon_open.c, sysxtok, bos411, 9428A410j 5/26/94 16:58:38";
/*
 *   COMPONENT_NAME: SYSXTOK
 *
 *   FUNCTIONS: get_mem
 *		get_mem_undo
 *		sb_setup
 *		sb_undo
 *		tokopen
 *		tokopen_cleanup
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

/*****************************************************************************/
/*
 * NAME:     tokopen
 *
 * FUNCTION: open entry point from kernel
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
tokopen (
    ndd_t		*p_ndd)
{
    int			rc;
    dds_t		*p_dds;
    int			alloc_size;
    int			iocc;
    uchar		pos2;

    /*
     *  Need to pin the code.
     */
    if (rc = pincode(tokconfig)) {
	return (rc);
    }
    
    /*
     *  Get adapter structure
     */
    p_dds = (dds_t *) (((unsigned long) p_ndd) - offsetof(dds_t, ndd));
    TRACE_BOTH(MON_OTHER, "OPNb", (int)p_dds, 0, 0);

    if (WRK.adap_state != CLOSED_STATE) {
	TRACE_BOTH(MON_OTHER, "OPNe", (int)p_dds, EBUSY, 0);
	unpincode(tokconfig);
	return(EBUSY);
    }
    
    /*
     *  Set up state machine and NDD flags
     */
    NDD.ndd_flags |= NDD_UP;
    WRK.adap_state = OPEN_PENDING;
    WRK.open_status = OPEN_STATUS0;

    /*
     * save the ndd start time for statistics
     */
    WRK.ndd_stime = lbolt;
    bzero(&NDD.ndd_genstats, sizeof(ndd_genstats_t));

    /*
     *  Allocate & setup ACA
     */
    if (rc = get_mem(p_dds)) {
	tokopen_cleanup(p_dds);
	return(rc);
    }
    WRK.open_status = OPEN_STATUS1;
   
#ifdef TOK_FREEZE_DUMP
    /*
     * allocate memory for freeze dump
     */
    WRK.freeze_data = xmalloc(TOK_FREEZE_DUMP_SIZE, PGSHIFT, pinned_heap);
    bzero( WRK.freeze_data, TOK_FREEZE_DUMP_SIZE);
#endif
 
    /*
     *  Setup SSB and SCB
     */
    sb_setup(p_dds);
    
    /*
     *  Setup transmit list
     */
    if (rc = tx_setup(p_dds)) {
	tokopen_cleanup(p_dds);
	return(rc);
    }
    WRK.open_status = OPEN_STATUS2;
    
    d_master(WRK.dma_chnl_id, DMA_READ|DMA_NOHIDE,
	     WRK.p_mem_block, PAGESIZE,
	     &(WRK.mem_block_xmd),
	     WRK.p_d_mem_block);
   
    /*
     * Do a d_kmove of the open options just to determine if the machine
     * is cache consistent and bcopys should be done or if d_kmoves
     * should be done.  (The open options will be copied again later.)
     */

    rc = d_kmove (&(WRK.adap_open_opts),
		  WRK.p_d_open_opts,
		  sizeof(tok_adap_open_options_t),
		  WRK.dma_chnl_id,
		  DDI.bus_id,
		  DMA_WRITE_ONLY);
    if (rc == EINVAL) {
	WRK.do_dkmove = FALSE;
    } else {
	WRK.do_dkmove = TRUE;
    }
 
    /*
     *  Receive setup
     */
    if (rc = tok_recv_setup(p_dds)) {
	tokopen_cleanup(p_dds);
	return(rc);
    }
    WRK.open_status = OPEN_STATUS3;
    
    /*
     *  add our interrupt routine to kernel's interrupt chain
     */
    if (rc = i_init ((struct intr *)(&(IHS)))) {
	tokopen_cleanup(p_dds);
	return(ENOCONNECT);
    }
    WRK.open_status = OPEN_STATUS4;
    
    /* 
     * Enable the card. 
     */
    iocc = (int)IOCC_ATT( (ulong)DDI.bus_id,
	(ulong)(IO_IOCC + (DDI.slot << 16)));
    TOK_GETPOS( iocc + POS_REG_2, &pos2 );
    TOK_PUTPOS( iocc + POS_REG_2, (pos2 | (CARD_ENABLE) ) );
    IOCC_DET( iocc );

    TRACE_DBG(MON_OTHER, "DD_L", (int)p_dds, 0, 0);
    simple_lock(&DD_LOCK);
    /*
     *  first open for any adapter
     */
    if ((++mon_dd_ctrl.num_opens) == 1) {
	/*
	 *  register for dump
	 */
	dmp_add ( ( ( void (*) ())tok_cdt_func) );
    }
    simple_unlock(&DD_LOCK);
    TRACE_DBG(MON_OTHER, "DD_U", (int)p_dds, 0, 0);
    
    /* 
     * activate ("open" or "start" or "connect") the adapter
     */
    WRK.bcon_sb_sent = FALSE;
    WRK.connect_sb_sent = FALSE;
    ds_act (p_dds);

    WRK.open_status = OPEN_COMPLETE;

    TRACE_BOTH(MON_OTHER, "OPNe", (int)p_dds, rc, 0);
    return (rc);
    
} /* end tokopen */

/*****************************************************************************/
/*
 * NAME:     tokopen_cleanup
 *
 * FUNCTION: cleanup for open routine
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *
 */
/*****************************************************************************/
void
tokopen_cleanup(dds_t *p_dds)
{
    TRACE_BOTH(MON_OTHER, "OPCb", p_dds, WRK.open_status, 0);
    /*
     *  Switch on open_status
     */
    switch(WRK.open_status) {
    case OPEN_COMPLETE:
	/*
	 * this cleanup is done in the tokclose routine
	 */

    case OPEN_STATUS4:
	i_clear((struct intr *)(&(IHS)));
	
    case OPEN_STATUS3:
	tok_recv_undo(p_dds);

    case OPEN_STATUS2:
	clean_tx(p_dds);
	tx_undo(p_dds);

	(void) d_complete(WRK.dma_chnl_id, DMA_READ,
			WRK.p_mem_block, PAGESIZE,
			&WRK.mem_block_xmd,
			WRK.p_d_mem_block);

    case OPEN_STATUS1:
	sb_undo(p_dds);

#ifdef TOK_FREEZE_DUMP
	xmfree(WRK.freeze_data, pinned_heap);
#endif

	get_mem_undo(p_dds);

    case OPEN_STATUS0:
	WRK.adap_state = CLOSED_STATE;
	NDD.ndd_flags &=
		~(NDD_UP | NDD_RUNNING | NDD_LIMBO | NDD_ALTADDRS | \
		  TOK_RECEIVE_FUNC | TOK_RECEIVE_GROUP);
    }
    
    TRACE_BOTH(MON_OTHER, "OPCe", p_dds, 0, 0);

    unpincode(tokconfig);

    return;

} /* End of tokopen_cleanup() */

/*--------------------------------------------------------------------*/
/*********************  Get Memory Block                ***************/
/*--------------------------------------------------------------------*/

/*
*  This function will:
*          - xmalloc a PAGESIZE chunk of memory for the Adapter Control Area
*          - zero out the Adapter Control Area
*          - set up DMA for the control area using the LAST TCW
*          - set up the xmem descriptor for the control area
*
*  RETURN CODES:
*      0        - Good return
*      ENOMEM   - Unable to get required memory
*      ENOCONNECT - d_init failed
*/

int get_mem(dds_t *p_dds)

{  /* begin function get_mem */
    int rc;

    /*
     *  Initialize the DMA channel via the d_init() kernel
     *  service routine.  If the d_init() completes successfully
     *  call the d_unmask() kernel service to enable the channel
     *  off.
     */
    
    /* get dma channel id by calling d_init */
    WRK.dma_chnl_id = d_init( DDI.dma_arbit_lvl, MICRO_CHANNEL_DMA,
				DDI.bus_id );
    
    if (WRK.dma_chnl_id == DMA_FAIL) {
        WRK.footprint = D_INIT_FAIL;
        logerr(p_dds, ERRID_CTOK_DEVICE_ERR, __LINE__, __FILE__);
        return(ENOCONNECT);
    }

    /* go ahead and enable the dma channel by callin d_unmask */
    d_unmask(WRK.dma_chnl_id);
    
    /* get Page size chunk of memory */
    WRK.p_mem_block = xmalloc(PAGESIZE, PGSHIFT, pinned_heap);
    
    if (WRK.p_mem_block == NULL)
    {
	(void)d_clear(WRK.dma_chnl_id);
	WRK.footprint = TOK_XMALLOC_FAIL;
	logerr(p_dds, ERRID_CTOK_MEM_ERR, __LINE__, __FILE__);
	return(ENOMEM);
    }
    
    /* set up cross memory descriptor */
    WRK.mem_block_xmd.aspace_id = XMEM_INVAL;
    rc = xmattach( WRK.p_mem_block, PAGESIZE,
		  &(WRK.mem_block_xmd), SYS_ADSPACE);
    if (rc == XMEM_FAIL )
    {
	xmfree(WRK.p_mem_block, pinned_heap);
	(void)d_clear(WRK.dma_chnl_id);
	WRK.footprint = TOK_XMATTACH_FAIL;
        logerr(p_dds, ERRID_CTOK_DEVICE_ERR, __LINE__, __FILE__);
	return(ENOCONNECT);
    }
    
    /*
     *   Setup the Adapter bringup timer structures
     */
    while (w_init ((struct watchdog *)(&(BUWDT))));
    WRK.bu_wdt_inited = TRUE;
    p_dds->bu_wdt_p_dds = p_dds;
    BUWDT.func = bringup_timer;
    
    /*
     *   Setup the transmit timeout timer structures
     */
    while (w_init ((struct watchdog *)(&(XMITWDT))));
    WRK.xmit_wdt_inited = TRUE;
    WRK.xmit_wdt_active = FALSE;
    p_dds->xmit_wdt_p_dds = p_dds;
    XMITWDT.func = xmit_timeout;

    /* ZERO out the control area memory */
    bzero(WRK.p_mem_block, PAGESIZE);
    
    /* set the DMA address of the control block area of memory */
    /* This will force the use of the last available TCW. */
    WRK.p_d_mem_block = (ushort *)( DDI.tcw_bus_mem_addr +
		(NTCW << DMA_L2PSIZE) );
    
    WRK.bugout = NULL;
    
    return(0);
} /* end function get_mem */

/*--------------------------------------------------------------------*/
/*********************  System Block Setup              ***************/
/*--------------------------------------------------------------------*/

/*
*
*  This function will set up the following in the Adapter Control Area (ACA):
*  Each will be placed in the ACA according to their respective ACA BASE
*  offsets defined in tokbits.h.
*
*      SCB - System Control Block
*      SSB - System Status Block
*      Product ID        - Address where the Product ID information is stored
*      Adapter Error Log - Location where the adapter will DMA the adapter
*                          error log counters.
*      Ring Information  - Location where the adapter will DMA the adapter
*                          Token-Ring Information.
*      Receive Chain     - Starting location of the Adapter's Receive Chain
*      Transmit Chain    - Starting location of the Adapter's TX chain.
*
*  Each will be placed on an IOCC Cache (64 byte) boundary.  The Bus address
*  of the memory will be calculated and stored in the work area of the DDS.
*
*  INPUT: Pointer to the DDS
*
*  RETURN CODES:
*
*
*/
void
sb_setup(dds_t *p_dds)

{
    int	rc;
    unsigned short *p_temp; /* used to split the bus address into 2 parts */
    t_tx_list    *p_d_tmp;
    
    p_temp = (ushort *) &p_d_tmp;

    /* SCB setup */
    WRK.p_scb = (t_scb *)( (int)WRK.p_mem_block + ACA_SCB_BASE);
    
    WRK.p_d_scb = (t_scb *)( (int)WRK.p_d_mem_block + ACA_SCB_BASE);
    p_d_tmp = (t_tx_list *)WRK.p_d_scb;
    
    WRK.adap_iparms.scb_add1 = *p_temp++;
    WRK.adap_iparms.scb_add2 = *p_temp--;
    
    
    /* SSB set up */
    
    WRK.p_ssb = (t_ssb *)( (int)WRK.p_mem_block + ACA_SSB_BASE);
    
    WRK.p_d_ssb = (t_ssb *)( (int)WRK.p_d_mem_block + ACA_SSB_BASE);
    p_d_tmp = (t_tx_list *)WRK.p_d_ssb;
    
    WRK.adap_iparms.ssb_add1 = *p_temp++;
    WRK.adap_iparms.ssb_add2 = *p_temp--;
    
    
    
    WRK.p_prod_id = (tok_prod_id_t *)( (int)WRK.p_mem_block +
			  ACA_PROD_ID_BASE);
    
    WRK.p_d_prod_id = (tok_prod_id_t *)( (int)WRK.p_d_mem_block +
			  ACA_PROD_ID_BASE);
    p_d_tmp = (t_tx_list *)WRK.p_d_prod_id;
    
    WRK.adap_open_opts.prod_id_addr1 = *p_temp++;
    WRK.adap_open_opts.prod_id_addr2 = *p_temp--;
    

    /* Adapter Error Log Set up */
    WRK.p_errlog = (tok_adap_error_log_t *)
	    ( (int)WRK.p_mem_block + ACA_ADAP_ERR_LOG_BASE);
    WRK.p_d_errlog = (tok_adap_error_log_t *)
	    ( (int)WRK.p_d_mem_block + ACA_ADAP_ERR_LOG_BASE);

    /* Ring information Set up */
    WRK.p_ring_info = (tok_ring_info_t *)
	    ( (int)WRK.p_mem_block + ACA_RING_INFO_BASE);
    WRK.p_d_ring_info = (tok_ring_info_t *)
	    ( (int)WRK.p_d_mem_block + ACA_RING_INFO_BASE);

    /* Adapter Address Set up */
    WRK.p_adap_addr = (tok_adap_addr_t *)
	    ( (int)WRK.p_mem_block + ACA_ADAP_ADDR_BASE);
    WRK.p_d_adap_addr = (tok_adap_addr_t *)
	    ( (int)WRK.p_d_mem_block + ACA_ADAP_ADDR_BASE);

    /* Open options Set UP */
    WRK.p_open_opts = (tok_adap_open_options_t *)
	    ( (int)WRK.p_mem_block + ACA_OPEN_BLOCK_BASE);
    WRK.p_d_open_opts = (tok_adap_open_options_t *)
	    ( (int)WRK.p_d_mem_block + ACA_OPEN_BLOCK_BASE);

    /*                                                            */
    /*  Include the system block for component dump table.        */
    /*                                                            */
    add_cdt ("ACA", WRK.p_mem_block, PAGESIZE);
} /* end function sb_setup */
/*--------------------------------------------------------------------*/
/***************        System Block Undo               ***************/
/*--------------------------------------------------------------------*/

int sb_undo(dds_t *p_dds)

{  /* begin function  */

    
    /*
     *  set state machine to note that the ACA is now
     * NOT available to use.
     */
    WRK.limbo = PARADISE;
    
    /* Reset pointers */
    WRK.p_scb = NULL;
    WRK.p_d_scb = NULL;
    
    WRK.p_ssb = NULL;
    WRK.p_d_ssb = NULL;
    
    /* Product ID Information Undo */
    WRK.p_prod_id = NULL;
    WRK.p_d_prod_id = NULL;
    
    /* Adapter Error Log Undo */
    WRK.p_errlog = NULL;
    WRK.p_d_errlog = NULL;
    
    /*
     *   Reset the adapter initialization and open parameters
     */
    
    cfg_adap_parms(p_dds);
    
    /*                                                            */
    /*  Remove the system block from the component dump table.    */
    /*                                                            */
    del_cdt ("ACA", WRK.p_mem_block, PAGESIZE);
    
    return(0);
}  /* end function sb_undo */

/*--------------------------------------------------------------------*/
/***************        Memory Block Undo               ***************/
/*--------------------------------------------------------------------*/

int
get_mem_undo(dds_t *p_dds)
{
    (void)xmdetach( &(WRK.mem_block_xmd) );
    
    xmfree(WRK.p_mem_block, pinned_heap);
    WRK.p_d_mem_block = NULL;
    WRK.p_mem_block = NULL;
    
    WRK.bugout = NULL;
    
    (void)d_clear(WRK.dma_chnl_id);
    
    w_stop(&XMITWDT);		/* cleanup transmit timeout watchdog timer */

    if (WRK.xmit_wdt_inited) {
	while (w_clear(&XMITWDT));
	WRK.xmit_wdt_inited = FALSE;
    }
    
    w_stop(&BUWDT);		/* cleanup general purpose watchdog timer */

    if (WRK.bu_wdt_inited) {
	while (w_clear(&BUWDT));
	WRK.bu_wdt_inited = FALSE;
    }
    
    return(0);
} /* end function get_mem_undo */
