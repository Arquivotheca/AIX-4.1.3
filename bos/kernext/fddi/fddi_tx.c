static char sccsid[] = "@(#)75	1.17  src/bos/kernext/fddi/fddi_tx.c, sysxfddi, bos411, 9437C411a 9/15/94 14:23:57";
/*
 *   COMPONENT_NAME: SYSXFDDI
 *
 *   FUNCTIONS: fddi_output
 *		fddi_tx_to
 *		free_tx
 *		init_tx
 *		send_frame
 *		tx_handler
 *		undo_tx
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

/* externs */
extern	fddi_tbl_t	fddi_tbl;

/*
 * NAME:     fddi_output
 *
 * FUNCTION: services output requests
 *
 * EXECUTION ENVIRONMENT:  Kernel process thread
 *
 *	This function can be called with interrupts disabled to
 *	the same level as our operation level, a less favored
 *	level, or wide open. We need to provide for the most secure
 *	thread by disabling interrupts of other threads to the most 
 *	protected case.
 *
 * NOTES: 
 *
 *	This routine does NOT perform any parameter checks by design!
 *
 *	This routine can service multiple frame writes. Each frame
 *	is made up of CFDDI_MAX_GATHERS worth of mbufs chained 
 *	together with the m_next field of the mbuf. In turn, 
 *	frames are chained together with the m_nextpkt field.  
 *
 *	Mbuf chains look like:
 *
 *   p_mbuf___     ____	     ____      ____   multiple mbufs in a frame
 *            |-->|    |--->|	 |--->|	   |---|     connected with the m_next
 *      	  |____|    |____|    |____|	     field
 *		    |
 *  Frames chained  |
 *  by m_nextpkt   _V__      ____       
 *  field  	  |    |--->|	 |---|  
 *      	  |____|    |____|
 *                 _|_
 *	
 * RECOVERY OPERATION:
 *
 *	If the queue on the adapter lacks the room to do any of the 
 *	request then the routine returns EAGAIN to the caller.
 *	Otherwise, the driver will put what it can on the queue and free the 
 *	rest of the request.  In this case it will return success.
 *
 * RETURNS:  
 *
 * 	0	- Success
 *	EAGAIN	- Indicates not enough resouces to service request
 *	ENETUNREACH - Indicates Network Recovery Mode
 *	EINVAL	- Called while the driver is an illegal state or LLC_DOWN
 *			and this routine is called with an llc frame
 *	ENETDOWN - Indicates Network is in unrecoverable state
 */

int
fddi_output (
	fddi_acs_t	*p_acs,
	struct mbuf	*p_mbuf) 	/* chain of frames */
{
	int		rc;		/* return code */
	int		tx_cmd_flag;     /* flag set if a packet was sent to 
					 * send_frame (to tell the routine
					 * to issue a xmit command to the
					 * adapter).
					 */
	int		ipri;		/* save interrupt priority */
	struct mbuf	*p_tmp;

  	/*
   	 * Accounting for MIBs - ifHCOutUcastPkts, ifHCOutMulticastPkts 
   	 * and ifHCOutBroadcastPkts.
   	 */
  	p_tmp = p_mbuf;

  	while (p_tmp) 
	{
		if (p_tmp->m_flags & M_BCAST) 
		{
	  		if (++p_acs->ndd.ndd_genstats.ndd_ifOutBcastPkts_lsw == 0)
	    			p_acs->ndd.ndd_genstats.ndd_ifOutBcastPkts_msw++;
		}
		else 	
		if (p_tmp->m_flags & M_MCAST)
		{
		  	if (++p_acs->ndd.ndd_genstats.ndd_ifOutMcastPkts_lsw == 0)
		    		p_acs->ndd.ndd_genstats.ndd_ifOutMcastPkts_msw++;
		}
		else 
		{
			if (++p_acs->ndd.ndd_genstats.ndd_ifOutUcastPkts_lsw == 0)
		  		p_acs->ndd.ndd_genstats.ndd_ifOutUcastPkts_msw++;
		}
		p_tmp = p_tmp->m_nextpkt;
  	}

	/*
	 * Lock to syncronize with the tx_handler (tx completion routine) and
  	 * network recovery routines.
	 */
	ipri = disable_lock ( CFDDI_OPLEVEL, &p_acs->tx.lock);

	FDDI_TTRACE("TfoB",p_acs, p_acs->tx.hdw_in_use,p_acs->tx.sw_in_use);
	FDDI_TTRACE("TfoC",p_mbuf,0,0);

	/* check state */
	if (p_acs->dev.state != OPEN_STATE)
	{
		if (p_acs->dev.state == LIMBO_STATE || 
			p_acs->dev.state == LIMBO_RECOVERY_STATE)
		{
			FDDI_ETTRACE("Tfo1",p_acs, p_acs->dev.state,
				p_acs->ndd.ndd_flags);
			rc = ENETUNREACH;
		}
		else
		if (p_acs->dev.state == DEAD_STATE)
		{
			FDDI_ETTRACE("Tfo2",p_acs, p_acs->dev.state,0);
			rc = ENETDOWN;
		}
		else 
		{
			FDDI_ETTRACE("Tfo3",p_acs, p_acs->dev.state,0);
			rc = EINVAL;
		}
		unlock_enable(ipri, &p_acs->tx.lock);
		return (rc);
	}

	if ((p_acs->ndd.ndd_flags & CFDDI_NDD_LLC_DOWN) && 
		(FDDI_FC(p_mbuf) == FDDI_FC_LLC))
	{
		FDDI_ETTRACE("Tfo4",p_acs,FDDI_FC(p_mbuf),p_acs->ndd.ndd_flags);
		unlock_enable(ipri, &p_acs->tx.lock);
		return(ENETUNREACH);
	}
		
	/* 
	 * Transmit the request one frame at a time 
	 * 	Tx all the frames we can fit on the queue.
	 * Initialize the rc to EAGAIN (and set it to 0 when one frame has been
 	 * added to a queue).  
	 */
	rc = EAGAIN;
	tx_cmd_flag = FALSE;
	while (p_mbuf)
	{
		/* 
		 * Check to see if anything is on the software tx queue (if so 
		 * the frames must go on that queue to preserve order) otherwise
		 * check the adapter queue to see if the frame will fit.
		 */
		if ((p_acs->tx.sw_in_use) || 
			(QUE_FULL (p_acs->tx.hdw_in_use, MBUF_CNT(p_mbuf),
							FDDI_MAX_TX_DESC)))
		{
			/* Check the software tx queue for room */
			if (QUE_FULL(p_acs->tx.sw_in_use, MBUF_CNT(p_mbuf), 
				p_acs->dds.tx_que_sz))
			{
				/* 
				 * if we have put a frame from from this 
				 * request on the queue, then free the rest 
				 * of the mbufs.
				 */
				while (p_mbuf)
				{
					if (rc != EAGAIN)
					{
						p_tmp = p_mbuf;
						p_mbuf = p_mbuf->m_nextpkt;
						m_freem(p_tmp);
						p_acs->ndd.ndd_genstats.ndd_opackets_drop++;
					}
					else
					{
						p_mbuf = p_mbuf->m_nextpkt;
					}
					p_acs->ndd.ndd_genstats.ndd_xmitque_ovf++;
				}

				break;		
			}


			/* 
		   	 * Set rc to 0 since one frame, at least, is going on 
			 * the software queue.
			 */
			rc = 0;

			/*
		 	 * Netpmon trace.  Traces the queuing of the packet
		 	 */
			FDDI_TTRACE("WQUE", p_acs->dds.slot, p_mbuf, 
				p_mbuf->m_pkthdr.len);

			p_acs->tx.sw_in_use += MBUF_CNT(p_mbuf);
			if (p_acs->tx.sw_in_use > 
					p_acs->ndd.ndd_genstats.ndd_xmitque_max)
			{
				p_acs->ndd.ndd_genstats.ndd_xmitque_max=
					p_acs->tx.sw_in_use;
			}

			p_tmp = p_acs->tx.p_sw_que;

			if (p_tmp == 0)
			{
				p_acs->tx.p_sw_que = p_mbuf;
				p_mbuf = p_mbuf->m_nextpkt;
				p_acs->tx.p_sw_que->m_nextpkt = 0;
			}
			else 
			{
				while (p_tmp->m_nextpkt != 0)
					p_tmp = p_tmp->m_nextpkt;

				p_tmp->m_nextpkt = p_mbuf;
				p_mbuf = p_mbuf->m_nextpkt;
				p_tmp->m_nextpkt->m_nextpkt = 0;
			}
		}
		else
		{

			/*
		 	 * Netpmon trace.  Traces the queuing of the packet
		 	 */
			FDDI_TTRACE("WQUE", p_acs->dds.slot, p_mbuf, 
				p_mbuf->m_pkthdr.len);

			/* transmit this frame */
			if (send_frame (p_acs, p_mbuf))
			{
				tx_cmd_flag = FALSE;
				break;
			}

			/* 
		   	 * Set rc to 0 since one frame, at least, is going on 
			 * the software queue.
			 */
			rc = 0;

			tx_cmd_flag = TRUE;

			p_mbuf = p_mbuf->m_nextpkt;
		}
	}

	/*
	 * interrupt the adapter with a transmit request
	 *	(once per request)
	 */
	if (tx_cmd_flag)
	{
		caddr_t 	ioa;

		ioa = (caddr_t) BUSIO_ATT (p_acs->dds.bus_id, 
				(caddr_t) p_acs->dds.bus_io_addr);
		PIO_PUTSRX (ioa + FDDI_NS1_REG, FDDI_NS1_XMA);
		BUSIO_DET (ioa);

		/* start (or restart) watchdog timer */
		w_start (&(p_acs->tx.wdt));
	}

	unlock_enable(ipri, &p_acs->tx.lock);

	/* 
	 * Check to see if there has been a pio error.  If so bugout.
	 */
	if (p_acs->dev.pio_rc)
	{
		FDDI_ETTRACE("Tfo5",p_acs, 0,0);
		if (p_mbuf != 0)
			p_mbuf=p_mbuf->m_nextpkt;
		while (p_mbuf)
		{
			p_tmp = p_mbuf;
			p_mbuf = p_mbuf->m_nextpkt;
			m_freem(p_tmp);
			p_acs->ndd.ndd_genstats.ndd_opackets_drop++;
		}
		fddi_logerr(p_acs, ERRID_CFDDI_PIO, __LINE__, __FILE__,
				0, 0, 0);
		bugout(p_acs,NDD_PIO_FAIL,0,0,FALSE);
		return(0);
	}


	FDDI_TTRACE("TfoE",rc, p_acs->tx.hdw_in_use,p_acs->tx.sw_in_use);
	
	return (rc);
}

/*
 * NAME:     send_frame
 *
 * FUNCTION: Physically send a frame of mbuf(s) out
 *
 * EXECUTION ENVIRONMENT: Process or interrupt thread with interrupts disabled.
 *
 * NOTES:
 *	This routine assumes that the frame will fit on the queue; this is 
 * 	assumed to have been checked in the calling routine.  Also the tx lock
 *	is assumed to be held.
 *
 *	Setup the SOF descriptor with SOF bit set in control, the
 *	mbuf of the first buffer in the frame (0 len or not), and
 *	any extra processing due to the presence of an extension.
 *	Then for each descriptor setup the dma. Finally, set the
 *	EOF bit in the last descriptor that got an mbuf and do
 *	the Physical IO to effect the transmit.
 *
 *	The dma is usually done directly from the mbuf data to the adapter.
 *	However, on short frames of < FDDI_SF_BUFSIZ the data is copied to
 *	a 'smallframe' cache that the dma controller already knows about. 
 *	This saves time setting up dma. 
 *
 *	The 'smallframe' cache is carved up into FDDI_SF_BUFSIZ chunks or 
 *	sections. The number of sections in the 'smallframe' cache is equal to
 *	the number of  descriptors on the adapter so that there are always 
 *	the same number of free sections as there are free descriptors
 *	on the adapter.
 *
 * RETURNS:  a Boolean set to whether or not there has been a pio failure.
 */

static int
send_frame (
	fddi_acs_t	*p_acs,		/* per adapter structure */
	struct mbuf 	*p_mbuf) 	/* frame */
{
	fddi_adap_t	adap;		/* for doing the PIO to shared mem */
	uchar		sof_idx;	/* index to the SOF descriptor for tx */
	short		eof_jump;	/* relative index to EOF from SOF */
	fddi_tx_desc_t	*p_sof;		/* start of frame descriptor */
	fddi_tx_desc_t	*p_tx;		/* tx host descriptor */
	int		bus;		/* for PIO to shared memory */


	FDDI_TTRACE("TsfB",p_acs, p_acs->tx.hdw_in_use,p_acs->tx.sw_in_use);
	FDDI_TTRACE("TsfC",p_acs->tx.hdw_nxt_req,0,0);

	/*
	 * Call ndd_trace if it is enabled
	 */
	if (p_acs->ndd.ndd_trace) 
	{
		(*(p_acs->ndd.ndd_trace))(p_acs, p_mbuf, 
			p_mbuf->m_data, p_acs->ndd.ndd_trace_arg);
	}

	/* 
	 * setup SOF host descriptor 
	 *	(remember sof_idx for the PIO at bottom of routine)
	 */

	sof_idx = p_acs->tx.hdw_nxt_req;
	p_sof = &(p_acs->tx.desc[sof_idx]);
	p_sof->p_mbuf = p_mbuf;
	INCREMENT (p_acs->tx.hdw_nxt_req, 1, FDDI_MAX_TX_DESC);

	/* skip leading 0 len buffers */
	while ((p_mbuf) && (p_mbuf->m_len == 0))
	{
		p_mbuf = p_mbuf->m_next;
	}

	/* initialize */
	p_tx = p_sof;
	eof_jump = 0;

	/* process the mbufs in this frame */
	while (TRUE)
	{
		/* another descriptor in use (ready for tx) */
		p_acs->tx.hdw_in_use ++;	

		/* 
		 * Setup the dma for this descriptor 
		 *	based on len of the mbuf: small bufs are
		 *	copied to a small frame area already setup
		 *	for DMA.  Save the length of this mbuf in the descriptor
		 */
		if ((p_tx->adap.cnt = p_mbuf->m_len) < FDDI_SF_BUFSIZ)
		{
			/* Setup DMA address for using the 'smallframe' cache */
			*(uint *) &(p_tx->adap.addr_hi) = p_tx->p_d_sf;

			/* 
			 * Copy mbuf data to 'smallframe' cache 
			 * NOTE : This routine will have to change when 
			 * 	the driver wants to accept user space data.
			 */
 			bcopy (MTOD(p_mbuf, char *), p_tx->p_sf, p_mbuf->m_len);

			d_cflush ( p_acs->dev.dma_channel,
				p_tx->p_sf,
				p_mbuf->m_len,
				p_tx->p_d_sf);
		}
		else
		{
			/* 
			 * Set DMA address for mbuf 
			 * (this sets the hi and lo address bits)
			 */
			*(uint *) &(p_tx->adap.addr_hi) = (p_tx->p_d_addr | 
					(MTOD(p_mbuf, uint) & (PAGESIZE - 1)));

			d_master (p_acs->dev.dma_channel, 
				DMA_WRITE_ONLY, 
				MTOD (p_mbuf, char *), 
				(size_t) p_mbuf->m_len,
				M_XMEMD (p_mbuf), 
				(char *) *(uint *) &p_tx->adap.addr_hi);
		}

		/* 
		 * Initialize the control field of the transmit descriptor
		 */
		p_tx->adap.ctl = FDDI_TX_CTL_BDV;

		/* get next mbuf in frame */
		p_mbuf = p_mbuf->m_next;

		/* skip leading 0 len buffers */
		while ((p_mbuf) && (p_mbuf->m_len == 0))
		{
			p_mbuf = p_mbuf->m_next;
		}

		/* check end */
		if (p_mbuf == NULL)
		{
			/* last mbuf was just processed */
			break;
		}

		/*
		 * There is another mbuf so increment
		 *	counters and indexes
		 */
		p_tx = &(p_acs->tx.desc[p_acs->tx.hdw_nxt_req]);
		INCREMENT (p_acs->tx.hdw_nxt_req, 1, FDDI_MAX_TX_DESC);
		eof_jump ++;

	} /* end while (TRUE) */

	/* 
	 * p_tx points to the last descriptor so set EOF control 
	 *	Set IFX (interrupt when done) bit on all the EOF descriptors. 
	 *	If we post multiple writes then we
	 *	will get multiple interrupts but in fact all the
	 *	descriptors that are ready will be handled on the first
	 * 	interrupt.
	 * p_sof still points to the first frame and it also has 
	 *  	another flag to turn on in the ctl field of the 
	 *	descriptor so 'or' it in.
	 * The trf mask includes the bits in the ext flag field which
	 *	control the processing of the frame.  The bits corresponds to 
 	 *	the adapter's descriptor definition of bits (shifted slightly)
	 */
	
	p_tx->adap.ctl |= FDDI_TX_CTL_EOF | FDDI_TX_CTL_IFX;
	p_sof->adap.ctl |= FDDI_TX_CTL_SOF | 
		(*(MTOD(p_sof->p_mbuf, short *)) & FDDI_TRF_MSK);

	/* 
	 * save the relative offset
	 *	from the SOF to the EOF in the eof_jump field
	 */
	p_sof->eof_jump = eof_jump;

	/* 
	 * do the PIO starting from the SOF descriptor 
	 *	walking to the EOF descriptor until 
	 *	all the descriptors are posted.
	 */
	bus = BUSMEM_ATT (p_acs->dds.bus_id, (ulong)p_acs->dds.bus_mem_addr);
	adap.stat = 0;

	while (TRUE)
	{
		/*
		 * Do all the byte swapping into auto storage and
		 *	PIO the whole structure with one streaming 
		 *	block copy.
		 */
		*(uint *) &(adap.addr_hi) = 
				SWAPLONG (*(uint *) &(p_sof->adap.addr_hi));
		adap.cnt = SWAPSHORT (p_sof->adap.cnt);
		adap.ctl = SWAPSHORT (p_sof->adap.ctl);

		/* PIO the tx descriptor to the adapter. */
		PIO_PUTSTRX (bus + p_sof->offset, &adap, sizeof (adap));

		/* check finish condition */
		if ((p_sof == p_tx) || (p_acs->dev.pio_rc))
		{
			/* 
			 * all descriptors are now posted on the 
			 *	adapter - we're done 
			 */
			break;
		}
		/* go to the next descriptor */
		INCREMENT (sof_idx, 1, FDDI_MAX_TX_DESC);
		p_sof = &(p_acs->tx.desc[sof_idx]);
	} 
	BUSMEM_DET(bus);


	FDDI_TTRACE("TsfE",p_acs, p_acs->tx.hdw_in_use,p_acs->tx.sw_in_use);
	/* ok */
	return (p_acs->dev.pio_rc);
}

/*
 * NAME:     tx_handler
 *
 * FUNCTION: transmit complete interrupt handler
 *
 * EXECUTION ENVIRONMENT: interrupt environment
 *
 * NOTES:
 *	This routine is invoked in response to a TIA (transmit command 
 *	interrupt in the HSR). 
 *
 *	This routine will get and hold the tx lock for the length of the routine
 *
 *	The completions are processed a frame at a time:
 *		Check to see if the next frame to be completed, pointed to 
 * 		by the 'hdw_nxt_cmplt' index, is complete.  If it is 
 *		free the mbuf and increment the counters.
 *	
 *		Check to see if there are any frames on the software queue,
 *		if there are and they will fit, move them to the hardware queue.
 *		
 *		Loop to the top and check the next frame until out of frames or
 *		one is not completed.
 *
 * RETURNS:  
 *
 *	0 	- ok
 */

void
tx_handler (
        fddi_acs_t	*p_acs,
	int		bus)
{
	fddi_tx_desc_t	*p_sof;		/* ptr for start of frame */
	fddi_tx_desc_t	*p_eof;		/* ptr for end of frame */
	uchar		sof_idx;	/* index for the sof descriptor */
	uchar		eof_idx;	/* index for the eof descriptor */
	ushort		stat;		/* status field read from adapter */
	uchar		desc_cnt;	/* the no of descr in current frame */
	int		rc;		/* return code from called funcs */
	int		tx_rc;		/* return code to fddi_frame_done () */
	cfddi_hdr_t 	*p_hdr;
	int		ipri;
	caddr_t 	ioa;
	struct mbuf	*p_tmp; 	/* Temp handler of mbufs for chains */
	struct mbuf 	*p_mbuf;

	
	ipri = disable_lock(CFDDI_OPLEVEL,&p_acs->tx.lock);
	FDDI_TTRACE("TthB",p_acs, p_acs->tx.hdw_in_use,p_acs->tx.sw_in_use);
	FDDI_TTRACE("TthC",p_acs->tx.hdw_nxt_cmplt,0,0);

	/* stop tx watchdog timer */
	w_stop (&(p_acs->tx.wdt));

	p_tmp = p_mbuf = 0;

	stat = 0;

	if (++p_acs->ndd.ndd_genstats.ndd_xmitintr_lsw == 0)
		p_acs->ndd.ndd_genstats.ndd_xmitintr_msw++;

	/*
	 * Loop until at most all the tx descriptors in use are completed. 
	 * 	We can break out of this loop with tx.hdw_in_use > 0 
	 *	because adapter hasn't transmitted this one yet.
	 */
	while (p_acs->tx.hdw_in_use > 0)
	{
		/* 
		 * Get SOF descriptor for this frame 
		 */
		sof_idx = p_acs->tx.hdw_nxt_cmplt;
        	p_sof = &(p_acs->tx.desc[sof_idx]);

		/*  
		 * Get EOF descriptor for this frame 
		 */
		if (p_sof->eof_jump == 0)
		{
			/* this is a one descriptor frame */
			p_eof = p_sof;
		}
		else
		{
			/* multi descriptor frame */
			eof_idx = sof_idx;
			INCREMENT (eof_idx, p_sof->eof_jump, FDDI_MAX_TX_DESC);
			p_eof = &(p_acs->tx.desc[eof_idx]);
		}
		/* 
		 * Get the adapter status on the EOF descriptor
		 *	Read the status of the EOF descriptor regardless
		 *	of the number of descriptors this frame contains.
		 */
		PIO_GETSRX (bus + p_eof->offset + offsetof (fddi_adap_t, stat),
				&stat);

		if (p_acs->dev.pio_rc)
		{
			break;
		}

		if (stat == 0)
		{
			/* 
			 * done processing this interrupt:
			 * 	adapter hasn't transmitted this one yet 
			 */
			break;
		}

		/*
		 * Netpmon trace.  Traces the completion of the transmit
		 */
		FDDI_TTRACE("WEND", p_acs->dds.slot, p_sof->p_mbuf, 
				p_sof->p_mbuf->m_pkthdr.len);

		if (stat & FDDI_TX_STAT_BTI)
		{
			/* 
			 * unrecoverable error encountered 
			 */
			break;
		}

		/* 
		 * calculate descriptor count and adjust index and counter 
		 */
		desc_cnt = p_sof->eof_jump + 1;
		p_acs->tx.hdw_in_use -= desc_cnt;
		INCREMENT (p_acs->tx.hdw_nxt_cmplt, desc_cnt, FDDI_MAX_TX_DESC);

		if (stat & FDDI_TX_STAT_ERR)
		{
			FDDI_ETTRACE("Tth ",p_acs, stat, p_sof->p_mbuf);
			FDDI_ASSERT(0);
			p_acs->ndd.ndd_genstats.ndd_oerrors++;
		}
		else 
		{
			p_hdr = (cfddi_hdr_t *)p_sof->p_mbuf->m_data;
			if (FDDI_IS_BCAST(p_hdr->dest))
			{
				p_acs->ls.bcast_tx_ok++;
			}
			else if (p_hdr->dest[0] & FDDI_GRP_ADDR)
			{
				p_acs->ls.mcast_tx_ok++;
			}

			/* Count the packet in statistics */
		

			if (++p_acs->ndd.ndd_genstats.ndd_opackets_lsw == 0)
				p_acs->ndd.ndd_genstats.ndd_opackets_msw++;

			p_acs->ndd.ndd_genstats.ndd_obytes_lsw += 
				p_sof->p_mbuf->m_pkthdr.len;

			if (p_sof->p_mbuf->m_pkthdr.len > 
					p_acs->ndd.ndd_genstats.ndd_obytes_lsw)
				p_acs->ndd.ndd_genstats.ndd_obytes_msw++;
		}


		p_tmp = p_sof->p_mbuf;
		while (p_tmp->m_next != 0)
			p_tmp = p_tmp->m_next;
		p_tmp->m_next = p_mbuf;
		p_mbuf = p_sof->p_mbuf;

		p_sof->p_mbuf = 0;
	
		/* 
		 * Time to see if anything is on the user's tx que 
		 */
		if (p_acs->tx.sw_in_use)
		{
			while (!(QUE_FULL(p_acs->tx.hdw_in_use,
				MBUF_CNT(p_acs->tx.p_sw_que),FDDI_MAX_TX_DESC))
				&& (p_acs->tx.sw_in_use))
			{
				p_acs->tx.sw_in_use -= 
					MBUF_CNT(p_acs->tx.p_sw_que);

				send_frame (p_acs, p_acs->tx.p_sw_que); 

				p_acs->tx.p_sw_que=
					p_acs->tx.p_sw_que->m_nextpkt;
			}
			/*
	 		 * interrupt the adapter with a transmit request
	 		 */
		
			ioa = (caddr_t) BUSIO_ATT (p_acs->dds.bus_id, 
				(caddr_t)p_acs->dds.bus_io_addr);
			PIO_PUTSRX (ioa + FDDI_NS1_REG, FDDI_NS1_XMA);
			BUSIO_DET (ioa);
		}			
				
	}

	/* 
	 * If there are still frames outstanding, restart the tx timer.
	 */
	if (p_acs->tx.hdw_in_use != 0)
	{
		w_start (&(p_acs->tx.wdt));
	}

	unlock_enable(ipri,&p_acs->tx.lock);

	if (p_mbuf != 0)
		m_freem(p_mbuf);

	if (p_acs->dev.pio_rc)
	{
		FDDI_ETTRACE("Tth3",p_acs, 0,0);
		bugout (p_acs, CFDDI_TX_ERROR, 0, stat, TRUE);
		return;
	}

	if (stat & FDDI_TX_STAT_BTI)
	{
		/* 
		 * unrecoverable error encountered 
		 */
		FDDI_ETTRACE("Tth4",p_acs, stat,0);
		p_acs->ndd.ndd_genstats.ndd_oerrors++;
		fddi_logerr(p_acs, ERRID_CFDDI_TX_ERR, __LINE__, 
			__FILE__, stat, eof_idx, 0);
		enter_limbo (p_acs, CFDDI_TX_ERROR, stat, TRUE);
		return;
	}

	FDDI_TTRACE("TthE",p_acs, p_acs->tx.hdw_in_use,p_acs->tx.sw_in_use);
	FDDI_TTRACE("TthC",p_acs->tx.hdw_nxt_req,0,0);
	return;
}

/*
 * NAME: fddi_tx_to
 *                                                                    
 * FUNCTION: watchdog timer expiration for tx.
 *                                                                    
 * EXECUTION ENVIRONMENT: timer thread
 *
 * NOTES: 
 *
 * RETURNS: 
 */  
void
fddi_tx_to (
	struct	watchdog *p_wdt)
{
	fddi_acs_t	*p_acs;
	int		ipri;


	/* get ACS */
	p_acs = (fddi_acs_t *) ((uint) p_wdt - 
		(offsetof (fddi_acs_tx_t, wdt) + offsetof (fddi_acs_t, tx)));

	FDDI_ETRACE("Ttt ",p_acs,p_acs->dev.hsr_events, p_acs->tx.hdw_in_use);

	fddi_logerr(p_acs, ERRID_CFDDI_TX_ERR, __LINE__, __FILE__,
				0, 0, 0);
	enter_limbo (p_acs, CFDDI_TX_ERROR, 0, FALSE);

	/* ok */
	return ;
}

/*
 * NAME: init_tx
 *                                                                    
 * FUNCTION: initialize the transmit data objects
 *                                                                    
 * EXECUTION ENVIRONMENT: process environment only
 *                                                                   
 * NOTES: 
 *
 *	This routine assumes that the small frame cache has already been 
 *	allocated, and that the services such as d_init have already been run.
 *	This routine does nothing to the adapter itself.
 *
 * 	Setup dma for the 'smallframe' cache used to transmit short frames 
 *	(by convention use the dma region at the end of tx region)
 * 
 *	Tx watchdog timer is initialized here, cleared in the 'free_tx'
 *	routine, started in the 'send_frame' routine, and stopped in the 
 *	'tx_handler'.
 *
 * RETURNS: 
 *	0	- successful
 *	ENOMEM	- Indicates a failure to malloc the small frame cache
 */  

int
init_tx (
	fddi_acs_t	*p_acs)
{
	fddi_tx_desc_t	*p_host;	/* ptr to descriptor */
	char		*p_sf;		/* ptr to smallframe cache */
	int		i;		/* index */


	FDDI_TTRACE("TitB",p_acs, 0, 0);

	/*
	 * Allocate pinned memory for the tx 'smallframe' cache 
	 */
	p_acs->tx.p_sf_cache = xmalloc(FDDI_SF_CACHE,DMA_L2PSIZE, pinned_heap);

	if (p_acs->tx.p_sf_cache == NULL)
	{
		FDDI_ETTRACE("Tit1",p_acs, 0,0);
		return (ENOMEM);
	}

	/* Setup generic xmem descriptor */
	p_acs->tx.xmd.aspace_id = XMEM_GLOBAL;

	/* Do one time d_master of 'smallframe' cache sections */
	d_master (p_acs->dev.dma_channel, 
		DMA_WRITE_ONLY, 
		p_acs->tx.p_sf_cache,
		(size_t) FDDI_SF_CACHE, 
		&p_acs->tx.xmd,
		(char *) p_acs->tx.p_d_sf);

	/* 
	 * Setup each tx host descriptor 
	 *	Initialize descriptor
	 *	setup dma address
	 *	setup offset value (never changes)
	 *	setup cache buffer and dma window for smallframes
  	 * Note
	 *	The dma and buffer space is assigned to the descriptors from 
	 *	bottom up.  This is done to aid in avoiding problems with the
 	 * 	cache since, the buffer will never have a d_complete issued on
	 *	it.
	 */

	p_host = &(p_acs->tx.desc[0]);
	for (i = 0; i < FDDI_MAX_TX_DESC; i++, p_host++)
	{
		int x;

		x = FDDI_MAX_TX_DESC-i-1;
		bzero (p_host, sizeof (p_host));

		p_host->p_d_addr = p_acs->tx.p_d_base + (3*x*DMA_PSIZE);
		p_host->offset = (i) * sizeof (fddi_adap_t);
		p_host->p_sf = p_acs->tx.p_sf_cache + (x*FDDI_SF_BUFSIZ);
		p_host->p_d_sf = p_acs->tx.p_d_sf + (x*FDDI_SF_BUFSIZ);
	}

	/* 
	 * initialize host tx queue variables 
	 *	and sleep cell
	 */
	p_acs->tx.hdw_in_use = 0;
	p_acs->tx.hdw_nxt_req = 0;
	p_acs->tx.hdw_nxt_cmplt = 0;
	p_acs->tx.sw_in_use = 0;
	p_acs->tx.p_sw_que = 0;

	lock_alloc(&p_acs->tx.lock, LOCK_ALLOC_PIN, FDDI_TX_LOCK, 
		p_acs->dds.slot);
	simple_lock_init(&p_acs->tx.lock);

	/* 
	 * initialize tx watchdog timer 
	 */
	p_acs->tx.wdt.restart = FDDI_TX_WDT_RESTART;
	p_acs->tx.wdt.func = fddi_tx_to;
	while (w_init (&p_acs->tx.wdt));

	FDDI_TTRACE("TitE",p_acs, 0, 0);
	return (0);
}

/*
 * NAME: free_tx
 *                                                                    
 * FUNCTION:  free what was allocated in init_tx
 *                                                                    
 * EXECUTION ENVIRONMENT: process environment
 *                                                                   
 * NOTES: 
 *	Assumes that there aren't any users at present.
 *
 * RETURNS: 
 */  
void
free_tx (
	fddi_acs_t	*p_acs)
{
	int		i;
	int		rc;

	FDDI_TTRACE("TftB",p_acs, 0, 0);

	xmfree(p_acs->tx.p_sf_cache, pinned_heap);

	/* remove tx timer from system */
	while (w_clear (&(p_acs->tx.wdt)));

	/* 
	 * reset dma
	 */
	rc = d_complete (
		p_acs->dev.dma_channel, 
		DMA_WRITE_ONLY, 
		p_acs->tx.p_sf_cache,
		FDDI_SF_BUFSIZ, 
		&(p_acs->tx.xmd),
		(caddr_t)p_acs->tx.p_d_sf);

	if (rc != DMA_SUCC)
	{
		/* 
		 * log this dma error : not serious because we
		 *	are shutdown.
		 */
		FDDI_ETTRACE("Tft1",p_acs, 0,0);
		FDDI_ASSERT(0);
	}

	/* 
	 * initialize host tx queue variables 
	 *	and sleep cell
	 */
	p_acs->tx.hdw_in_use = 0;
	p_acs->tx.hdw_nxt_req = 0;
	p_acs->tx.hdw_nxt_cmplt = 0;
	p_acs->tx.sw_in_use = 0;
	p_acs->tx.p_sw_que = 0;
	lock_free(&p_acs->tx.lock);

	FDDI_TTRACE("TftE",p_acs, 0, 0);
	/* ok */
	return ;
}


/*
 * NAME: undo_tx
 *                                                                    
 * FUNCTION:  clear the transmit queues
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt thread
 *                                                                   
 * NOTES: 
 *	This routine cleans up the transmit queues for the network recovery
 *	functions.
 *
 *	Assumes the transmit lock is held by calling routine.
 *
 * RETURNS: 
 */  
void
undo_tx (
	fddi_acs_t	*p_acs)
{
	int		i;
	struct mbuf	*p_mbuf;

	FDDI_TTRACE("TutB",p_acs, 0, 0);

	for (i=0; i<p_acs->tx.hdw_in_use; i++)
	{
		if (p_acs->tx.desc[p_acs->tx.hdw_nxt_cmplt].p_mbuf != 0)
		{
			m_freem(p_acs->tx.desc[p_acs->tx.hdw_nxt_cmplt].p_mbuf);
			p_acs->tx.desc[p_acs->tx.hdw_nxt_cmplt].p_mbuf = 0;
		}
		INCREMENT(p_acs->tx.hdw_nxt_cmplt, 1, FDDI_MAX_TX_DESC);
	}

	p_acs->ndd.ndd_genstats.ndd_opackets_drop += p_acs->tx.hdw_in_use + 
		p_acs->tx.sw_in_use;

	while (p_acs->tx.p_sw_que)
	{
		p_mbuf = p_acs->tx.p_sw_que;
		p_acs->tx.p_sw_que = p_acs->tx.p_sw_que->m_nextpkt;
		m_freem(p_mbuf);
	}
	
	/* 
	 * initialize host tx queue variables 
	 *	and sleep cell
	 */
	p_acs->tx.hdw_in_use = 0;
	p_acs->tx.hdw_nxt_req = 0;
	p_acs->tx.hdw_nxt_cmplt = 0;
	p_acs->tx.sw_in_use = 0;
	p_acs->tx.p_sw_que = 0;

	FDDI_TTRACE("TutE",p_acs, 0, 0);
	/* ok */
	return ;
}

