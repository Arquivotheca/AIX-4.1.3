static char sccsid[] = "@(#)09	1.10  src/bos/kernext/tok/trmon_xmit.c, sysxtok, bos411, 9428A410j 5/27/94 11:38:25";
/*
 *   COMPONENT_NAME: SYSXTOK
 *
 *   FUNCTIONS: clean_tx
 *		move_tx_list
 *		tx_chain_undo
 *		tx_limbo_startup
 *		tx_setup
 *		tx_undo
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

/*--------------------------------------------------------------------*/
/************           Transmit List Setup             ***************/
/*--------------------------------------------------------------------*/

/*
 *
 *  Set up the Transmit List Chain so the adapter can have access to it. It
 *  will first setup the Transmit TCW management table then for EACH Transmit
 *  List element do the following:
 *      - Put on a IOCC cache boundary (64 bytes)
 *      - Set TX CSTAT to 0
 *      - calculate the Bus address of the memory and store it in
 *        the previous List element's forward pointer
 *
 *  RETURN CODES:
 *      0 - Successful completion
 *      ENOBUFS - No Bus Address space available
 */

int
tx_setup(register dds_t *p_dds)
{
    int i, j;
    t_tx_list *p_tmp;
    t_tx_list *p_d_tmp;
    
    WRK.p_tx_head = (t_tx_list *)( (int)WRK.p_mem_block +
					 ACA_TX_CHAIN_BASE);
    
    WRK.p_d_tx_head = (t_tx_list *)( (int)WRK.p_d_mem_block
					   + ACA_TX_CHAIN_BASE);
    
    if (DDI.ring_speed) {
	if (DDI.xmt_que_size < 100) {
	    WRK.tx_buf_count = TX_16_MEG_MIN_BUF;
	} else {
	    WRK.tx_buf_count = TX_16_MEG_MAX_BUF;
	}
    } else {
	if (DDI.xmt_que_size < 100) {
	    WRK.tx_buf_count = TX_4_MEG_MIN_BUF;
	} else {
	    WRK.tx_buf_count = TX_4_MEG_MAX_BUF;
	}
    }
    
    WRK.tx_buf_size = WRK.tx_buf_count * TX_BUF_SIZE;
    WRK.xmit_buf = xmalloc(WRK.tx_buf_size, PGSHIFT, pinned_heap);
    if (WRK.xmit_buf == NULL) {
	return(ENOMEM);
    }
    WRK.xbuf_xd.aspace_id = XMEM_INVAL;
    if (xmattach(WRK.xmit_buf, WRK.tx_buf_size, (struct xmem *)&WRK.xbuf_xd,
		 SYS_ADSPACE) != XMEM_SUCC)
    {
	logerr( p_dds, ERRID_CTOK_DEVICE_ERR, __LINE__, __FILE__);
	xmfree(WRK.xmit_buf, pinned_heap);
	return(ENOMEM);
    }
    
    /* 
     * Set up variable for Transmit list
     */
    WRK.tx_que_next_buf  = 0;
    WRK.tx_que_next_out  = 0;
    WRK.tx_que_next_in  = 0;
    WRK.tx_buf_use_count  = 0;
    WRK.tx_buf_next_in  = 1;
    WRK.tx_list_use_count  = 0;
    WRK.tx_list_next_in  = 1;
    WRK.xmits_queued = 0;
    WRK.xmits_adapter = 0;
    
    for (i = 0; i < WRK.tx_buf_count; i++) {
	WRK.tx_buf_des[i].io_addr =
	    DDI.tcw_bus_mem_addr + TX_AREA_OFFSET +
	    (i * TX_BUF_SIZE);
	WRK.tx_buf_des[i].sys_addr = WRK.xmit_buf +
	    (i * TX_BUF_SIZE);
	d_master(WRK.dma_chnl_id, DMA_WRITE_ONLY,
		 WRK.tx_buf_des[i].sys_addr, TX_BUF_SIZE,
		 (struct xmem *)&WRK.xbuf_xd,
		 (char *)WRK.tx_buf_des[i].io_addr);
    }
    
    p_d_tmp = WRK.p_d_tx_head;
    p_tmp = WRK.p_tx_head;
    
    /*
     *   Setup the Tx Chain List Elements
     */
    for (i=0 ; i < TX_CHAIN_SIZE; i++ ) {
	p_tmp[i].tx_cstat = 0;
	p_tmp[i].p_tx_elem = NULL;
	
	for (j=0; j<=2; ++j) {
	    p_tmp[i].gb[j].cnt = 0;
	    p_tmp[i].gb[j].addr_hi = NULL;
	    p_tmp[i].gb[j].addr_lo = NULL;
	}
	
	p_tmp[i].p_d_fwdptr = &p_d_tmp[i+1];
	WRK.p_d_tx_fwds[i] = &p_d_tmp[i+1];
    }
    
    /* Set up last TX Chain element */
    p_tmp[TX_CHAIN_SIZE-1].p_d_fwdptr = WRK.p_d_tx_head;
    WRK.p_d_tx_fwds[TX_CHAIN_SIZE-1] = WRK.p_d_tx_head;
    
    WRK.p_tx_tail = &p_tmp[TX_CHAIN_SIZE-1];
    WRK.p_d_tx_tail = &p_d_tmp[TX_CHAIN_SIZE-1];
    
    WRK.p_tx_next_avail = WRK.p_tx_head;
    WRK.p_d_tx_next_avail = WRK.p_d_tx_head;
    WRK.p_tx_1st_update = WRK.p_tx_head;
    WRK.p_d_tx_1st_update = WRK.p_d_tx_head;
    WRK.issue_tx_cmd = TRUE;
    
    /* Flush the System Cache */
    vm_cflush(WRK.p_mem_block, PAGESIZE);
    
    return(0);

}  /* end function tx_setup */

/*--------------------------------------------------------------------*/
/***************        Transmit Undo                   ***************/
/*--------------------------------------------------------------------*/
int
tx_undo(register dds_t *p_dds)
{
    int i, rc;
    
    WRK.p_tx_head = NULL;
    WRK.p_d_tx_head = NULL;
    WRK.p_tx_tail = NULL;
    WRK.p_d_tx_tail= NULL;
    WRK.issue_tx_cmd = TRUE;
    
    for (i = 0; i < WRK.tx_buf_count; i++) {
	rc = d_complete(WRK.dma_chnl_id, DMA_WRITE_ONLY,
		   WRK.tx_buf_des[i].sys_addr, TX_BUF_SIZE,
		   (struct xmem *)&WRK.xbuf_xd,
		   (char *)WRK.tx_buf_des[i].io_addr);
	if ( rc != DMA_SUCC ) {
	    logerr( p_dds, ERRID_CTOK_DEVICE_ERR, __LINE__, __FILE__);
	}
    }
    
    xmdetach(&(WRK.xbuf_xd));
    
    xmfree(WRK.xmit_buf, pinned_heap);
    
    return(0);
}

/*
 * NAME: clean_tx
 *
 * FUNCTION:
 *     Cleans out the TX chain and TX queue of all completed and/or
 *     pending TX requests in preparation for going into limbo mode.
 *
 *     The TX blocking variable(s) are set so as to prevent any writes
 *     from being accepted while in limbo mode.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes on the interrupt thread.
 */
int
clean_tx( dds_t *p_dds)
{
    int rc, i, tx_ds_num;
    xmt_elem_t  *p_tx_elem;
    xmit_des_t	*p_xmit_des;
    
    tx_chain_undo(p_dds);
    
    /*
     *  Loop until caught up to tx list with all packets completed
     */
    while (WRK.tx_que_next_out != WRK.tx_que_next_in)
    {
	if (WRK.xmit_queue[WRK.tx_que_next_out].in_use) {
	    NDD.ndd_genstats.ndd_opackets_drop++;

	    p_tx_elem = &WRK.xmit_queue[WRK.tx_que_next_out];
	    tx_ds_num = p_tx_elem->tx_ds_num;

	    for (i = 0; i < p_tx_elem->num_bufs; i++) {
		p_xmit_des = &WRK.tx_buf_des[tx_ds_num];
		rc = d_complete(WRK.dma_chnl_id, DMA_WRITE_ONLY,
			p_xmit_des->sys_addr,
			p_xmit_des->count,
			(struct xmem *)&WRK.xbuf_xd,
			(char *)p_xmit_des->io_addr);
		if ( rc != DMA_SUCC ) {
		    logerr( p_dds, ERRID_CTOK_DEVICE_ERR, __LINE__, __FILE__);
		}
		tx_ds_num++;
		if (tx_ds_num >= WRK.tx_buf_count) {
		    tx_ds_num = 0;
		}
	    }

	    m_freem(p_tx_elem->mbufp);
	    p_tx_elem->in_use = FALSE;

	    XMITQ_INC(WRK.tx_que_next_out);
	}
    }
    
    WRK.p_tx_next_avail = WRK.p_tx_head;
    WRK.p_d_tx_next_avail = WRK.p_d_tx_head;
    WRK.p_tx_1st_update = WRK.p_tx_head;
    WRK.p_d_tx_1st_update = WRK.p_d_tx_head;
    WRK.issue_tx_cmd = TRUE;
    
    /*
     * empty transmit queue
     */
    while (WRK.tx_que_next_in != WRK.tx_que_next_buf)
    {
	p_tx_elem = &WRK.xmit_queue[WRK.tx_que_next_in];
	m_freem(p_tx_elem->mbufp);

	XMITQ_INC(WRK.tx_que_next_in);
    }

    WRK.tx_buf_use_count = 0;
    WRK.tx_list_use_count = 0;
    WRK.tx_que_next_in = 0;
    WRK.tx_que_next_buf = 0;
    WRK.tx_que_next_out = 0;
    WRK.xmits_queued = 0;
    WRK.xmits_adapter = 0;
    
    return(0);
}  /* end function clean_tx() */

int
tx_chain_undo(dds_t *p_dds)
{
    t_tx_list   *p_tx_tmp;
    t_tx_list   *p_d_tx_tmp;
    t_tx_list   tmp_tx_list;
    uint	done=FALSE;
    
    p_tx_tmp = WRK.p_tx_1st_update;
    p_d_tx_tmp = WRK.p_d_tx_1st_update;
    
    while ( (p_tx_tmp != WRK.p_tx_next_avail) && !done ) {
	move_tx_list( p_dds, &tmp_tx_list, 
		     p_tx_tmp, p_d_tx_tmp, DMA_READ);
	
	if (tmp_tx_list.tx_cstat== 0) {
	    done = TRUE;
	}
	
	/*
	 * Zero out the CSTAT, data count, and address fields.
	 * Put the change back to the Adapter Control Area
	 */
	tmp_tx_list.tx_cstat = 0;
	tmp_tx_list.frame_size = 0;
	tmp_tx_list.p_tx_elem = NULL;
	bzero(&tmp_tx_list.gb, sizeof(tmp_tx_list.gb));
	
	move_tx_list( p_dds, &tmp_tx_list, 
		     p_tx_tmp, p_d_tx_tmp, DMA_WRITE_ONLY);
	
	NEXT_TX_AVAIL(p_tx_tmp, p_dds);
	NEXT_D_TX_AVAIL(p_d_tx_tmp, p_dds);
    }
    return(0);
}

/*
 * NAME: tx_limbo_startup
 *
 * FUNCTION:
 *     This function prepares the device driver for transmitting of data
 *     after successfully cycling through limbo mode.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes on the interrupt level.
 *
 * (NOTES:) More detailed description of the function, down to
 *      what bits / data structures, etc it manipulates.
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *      software error recovery.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */

void tx_limbo_startup( dds_t *p_dds)
{
    int		 rc, i;
    t_tx_list	*p_d_tmp;	/* Temp. TX List Bus address */
    t_tx_list	*p_tmp;		/* Temp. TX List virtual address */
    t_tx_list	tmp_tx_list;	/* Temp. Tx list element */
    
    bzero(&tmp_tx_list, sizeof(t_tx_list) );
    
    p_d_tmp = WRK.p_d_tx_head;	/* start at the head */
    p_tmp = WRK.p_tx_head;		/* start at the head */
    
    /*
     * Re-initialize the Transmit Chain in the ACA
     */
    for ( i=0; i < TX_CHAIN_SIZE; ++i)
    {
	tmp_tx_list.p_d_fwdptr = WRK.p_d_tx_fwds[i];
	if (WRK.do_dkmove) {
	    d_kmove(&tmp_tx_list, p_d_tmp,
		     (sizeof(t_tx_list)),
		     WRK.dma_chnl_id,
		     DDI.bus_id, DMA_WRITE_ONLY);
	} else {
	    bcopy(&tmp_tx_list, p_tmp, sizeof(t_tx_list));
	}
	
	++p_d_tmp; 	/* move to next Tx list element in chain */
	++p_tmp; 	/* move to next Tx list element in chain */
	
    } /* end for loop */
    
    WRK.issue_tx_cmd = TRUE;
    
}  /* end function tx_limbo_startup() */



/*
 * NAME: move_tx_list
 *
 * FUNCTION:
 *
 *	Moves a Transmit List element either to or from the Transmit Chain.
 *	The TX chain resides in the ACA.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes on the interrupt or on the process thread.
 *
 * NOTES:
 *
 *	DMA_READ:	Specifying this flag causes this routine to move
 *			the TX list element from the TX Chain.  The TX element
 *			and the Bus addresses for this list are placed into
 *			the TX list element after the ACA's copy is d_kmoved.
 *
 *	DMA_WRITE_ONLY:	Specifying this flag causes this routine to move
 *			the TX list to the TX chain.  The TX element and the
 *			Bus addresses for this list are saved in the work
 *			section of the DDS before the list is moved to the
 *			ACA via the d_kmove() kernel service.
 *      
 *
 * RECOVERY OPERATION:
 *	None
 *
 * DATA STRUCTURES:
 *
 * RETURNS: nothing
 */

void
move_tx_list( 
    dds_t 	*p_dds,		/* DDS pointer */
    t_tx_list	*tmp_tx_list,	/* entry being moved */
    t_tx_list	*p_tx_list,	/* TX list buff ptr */
    t_tx_list	*p_d_tx_list, 	/* TX list BUS addr  */
    int		flag)		/* direction flag */
{
    int rc, i;
    
    switch ( flag )
    {
    case DMA_READ:
	/*
	 * get the TX list element
	 * from the ACA.
	 */
	if (WRK.do_dkmove) {
	    d_kmove( tmp_tx_list, p_d_tx_list,
		     (unsigned int)TX_LIST_SIZE, 
		     WRK.dma_chnl_id,
		     DDI.bus_id, DMA_READ);
	} else {
	    bcopy( p_tx_list, tmp_tx_list,
		  (unsigned int)TX_LIST_SIZE);
	}
	
	break;
    case DMA_WRITE_ONLY:
	/*
	 * the transmit chain forward pointer in the temporary transmit list
	 * may not be correct (it is likely 0), so get the correct one
	 */
	i = GET_TX_INDEX(p_d_tx_list, WRK.p_d_tx_head);
	tmp_tx_list->p_d_fwdptr = WRK.p_d_tx_fwds[i];

	/*
	 * move the TX list element
	 * into the ACA.
	 */
	if (WRK.do_dkmove) {
	    d_kmove( tmp_tx_list, p_d_tx_list,
		     (unsigned int)TX_LIST_SIZE, 
		     WRK.dma_chnl_id,
		     DDI.bus_id, DMA_WRITE_ONLY);
	} else {
	    bcopy( tmp_tx_list, p_tx_list,
		  (unsigned int)TX_LIST_SIZE);
	}
	break;
    } /* end switch */
} /* end move_tx_list() */
