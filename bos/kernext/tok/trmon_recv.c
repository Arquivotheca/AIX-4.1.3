static char sccsid[] = "@(#)13	1.14  src/bos/kernext/tok/trmon_recv.c, sysxtok, bos411, 9428A410j 6/7/94 16:14:29";
/*
 *   COMPONENT_NAME: SYSXTOK
 *
 *   FUNCTIONS: arm_recv_list
 *		clear_recv_chain
 *		load_recv_chain
 *		read_recv_chain
 *		tok_recv_init
 *		tok_recv_setup
 *		tok_recv_undo
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

/*-------------------  T O K _ R E C V _ S E T U P  --------------------*/
/*                                                                      */
/*  NAME: tok_recv_setup                                                */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Initializes the receive portion of the driver for handling      */
/*      receive data from the adapter.                                  */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      This is a top-half routine in this driver and can only be       */
/*      executed under a user process.                                  */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Clears all receive data structures to init state; including     */
/*      the read index, mbuf list, and read address lists.              */
/*                                                                      */
/*  RETURNS:                                                            */
/*      0               Completed Successfully.                         */
/*      ENOMEM          Not enough memory/mbufs.                        */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: tok_recv_init,				*/
/*                              load_recv_chain, clear_recv_chain       */
/*                                                                      */
/*----------------------------------------------------------------------*/

tok_recv_setup (dds_t   *p_dds)
{
    int    i;
    
    /*
     *  init data structures
     */
    tok_recv_init(p_dds);

    /*
     *  get all recv mbuf's, if error, free up any left, 
     *  free up receive TCW's, and return error
     */
    if (load_recv_chain(p_dds)) {
	clear_recv_chain(p_dds, TRUE);
	for (i = 0; i < RCV_CHAIN_SIZE; i++) {
	    WRK.recv_addr[i] = NULL;
	}
	return(ENOMEM);
    }
    return(0);                      /* made it */
}

/*-------------------  T O K _ R E C V _ U N D O   ---------------------*/
/*                                                                      */
/*  NAME: tok_recv_undo                                                 */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Stops receive processing, frees all receive mbufs and receive   */
/*      TCW's, and changes the receive state.                           */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      This is a top-half routine in this driver and can only be       */
/*      executed under a user process.                                  */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Clears all receive data structures to init state; including     */
/*      the read index, mbuf list, and read address lists.              */
/*                                                                      */
/*  RETURNS: void                                                       */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: clear_recv_chain			*/
/*                                                                      */
/*----------------------------------------------------------------------*/
void
tok_recv_undo(dds_t   *p_dds)
{
    int    i;
    
    WRK.recv_mode = FALSE;          /* change receive state     */
    clear_recv_chain(p_dds, TRUE);  /* free mbufs		*/
    /*
     *  Free each TCW from the receive chain:
     */
    for (i = 0; i < RCV_CHAIN_SIZE; i++) {
	WRK.recv_addr[i] = NULL;
    }
}

/*---------------------  T O K _ R E C V _ I N I T  --------------------*/
/*                                                                      */
/*  NAME: tok_recv_init                                                 */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Initializes the receive data structures.                        */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      This is a top-half routine in this driver and can only be       */
/*      executed under a user process.   Assumes that the ACA has       */
/*      been allocated and d_mastered.                                  */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Clears all receive data structures to init state; including     */
/*      the read index, mbuf list, read list, and address lists.        */
/*                                                                      */
/*  RETURNS:  Nothing                                                   */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: None                                    */
/*                                                                      */
/*----------------------------------------------------------------------*/
void
tok_recv_init (dds_t   *p_dds)
{
    int    i, j;
    
    /*  Set all receive variables to initial state:                 */
    /*                                                              */
    WRK.recv_mode     = FALSE;
    WRK.read_index    = 0;
    for (i = 0, j = 0; i < RCV_CHAIN_SIZE; i++, j += CLBYTES) {
	WRK.recv_addr[i] = DDI.tcw_bus_mem_addr + RECV_AREA_OFFSET + j;
	WRK.recv_mbuf[i] = NULL;
    }
    /*  Create the receive chain by initializing the pointer to     */
    /*  each receive list.  Create both DMA and virtual address     */
    /*  lists.                                                      */
    /*                                                              */
    
    WRK.recv_list[0] =
	(recv_list_t *)( (int)WRK.p_d_mem_block + ACA_RCV_CHAIN_BASE);
    WRK.recv_vadr[0] =
	(recv_list_t *)( (int)WRK.p_mem_block + ACA_RCV_CHAIN_BASE);
    for (i = 1; i < RCV_CHAIN_SIZE; i++) {
	WRK.recv_list[ i ] = WRK.recv_list[ i-1 ] + 1;
	WRK.recv_vadr[ i ] = WRK.recv_vadr[ i-1 ] + 1;
    }
}

/*-------------------  L O A D _ R E C V _ C H A I N  ------------------*/
/*                                                                      */
/*  NAME: load_recv_chain                                               */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Loads the receive chain with mbufs where needed.                */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      This routine can be executed by both the interrupt and          */
/*      process call threads of the driver.                             */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Fills the recv_mbuf chain if possible; d_master's recv_addr     */
/*      array elements for dma where mbufs are added.                   */
/*                                                                      */
/*  RETURNS:                                                            */
/*      0               Completed successfully.                         */
/*      ENOBUFS         Failed (Not enough mbufs)                       */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  arm_recv_list				*/
/*                                                                      */
/*----------------------------------------------------------------------*/

load_recv_chain (dds_t   *p_dds)
{
    int     i;
    
    TRACE_DBG(MON_RECV, "mbrB", (int)p_dds, 0, 0);
    /*                                                               */
    /*  Starting at the next receive list to read, walk through the  */
    /*  receive chain and fill in any needed mbufs until either the  */
    /*  chain is full or we run out of mbufs; in the latter case,    */
    /*  quit							     */
    /*                                                               */
    i = WRK.read_index;              /* next to read */
    while (TRUE) {
	/*
	 *  Arm the receive list and try to get an mbuf for it
	 *  if necessary
	 */
	if (!WRK.recv_mbuf[i]) {
	    if (WRK.recv_mbuf[i] = m_getclust(M_DONTWAIT, MT_DATA) ) {
		WRK.recv_mbuf[i]->m_flags |= M_PKTHDR;
		WRK.recv_mbuf[i]->m_len = CLBYTES;
		d_master (WRK.dma_chnl_id, DMA_READ|DMA_NOHIDE,
			MTOD( WRK.recv_mbuf[i], ushort *),
			WRK.recv_mbuf[i]->m_len,
			M_XMEMD(WRK.recv_mbuf[i]) ,WRK.recv_addr[i]);
	    } else {
		NDD.ndd_genstats.ndd_nobufs++;
		return (ENOMEM);
	    }
	}

	arm_recv_list(p_dds, i);

	i = (i + 1) % RCV_CHAIN_SIZE;       /* next recv list */
	if (i == WRK.read_index) {	    /* last one? */
	    break;
	}
    }
    TRACE_DBG(MON_RECV, "mbrE", (int)p_dds, 0, 0);
    return(0);
}

/*-------------------  R E A D _ R E C V _ C H A I N  ------------------*/
/*                                                                      */
/*  NAME: read_recv_chain                                               */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Reads completed mbufs from the receive chain and passes         */
/*      them as receive packets to the demuxer.				*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      This routine can be executed by both the interrupt and          */
/*      process call threads of the driver.                             */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Updates the read index to the next read list to be read.        */
/*      Modifies the mbuf array and receive lists.                      */
/*                                                                      */
/*  RETURNS: Nothing                                                    */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  none					*/
/*                                                                      */
/*  NOTE: The adapter does not update the receive list data if it is    */
/*        not a start of frame or end of frame.                         */
/*                                                                      */
/*----------------------------------------------------------------------*/
void
read_recv_chain (
    dds_t           *p_dds,
    recv_list_t     *last)
{
    int			size, worklen, i, sof = FALSE, mof = FALSE;
    int			eof = FALSE, no_mbuf = FALSE;
    recv_list_t		recvlist;
    struct mbuf		*m, *mnew, *mhead, *mtail;
    int			bus;
    int			rc;
    
    mhead = mtail = NULL;
    while (TRUE) {
	i = WRK.read_index;          /* start at next recv list */
	/*
	 *  move the receive list image from the IOCC cache to main
	 *  memory (local image) pointed to by recvlist.
	 */
	if (WRK.do_dkmove) {
	    d_kmove (&recvlist,
		      WRK.recv_list[i],
		      sizeof(recv_list_t),
		      WRK.dma_chnl_id,
		      DDI.bus_id,
		      DMA_READ);
	} else {
	    bcopy (WRK.recv_vadr[i], &recvlist, sizeof(recv_list_t));
	}
	
	/*
	 *  The sof flag is set when the start of a frame flag is set in
	 *  a received frame.
	 *  If the valid bit is still set, and not start of frame,
	 *  then nothing has been received into this mbuf (so we didn't
	 *  get anything).  If the start of frame and VALID = TRUE,
	 *  this is a frame that crosses several receive lists (the
	 *  receive lists in the middle have valid set).
	 */
	if ((recvlist.status & VALID) && !sof) {
	    break;
	}
	
	m = WRK.recv_mbuf[i];
	rc =  d_complete (WRK.dma_chnl_id, DMA_READ,
			  MTOD( m, unsigned short *), m->m_len,
			  M_XMEMD(m), WRK.recv_addr[i]);
	    
	if ( rc != DMA_SUCC ) {
		if ( mhead ) {	/* if any mbufs allocated, free them */
		    m_freem( mhead );
		}
		logerr( p_dds, ERRID_CTOK_DEVICE_ERR, __LINE__, __FILE__);
		enter_limbo( p_dds, TOK_DMA_FAIL, ERRID_CTOK_DEVICE_ERR,
			0, FALSE );
		return;
	}
	
	/*
	 *  netpmon trace entry point
	 */
	TRACE_SYS(MON_RECV, TRC_RDAT, p_dds->seq_number,
		recvlist.frame_size, 0);
	
	/*
	 *  See if we are at the beginning of a new frame; if so,
	 *  initialize the working length to the total size of the
	 *  frame.
	 */
	if (recvlist.status & START_OF_FRAME) {
	    sof   = TRUE;
	    eof   = FALSE;
	    size  = worklen = recvlist.frame_size;
	} else {
	    mof = TRUE;
	}

	/*
	 *  worklen is initially set to the size of the frame.
	 *  m_len is initially (when the mbuf is setup) set to CLBYTES.
	 *  set m_len to the length of the frame or leave it at CLBYTES.
	 *  set worklen to 0 or the length of the remaining frame.
	 */
	m->m_len = MIN( m->m_len, worklen );
	worklen = MAX( 0, worklen - m->m_len );

	/*
	 * If the number of received bytes is below our threshold
	 * copy into a new mbuf and leave the old one mapped. This
	 * avoids another d_master call.  Also, if the data is less
	 * than 256 bytes then get a small mbuf instead of a cluster.
	 * Otherwise, get a new cluster to receive data into.
	 *
	 * m points at the mbuf to be passed to the user
	 */
	if (m->m_len <= (CLBYTES / 2)) {
	    if (m->m_len <= MHLEN) {
		mnew = m_gethdr(M_DONTWAIT, MT_HEADER);
		TRACE_DBG(MON_OTHER, "Mhdr", p_dds, mnew, 0);
	    } else {
		mnew = m_getclust(M_DONTWAIT, MT_DATA);
		if (mnew != NULL) {
			mnew->m_flags |= M_PKTHDR;
		}
	    }
	    if ( mnew ) {
		bcopy(mtod(m, caddr_t), mtod(mnew, caddr_t), m->m_len);
		mnew->m_len = m->m_len;
		cache_inval(mtod(m, caddr_t), m->m_len);
		m->m_len = CLBYTES;
	    } else {
		NDD.ndd_genstats.ndd_nobufs++;
		no_mbuf = TRUE;
		cache_inval(mtod(m, caddr_t), m->m_len);
		m->m_len = CLBYTES;
	    }
	    m = mnew;
	    
	} else {
	    mnew = m_getclust(M_DONTWAIT, MT_DATA);
	    if ( mnew ) {
		mnew->m_flags |= M_PKTHDR;
		mnew->m_len = CLBYTES;
		WRK.recv_mbuf[i] = mnew;
		d_master (WRK.dma_chnl_id, DMA_READ|DMA_NOHIDE,
		      MTOD( mnew, ushort *), mnew->m_len,
		      M_XMEMD(mnew) ,WRK.recv_addr[i]);
	    } else {
		NDD.ndd_genstats.ndd_nobufs++;
		no_mbuf = TRUE;
		cache_inval(mtod(m, caddr_t), m->m_len);
		m->m_len = CLBYTES;
		m = NULL;
	    }
	}

	/*
	 * ready this receive list entry to again receive data
	 */
	arm_recv_list(p_dds, i);

	/*
	 *  If we are at the beginning of a new frame (sof is set, but
	 *  mof hasn't been set yet), begin a new linked list of mbufs.
	 *  Otherwise add this mbuf to the end of the current list.
	 *
	 *  NOTE: the adapter has been known to not pass frames up
	 *  correctly, so make sure that we have gotten a start of frame
	 *  Also, we may have not been able to get an mbuf for a previous
	 *  part of this frame and mtail may be null as a result.
	 */
	if (sof && (!mof)) {
	    mhead = mtail = m;
	} else {
	    if (mtail) {
		mtail->m_next = m;
		mtail = m;
	    } else {
		m_freem( m );
	    }
	}
	
	/*                                                           */
	/*  If this is the end of the frame, submit this list of     */
	/*  mbufs to the receive handler.                            */
	/*                                                           */
	if (recvlist.status & END_OF_FRAME) {
	    if (mhead && (!no_mbuf)) {
		if (ULONG_MAX == NDD.ndd_genstats.ndd_ipackets_lsw) {
		    NDD.ndd_genstats.ndd_ipackets_msw++;
		}
		NDD.ndd_genstats.ndd_ipackets_lsw++;
		if ((ULONG_MAX - size) < NDD.ndd_genstats.ndd_ibytes_lsw) {
		    NDD.ndd_genstats.ndd_ibytes_msw++;
		}
		NDD.ndd_genstats.ndd_ibytes_lsw += size;
		/*
		 * broadcasts are to C000 FFFF FFFF or FFFF FFFF FFFF
		 * multicasts are to a group address (first bit of @ is
		 * on) which are not broadcasts
		 * (the address starts 2 bytes into the data)
		 */
		if (*(uchar *)((mhead->m_data)+2) & MULTI_BIT_MASK) {
		    if (*((int *)((mhead->m_data)+4)) == -1) {
			mhead->m_flags |= M_BCAST;
			TOKSTATS.bcast_recv++;
		    } else {
			mhead->m_flags |= M_MCAST;
			TOKSTATS.mcast_recv++;
		    }
		}

		/*
		 *  Send the data up to the user.
		 */
		mhead->m_pkthdr.len = size;
		TRACE_SYS(MON_RECV, TRC_RNOT, p_dds->seq_number, mhead, size);
		NDD.nd_receive(&NDD, mhead);
		TRACE_SYS(MON_RECV, TRC_REND, mhead, 0, 0);
	    } else {
		NDD.ndd_genstats.ndd_ipackets_drop++;
		no_mbuf = FALSE;
		m_freem( mhead );
	    }
	    mhead = mtail = NULL;
	    eof   = TRUE;
	    sof   = FALSE;
	    mof   = FALSE;
	}
	WRK.read_index = (WRK.read_index + 1) % RCV_CHAIN_SIZE;
	if (WRK.recv_list[i] == last) {
	    if (!eof) {
		if ( mhead ) {
		    m_freem( mhead );
		}

		logerr( p_dds, ERRID_CTOK_DEVICE_ERR, __LINE__, __FILE__);
		enter_limbo( p_dds, NDD_RCV_ERROR, ERRID_CTOK_DEVICE_ERR,
			0, FALSE );
	    }
	    break;                  /* quit if this was the last one */
	}
    }
    bus = BUSIO_ATT(DDI.bus_id, DDI.io_port);
    TOK_PUTSRX(bus + COMMAND_REG, RCV_VALID);
    BUSIO_DET(bus);

    if (WRK.pio_rc) {
	bug_out( p_dds, ERRID_CTOK_DEVICE_ERR, NDD_PIO_FAIL, 0 );
    }
}

/*-----------------  C L E A R _ R E C V _ C H A I N  ------------------*/
/*                                                                      */
/*  NAME: clear_recv_chain                                              */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      D_completes and frees all remaining mbufs in the receive        */
/*      chain and clears all receive list elements.                     */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      This is a top-half routine in this driver and can only be       */
/*      executed under a user process.                                  */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Clears the recv_mbuf array and clears receive list elements     */
/*      of the receive chain.  Frees the associated mbuf if requested	*/
/*                                                                      */
/*  RETURNS: Nothing                                                    */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  bzero, d_complete, d_kmove		*/
/*                                                                      */
/*----------------------------------------------------------------------*/

clear_recv_chain (
    dds_t    *p_dds,
    int	     free_mbuf)
{
    int   i, rc;
    struct mbuf     *m;
    volatile recv_list_t    recvlist;

    /*
     *  Clear each receive list in the chain
     */
    bzero(&recvlist, sizeof(recvlist));     /* clear receive list */
    for (i = 0; i < RCV_CHAIN_SIZE; i++) {
	/*
	 *  Update the receive dma image of the list by d_moving it
	 *  through the IOCC cache into system memory.
	 */
	if (WRK.do_dkmove) {
	    d_kmove (&recvlist,
		  WRK.recv_list[i],
		  sizeof(recvlist),
		  WRK.dma_chnl_id,
		  DDI.bus_id,
		  DMA_WRITE_ONLY);
	} else {
		bcopy (&recvlist, WRK.recv_vadr[i], sizeof(recvlist));
	}

	/*
	 *  If the mbuf is to be freed (and is allocated),
	 *  call d_complete to "un_dma" it, and then free it
	 */
	if ( (m = WRK.recv_mbuf[i]) && free_mbuf) {
	    rc = d_complete (WRK.dma_chnl_id, DMA_READ,
			 MTOD( m, unsigned short *), m->m_len,
			 M_XMEMD(m),
			 WRK.recv_addr[i]);
	
	    if ( rc != DMA_SUCC ) {
		TRACE_BOTH(MON_OTHER, "FooT", RCV_CRC_0, rc, 0);
		logerr( p_dds, ERRID_CTOK_DEVICE_ERR, __LINE__, __FILE__);
	    }
	    m_freem(m);
	    WRK.recv_mbuf[i] = NULL;
	}
    }
    
    WRK.read_index    = 0;
} /* end clear_recv_chain */

/*--------------------  A R M _ R E C V _ L I S T  ---------------------*/
/*                                                                      */
/*  NAME: arm_recv_list                                                 */
/*                                                                      */
/*  FUNCTION:                                                           */
/*    Rearms the receive list indexed by i and d_kmoves it thru the     */ 
/*    IOCC cache back to the receive chain.				*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      This routine can be executed by both the interrupt and          */
/*      process call threads of the driver.                             */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*                                                                      */
/*  RETURNS: VOID                                                       */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  d_kmove, bcopy                         */
/*                                                                      */
/*----------------------------------------------------------------------*/
void
arm_recv_list (
	dds_t *p_dds,                  /* pointer to dds     */
	int   i)                       /* which receive list */
{
	volatile recv_list_t	recvlist;
	uchar		*recvaddr;
	int		rc;
    
	recvaddr = WRK.recv_addr[i];
	recvlist.next    = WRK.recv_list [ (i + 1) % RCV_CHAIN_SIZE ];
	recvlist.status = (FRAME_INTERRUPT | VALID);
	recvlist.frame_size = 0;
	recvlist.count  = CLBYTES;
	recvlist.addr_hi = ADDR_HI( recvaddr );
	recvlist.addr_lo = ADDR_LO( recvaddr );
    
	/*
	 *  Update the receive dma image of the list by d_moving it
	 *  through the IOCC cache into system memory.
	 */
	if (WRK.do_dkmove) {
	    d_kmove (&recvlist, 
		  WRK.recv_list[i], 
		  sizeof(recvlist),
		  WRK.dma_chnl_id, 
		  DDI.bus_id, 
		  DMA_WRITE_ONLY);
	} else {
		bcopy (&recvlist, WRK.recv_vadr[i], sizeof(recvlist));
	}
}
