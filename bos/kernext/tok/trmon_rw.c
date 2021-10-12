static char sccsid[] = "@(#)12	1.15  src/bos/kernext/tok/trmon_rw.c, sysxtok, bos41B, 9504A 1/17/95 12:06:00";
/*
 *   COMPONENT_NAME: SYSXTOK
 *
 *   FUNCTIONS: list_error
 *		tok_output
 *		tok_receive
 *		tok_xmit
 *		tx_done
 *		xmit_timeout
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

/*----------------------  T O K _ R E C E I V E  -----------------------*/
/*                                                                      */
/*  NAME: tok_receive                                                   */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Processes receive interrupts; reads frames from the adapter,    */
/*      replenishes mbufs, and restarts the adapter as needed.          */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Called by the interrupt handler.                                */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Modifies the mbuf list, read address list, and read index.      */
/*                                                                      */
/*  RETURNS:                                                            */
/*      0               Completed successfully.                         */
/*      -1              Receive processing is down.                     */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: read_recv_chain			        */
/*                                                                      */
/*----------------------------------------------------------------------*/
void
tok_receive (
    dds_t           *p_dds,
    intr_elem_t     *iwe)
{
    recv_list_t    *last = NULL;
    
    TRACE_DBG(MON_RECV, "recB", (int)p_dds, iwe->stat2, 0);

    if (ULONG_MAX == NDD.ndd_genstats.ndd_recvintr_lsw) {
	NDD.ndd_genstats.ndd_recvintr_msw++;
    }
    NDD.ndd_genstats.ndd_recvintr_lsw++;

    if (!WRK.recv_mode)              /* ignore if not recv mode */
    {
	TRACE_SYS(MON_OTHER, "FooT", (int)p_dds, RCV_TRCV_0, 0);
	return;
    }
    
    if (iwe->stat0 & RECEIVE_SUSPENDED) {
	TRACE_BOTH(MON_OTHER, "FooT", (int)p_dds, RCV_TRCV_1, iwe->stat0);
	TRACE_BOTH(MON_OTHER, "FooT", RCV_TRCV_2, iwe->stat1, iwe->stat2);
	/*
	 *  NOTE:
	 *      We have an adapter command failure.
	 *      This is a limbo entry condition.
	 */
	NDD.ndd_genstats.ndd_ierrors++;
	WRK.limbo_iwe = *iwe;
	
	if ( WRK.limbo == PARADISE ) {
	    logerr( p_dds, ERRID_CTOK_DEVICE_ERR, __LINE__, __FILE__);
	    enter_limbo(p_dds, NDD_CMD_FAIL, ERRID_CTOK_DEVICE_ERR,
			iwe->stat0, FALSE);
	} else {
	    if ( !(WRK.limbo == NO_OP_STATE) ) {
		cycle_limbo(p_dds, NDD_CMD_FAIL, ERRID_CTOK_DEVICE_ERR);
	    }
	}
	return;
    }
    last = (recv_list_t *)((iwe->stat1 << 16) | iwe->stat2);
    
    if (last == NULL) {      /* check for invalid ending point */
	return;
    }
    
    read_recv_chain(p_dds, last);           /* read frames from adapter */

    TRACE_DBG(MON_RECV, "recE", (int)p_dds, 0, 0);
} /* End of tok_receive() */

/*****************************************************************************/
/*
 * NAME: tok_xmit
 *
 * FUNCTION: add packet to adapter transmit list, and start it transmitting
 *
 * EXECUTION ENVIORNMENT:
 *	Called under process and interrupt level
 *
 * NOTES:
 *	Caller must insure there is room on the transmit queue and there
 *	is a packet to transmit
 *
 * RETURNS:
 *	NONE
 */
/*****************************************************************************/
void
tok_xmit (
    dds_t		*p_dds)		/* adapter to transmit */
{
    xmt_elem_t		*p_tx_elem;
    xmit_des_t		*xd;
    t_tx_list		tmp_tx_list[3];
    t_tx_list		*start_tx_list;
    t_tx_list		*p_1list, *p_d_1list;
    int			rc;
    int			ioa;
    struct mbuf		*p_mbuf;
    caddr_t		bptr, mptr;
    int			blen, mlen, num_lists, num_bufs, tx_ds_num, i, l, g;
    
    /* 
     * while there are more transmit elements on software queue then
     * start a tranmission
     */
    start_tx_list = WRK.p_d_tx_next_avail;
    while(TRUE) {
	/* 
	 * sanity checks
	 */
	ASSERT(WRK.tx_que_next_in >= 0);
	ASSERT(WRK.tx_que_next_buf >= 0);
	ASSERT(WRK.tx_que_next_out >= 0);
	ASSERT(WRK.tx_que_next_in < DDI.xmt_que_size);
	ASSERT(WRK.tx_que_next_buf < DDI.xmt_que_size);
	ASSERT(WRK.tx_que_next_out < DDI.xmt_que_size);
	ASSERT(WRK.tx_list_next_in >= 0);
	ASSERT(WRK.tx_list_next_in <= TX_CHAIN_SIZE);
	
	p_tx_elem = &WRK.xmit_queue[WRK.tx_que_next_in];
	xd = &WRK.tx_buf_des[WRK.tx_buf_next_in];
	p_tx_elem->tx_ds_num = WRK.tx_buf_next_in;
	p_tx_elem->in_use = TRUE;

	/*
	 * trace if necessary
	 */
	if (NDD.ndd_trace) {
		NDD.ndd_trace(&NDD, p_tx_elem->mbufp,
			mtod(p_tx_elem->mbufp, caddr_t),
			NDD.ndd_trace_arg);
	}

	/* 
	 * copy data from mbuf(s) into transmit buffer(s)
	 */
	xd->count = 0;
	p_mbuf = p_tx_elem->mbufp;
	mlen = p_mbuf->m_len;
	mptr = mtod(p_mbuf,caddr_t);
	bptr = xd->sys_addr;
	blen = TX_BUF_SIZE;
	while (p_mbuf) {
		if (mlen < blen) {
			/*
			 * space left in xmit buffer after copying mbuf data,
			 * update pointers if another mbuf
			 */
			bcopy(mptr, bptr, mlen);
			xd->count = xd->count + mlen;
			if (p_mbuf = p_mbuf->m_next) {
				bptr += mlen;
				blen -= mlen;
				mlen = p_mbuf->m_len;
				mptr = mtod(p_mbuf,caddr_t);
			}
		} else {
			if (mlen == blen) {
			    /*
			     * xmit buffer full after copying mbuf data,
			     * need to get another buffer if more mbufs
			     */
			    bcopy(mptr, bptr, mlen);
			    xd->count = TX_BUF_SIZE;
			    if (p_mbuf = p_mbuf->m_next) {
				mlen = p_mbuf->m_len;
				mptr = mtod(p_mbuf,caddr_t);
				WRK.tx_buf_use_count++;
				WRK.tx_buf_next_in++;
				if (WRK.tx_buf_next_in >= WRK.tx_buf_count) {
					WRK.tx_buf_next_in = 0;
				}
				xd = &WRK.tx_buf_des[WRK.tx_buf_next_in];
				xd->count = 0;
				bptr = xd->sys_addr;
				blen = TX_BUF_SIZE;
			    }
			} else {
			    /*
			     * mbuf data will not fit in xmit buffer,
			     * fill this xmit buffer and get next buffer
			     */
			    bcopy(mptr, bptr, blen);
			    xd->count = TX_BUF_SIZE;
			    mptr += blen;
			    mlen -= blen;
			    WRK.tx_buf_use_count++;
			    WRK.tx_buf_next_in++;
			    if (WRK.tx_buf_next_in >= WRK.tx_buf_count) {
					WRK.tx_buf_next_in = 0;
			    }
			    xd = &WRK.tx_buf_des[WRK.tx_buf_next_in];
			    xd->count = 0;
			    bptr = xd->sys_addr;
			    blen = TX_BUF_SIZE;
			}
		}
	}
	
	WRK.xmits_queued--;
	WRK.xmits_adapter++;

	XMITQ_INC(WRK.tx_que_next_in);

	WRK.tx_buf_use_count++;
	WRK.tx_buf_next_in++;
	if (WRK.tx_buf_next_in >= WRK.tx_buf_count) {
		WRK.tx_buf_next_in = 0;
	}

	num_bufs = p_tx_elem->num_bufs;
	tx_ds_num = p_tx_elem->tx_ds_num;
	xd = &WRK.tx_buf_des[tx_ds_num];
	l = 0;				/* transmit list number */
	g = 0;				/* gather location number */
	bzero(&tmp_tx_list, sizeof(tmp_tx_list) );
	for (i = 0; i < num_bufs; ++i) {
		/* set up the temp. TX List Chain element */
		tmp_tx_list[l].gb[g].cnt = (ushort)xd->count;
		tmp_tx_list[l].gb[g].addr_hi = (ushort)ADDR_HI(xd->io_addr);
		tmp_tx_list[l].gb[g].addr_lo = (ushort)ADDR_LO(xd->io_addr);

		/*
		 * if we are on at least the second gather location, set the
		 * gather flag in the count field of the preceeding gather
		 * (so the adapter knows to do a gather)
		 */
		if (g) {
			tmp_tx_list[l].gb[g-1].cnt |= TX_GATHER;
		}

		if (vm_cflush(xd->sys_addr, xd->count)) {
			logerr( p_dds, ERRID_CTOK_DEVICE_ERR,
				__LINE__, __FILE__);
			enter_limbo(p_dds, TOK_DMA_FAIL, ERRID_CTOK_DEVICE_ERR,
				0, TRUE);
			return;
		}
		/*
		 * don't increment indices/pointers if not required
		 */
		if ( (i + 1) < num_bufs) {
			tx_ds_num++;
			if (tx_ds_num >= WRK.tx_buf_count) {
				tx_ds_num = 0;
			}
			xd = &WRK.tx_buf_des[tx_ds_num];

			g++;
			/* index to next transmit list */
			switch (i) {
			case 2:
				g = 0;
				l = 1;
				break;
			case 5:
				g = 0;
				l = 2;
			}
		}
	}
	tmp_tx_list[0].frame_size =(ushort)p_tx_elem->mbufp->m_pkthdr.len;
	tmp_tx_list[0].p_tx_elem = p_tx_elem;

	/*
	 * set CSTAT and move the TX list chain element(s) to the ACA
	 *
	 * NOTE: if a frame requires more than one TX list element, move
	 * the first TX list element last so that the adapter will not try
	 * to read the other elements before they are written to the ACA.
	 */
	switch (p_tx_elem->num_lists) {
	case 1:
		tmp_tx_list[0].tx_cstat = ( TX_START_OF_FRAME |
				TX_VALID_CHAIN_EL | TX_END_OF_FRAME |
				TX_FRAME_INTERRUPT );
		move_tx_list( p_dds, &tmp_tx_list[0], WRK.p_tx_next_avail, 
			  WRK.p_d_tx_next_avail, DMA_WRITE_ONLY);
		break;
	case 2:
		tmp_tx_list[0].tx_cstat = ( TX_START_OF_FRAME |
					    TX_VALID_CHAIN_EL |
					    TX_FRAME_INTERRUPT );
		tmp_tx_list[1].tx_cstat = ( TX_VALID_CHAIN_EL |
					    TX_END_OF_FRAME |
					    TX_FRAME_INTERRUPT );

		p_1list = WRK.p_tx_next_avail;
		p_d_1list = WRK.p_d_tx_next_avail;

		NEXT_TX_AVAIL(WRK.p_tx_next_avail, p_dds);
		NEXT_D_TX_AVAIL(WRK.p_d_tx_next_avail, p_dds);
		WRK.tx_list_use_count++;
		move_tx_list( p_dds, &tmp_tx_list[1], WRK.p_tx_next_avail, 
			  WRK.p_d_tx_next_avail, DMA_WRITE_ONLY);

		move_tx_list( p_dds, &tmp_tx_list[0], p_1list, 
			  p_d_1list, DMA_WRITE_ONLY);
		break;
	case 3:
		tmp_tx_list[0].tx_cstat = ( TX_START_OF_FRAME |
					    TX_VALID_CHAIN_EL |
					    TX_FRAME_INTERRUPT );
		tmp_tx_list[1].tx_cstat = ( TX_VALID_CHAIN_EL |
					    TX_FRAME_INTERRUPT );
		tmp_tx_list[2].tx_cstat = ( TX_VALID_CHAIN_EL |
					    TX_END_OF_FRAME |
					    TX_FRAME_INTERRUPT );

		p_1list = WRK.p_tx_next_avail;
		p_d_1list = WRK.p_d_tx_next_avail;

		NEXT_TX_AVAIL(WRK.p_tx_next_avail, p_dds);
		NEXT_D_TX_AVAIL(WRK.p_d_tx_next_avail, p_dds);
		WRK.tx_list_use_count++;
		move_tx_list( p_dds, &tmp_tx_list[1], WRK.p_tx_next_avail, 
			  WRK.p_d_tx_next_avail, DMA_WRITE_ONLY);

		NEXT_TX_AVAIL(WRK.p_tx_next_avail, p_dds);
		NEXT_D_TX_AVAIL(WRK.p_d_tx_next_avail, p_dds);
		WRK.tx_list_use_count++;
		move_tx_list( p_dds, &tmp_tx_list[2], WRK.p_tx_next_avail, 
			  WRK.p_d_tx_next_avail, DMA_WRITE_ONLY);

		move_tx_list( p_dds, &tmp_tx_list[0], p_1list, 
			  p_d_1list, DMA_WRITE_ONLY);
	}

	NEXT_TX_AVAIL(WRK.p_tx_next_avail, p_dds);
	NEXT_D_TX_AVAIL(WRK.p_d_tx_next_avail, p_dds);
	
	WRK.tx_list_use_count++;

	/* 
	 * if all the transmit lists/buffers are in use then exit
	 */
	p_tx_elem = &WRK.xmit_queue[WRK.tx_que_next_in];
	if ((WRK.tx_que_next_in == WRK.tx_que_next_buf) ||
           ((WRK.tx_list_use_count + p_tx_elem->num_lists) > TX_CHAIN_SIZE ) ||
	   (p_tx_elem->num_bufs > (WRK.tx_buf_count - WRK.tx_buf_use_count))) {
	    break;
	}
    }
    
    ioa = BUSIO_ATT(DDI.bus_id, DDI.io_port);
    if (WRK.issue_tx_cmd) {
	WRK.issue_tx_cmd = FALSE;
	issue_scb_command (p_dds, ADAP_XMIT_CMD, start_tx_list);
    } else {
	TOK_PUTSRX(ioa + COMMAND_REG, TX_VALID);
    }
    BUSIO_DET(ioa);

    if (WRK.pio_rc) {
	bug_out( p_dds, ERRID_CTOK_DEVICE_ERR, NDD_PIO_FAIL, 0 );
    }

} /* End of tok_xmit() */

/*****************************************************************************/
/*
 * NAME: tok_output
 *
 * FUNCTION: write entry point
 *
 * EXECUTION ENVIORNMENT:
 *	Can be called from interrupt or process level
 *
 * RETURNS:
 *	0 if successful
 *	errno value on failure
 */
/*****************************************************************************/
int
tok_output (
    ndd_t		*p_ndd,
    struct mbuf 	*p_mbuf)
{
    dds_t *p_dds;
    int txpri;
    xmt_elem_t *xlm;
    struct mbuf *p_first_mbuf, *p_next_mbuf, *p_cur_mbuf;
    struct mbuf *p_free_list, *p_tmp;
    
    /*
     *  Get adapter structure
     */
    p_dds = (dds_t *) (((unsigned long) p_ndd) - offsetof(dds_t, ndd));

    p_free_list = NULL;

    /*
     * Accounting for MIBs - ifHCOutUcastPkts, ifHCOutMulticastPkts 
     * and ifHCOutBroadcastPkts.
     */
    p_cur_mbuf = p_mbuf;
    while (p_cur_mbuf) {
	if (p_cur_mbuf->m_flags & (M_BCAST | M_MCAST)) {
	    if (p_cur_mbuf->m_flags & M_BCAST) {
		if (NDD.ndd_genstats.ndd_ifOutBcastPkts_lsw == ULONG_MAX)
		    NDD.ndd_genstats.ndd_ifOutBcastPkts_msw++;
		NDD.ndd_genstats.ndd_ifOutBcastPkts_lsw++;
	    } else {
		if (NDD.ndd_genstats.ndd_ifOutMcastPkts_lsw == ULONG_MAX)
		    NDD.ndd_genstats.ndd_ifOutMcastPkts_msw++;
		NDD.ndd_genstats.ndd_ifOutMcastPkts_lsw++;
	    }
	} else {
	    if (NDD.ndd_genstats.ndd_ifOutUcastPkts_lsw == ULONG_MAX)
		NDD.ndd_genstats.ndd_ifOutUcastPkts_msw++;
	    NDD.ndd_genstats.ndd_ifOutUcastPkts_lsw++;
	}
	p_cur_mbuf = p_cur_mbuf->m_nextpkt;
    }

    TRACE_DBG(MON_OTHER, "TX_L", (int)p_dds, 0, 0);
    txpri = disable_lock(PL_IMP, &TX_LOCK);
    
    if (WRK.adap_state != OPEN_STATE) {
	if (WRK.adap_state == DEAD_STATE) {
	    unlock_enable(txpri, &TX_LOCK);
	    TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);
	    return(ENETDOWN);
	} else {
	    unlock_enable(txpri, &TX_LOCK);
	    TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);
	    return(ENETUNREACH);
	}
    }
    
    p_first_mbuf = p_mbuf;
    while (p_mbuf) {
	if (XMITQ_FULL) {
	    if (p_mbuf == p_first_mbuf) {
		/*
		 * None of the mbufs fit on the transmit queue, they are all
		 * the responsibility of the caller.
		 */
		while (p_mbuf) {
		    NDD.ndd_genstats.ndd_xmitque_ovf++;
		    p_mbuf = p_mbuf->m_nextpkt;
		}
		unlock_enable(txpri, &TX_LOCK);
		TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);
		return(EAGAIN);
	    } else {
		/*
		 * Since some of the mbufs fit on the transmit queue, the
		 * device driver has to free those mbufs that didn't fit
		 * on the transmit queue.  No error is returned.
		 */
		while (p_mbuf) {
		    NDD.ndd_genstats.ndd_xmitque_ovf++;
		    NDD.ndd_genstats.ndd_opackets_drop++;
		    p_next_mbuf = p_mbuf->m_nextpkt;

		    p_tmp = p_mbuf;
		    while (p_tmp->m_next != NULL)
			p_tmp = p_tmp->m_next;
		    p_tmp->m_next = p_free_list;
		    p_free_list = p_mbuf;

	            p_mbuf = p_next_mbuf;
		}
	    }
	} else {
	    xlm = &WRK.xmit_queue[WRK.tx_que_next_buf];
	    xlm->mbufp = p_mbuf;
	    xlm->num_bufs = (p_mbuf->m_pkthdr.len + (TX_BUF_SIZE -1)) /
				TX_BUF_SIZE;
	    xlm->num_lists = (xlm->num_bufs + 2) / 3;
	    p_next_mbuf = p_mbuf->m_nextpkt;
	    p_mbuf->m_nextpkt = NULL;
	    XMITQ_INC(WRK.tx_que_next_buf);
	    WRK.xmits_queued++;
	    /*
	     * WRK.xmits_queued == # of packets not given to adapter
	     * WRK.xmits_adapter == # of packets adapter transmitting
	     * 		(there is a corresponding Q element until the
	     *		 transmit done interrupt is received)
	     * (1 will have been added to the user's requested queue size
	     *  to allow for the unused queue element - so the head and tail
	     *  pointers are equal only when the queue is empty)
	     */
	    if ((WRK.xmits_queued + WRK.xmits_adapter) >
			NDD.ndd_genstats.ndd_xmitque_max) {
		NDD.ndd_genstats.ndd_xmitque_max =
			WRK.xmits_queued + WRK.xmits_adapter;
	    }
    	    TRACE_SYS(MON_XMIT, TRC_WQUE, p_dds->seq_number, (int)p_mbuf,
			p_mbuf->m_pkthdr.len);
	    p_mbuf = p_next_mbuf;
	}
    }

    /*
     * handle the transmit timeout watchdog timer
     *
     * - the timer is started when a transmit packet is received (if it is
     *   not already running)
     * - when the timer expires
     *   - if the hardware transmit queue is empty, all is OK
     *   - the current ndd_opackets_lsw field is compared with its value when
     *     the timer was started (saved in xmit_wdt_opackets) and if something
     *     has been transmitted OK the timer is just restarted
     *   - otherwise error recovery is initiated
     * - this greatly reduces the time spent checking for transmit timeouts
     */
    if (!WRK.xmit_wdt_active) {
	WRK.xmit_wdt_active = TRUE;
	XMITWDT.count = 0;
	XMITWDT.restart = TRANSMIT_TIMEOUT;
	WRK.xmit_wdt_opackets = NDD.ndd_genstats.ndd_opackets_lsw;
	TRACE_DBG(MON_XMIT, "XTOs", p_dds, NDD.ndd_genstats.ndd_ipackets_lsw,
		WRK.xmit_wdt_opackets);
	w_start(&XMITWDT);
    }

    /* 
     *  if there are tranmit lists available then process transmit request
     */
    xlm = &WRK.xmit_queue[WRK.tx_que_next_in];
    if ( ((WRK.tx_list_use_count + xlm->num_lists) <= TX_CHAIN_SIZE ) &&
	 (xlm->num_bufs <= (WRK.tx_buf_count - WRK.tx_buf_use_count)) ) {
	tok_xmit(p_dds);
    }
    
    unlock_enable(txpri, &TX_LOCK);

    if (p_free_list != NULL)
	m_freem(p_free_list);

    TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);
    return(0);

} /* End of tok_output() */

/*--------------------------------------------------------------------*/
/*                                                                    */
/*************                  tx_done                 ***************/
/*                                                                    */
/*--------------------------------------------------------------------*/
/*
 *  This routine handles a transmit interrupt. The interrupt to be processed
 *  resides in the interrupt work elemnt.  The interrupt element is as follows:
 *
 *          p_iwe->cmd = ADAP_XMIT_CMD
 *          p_iwe->stat0 = The SSB transmit completion code
 *          p_iwe->stat1 = The high 2 bytes of a TX Chain element
 *          p_iwe->stat2 = The low 2 bytes of a TX Chain element
 *
 *  First the stat0 field is check for a transmit command complete.
 *      if so, the issue_tx_cmd flag is set to true.  This is done, so that
 *      when another write request is issued, a new ADAP_XMIT_CMD will be
 *      issued.  Otherwise just a TX_VALID interrupt would be issued.  
 *
 *  If the TX command has not completed, then the stat0 is checked for a
 *  LIST ERROR.
 *      if so, Network Recovery Mode is entered.
 *
 *  If neither of the above 2 events occured, the processing of the TX chain
 *  is begun.  Processing of the TX chain continues until A TX chain element
 *  is found that has a tx_cstat set to TX_VALID_CHAIN_EL or TX_START_OF_FRAME
 *  or to 0 (a Zero indicates that this element has nothing in it).
 *
 *      Processing of the TX chain begins with p_tx_1st_update.  When
 *      processing has completed, the p_tx_1st_update is set to the current
 *      TX chain element index.
 *
 *  The TX chain is then repopulated with transmit requests from the TX queue.
 *  The TX chain is repopulated via the tok_xmit routine.  After the
 *  TX chain is repopulated, either a transmit command or a TX_VALID interrupt
 *  is issued to the adapter.
 */

void
tx_done(register dds_t *p_dds,
    intr_elem_t *p_iwe)
{
    t_tx_list   *p_tx_tmp, *p_d_tx_tmp, *p_tx_tmp_last;
    t_tx_list   tmp_tx_list;		/* temp Transmit List Element */
    int 	txpri, rc, len, tx_ds_num, i, num_lists;
    xmt_elem_t  *xlm;
    xmit_des_t	*xd;
    struct mbuf *p_free_list, *p_tmp;

    struct mbuf     *p_mbuf;
    unsigned short  *p_mbuf_data;
    
    TRACE_DBG(MON_XMIT, "txoB", (int)p_dds, (ulong) WRK.p_tx_next_avail,
		WRK.p_tx_1st_update);

    TRACE_DBG(MON_OTHER, "TX_L", (int)p_dds, 0, 0);

    p_free_list = NULL;

    txpri = disable_lock(PL_IMP, &TX_LOCK);
    
    if (ULONG_MAX == NDD.ndd_genstats.ndd_xmitintr_lsw) {
	NDD.ndd_genstats.ndd_xmitintr_msw++;
    }
    NDD.ndd_genstats.ndd_xmitintr_lsw++;

    /*
     *   If we are in limbo mode AND we come into the
     *   transmit INTR processing routine, this INTR work element
     *   will have already have been processed via the clean_tx()
     *   routine that is called when we enter limbo mode.
     */
    if(WRK.limbo != PARADISE) {
	TRACE_DBG(MON_OTHER, "FooT", TX_TXOP_5, (int)p_dds, (ulong)WRK.limbo);
	unlock_enable(txpri, &TX_LOCK);
	TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);
	return;
    }
    
    /*
     *  Get current location of TX chain element to update.
     *  Get both virtual and BUS address of element.
     */
    p_tx_tmp = WRK.p_tx_1st_update;
    p_d_tx_tmp = WRK.p_d_tx_1st_update;
    
    if ((p_iwe->stat0 & 0xff00) == LIST_ERROR) {
	list_error(p_dds, p_iwe);
	unlock_enable(txpri, &TX_LOCK);
	TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);
	return;
    }
    
    if ((p_iwe->stat0 & 0xff00) == TX_CMD_COMPLETE) {
	/*   The adapter transmit command has commpleted
	 *   Another transmit command must be issued to restart
	 *   transmission.
	 */
	TRACE_DBG(MON_XMIT, "FooT", TX_TXOP_0, (int)p_dds, 0);
	WRK.issue_tx_cmd = TRUE;
    } else {
	while (TRUE) {
	    move_tx_list( p_dds, &tmp_tx_list, 
			      p_tx_tmp, p_d_tx_tmp, DMA_READ);

	    if ( (tmp_tx_list.tx_cstat & TX_VALID_CHAIN_EL)  ||
		(tmp_tx_list.tx_cstat == 0) ) {
		break;
	    }
	    
	    if (tmp_tx_list.tx_cstat & TX_ERROR) {
		TRACE_BOTH(MON_OTHER, "FooT", TX_TXOP_4, (ulong)p_dds,
		       (ulong)tmp_tx_list.tx_cstat );
		NDD.ndd_genstats.ndd_oerrors++;
	    }
	    
	    /*
	     *   issue the d_complete for the transmit data area(s)
	     */
	    tx_ds_num = tmp_tx_list.p_tx_elem->tx_ds_num;
	    for (i = 0; i < tmp_tx_list.p_tx_elem->num_bufs; ++i) {
		WRK.tx_buf_use_count--;
		xd = &WRK.tx_buf_des[tx_ds_num];
		rc = d_complete(WRK.dma_chnl_id, DMA_WRITE_ONLY,
				xd->sys_addr,
				xd->count,
				(struct xmem *)&WRK.xbuf_xd,
				xd->io_addr);
		if ( rc != DMA_SUCC ) {
		    logerr( p_dds, ERRID_CTOK_DEVICE_ERR, __LINE__, __FILE__);
		    enter_limbo( p_dds, TOK_DMA_FAIL, ERRID_CTOK_DEVICE_ERR,
				 0, FALSE );
    		    unlock_enable(txpri, &TX_LOCK);
		    if (p_free_list != NULL)
			m_freem(p_free_list);
		    return;
		}
		tx_ds_num++;
		if (tx_ds_num >= WRK.tx_buf_count) {
		    tx_ds_num = 0;
		}
	    }

	    if (tmp_tx_list.p_tx_elem->mbufp->m_flags & M_MCAST)
		TOKSTATS.mcast_xmit++;

	    if (tmp_tx_list.p_tx_elem->mbufp->m_flags & M_BCAST)
		TOKSTATS.bcast_xmit++;
	    
	    len = tmp_tx_list.p_tx_elem->mbufp->m_pkthdr.len;
	    TRACE_SYS(MON_XMIT, TRC_WEND, p_dds->seq_number,
			tmp_tx_list.p_tx_elem->mbufp, len);
	
	    if (ULONG_MAX == NDD.ndd_genstats.ndd_opackets_lsw)
		NDD.ndd_genstats.ndd_opackets_msw++;
	    NDD.ndd_genstats.ndd_opackets_lsw++;

	    if ((ULONG_MAX - len) < NDD.ndd_genstats.ndd_obytes_lsw)
		NDD.ndd_genstats.ndd_obytes_msw++;
	    NDD.ndd_genstats.ndd_obytes_lsw += len;

            p_tmp = tmp_tx_list.p_tx_elem->mbufp;
	    while (p_tmp->m_next != NULL)
		p_tmp = p_tmp->m_next;
	    p_tmp->m_next = p_free_list;
	    p_free_list = tmp_tx_list.p_tx_elem->mbufp;

	    tmp_tx_list.p_tx_elem->in_use = FALSE;
	    XMITQ_INC(WRK.tx_que_next_out);
	    WRK.xmits_adapter--;

	    num_lists = tmp_tx_list.p_tx_elem->num_lists;
	    for (i = 0; i < num_lists; ++i) {
		WRK.tx_list_use_count--;
	    
		/*
		 * Zero out the CSTAT, data count, and address fields.
		 * Put the change back to the Adapter Control Area
		 */
		tmp_tx_list.tx_cstat = 0;
		tmp_tx_list.frame_size = 0;
		tmp_tx_list.p_tx_elem = NULL;
		bzero(&tmp_tx_list.gb, sizeof(tmp_tx_list.gb));
	    
		/* Clear out the working TX chain element */
		move_tx_list( p_dds, &tmp_tx_list, p_tx_tmp, 
			      p_d_tx_tmp, DMA_WRITE_ONLY);
	    
		p_tx_tmp_last = p_tx_tmp;
		NEXT_TX_AVAIL(p_tx_tmp, p_dds);
		NEXT_D_TX_AVAIL(p_d_tx_tmp, p_dds);

		if ( (i + 1) < num_lists) {
		    move_tx_list( p_dds, &tmp_tx_list, 
				  p_tx_tmp, p_d_tx_tmp, DMA_READ);
		}
	    }
	    
	    if (p_tx_tmp == WRK.p_tx_next_avail) {
		/* The Transmit Chain is empty */
		break;
	    }

	    /*
	     * if the "offset" of the transmit list element just processed
	     * matches the offset from the SSB than stop processing transmit
	     * lists (even though the transmit lists themselves may indicate
	     * that they have been processed)
	     */
	    if ( ((ushort)p_tx_tmp_last & 0xFFF) == (p_iwe->stat2 & 0xFFF) ) {
		break;
	    }
	    
	} /* endwhile */
    } /* end else if !TX error */
    
    /* update the 1st TX Chain element to update pointer */
    WRK.p_tx_1st_update = p_tx_tmp;
    WRK.p_d_tx_1st_update = p_d_tx_tmp;
    
    xlm = &WRK.xmit_queue[WRK.tx_que_next_in];
    if ((WRK.tx_que_next_in != WRK.tx_que_next_buf) &&
        ((WRK.tx_list_use_count + xlm->num_lists) <= TX_CHAIN_SIZE ) &&
	(xlm->num_bufs <= (WRK.tx_buf_count - WRK.tx_buf_use_count)) ) {
	tok_xmit(p_dds);
    }
    
    unlock_enable(txpri, &TX_LOCK);
    if (p_free_list != NULL)
	m_freem(p_free_list);
    TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);
    TRACE_DBG(MON_XMIT, "txoE", p_dds, (ulong)WRK.p_tx_next_avail,
		WRK.p_tx_1st_update);

} /* end function tx_done */

void
list_error(
    dds_t	*p_dds,
    intr_elem_t *p_iwe) /* off level work que element */
{
    /*
     *   The adapter detected one of the following TX List errors:
     *       - Illegal Frame size
     *       - Transmit Threshold
     *       - Odd Address - (Enter Limbo condition)
     *       - Start of Frame - (Enter Limbo condition)
     *       - Unauthorized Access Priority
     *       - Unauthorized Mac Frame
     *       - Illegal Frame Format
     */
    
    TRACE_BOTH(MON_OTHER, "FooT", TX_TXOP_1, WRK.p_tx_1st_update, 0);
    ++NDD.ndd_genstats.ndd_oerrors;
    
    /*
     * log the error
     */
    logerr( p_dds, ERRID_CTOK_DEVICE_ERR, __LINE__, __FILE__);
    WRK.limbo_iwe = *p_iwe;
    
    /*
     *   We have an enter limbo condition.
     */
    TRACE_BOTH(MON_OTHER, "FooT", TX_TXOP_3, (ulong) p_iwe->stat0,
	(ulong) p_iwe->stat1);
    TRACE_BOTH(MON_OTHER, "FooT", TX_TXOP_4, (ulong) p_iwe->stat2, 0);
    enter_limbo(p_dds, NDD_CMD_FAIL, ERRID_CTOK_DEVICE_ERR, p_iwe->stat0,
		TRUE);
} /* End of list_error() */

/*
 *  FUNCTION:   xmit_timeout()
 *
 *  INPUT:      p_wdt - pointer to watchdog structure in the DDS
 *
 */
void
xmit_timeout(struct watchdog *p_wdt)
{
	dds_t	*p_dds = *(dds_t **) ((ulong)p_wdt + sizeof(struct watchdog));
	int		txpri;
    
	TRACE_BOTH(MON_OTHER, "ttoB", (int)p_dds,
		NDD.ndd_genstats.ndd_ipackets_lsw,
		NDD.ndd_genstats.ndd_opackets_lsw);
    
	TRACE_DBG(MON_OTHER, "TX_L", (int)p_dds, 0, 0);
	txpri = disable_lock(PL_IMP, &TX_LOCK);

	/*
	 * if hardware transmit queue is empty, all done, nothing to do
	 */
	if (!WRK.tx_list_use_count) {
		WRK.xmit_wdt_active = FALSE;

		unlock_enable(txpri, &TX_LOCK);
		TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);

		TRACE_DBG(MON_XMIT, "ttoE", (int)p_dds,0, 0);
		return;
	}

	/*
	 * the transmit queue is not empty
	 *
	 * if any packets have been transmitted, just restart the timer
	 * and check again later
	 */
	if (WRK.xmit_wdt_opackets != NDD.ndd_genstats.ndd_opackets_lsw) {
		TRACE_DBG(MON_XMIT, "XTOs", p_dds,
			NDD.ndd_genstats.ndd_ipackets_lsw,
			WRK.xmit_wdt_opackets);
        	XMITWDT.count = 0;
        	XMITWDT.restart = TRANSMIT_TIMEOUT;
        	WRK.xmit_wdt_opackets = NDD.ndd_genstats.ndd_opackets_lsw;
        	w_start(&XMITWDT);

		unlock_enable(txpri, &TX_LOCK);
		TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);

		TRACE_DBG(MON_XMIT, "ttoE", (int)p_dds,0, 0);
		return;
	}

	/*
	 * transmission has stopped, log an error, and start error recovery
	 */
	WRK.xmit_wdt_active = FALSE;
	TOKSTATS.tx_timeouts++;

	logerr(p_dds, ERRID_CTOK_DEVICE_ERR, __LINE__, __FILE__);

	/*
	 * flush the HW & SW transmit queues
	 * this may also be done in limbo, but this handles the case where
	 * close is in progress & enter_limbo is just a return
	 */
	clean_tx(p_dds);

	enter_limbo(p_dds, NDD_TX_TIMEOUT, ERRID_CTOK_DEVICE_ERR, 0, TRUE);

	unlock_enable(txpri, &TX_LOCK);
	TRACE_DBG(MON_OTHER, "TX_U", (int)p_dds, 0, 0);
    
	TRACE_DBG(MON_XMIT, "ttoE", (int)p_dds, 0, 0);
    
}  /* end function xmit_timeout */
