static char sccsid[] = "@(#)74	1.8  src/bos/kernext/fddi/fddi_rx.c, sysxfddi, bos411, 9428A410j 6/8/94 09:08:20";
/*
 *   COMPONENT_NAME: SYSXFDDI
 *
 *   FUNCTIONS: free_rx
 *		init_rx
 *		rx_handler
 *		start_rx
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
#include <sys/mbuf.h>
#include <sys/sleep.h>
#include <sys/malloc.h>

/*
 * NAME: rx_handler
 *
 * FUNCTION: interrupt handler for rx complete
 *
 * EXECUTION ENVIRONMENT: interrupt thread
 *
 * NOTES: 
 *	Checks the next descriptor to see if it has been filled by the adapter.
 *	If so copy it out into an mbuf.  Send it to the demuxer either in 
 * 	the receive routine or if it is a bad frame in a status block by the
 *	stat routine.  Then check the next frame to see if it has data in it.
 *
 * RETURNS: 
 */
void
rx_handler (
	fddi_acs_t	*p_acs,
	int		bus)
{
	uint		ctl;
	short		len;
	short		len1;
	ushort		stat;
	ndd_statblk_t	statblk;
	cfddi_hdr_t	*p_hdr;
	fddi_rx_desc_t	*p_rx;
	struct mbuf	*p_frame;

	FDDI_RTRACE("RrhB",p_acs, p_acs->rx.nxt_rx,0);

	if (++p_acs->ndd.ndd_genstats.ndd_recvintr_lsw == 0)
		p_acs->ndd.ndd_genstats.ndd_recvintr_msw++;

	while (TRUE)
	{
		/* get ptr to next rx desc to be processed */
		p_rx = &p_acs->rx.desc[p_acs->rx.nxt_rx];

		/* 
		 * get the ctl and stat fields of the descriptor with 
		 * one get long, the order of the shorts will be 
		 * reversed from the swap long.
		 */
		PIO_GETLRX(bus+p_rx->offset+ offsetof(fddi_adap_t, ctl),&ctl);
		if (p_acs->dev.pio_rc)
		{
			FDDI_ERTRACE("Rrh1",p_acs, p_acs->rx.nxt_rx,0);
			fddi_logerr(p_acs, ERRID_CFDDI_PIO, __LINE__, __FILE__,
				0, 0, 0);
			bugout(p_acs, NDD_PIO_FAIL, 0, 
				p_acs->dev.pio_rc, TRUE);
			return;
		}

		/* get the individual fields out of the int */
		stat = ctl >> 16;
		ctl = ctl & 0xFFFF;

		/*
		 * if the desc has the BDV bit set OR
		 * the adapter has not set SV (status valid)
		 * there is nothing for us to process
		 */
		if ((ctl & FDDI_RX_CTL_BDV) || (!(stat & FDDI_RX_STAT_SV)))
			break;
		
		PIO_GETSRX(bus+p_rx->offset+offsetof(fddi_adap_t,cnt), &len);
		if (p_acs->dev.pio_rc)
		{
			FDDI_ERTRACE("Rrh2",p_acs, p_acs->rx.nxt_rx,0);
			fddi_logerr(p_acs, ERRID_CFDDI_PIO, __LINE__, __FILE__,
				0, 0, 0);
			bugout(p_acs, NDD_PIO_FAIL, 0, p_acs->dev.pio_rc, 
				TRUE);
			return;
		}

		/*
		 * Netpmon trace.  Traces the reception of a packet
		 */
		FDDI_RTRACE("RDAT", p_acs->dds.slot, len, 0);

		d_bflush (p_acs->dev.dma_channel, p_acs->dds.bus_id, 
			p_rx->p_d_addr);
			
		/* Get an mbuf (little or big) to copy the data into. */
		if (len <= MHLEN)
			p_frame = m_gethdr(M_DONTWAIT, MT_DATA);
		else
			p_frame = m_getclust(M_DONTWAIT, MT_DATA);

		if (p_frame == NULL)
		{
			FDDI_ERTRACE("Rrh3",p_acs, p_acs->rx.nxt_rx,0);
			p_acs->ndd.ndd_genstats.ndd_ipackets_drop++;
			p_acs->ndd.ndd_genstats.ndd_nobufs++;

			FDDI_REARM();
			if (p_acs->dev.pio_rc)
			{
				FDDI_ERTRACE("Rrh4",p_acs, p_acs->rx.nxt_rx,0);
				fddi_logerr(p_acs, ERRID_CFDDI_PIO, 
					__LINE__, __FILE__,
					0, 0, 0);
				bugout(p_acs, NDD_PIO_FAIL, 0, 
					p_acs->dev.pio_rc, TRUE);
			}
			return;
		}

		/* 
	 	 * if the data wont fit in one cluster then get another buffer; 
		 * remembering the amount of data to put in the 
	 	 * first buffer in len1.
	 	 */
		if ( len <= CLBYTES )
		{
			p_frame->m_len = len;
			p_frame->m_next = NULL;

			bcopy(p_rx->p_buf,MTOD(p_frame, char *),len);
		}
		else
		{
			/* Get an mbuf (little or big) to copy the data into. */
			if (len <= (MHLEN+CLBYTES))
				p_frame->m_next = m_gethdr(M_DONTWAIT, MT_DATA);
			else
				p_frame->m_next =m_getclust(M_DONTWAIT,MT_DATA);

			if (p_frame->m_next == NULL)
			{
				FDDI_ERTRACE("Rrh5",p_acs, p_acs->rx.nxt_rx,0);
				p_acs->ndd.ndd_genstats.ndd_ipackets_drop++;
				p_acs->ndd.ndd_genstats.ndd_nobufs++;

				m_freem(p_frame);
				FDDI_REARM();
				if (p_acs->dev.pio_rc)
				{
					FDDI_ERTRACE("Rrh6",p_acs, 
						p_acs->rx.nxt_rx,0);
					fddi_logerr(p_acs, ERRID_CFDDI_PIO, 
						__LINE__, __FILE__,
						0, 0, 0);
					bugout(p_acs, NDD_PIO_FAIL, 0, 
						p_acs->dev.pio_rc, TRUE);
				}
				return;
			}

			/* 
		 	 * copy the first chunk of the frame
		 	 * into the p_frame mbuf.
		 	 */
			p_frame->m_len = CLBYTES;

			bcopy(p_rx->p_buf,MTOD(p_frame, char *),CLBYTES);

			/*
		 	 * copy the 2nd part of the frame into
		 	 * the p_frame->m_next mbuf
		 	 */
			p_frame->m_next->m_len = len-CLBYTES;

			p_frame->m_next->m_next = NULL;

			bcopy(p_rx->p_buf + CLBYTES,
			 	MTOD(p_frame->m_next, char *), (len-CLBYTES));
		}

		p_frame->m_pkthdr.len = len;
		p_frame->m_flags |= M_PKTHDR;

		/* 
		 * reset the buffer on the adapter for another receive.
		 */
		FDDI_REARM();

		if (p_acs->dev.pio_rc)
		{
			FDDI_ERTRACE("Rrh7",p_acs, p_acs->rx.nxt_rx,0);
			fddi_logerr(p_acs, ERRID_CFDDI_PIO, __LINE__, __FILE__,
				0, 0, 0);
			bugout(p_acs, NDD_PIO_FAIL, 0, p_acs->dev.pio_rc, 
				TRUE);
			m_freem(p_frame);
			return;
		}

		if ((stat & FDDI_RX_STAT_BTS) || (stat & FDDI_RX_STAT_FCS))
		{
			p_acs->ndd.ndd_genstats.ndd_ibadpackets++;

			statblk.code = NDD_BAD_PKTS;
			statblk.option[0] = 0;
			statblk.option[1] = p_frame;
			p_acs->ndd.nd_status(p_acs, &statblk);
			m_freem(p_frame);
		}
		else
		{
			/* Count the packet in statistics */
			p_hdr = (cfddi_hdr_t *)p_frame->m_data;

			if (FDDI_IS_BCAST(p_hdr->dest))
			{
				p_frame->m_flags |= M_BCAST;
				p_acs->ls.bcast_rx_ok++;
			}
			else if (p_hdr->dest[0] & FDDI_GRP_ADDR)
			{
				p_frame->m_flags |= M_MCAST;
				p_acs->ls.mcast_rx_ok++;
			}


			if (++p_acs->ndd.ndd_genstats.ndd_ipackets_lsw == 0)
				p_acs->ndd.ndd_genstats.ndd_ipackets_msw++;

			p_acs->ndd.ndd_genstats.ndd_ibytes_lsw += len;
			if (len > p_acs->ndd.ndd_genstats.ndd_ibytes_lsw)
				p_acs->ndd.ndd_genstats.ndd_ibytes_msw++;

			/*
		         * Netpmon trace.  Traces the delivery of the packet
		 	 */
			FDDI_TTRACE("RNOT", p_acs->dds.slot, p_frame, 
				p_frame->m_pkthdr.len);

			p_acs->ndd.nd_receive(p_acs,p_frame);

			FDDI_RTRACE("REND", p_acs->dds.slot, p_frame, 0);
		}
	
	} /* end while (TRUE) */

	FDDI_RTRACE("RrhE",p_acs, p_acs->rx.nxt_rx,0);
	/* ok */
	return;
}


/*
 * NAME: init_rx
 *                                                                    
 * FUNCTION: Do all the preparation to receive.  Obtain the resources and 
 *	calculate the the resource allocation to the individual descriptor.
 *                                                                    
 * EXECUTION ENVIRONMENT: called in process environment only
 *                                                                   
 * NOTES: 
 * 	Assumes that the receive buffer has been allocated.
 *
 * ASSUMPTIONS:
 *
 * RECOVERY OPERATION: 
 *
 *
 * DATA STRUCTURES: 
 *
 *	rx descriptor array in the open structure
 *
 * RETURNS: 
 *
 *	0 	- ok
 *	ENOMEM	- Indicates a failure to allocate the Receive Frames
 */  

int
init_rx (
	fddi_acs_t	*p_acs)
{
	int 		x,i;
	fddi_rx_desc_t	*p_rx;
	uint		p_d_addr;
	struct xmem	*p_xmd;
	caddr_t 	p_tmp;
	struct mbreq	mbreq;


	FDDI_RTRACE("RirB",p_acs, 0,0);

	/*
	 * Allocate pinned memory for rx cache.
	 * NB:
	 *	We use the length of the adjusted buffer size
	 *	to avoid problems with cache inconsistency
	 *	It should be 4500 bytes rounded up to the 
	 *	64 byte cache boundary.
	 */
	p_acs->rx.p_rx_cache = xmalloc((p_acs->rx.l_adj_buf*FDDI_MAX_RX_DESC), 
			DMA_L2PSIZE, pinned_heap);

	if (p_acs->rx.p_rx_cache == NULL)
	{
		FDDI_ERTRACE ("Rir1", p_acs, 0, 0);
		return (ENOMEM);
	}

	/*
	 * Initialize the rearm values which will stay constant
	 *	through this configuration (used to init the constant
 	 * 	section of a rx descriptor
	 */
	p_acs->rx.arm_val.cnt = SWAPSHORT( p_acs->rx.l_adj_buf);
	p_acs->rx.arm_val.ctl = SWAPSHORT(FDDI_RX_CTL_BDV| FDDI_RX_CTL_IFR);
	p_acs->rx.arm_val.stat = 0;

	/* 
	 * Get generic xmem descriptor for the mbuf
	 */
	p_acs->rx.xmd.aspace_id = XMEM_GLOBAL;
	p_d_addr = p_acs->dds.dma_base_addr;

	/* setup dma */
	d_master(p_acs->dev.dma_channel, DMA_READ | DMA_NOHIDE, 
		p_acs->rx.p_rx_cache, p_acs->rx.l_adj_buf*FDDI_MAX_RX_DESC,
		&p_acs->rx.xmd, p_d_addr);

	p_tmp = p_acs->rx.p_rx_cache;

	/* Calculate the values of memory locations for each descriptor */
	for (i=0; i<FDDI_MAX_RX_DESC; i++)
	{
		p_acs->rx.desc[i].offset = i * sizeof(struct fddi_adap)
			+ FDDI_RX_SHARED_RAM; 
		p_acs->rx.desc[i].p_buf = p_tmp;
		p_acs->rx.desc[i].p_d_addr = p_d_addr;

		p_d_addr += p_acs->rx.l_adj_buf;
		p_tmp += p_acs->rx.l_adj_buf;
	}

	/* initialize rx indexes */
	p_acs->rx.nxt_rx = 0;

	FDDI_RTRACE("RirE",p_acs, 0,0);
	return (0);
}

/*
 * NAME: free_rx
 *                                                                    
 * FUNCTION: undo the initialzation done in init_rx
 *                                                                    
 * EXECUTION ENVIRONMENT: called by process environment only
 *                                                                   
 * NOTES: 
 *
 * RETURNS: 
 *		0 - successful
 */  

void
free_rx (
	fddi_acs_t	*p_acs)
{
	int 		i;

	FDDI_RTRACE("RfrB",p_acs, 0,0);

	xmfree(p_acs->rx.p_rx_cache, pinned_heap);

	d_complete(p_acs->dev.dma_channel, DMA_READ | DMA_NOHIDE, 
		p_acs->rx.p_rx_cache, 
		p_acs->rx.l_adj_buf*FDDI_MAX_RX_DESC,
		&p_acs->rx.xmd, p_acs->dds.dma_base_addr);

	/* initialize rx indexes */
	p_acs->rx.nxt_rx = 0;

	FDDI_RTRACE("RfrE",p_acs, 0,0);
	return;
}

/*
 * NAME: start_rx
 *                                                                    
 * FUNCTION: set the receive descriptors on the adapter, and send the initial
 * 		receive command.
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt thread.
 *                                                                   
 * RETURNS: 
 *	Boolean : set to whether there has been a pio failure
 *
 */  

int
start_rx (
	fddi_acs_t	*p_acs)
{
	int 		i;
	int		bus;
	int 		ioa;

	bus = BUSMEM_ATT (p_acs->dds.bus_id, (ulong)p_acs->dds.bus_mem_addr);

	/* Calculate the values of memory locations for each descriptor */
	for (i=0; i<FDDI_MAX_RX_DESC; i++)
	{
		PIO_PUTLRX(bus+p_acs->rx.desc[i].offset, 
			p_acs->rx.desc[i].p_d_addr);

		PIO_PUTSTRX(bus+p_acs->rx.desc[i].offset+
			offsetof(fddi_adap_t,cnt),
			&p_acs->rx.arm_val,
			sizeof(p_acs->rx.arm_val));
	}

	BUSMEM_DET(bus);

	p_acs->rx.nxt_rx = 0;

	ioa = BUSIO_ATT(p_acs->dds.bus_id, p_acs->dds.bus_io_addr);

        /* issue rcv cmd to the NS1 register */
        PIO_PUTSRX(ioa+FDDI_NS1_REG, FDDI_NS1_RX);
	
        BUSIO_DET(ioa);

	return(p_acs->dev.pio_rc);
}
