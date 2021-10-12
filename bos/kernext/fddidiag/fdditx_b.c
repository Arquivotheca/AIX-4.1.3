static char sccsid[] = "@(#)93	1.2  src/bos/kernext/fddidiag/fdditx_b.c, diagddfddi, bos411, 9428A410j 11/8/93 09:52:05";
/*
 *   COMPONENT_NAME: DIAGDDFDDI
 *
 *   FUNCTIONS: MBUF_CNT
 *		PROCESS_EXT
 *		fddi_chk_frame
 *		fddi_chk_kernel_mbufs1197
 *		fddi_chk_state
 *		fddi_dma_cleanup
 *		fddi_fastwrite
 *		fddi_frame_done
 *		fddi_tx
 *		fddi_tx_to
 *		fddi_undo_start_tx1626
 *		proc_ext
 *		que_full_handler1349
 *		send_frame
 *		setup_ext
 *		tx_handler
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "fddiproto.h"
#include <sys/trchkid.h>	/* for performance hooks */
#include <sys/dma.h>
#include <sys/mbuf.h>
#include <sys/errno.h>
#include <sys/poll.h>
#include "fddi_comio_errids.h"
#include <sys/sleep.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>

/* externs */
extern	fddi_ctl_t	fddi_ctl;

/* statics */
static void send_frame (
	fddi_acs_t	*p_acs,		/* per adapter structure */
	struct mbuf 	*p_mbuf); 	/* frame */

static void setup_ext(
	fddi_acs_t	*p_acs,		/* per adapter struc */
	fddi_open_t	*p_open,	/* per open structure */
	cio_write_ext_t	*p_ext);	/* write extension */

static void proc_ext (
	fddi_acs_t	*p_acs,
	fddi_tx_t	*p_sof,
	fddi_open_t	*p_open,
	uint		tx_rc,
	ushort		stat);

static int que_full_handler (
	fddi_acs_t 	*p_acs,
	fddi_open_t	*p_open,
	struct mbuf	*p_mbuf,
	cio_write_ext_t *p_ext);
/*
 * NAME: fddi_tx
 *                                                                    
 * FUNCTION: resource checking function for fddiwrite() entry point
 *                                                                    
 * EXECUTION ENVIRONMENT: Process thread with interrupts disabled.
 *
 *	Called by: fddi_write()
 *
 *	This routine will disable interrupts upon being called on the
 *	process thread. The code and data in this function are non-pageable.
 *
 *	This is not executing in an interrupt environment so e_sleep 
 *	may be called as long as we are sleeping on pinned data.
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 *	The host descriptors are filled in here. The indexes to manage the
 *	host descriptors are checked and potentially updated.
 *
 * RETURNS: 
 *
 *	0 	- successful
 *	rc 	- from que_full_handler()
 */  

int
fddi_tx (
	fddi_acs_t	*p_acs, 	/* the ACS */
	fddi_open_t	*p_open, 	/* caller's open structure */
	cio_write_ext_t	*p_ext, 	/* caller's extension */
	struct	mbuf	*p_mbuf,	/* mbufs to transmit */ 
	short		gather_cnt)	/* no. of mbufs to tx */
{
	int		ipri;		/* previous interrupt level */
	int		rc;

	/* serialize with the offlevel */
	ipri = i_disable (INTOFFL1);

	/* 
	 * Check to see if the software queue has anything on it (if so the
	 * frame must go on the software queue to preserve the order received)
	 * otherwise check to see if the adapter's tx descriptor queue is full. 
	 */
	if ((p_acs->tx.tq_cnt) || (QUE_FULL (p_acs->tx.in_use, gather_cnt,
						FDDI_MAX_TX_DESC)))
	{
		fddi_tx_que_t	*p_tx_que;

		FDDI_DBTRACE("Wrt1",p_acs->tx.tq_cnt, 
			p_acs->tx.in_use+gather_cnt, p_acs->dds.tx_que_sz);
		
		/* Check to see if the software tx queue is full. */
		while (TQ_FULL (p_acs->tx.in_use, gather_cnt))
		{
			/*
		 	* handler que full condition
		 	*/
			rc = que_full_handler (p_acs, p_open, p_mbuf, p_ext);
			if (rc)
			{
				/*
			 	* user may not want to wait or error occured
			 	*/
				i_enable (ipri);
				p_acs->ras.ds.tx_que_ovflw++;
				return (rc);
			}
		} /* end while (QUE_FULL) */
		
		DESC_TQ(p_tx_que, p_acs->tx.tq_in);


		p_tx_que->p_mbuf = p_mbuf;
		if (p_ext)
		{
			p_tx_que->p_open = p_open;
			p_tx_que->wr_ext = *p_ext;
		}
		else 
		{
			p_tx_que->p_open = 0;
		}
		p_tx_que->gather_cnt = gather_cnt;
		p_acs->tx.tq_cnt += p_tx_que->gather_cnt;

		FDDI_DBTRACE("Wrt3",p_acs->tx.tq_cnt, p_acs->tx.tq_in, 
				p_acs->tx.tq_out);

		INCRE_TQ(p_acs->tx.tq_in);

		if ( p_acs->tx.tq_cnt > p_acs->ras.cc.xmt_que_high)
			p_acs->ras.cc.xmt_que_high = p_acs->tx.tq_cnt;
		i_enable(ipri);
		return(0);	
	}

	
	if (p_ext)
	{
		/* 
		 * remember extension for tx completion time 
		 */
		setup_ext (p_acs, p_open, p_ext);

		send_frame (p_acs, p_mbuf);
	}
	else 
		send_frame (p_acs, p_mbuf);

	/*
	 * interrupt the adapter with a transmit request
	 */
	{
		caddr_t 	ioa;

		ioa = (caddr_t) BUSIO_ATT (p_acs->dds.bus_id, 
				(caddr_t)p_acs->dds.bus_io_addr);
		FDDI_PTRACE("txcn");
		PIO_PUTSRX (ioa + FDDI_NS1_REG, FDDI_NS1_XMA);
		BUSIO_DET (ioa);
	}
	/* start watchdog timer */
	w_start (&(p_acs->tx.wdt));

	/* ok */
	i_enable (ipri);
	return (0);
}

/*
 * NAME:     fddi_fastwrite
 *
 * FUNCTION: services fast write requests for kernel processes
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
 *	This routine services multiple frame writes. Each frame
 *	is made up of FDDI_MAX_GATHERS worth of mbufs chained 
 *	together with the m_next field of the mbuf. In turn, 
 *	frames are chained together with the m_nextpkt field.  
 *
 *	Mbuf chains look like:
 *
 *   p_mbuf___     ____	     ____      ____
 *            |-->|    |--->|	 |--->|	   |---|
 *      	  |____|    |____|    |____|
 *		    |
 *  Frames chained  |
 *  by m_nextpkt field _V__      ____       mbufs in frames connected by the
 *      	  |    |--->|	 |---|  m_next field
 *      	  |____|    |____|
 *                 _|_
 *	
 * RECOVERY OPERATION:
 *
 *	If the queue on the adapter lacks the room to do the entire
 *	request then the routine returns EAGAIN to the caller.
 *	However, all the packets that can go out will be transmitted.
 *
 * DATA STRUCTURES:
 *
 *	Uses the same implementation as the regular write entry point -
 *	fddi_write() with the exception of using the m_nextpkt field to chain
 *	together multiple frames.
 *
 * RETURNS:  
 *
 *	ENXIO	- invalid devno
 *	EAGAIN	- Indicates not enough resouces to service request
 *	ENETUNREACH - Indicates Network Recovery Mode
 *	ENOCONNECT - Indicates Network was not STARTed
 *	ENETDOWN - Indicates Network is in unrecoverable state
 */

/*
 * The MBUF_CNT macro will determine if we were passed a packet of
 *	1, 2 or 3 mbufs. These are the only legal values and no
 *	parameter checking is done (as is the case with fastwrite).
 */
#define MBUF_CNT(m)	\
	((m->m_next == NULL) ? 1 : (m->m_next->m_next == NULL) ? 2 : 3)
	
int
fddi_fastwrite (
	struct mbuf	*p_mbuf, 	/* chain of frames */
	int		devno) 		/* major/minor number */
{
	int		rc;		/* return code */
	int		ipri;		/* save interrupt priority */
	fddi_acs_t	*p_acs;		/* per adapter structure */
	struct mbuf	*p_tmp;


	FDDI_TRACE("TfrB", devno, p_mbuf, MTOD(p_mbuf, char *)); 

	/* No parameter checking ... */
	p_acs = fddi_ctl.p_acs[minor(devno)];

	/* check state */
	if (p_acs->dev.state != FDDI_OPEN)
	{
		/* check what state and return appropriate error code */
		rc = fddi_chk_state (p_acs, NULL);
		return (rc);
	}
		
	/*
	 * can be called on interrupt thread or PROCESS thread:
	 *	assume least protected case - PROCESS thread.
	 */
	ipri = i_disable (INTOFFL1);

	/* 
	 * Transmit the request one frame at a time 
	 * 	Tx all the frames we can fit on the queue.
	 */
	rc = EAGAIN;
	while (p_mbuf)
	{
		/* 
		 * Check to see if anything is on the software tx queue (if so 
		 * the frames must go on that queue to preserve order) otherwise
		 * check the adapter queue to see if the frame will fit.
		 */
		if ((p_acs->tx.tq_cnt) || 
			(QUE_FULL (p_acs->tx.in_use, MBUF_CNT(p_mbuf),
							FDDI_MAX_TX_DESC)))
		{
			fddi_tx_que_t *p_tx_que;
			FDDI_DBTRACE("Frt1",p_acs->tx.tq_cnt, p_acs->tx.in_use,
					MBUF_CNT(p_mbuf));

			/* Check the software tx queue */
			if (TQ_FULL(p_acs->tx.tq_cnt, MBUF_CNT(p_mbuf)))
			{
				/*
			 	 * Tx queue is full so bail out
			 	 */
				FDDI_DBTRACE("Frt2",p_acs->dds.tx_que_sz,0,0);
				p_acs->ras.ds.tx_que_ovflw++;
				break;
			}

			DESC_TQ(p_tx_que, p_acs->tx.tq_in);

			p_tx_que->p_mbuf = p_mbuf;
			p_tx_que->p_open = 0;
			p_tx_que->gather_cnt = MBUF_CNT(p_mbuf);
			p_acs->tx.tq_cnt += p_tx_que->gather_cnt;

			INCRE_TQ(p_acs->tx.tq_in);
			if ( p_acs->tx.tq_cnt > p_acs->ras.cc.xmt_que_high)
				p_acs->ras.cc.xmt_que_high = p_acs->tx.tq_cnt;
			FDDI_DBTRACE("Frt4",p_acs->tx.tq_cnt, p_acs->tx.tq_in, 
					p_acs->tx.tq_out);
			if (rc) 
				rc = 1;

		}
		else
		{
			/* transmit this frame */
			send_frame (p_acs, p_mbuf);
			rc = 0;
		}

		/* move to the next frame */
		p_mbuf = p_mbuf->m_nextpkt;
	}
	/*
	 * interrupt the adapter with a transmit request
	 *	(once per request)
	 */
	if (rc == 0)
	{
		caddr_t 	ioa;

		ioa = (caddr_t) BUSIO_ATT (p_acs->dds.bus_id, 
				(caddr_t) p_acs->dds.bus_io_addr);
		FDDI_PTRACE("txcf");
		PIO_PUTSRX (ioa + FDDI_NS1_REG, FDDI_NS1_XMA);
		BUSIO_DET (ioa);

		/* start (or restart) watchdog timer */
		w_start (&(p_acs->tx.wdt));
	}

	if (rc != EAGAIN)
	{
		while (p_mbuf)
		{
			p_tmp = p_mbuf;
			p_mbuf = p_mbuf->m_nextpkt;
			m_freem(p_tmp);
		}
	}

	FDDI_TRACE("TfrE", rc, 0, 0);

	/* reset interrupts */
	i_enable (ipri);

	if (rc == 1)
		rc = 0;
	return (rc);
}
/*
 * NAME:     send_frame
 *
 * FUNCTION: Physically send a frame of mbufs out
 *
 * EXECUTION ENVIRONMENT: Process thread with interrupts disabled.
 *
 * NOTES:
 *
 *	Called by: 
 *		fddi_tx () 
 *		fddi_tx_fast ()
 *
 *	Setup the SOF descriptor with SOF bit set in control, the
 *	mbuf of the first buffer in the frame (0 len or not), and
 *	any extra processing due to the presence of an extension.
 *	Then for each descriptor setup the dma. Finally, set the
 *	EOF bit in the last descriptor that got an mbuf and do
 *	the Physical IO to effect the transmit.
 *
 *	!!! There is a problem with filling in the TRF indicating
 *	    "Process with tx to media", (0x11), it is possible to
 *	    get back two responses.
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
 * RECOVERY OPERATION: 
 *
 *	PIO failures are handled in the PIO routines.
 *
 * DATA STRUCTURES:
 *
 *	The host descriptor contains a structure that is the same as
 *	the structure on the adapter. The SOF host descriptor will save
 *	the following:
 *
 *		number of buffers - in this frame for efficient completion
 *		write extension - for sending to user after completion 
 *		open pointer - for sending to user after completion
 *
 * RETURNS:  none
 */

static void
send_frame (
	fddi_acs_t	*p_acs,		/* per adapter structure */
	struct mbuf 	*p_mbuf) 	/* frame */
{
	fddi_adap_t	adap;		/* for doing the PIO to shared mem */
	uchar		sof_idx;	/* index to the SOF descriptor for tx */
	short		eof_jump;	/* relative index to EOF from SOF */
	fddi_tx_t	*p_sof;		/* start of frame descriptor */
	fddi_tx_t	*p_tx;		/* tx host descriptor */
	int		bus;		/* for PIO to shared memory */


	/* 
	 * setup SOF host descriptor 
	 *	(remember sof_idx for the PIO at bottom of routine)
	 */
	sof_idx = p_acs->tx.nxt_req;
	p_sof = &(p_acs->tx.desc[sof_idx]);
	p_sof->p_mbuf = p_mbuf;
	INCREMENT (p_acs->tx.nxt_req, 1, FDDI_MAX_TX_DESC);

	/* skip leading 0 len buffers */
	while ((p_mbuf) && (p_mbuf->m_len == 0))
	{
		p_mbuf = p_mbuf->m_next;
	}

	/* initialize */
	p_tx = p_sof;
	eof_jump = 0;

	/* process the other mbufs in this frame */
	while (TRUE)
	{
		/* another descriptor in use (ready for tx) */
		p_acs->tx.in_use ++;	

		/* 
		 * Setup the dma for this descriptor 
		 *	based on len of the mbuf: small bufs are
		 *	copied to a small frame area already setup
		 *	for DMA.
		 */
		if ((p_tx->adap.cnt = p_mbuf->m_len) < FDDI_SF_BUFSIZ)
		{
			/*
			 * Use a section of the 'smallframe' cache instead
			 *	of the mbuf for data transmit and dma
			 */
			p_tx->flags |= FDDI_TX_SMALLFRAME;

			/* Setup DMA address for using the 'smallframe' cache */
			*(uint *) &(p_tx->adap.addr_hi) = p_tx->p_d_sf;

			/* Copy mbuf data to 'smallframe' cache */
			bcopy (MTOD(p_mbuf, char *), 
				p_tx->p_sf, p_mbuf->m_len);

			/* flush smallframe cache */
			d_cflush (p_acs->dev.dma_channel, p_tx->p_sf, 
				p_mbuf->m_len, p_tx->p_d_sf);
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
		 * Buffer Descriptor is Valid (BDV) 
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
		p_tx = &(p_acs->tx.desc[p_acs->tx.nxt_req]);
		INCREMENT (p_acs->tx.nxt_req, 1, FDDI_MAX_TX_DESC);
		eof_jump ++;

	} /* end while (TRUE) */
	/* 
	 * p_tx points to the last descriptor so set EOF control 
	 *	Set IFX bit on all the EOF descriptors. If we
	 *	post multiple writes via fastwrite then yes we
	 *	will get multiple interrupts but in fact all the
	 *	interrupts will be handled in one off level thread.
	 *	Therefore, the overhead to figure out when to
	 *	turn on the IFX is more than just turning it on
	 *	all the time.
	 * p_sof still points to the first frame and it also has 
	 *  	another flag to turn on in the ctl field of the 
	 *	descriptor so or it in.
	 * The trf mask includes the bits in the ext flag field which
	 *	control the processing of the frame.  The bits corresponde to 
 	 *	the adapter's descriptor definition of bits (shifted slightly)
	 */
	
	p_tx->adap.ctl |= FDDI_TX_CTL_EOF | FDDI_TX_CTL_IFX;
	p_sof->adap.ctl |= FDDI_TX_CTL_SOF | 
		(*(MTOD(p_sof->p_mbuf, short *)) & FDDI_TRF_MSK);
FDDI_TRACE("_TX_", (*(MTOD(p_sof->p_mbuf, short *)) & FDDI_TRF_MSK), p_acs->tx.in_use, 0);

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
		PIO_PUTSTRX (bus + p_sof->offset, &adap, sizeof (adap));

		/* check finish condition */
		if (p_sof == p_tx) 
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

	/* ok */
	return;
}

/*
 * NAME: fddi_dma_cleanup
 *                                                                    
 * FUNCTION: cleanup the dma for each descriptor in this frame 
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment
 *                                                                   
 * NOTES: 
 *
 *	The 'smallframe' cache is used to transmit small frames. Here
 *	a tx complete from a 'smallframe' cache occured. 
 *	This routine does:
 *		d_complete  - to clean up the dma
 *
 * RECOVERY OPERATION: 
 *
 *	Note errors in the write extension 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: return code of the d_complete (dma success or failure)
 */  

int
fddi_dma_cleanup (
	fddi_acs_t	*p_acs,
	fddi_tx_t	*p_tx,		/* starts as sof descriptor */
	fddi_tx_t	*p_eof,		/* the eof descriptor */
	uchar		idx)		/* index of the sof descriptor */
{
	int	rc;
	int	err = DMA_SUCC;

	while (TRUE)
	{
		/* DMA cleanup: */
		if (p_tx->flags & FDDI_TX_SMALLFRAME)
		{
			/* 
			 * do DMA cleanup from 'smallframe' 
			 */
			/* Check DMA completion */
			rc = d_complete ( 
				p_acs->dev.dma_channel, 
				DMA_WRITE_ONLY,
				p_tx->p_sf, 
				(size_t) p_tx->adap.cnt,
				&(p_acs->tx.xmd), 
				(caddr_t) p_tx->p_d_sf);

			/* reset smallframe valid flag */
			p_tx->flags &= (~FDDI_TX_SMALLFRAME);
		}
		else
		{
			/* do DMA cleanup from mbuf */
			rc = d_complete (
				p_acs->dev.dma_channel, 
				DMA_WRITE_ONLY,
				MTOD (p_tx->p_mbuf, caddr_t),
				(size_t)p_tx->adap.cnt,
				M_XMEMD (p_tx->p_mbuf), 
				(caddr_t) p_tx->p_d_addr);
		}
		if (rc != DMA_SUCC)
		{
			/*
			 * DMA error occurred. 
			 *	Remember it then finish processing the
			 *	rest of the descriptors.
			 */
			FDDI_TRACE("Tdc1", p_acs, rc, p_tx->p_d_addr);
			fddi_logerr (p_acs, ERRID_FDDI_MC_ERR,
				__LINE__, __FILE__);
			err = rc;
		}
		if (p_tx == p_eof)
		{
			/* just processed eof - so were done */
			break;
		}
		INCREMENT (idx, 1, FDDI_MAX_TX_DESC);
		p_tx = &(p_acs->tx.desc[idx]);
	}

	/* finished */
	return (err);
}
/*
 * NAME:     fddi_frame_done
 *
 * FUNCTION: Finish processing transmitted frame
 *
 * EXECUTION ENVIRONMENT: interrupt thread
 *
 * NOTES:
 *
 *	This routine does:
 *
 *	updates RAS
 *	Free mbuf unless owner wants it back
 *	Process write extension
 *	Check for anyone interested in tx availability
 *
 *	Called by:
 *		tx_handler ()		- tx interrupt handler
 *		fddi_undo_init_tx ()	- undo initialization routine
 *		fddi_tx_to ()		- tx timeout routine
 *
 *	Calls to:
 *		m_freem ()
 *
 * RECOVERY OPERATION: none
 *
 * DATA STRUCTURES:
 *
 *	Uses the write extension if one exists.
 *	Check adapter tx available
 *
 *	Maintains (decrements) a counter of pending acknowledges
 *	for this open. This counter is incremented at write time,
 *	checked at close time where closes are delayed until it is 0.
 *
 * RETURNS:  none
 *
 */

/*
 * The PROCESS_EXT macro is used in two places so that checking for
 *	the extension only costs one instruction if it doesn't exist.
 */
#define PROCESS_EXT(err_code)						\
{									\
	/* check for extension */					\
	if (p_sof->flags & FDDI_TX_EXT)					\
	{								\
		/* process the extension */				\
		proc_ext (p_acs, p_sof, p_sof->p_open, err_code, stat);	\
	}								\
	else								\
	{								\
		/* free mbuf chain */					\
		m_freem (p_sof->p_mbuf);				\
		p_sof->p_mbuf = NULL;					\
	}								\
}

void
fddi_frame_done (
	fddi_acs_t	*p_acs,		/* per adapter structure */
	fddi_tx_t	*p_sof,		/* ptr to start of frame descriptor */
	int		err,		/* errors or zero if no errors */
	ushort		stat)		/* the status from the adapter */
{
	cio_stats_t	*p_cc;		/* ptr to stats */
	struct mbuf	*p_mbuf;	/* mbuf chain */
	fddi_open_t	*p_open;	/* used to run thru all opens */
	chan_t		chan;		/* used to run thru all opens */
	int		cnt;		/* used to run thru all opens */
	int		len;		/* Length of the packet */

	/* initialize */
	p_cc = &(p_acs->ras.cc); 
	FDDI_TRACE("TfdB", 
			p_acs, 
			p_acs->tx.in_use, 
			p_acs->tx.nxt_cmplt); 

	/* process errors */
	if ((err != DMA_SUCC) || (stat & FDDI_TX_STAT_ERR))
	{
		/* trace error count */
		FDDI_TRACE("Ter1", p_acs, err, stat);
 		fddi_logerr(p_acs, ERRID_FDDI_TX_ERR, __LINE__, __FILE__);

		/* 
		 * Take care of the extension 
		 */
		PROCESS_EXT (FDDI_TX_ERROR);

		/* frame has an error: update RAS error count */
		++p_cc->tx_err_cnt;
	}
	else
	{
		p_cc->tx_frame_lcnt++;
		if (p_cc->tx_frame_lcnt == 0)
			p_cc->tx_frame_mcnt++;

		/* RAS update: byte count */
		p_mbuf = p_sof->p_mbuf;
		len = 0;
		while (p_mbuf)
		{
			len += p_mbuf->m_len;
			p_mbuf = p_mbuf->m_next;
		}
		p_cc->tx_byte_lcnt += len;
		if (p_cc->tx_byte_lcnt < len)
			p_cc->tx_byte_mcnt++; 
		/* 
		 * Take care of the extension 
		 *	(p_mbuf is freed after call to PROCESS_EXT)
		 */
		PROCESS_EXT (CIO_OK);
	}
	/* check for sleeping callers */
	if (p_acs->tx.event != EVENT_NULL)
	{
		/* wakeup time for these processes */
		e_wakeup ((int *) &(p_acs->tx.event));
	}
	/* if notification is not needed then we are done */
	if (p_acs->tx.needed == FALSE)
	{
		FDDI_TRACE("TfdE", p_acs, err, stat);
		return;
	}
	/* 
	 * Someone is interested in this tx complete event.
	 * 	Check all possible channels for an open channel until
	 *	open_cnt worth of opens are checked.
	 */
	for (chan = 0, cnt = fddi_ctl.open_cnt; cnt > 0; chan++)
	{
		if ((p_open = fddi_ctl.p_open_tab [chan]) == NULL)
		{
			/* try next chan number */
			continue;
		}
		/* have open ptr so decrement count */
		cnt--;

		if (p_open->p_acs != p_acs)
			continue;

		/* check for kernel users with DNDELAY */
		if ((p_open->devflag & DKERNEL) && 
			(p_open->tx_fn_needed))
		{
			/* tell caller we got room now */
			(*p_open->tx_fn)(p_open->open_id);

			/* reset indicator */
			p_open->tx_fn_needed = FALSE;
		}
		/* check for select callers */
		if (p_open->selectreq & POLLOUT)
		{
			/* notify user of a POLLOUT condition */
			(void) selnotify (p_open->devno, 
					p_open->chan, 
					POLLOUT);

			/* reset indicator */
			p_open->selectreq = 0;
		}
	} 
	/* reset needed indicator */
	p_acs->tx.needed = FALSE;

	FDDI_TRACE("TfdE", p_acs, err, stat);
	return;
}
/*
 * NAME:     tx_handler
 *
 * FUNCTION: transmit complete interrupt handler
 *
 * EXECUTION ENVIRONMENT: interrupt environment
 *
 * NOTES:
 *	
 *	This routine is invoked in response to a TIA (transmit command 
 *	interrupt in the HSR). The adapter will have sent all the
 *	descriptors waiting through the last transmit command. 
 *
 *	The completions are processed a frame at a time:
 *	Starting from the 'nxt_cmplt' index which points at
 *	the start of a frame it finds the last buffer in the frame (the EOF)
 *	and reads its cooresponding adapter status to see if there are
 *	any errors. If not the entire frame (which can be up
 *	to FDDI_MAX_GATHERS buffers) is cleaned up without
 *	reading anything else on the adapter. If 'tx_in_use' is not
 *	exceeded then we advance to the next frame and check to see 
 *	if it is complete.
 *
 *	Finally, the frame is sent back to the user if requested.
 *	The open pointer is in the SOF descriptor of the frame to
 *	go back to the user.
 *
 * RECOVERY OPERATION:
 *
 *	This function can discover the following errors:
 *
 *	illegal transmit frame, 
 *	No start of frame,
 *	No end of frame
 *	card parity error
 *	tx abort 
 *
 *	NEF no end of frame and	NSF no start of frame
 *
 *	These errors indicate that the host and adapter
 *	are out of sync. This is real serious probably a
 *	timing problem with this software or the microcode
 *	We must go into the LIMBO state.
 *
 *	CPT card parity error 
 *
 *	This is a serious error because the adapter has already gone
 *	throught the configurable retries and the parity error
 *	persists. The adapter has halted transmission. We must
 *	go into the LIMBO state.
 *
 *	ABORT transmission was aborted by a host 
 *	!!! Thoughts on Abort:
 *
 *		"ABORT TRANSMIT COMMAND"
 *		We can't tell here if this is serious so we just process the
 *		transmit complete, set the status in the write_extension if
 *		present and pass back to the user. 
 *		Possible causes of this error:
 *		We may have just gone into LIMBO state in which case the 
 *		problem has been detected and this ABORT was given 
 *		to quiese the line.
 *		The ABORT transmit command may have been purposly given 
 *		by, for example, diagnostics...
 *
 * DATA STRUCTURES:
 *
 *	The per adapter work structure contains the tx descriptor 
 *	queue and the indexes to manage that circular queue.
 *
 *		fddi_tx_t	has all the info the adapter needs to do a tx
 *				and info to give complete tx to USER
 *
 *		nxt_cmplt	index into the circular queue where the 
 *				next complete  will be
 *
 *		nxt_req		index into the circular queue where the 
 *				next request should go
 *
 *	The original mbuf chain was saved so it can be returned if the user
 *	requested.
 *
 * RETURNS:  
 *
 *	0 	- ok
 *	nonzero	- an unrecoverable error was encountered
 */

int
tx_handler (
        fddi_acs_t	*p_acs,
	int		bus)
{
	fddi_tx_t	*p_sof;		/* ptr for start of frame */
	fddi_tx_t	*p_eof;		/* ptr for end of frame */
	uchar		sof_idx;	/* index for the sof descriptor */
	uchar		eof_idx;	/* index for the eof descriptor */
	ushort		stat;		/* status field read from adapter */
	uchar		desc_cnt;	/* the no of descr in current frame */
	int		rc;		/* return code from called funcs */
	int		tx_rc;		/* return code to fddi_frame_done () */

	/* performance trace hooks */
	/*
	 * !!!TRCHKL0T(HKWD_LWR | hkwd_xdone_in);
	 * TRCHKL0T(HKWD_LWR3 | hkwd_xdone_in);
	 */

	FDDI_TRACE ("TcmB", 
			p_acs, 
			p_acs->tx.in_use, 
			p_acs->tx.nxt_cmplt); 

	/* stop tx watchdog timer */
	w_stop (&(p_acs->tx.wdt));

	/*
	 * Loop until at most all the tx descriptors in use are completed. 
	 *	We can break out of this loop with tx_in_use > 0 
	 *	because adapter hasn't transmitted this one yet.
	 */
	while (p_acs->tx.in_use > 0)
	{
		/* 
		 * Get SOF descriptor for this frame 
		 */
		sof_idx = p_acs->tx.nxt_cmplt;
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
		if (stat == 0)
		{
			/* 
			 * done processing this interrupt:
			 * 	adapter hasn't transmitted this one yet 
			 */
			FDDI_TRACE ("Tcm1", 
					p_acs, 
					p_acs->tx.in_use, 
					p_acs->tx.nxt_cmplt);
			w_start (&(p_acs->tx.wdt));
			break;
		}
		if (stat & FDDI_TX_STAT_BTI)
		{
			/* 
			 * unrecoverable error encountered 
			 */
			fddi_logerr (p_acs, ERRID_FDDI_TX_ERR,
				__LINE__, __FILE__);
			ASSERT(0);
			fddi_enter_limbo (p_acs, FDDI_TX_ERROR, stat);
			return(EINVAL);
		}
		/* 
		 * cleanup dma for descriptor 
		 * NOTE: rc returned from dma_cleanup used in frame_done
		 */
		rc = fddi_dma_cleanup (p_acs, p_sof, p_eof, sof_idx);

		/* 
		 * This frame is done: take care of ras, errs, and acks
		 */
		(void) fddi_frame_done (p_acs, p_sof, rc, stat);

		/* 
		 * calculate descriptor count and adjust index and counter 
		 */
		desc_cnt = p_sof->eof_jump + 1;
		p_acs->tx.in_use -= desc_cnt;
		INCREMENT (p_acs->tx.nxt_cmplt, desc_cnt, FDDI_MAX_TX_DESC) 

		/* 
		 * Time to see if anything is on the user's tx que 
		 */
		if (p_acs->tx.tq_cnt)
		{
			fddi_tx_que_t *p_tx_que;

			FDDI_DBTRACE("Tth1",p_acs->tx.tq_cnt, p_acs->tx.in_use, 
					p_acs->tx.tq_in);
			DESC_TQ(p_tx_que, p_acs->tx.tq_out);
			FDDI_DBTRACE("Tth3", p_acs->tx.tq_cnt, p_acs->tx.tq_out,
					p_tx_que->gather_cnt);
			while (!(QUE_FULL(p_acs->tx.in_use,
				p_tx_que->gather_cnt, FDDI_MAX_TX_DESC)))
			{
				if (p_tx_que->p_open)
				{
					/* 
		 			* remember extension for tx cmplt time 
		 			*/
					setup_ext (p_acs, p_tx_que->p_open, 
						&p_tx_que->wr_ext);

					send_frame (p_acs, p_tx_que->p_mbuf);
				}
				else
					send_frame (p_acs, p_tx_que->p_mbuf); 

				p_acs->tx.tq_cnt -= p_tx_que->gather_cnt;

				INCRE_TQ(p_acs->tx.tq_out);
				
				if (!(p_acs->tx.tq_cnt))
					break;
				DESC_TQ(p_tx_que, p_acs->tx.tq_out);
				FDDI_DBTRACE("Tth3", p_acs->tx.tq_cnt, 
					p_acs->tx.tq_out, p_tx_que->gather_cnt);
			}
			/*
	 		 * interrupt the adapter with a transmit request
	 		 */
			{
				caddr_t 	ioa;
		
				ioa = (caddr_t) BUSIO_ATT (p_acs->dds.bus_id, 
					(caddr_t)p_acs->dds.bus_io_addr);
				FDDI_PTRACE("txcr");
				PIO_PUTSRX (ioa + FDDI_NS1_REG, FDDI_NS1_XMA);
				BUSIO_DET (ioa);
			}

			/* start watchdog timer */
			w_start (&(p_acs->tx.wdt));

			FDDI_DBTRACE("Tth4",p_acs->tx.tq_cnt, p_acs->tx.in_use, 
					0);
		}			
				
	}
	FDDI_TRACE ("TcmE", 
			p_acs, 
			p_acs->tx.in_use, 
			p_acs->tx.nxt_cmplt);
	return (0);
}

/*
 * NAME: fddi_chk_state
 *                                                                    
 * FUNCTION: check the current state of the device and return the
 *		appropriate code
 *                                                                    
 * EXECUTION ENVIRONMENT: process or interrupt environments 
 *                                                                   
 * NOTES: 
 *	called by: fddi_write () and fddi_fastwrite()
 *
 *	The state of the device is not in the open state. This routine
 *	checks what state it is in and returns the appropriate code.
 *	The write ext is filled out as well, if provided by the caller.
 *
 *	PHASE II will check for SMT frames.
 *
 * RECOVERY OPERATION: 
 *
 *	This is an error handling routine, called when the device is not
 *	in the OPEN state.
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *	as stated above in fddi_write() and fddi_fastwrite()
 */  

int
fddi_chk_state (
	fddi_acs_t	*p_acs,
	cio_write_ext_t *p_ext)		/* local ext pointer */
{
	if (p_acs->dev.state == FDDI_LLC_DOWN)
	{
		/* This is hit only by the fast_write entry point and
		 * due to the preservation of limited checking in that
		 * entry point, all frames are rejected in this state.
		 */
		return (ENETUNREACH);
	}
	if (p_acs->dev.state == FDDI_LIMBO)
	{
		/* this device is in network recovery mode */
		return (ENETUNREACH);
	}
	if (p_acs->dev.state == FDDI_DEAD)
	{
		/* device is unrecoverable */
		return (ENETDOWN);
	}
	if ((p_acs->dev.state == FDDI_INIT) || 
		(p_acs->dev.state == FDDI_NULL) ||
		(p_acs->dev.state == FDDI_DWNLD) ||
		(p_acs->dev.state == FDDI_DWNLDING))
	{
		/* not started */
		if (p_ext)
		{
			/* set caller status in extension */
			p_ext->status = CIO_NOT_STARTED;
			return(EIO);
		}
		return (ENOCONNECT);
	}
	/* internal programming error */
	ASSERT (0);

	/* generic error */
	return (EINVAL);
}
/*
 * NAME: fddi_chk_kernel_mbufs
 *                                                                    
 * FUNCTION: Check the mbuf chain to ensure it is a valid FDDI frame
 *                                                                    
 * EXECUTION ENVIRONMENT: called from a KERNEL process (must be pinned)
 *                                                                   
 * NOTES: 
 *
 *	Called by: fddi_write() and fddi_fastewrite()
 *
 *	This routine performs sanity checks on the mbuf chain
 *	as follows:
 *
 *		Check for room for the mandatory PAD characters
 *		Check for buffer crossing a PAGE boundary
 *		Check for max gather locations
 *		Check for min and max frame data size
 *
 * RECOVERY OPERATION: none
 *
 * DATA STRUCTURES: uses mbuf structure
 *
 * RETURNS: 
 *
 *	0	- indicates a good frame
 *		  and the number of mbufs is returned in p_gather_cnt
 *
 *	EINVAL	- indicates frame failed one of the tests
 *		  and the p_gather_cnt contains the buffer it failed on
 */  
int
fddi_chk_kernel_mbufs (
	struct mbuf	*p_mbuf,	/* chain of mbufs */  
	short		*p_gather_cnt,	/* return number of mbufs here */
	fddi_acs_t	*p_acs)
{
	uchar		*p_data;	/* pointer to data portion of mbuf */
	int		l_frame; 	/* total frame size */
	struct mbuf	*p_head;	/* head of the frame */
	uchar		fc;		/* frame control */
	int		rc;

	/* initialize */
	l_frame = *(p_gather_cnt) = 0;
	p_head = p_mbuf;

	/*
	 * Step through each frame one buffer at a time
	 *	checking PAD, Cross boundary, len and gathers.
	 */
	/* for each buffer in this frame */
	while (p_mbuf)
	{
		p_data = MTOD(p_mbuf, uchar *);

		/* check for data crossing a PAGE */
		if (FDDI_X_BOUND (p_data, p_mbuf->m_len))
		{
			/* data crosses boundary: fail */
			return (EINVAL);
		}

		/* only count nonzero size buffers */
		if (p_mbuf->m_len)
		{
			/* increment number of buffers */
			(*p_gather_cnt)++;
			l_frame += p_mbuf->m_len;
		}
		/* set to next mbuf */
		p_mbuf = p_mbuf->m_next;
	}
	/* check max gather locations */
	if ((*p_gather_cnt < 1) || (*p_gather_cnt > FDDI_MAX_GATHERS))
	{
		/* invalid gather count: fail */
		return (EINVAL);
	}
	/* find first nonzero mbuf to check address size */
	while (p_head->m_len == 0)
	{
		p_head = p_head->m_next;
	}
	/*
	 * check for legal frame: correct type and length
	 */
	rc = fddi_chk_frame (p_head, l_frame, p_acs);
	if (rc)
	{
		/* failed legal frame checks */
		return (rc);
	}
	/* successful */
	return (0);
}

/*
 * NAME: fddi_chk_frame
 *                                                                    
 * FUNCTION: performs sanity checks on the frame
 *                                                                    
 * EXECUTION ENVIRONMENT: process environment
 *                                                                   
 * NOTES: 
 *
 *	SMT and 2 byte address frames are not allowed.
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *	0	- successful
 *	EINVAL	- failed checks
 */  

int
fddi_chk_frame (
	struct mbuf	*p_frame,	/* frame ptr */
	int		l_frame,	/* frame length */
	fddi_acs_t	*p_acs)
{
	uchar		fc;		/* frame control */

	/* check for two byte address and SMT frames */
	/*
	 * PHASE 2 change: allow SMT frames
	 */
	fc = (MTOD(p_frame, fddi_hdr_t *))->fc;
	if (((fc & FDDI_FC_MSK) != FDDI_FC_LLC) && 
		((fc & FDDI_FC_MSK) != FDDI_FC_SMT))
	{
		/* no two byte addresses or SMT frames allowed for now */
		return (EINVAL);
	}
	/* 
	 * Check legal frame len 
	 */
	if (CHK_FRAME_SIZ(l_frame, fc))
	{
		/* invalid frame length */
		return (EINVAL);
	}

	if ((p_acs->dev.state == FDDI_LLC_DOWN) && 
		((fc & FDDI_FC_MSK) == FDDI_FC_LLC))
	{
		return(ENETUNREACH);
	}
	
	return (0);
}

/*
 * NAME: que_full_handler
 *                                                                    
 * FUNCTION:  handles a transmit que full condition
 *                                                                    
 * EXECUTION ENVIRONMENT: Process thread with interrupts disabled.
 *
 *	Called by: fddi_tx()
 *
 *	This routine is called with interrupts disabled while in a
 *	process thread. The code and data in this function are non-pageable.
 *
 *	This is not executing in an interrupt environment so e_sleep 
 *	may be called as long as we are sleeping on pinned data.
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *
 *	0 	- successful
 *	-1	- state changed while we were sleeping so recheck state 
 *	EAGAIN	- resources full
 *	EINTR	- interrupted system call (e_sleep)
 *	
 */
static int
que_full_handler (
	fddi_acs_t 	*p_acs,
	fddi_open_t	*p_open,
	struct mbuf	*p_mbuf,
	cio_write_ext_t *p_ext)
{
	int rc;
	/* see if caller wants to wait */
	if (p_open->devflag & DNDELAY)
	{
		/* process write extention */
		if (p_ext)
		{
			p_ext->status = CIO_TX_FULL;
			rc = EIO;
		}
		else
			rc = EAGAIN;

		if (p_open->devflag & DKERNEL)
		{
			/* remember that kernel proc wanted to tx */
			if (p_open->tx_fn_needed == FALSE)
			{
				/*
				 * indicate tx need in open and what acs
				 */
				p_open->tx_fn_needed = TRUE;
				p_acs->tx.needed = TRUE;
			}
		}
		else
		{
			/* free any USER mbufs */
			m_freem (p_mbuf);
		}
		return (rc);
	}
	/* kernel or user in delay mode */
	if (e_sleep (&(p_acs->tx.event), EVENT_SIGRET) != EVENT_SUCC)
	{
		/* interrupted system call */
		return (EINTR);
	}
	/* check device state */
	if (p_acs->dev.state != FDDI_OPEN)
	{
		return (-1);
	}
	/* ok */
	return (0);
}
/*
 * NAME: fddi_tx_to
 *                                                                    
 * FUNCTION: watchdog timer expiration for tx.
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *
 *		Apparently this can be called from process
 *		process or interrupt environment so we need to make
 *		sure we run at the level the normal command interrupt
 *		would run at.
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 */  
void
fddi_tx_to (
	struct	watchdog *p_wdt)
{
	fddi_acs_t	*p_acs;
	int		ipri;

	/* go to OFFlevel interrupt priority */
	ipri = i_disable ( INTCLASS2 );

	/* get ACS */
	p_acs = (fddi_acs_t *) ((uint) p_wdt - 
		(offsetof (fddi_acs_tx_t, wdt) + offsetof (fddi_acs_t, tx)));

	FDDI_DBTRACE("TtoB", p_acs, p_acs->tx.in_use, p_acs->tx.nxt_cmplt);

	p_acs->dev.oflv_events |= FDDI_TX_WDT_IO;
	if (p_acs->dev.oflv_running == FALSE)
	{
		/*
		 * schedule offlevel process and set running flag.
		 *	oflv_running is set to FALSE after the last
		 *	check of the hsr in fddi_oflv();
		 */
		i_sched (&p_acs->dev.ihs);
		p_acs->dev.oflv_running = TRUE;
	}
	/* return to priority at this functions' invocation */
	i_enable (ipri);

	/* trace this event */
	FDDI_DBTRACE("TtoE", p_acs, 0, 0);

	/* ok */
	return ;
}
/*
 * NAME: setup_ext
 *                                                                    
 * FUNCTION: setup the write extension called from normal path write
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *
 *	Called from the process thread with interrupts disabled.
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 */  
void
setup_ext(
	fddi_acs_t	*p_acs,
	fddi_open_t	*p_open,	/* per open structure */
	cio_write_ext_t	*p_ext)		/* write extension */
{
	fddi_tx_t	*p_sof;

	p_sof = &(p_acs->tx.desc[p_acs->tx.nxt_req]);

	/* 
	 * the extention is present so do some setup work
	 *	for completion time processing.
	 */
	if (p_ext->flag & CIO_ACK_TX_DONE)
	{
		/* 
		 * Keep track of acks pending
		 *     for fddi_close().
		 */
		p_open->tx_acks++;
	}
	/* copy the extension (passed in as auto var) */
	p_sof->wr_ext = *p_ext;
	p_sof->flags |= FDDI_TX_EXT;

	/* we will need the open struct at completion time */
	p_sof->p_open = p_open;

	/* ok */
	return ;
}
/*
 * NAME:     proc_ext
 *
 * FUNCTION: process the extension for this write
 *
 * EXECUTION ENVIRONMENT: interrupt thread
 *
 * NOTES:
 *
 *	This routine does:
 *
 *	Process write extension
 *
 *	Called by fddi_frame_done ()
 *	Calls to:
 *		m_freem ()
 *		fddi_report_status ()
 *
 * RECOVERY OPERATION: none
 *
 * DATA STRUCTURES:
 *
 *	Uses the write extension.
 *	Maintains (decrements) a counter of pending acknowledges
 *	for this open. This counter is incremented at write time,
 *	checked at close time where closes are delayed until it is 0.
 *
 * RETURNS:  none
 *
 */
static void
proc_ext (
	fddi_acs_t	*p_acs,
	fddi_tx_t	*p_sof,
	fddi_open_t	*p_open,
	uint		tx_rc,
	ushort		stat)
{
	cio_stat_blk_t	stat_blk;	/* status block for ACK TX DONE */
	ulong		flag;		/* contains the callers directives */

	/* process the extension */
	flag = p_sof->wr_ext.flag;

	/* Report status to user if requested. */
	if (flag & CIO_ACK_TX_DONE)
	{
		/*
		 * Build tx done status block:
		 *	the status is in the write ext status
		 *	field and the write id is in the write
		 *	ext as well.
		 */
		stat_blk.code = CIO_TX_DONE;
		stat_blk.option[0] = tx_rc;
		stat_blk.option[1] = p_sof->wr_ext.write_id;

		if (p_open->devflag & DKERNEL)
		{
			/* kernel processes get mbuf ptr */
			stat_blk.option[2] = (ulong) p_sof->p_mbuf;
		}
		else
		{
			/* user processes get zero */
			stat_blk.option[2] = 0;
		}	
		stat_blk.option[3] = stat;

		/* report status to caller */
		fddi_report_status (p_acs, p_open, &stat_blk);

		/* now one less tx acknowledgement pending */
		p_open->tx_acks--;
	}
	/* free mbuf unless directed not to */
	if ((flag & CIO_NOFREE_MBUF) == 0)
	{
		/* free mbuf chain */
		m_freem (p_sof->p_mbuf);
		p_sof->p_mbuf = NULL;
	}

	/* reset extension variables */
	p_sof->flags = 0;
	p_sof->p_open = NULL;

	/* done */
	return;
}
/*
 * NAME: fddi_undo_start_tx
 *                                                                    
 * FUNCTION: Clean or clear the tx objects 
 *                                                                    
 * EXECUTION ENVIRONMENT: process or interrupt environment
 *                                                                   
 * NOTES: 
 *
 *	This routine clears the data structures for tx so that
 *	they are in the INIT state (the state after a call to
 *	fddi_init_tx). 
 *
 *	Pending transmits are failed.
 *	Counters are reset.
 *	timer reset.
 *
 *	Assumptions:
 *		The adapter has been running but now is disabled. 
 *		The adapter's descriptors are reset so that
 *		transmits maybe restarted.
 *
 * RECOVERY OPERATION: none
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 */  

void
fddi_undo_start_tx (
	fddi_acs_t	*p_acs)
{
	uchar	first;		/* index of first armed descriptor */
	uchar	last;		/* index of last armed descriptor */
	int	rc;
	int	cnt;
	fddi_tx_que_t *p_tx_que;
	cio_stat_blk_t	stat_blk;	/* status block for ACK TX DONE */

	FDDI_TRACE("TunB", p_acs, 0, 0);
	if (p_acs->tx.in_use)
	{
		/* 
		 * complete all pending dma transfers
		 */
		first = last = p_acs->tx.nxt_cmplt;
		INCREMENT (last, p_acs->tx.in_use - 1, FDDI_MAX_TX_DESC);

		rc = fddi_dma_cleanup (p_acs, 
			&(p_acs->tx.desc[first]),
			&(p_acs->tx.desc[last]),
			first);

		fddi_logerr(p_acs, ERRID_FDDI_TX_ERR,
			__LINE__, __FILE__);

		/* cleanup pending frames */
		while (p_acs->tx.in_use > 0)
		{
			/*
			 * force frame completions with error
			 *	for all pending transmits
			 */
			(void) fddi_frame_done (p_acs, 
						&(p_acs->tx.desc[first]),
						(int) FDDI_TX_ERROR, 
						(ushort) 0);
			/*
			 * increment to the first descriptor of the
			 *	next frame and decrement the number
			 *	of transmits in use.
			 */
			cnt = (p_acs->tx.desc[first].eof_jump + 1);
			INCREMENT (first, cnt, FDDI_MAX_TX_DESC);
			p_acs->tx.in_use -= cnt;
			if (p_acs->tx.in_use < 0)
			{
				/*
				 * Internal programming error here or on
				 *	the adapter.
				 */
				FDDI_TRACE("Tun2", p_acs, cnt, 0);
				ASSERT (0);
			}
		}
	}		
	/* 
	 * reset queue indexes 
	 */
	p_acs->tx.nxt_cmplt = 0;
	p_acs->tx.nxt_req = 0;
	p_acs->tx.in_use = 0;

	/* stop watchdog timer */
	w_stop (&(p_acs->tx.wdt));

	/*
	 * The card is not active when this is called and therefore this
	 * code does not have to worry about loosing the desc pointed to 
	 * in a interrupt handler 
	 */
	while (p_acs->tx.tq_cnt > 0)
	{
		DESC_TQ(p_tx_que, p_acs->tx.tq_out);

		if (p_tx_que->p_open)
		{			
			/* Report status to user if requested. */
			if (p_tx_que->wr_ext.flag & CIO_ACK_TX_DONE)
			{
				/*
		 		* Build tx done status block:
		 		*	the status is in the write ext status
		 		*	field and the write id is in the write
		 		*	ext as well.
		 		*/
				stat_blk.code = CIO_TX_DONE;
				stat_blk.option[0] = FDDI_TX_ERROR;
				stat_blk.option[1] = p_tx_que->wr_ext.write_id;
		
				if (p_tx_que->p_open->devflag & DKERNEL)
				{
					/* kernel processes get mbuf ptr */
					stat_blk.option[2] = 
						(ulong) p_tx_que->p_mbuf;
				}
				else
				{
					/* user processes get zero */
					stat_blk.option[2] = 0;
				}	
				stat_blk.option[3] = 0;
		
				/* report status to caller */
				fddi_report_status (p_acs, 
					p_tx_que->p_open, &stat_blk);
		
				fddi_logerr(p_acs, ERRID_FDDI_TX_ERR,
					__LINE__, __FILE__);
			}
			/* free mbuf unless directed not to */
			if ((p_tx_que->wr_ext.flag & CIO_NOFREE_MBUF) == 0)
			{
				/* free mbuf chain */
				m_freem (p_tx_que->p_mbuf);
			}
		}							
		else /* no P_OPEN pointer */
		{					
			/* free mbuf chain */	
			m_freem (p_tx_que->p_mbuf);
		}			
		p_acs->tx.tq_cnt -= p_tx_que->gather_cnt;
		INCRE_TQ(p_acs->tx.tq_out);
		
	}

	p_acs->tx.tq_cnt = 0;
	p_acs->tx.tq_in = 0;
	p_acs->tx.tq_out = 0;

	FDDI_TRACE("TunE", p_acs, 0, 0);

	/* ok */
	return ;
}
