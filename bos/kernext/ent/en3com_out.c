static char sccsid[] = "@(#)44  1.11  src/bos/kernext/ent/en3com_out.c, sysxent, bos411, 9438C411a 9/21/94 17:23:36";
/*
 * COMPONENT_NAME: sysxent -- High Performance Ethernet Device Driver
 *
 * FUNCTIONS: 
 *		en3com_output
 *		en3com_xmit
 *		en3com_txq_put
 *		en3com_txq_get
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stddef.h>
#include <sys/types.h>
#include <sys/lock_def.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/intr.h>
#include <sys/timer.h>
#include <sys/watchdog.h>
#include <sys/dma.h>
#include <sys/malloc.h>
#include <sys/intr.h>
#include <sys/adspace.h>
#include <sys/iocc.h>
#include <sys/sleep.h>
#include <sys/err_rec.h>
#include <sys/mbuf.h>
#include <sys/dump.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/ethernet_mibs.h>
#include <sys/cdli_entuser.h>
#include <net/spl.h>


#include "en3com_dds.h"
#include "en3com_mac.h"
#include "en3com_hw.h"
#include "en3com_pio.h"
#include "en3com.h"




/*****************************************************************************/
/*
 * NAME:     en3com_output
 *
 * FUNCTION: Ethernet driver output routine.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      NS user by using the ndd_output field in the NDD on the NDD chain.
 *
 * INPUT:
 *      p_ndd           - pointer to the ndd.
 *	p_mbuf		- pointer to a mbuf (chain) for outgoing packets
 *
 * RETURNS:  
 *	0 - successful
 *	EAGAIN - transmit queue is full
 *      ENETUNREACH - device is currently unreachable
 *      ENETDOWN - device is down
 */
/*****************************************************************************/
en3com_output(
  ndd_t		*p_ndd,		/* pointer to the ndd in the dev_ctl area */
  struct mbuf	*p_mbuf)	/* pointer to a mbuf (chain) */

{
  en3com_dev_ctl_t   *p_dev_ctl = (en3com_dev_ctl_t *)(p_ndd->ndd_correlator);
  struct mbuf *p_cur_mbuf;
  struct mbuf *buf_tofree;
  struct mbuf *first_free = NULL;	/* list of mbuf to free at the end */
  struct mbuf *last_free = NULL;	/* list of mbuf to free at the end */
  int first = TRUE;
  int bus;
  struct mbuf *en3com_txq_put();
  int ipri;



  TRACE_SYS(HKWD_EN3COM_XMIT, "TtxB", (ulong)p_ndd, (ulong)p_mbuf, 0);
  
  /*
   * Accounting for MIBs - ifHCOutUcastPkts, ifHCOutMulticastPkts 
   * and ifHCOutBroadcastPkts.
   */
  p_cur_mbuf = p_mbuf;
  while (p_cur_mbuf) {
	if (p_cur_mbuf->m_flags & (M_BCAST | M_MCAST)) {
		if (p_cur_mbuf->m_flags & M_BCAST) {
		  if (p_ndd->ndd_genstats.ndd_ifOutBcastPkts_lsw == ULONG_MAX)
		    p_ndd->ndd_genstats.ndd_ifOutBcastPkts_msw++;
		  p_ndd->ndd_genstats.ndd_ifOutBcastPkts_lsw++;
		}
		else {
		  if (p_ndd->ndd_genstats.ndd_ifOutMcastPkts_lsw == ULONG_MAX)
		    p_ndd->ndd_genstats.ndd_ifOutMcastPkts_msw++;
		  p_ndd->ndd_genstats.ndd_ifOutMcastPkts_lsw++;
		}
	}
	else {
		if (p_ndd->ndd_genstats.ndd_ifOutUcastPkts_lsw == ULONG_MAX)
		  p_ndd->ndd_genstats.ndd_ifOutUcastPkts_msw++;
		p_ndd->ndd_genstats.ndd_ifOutUcastPkts_lsw++;
	}
	p_cur_mbuf = p_cur_mbuf->m_nextpkt;
  }

  ipri = disable_lock(PL_IMP, &TX_LOCK);

  if (p_dev_ctl->device_state != OPENED) {
	if (p_dev_ctl->device_state == DEAD) {
		unlock_enable(ipri, &TX_LOCK);
  		TRACE_SYS(HKWD_EN3COM_ERR, "Ttx1", ENETDOWN, 
			p_dev_ctl->device_state, 0);
		return(ENETDOWN);
	}
	else {
		unlock_enable(ipri, &TX_LOCK);
  		TRACE_SYS(HKWD_EN3COM_ERR, "Ttx2", ENETUNREACH, 
			p_dev_ctl->device_state, 0);
		return(ENETUNREACH);
	}
  }

  /*
   * if there is a transmit queue, put the packet onto the queue.
   */
  if (p_dev_ctl->txq_first) {
	/*
	 *  if the txq is full, return EAGAIN. Otherwise, queue as
	 *  many packets onto the transmit queue and free the
	 *  rest of the packets, return no error.
	 */
	if (p_dev_ctl->txq_len >= DDS.xmt_que_size) {
					
		unlock_enable(ipri, &TX_LOCK);

		while (p_mbuf) {
			p_ndd->ndd_genstats.ndd_xmitque_ovf++;
			p_mbuf = p_mbuf->m_nextpkt;
		}
  		TRACE_SYS(HKWD_EN3COM_ERR, "Ttx3", EAGAIN, 0, 0);
		return(EAGAIN);
	}
	else {
		buf_tofree = en3com_txq_put(p_dev_ctl, p_mbuf);
		unlock_enable(ipri, &TX_LOCK);

		while (p_cur_mbuf = buf_tofree) {
			p_ndd->ndd_genstats.ndd_xmitque_ovf++;
			p_ndd->ndd_genstats.ndd_opackets_drop++;
			buf_tofree = buf_tofree->m_nextpkt;
			p_cur_mbuf->m_nextpkt = NULL;
			m_freem(p_cur_mbuf);
				
		}
  		TRACE_SYS(HKWD_EN3COM_XMIT, "Ttx4", 0, 0, 0);
		return(0);
	}

  }

  bus = (int)BUSMEM_ATT((ulong)DDS.bus_id, DDS.bus_mem_addr);

  while (p_mbuf) {
	p_cur_mbuf = p_mbuf;

	/*
	 * If there is txd available, try to transmit the packet.
	 */
	if (!(WRK.txd_avail->flags & BDC_IN_USE)) {
		
		/* For netpmon performance tool */
  		TRACE_SYS(HKWD_EN3COM_XMIT, TRC_WQUE, p_dev_ctl->seq_number,
			(int)p_cur_mbuf, p_cur_mbuf->m_pkthdr.len);
		
		if (!en3com_xmit(p_dev_ctl, p_cur_mbuf, bus)) {
		    /*
		     * Transmit OK, free the packet. Queue the mbuf to 
		     * free list and free them all at the end.
		     */

		    first = FALSE;
	  	    p_mbuf = p_mbuf->m_nextpkt;

		    p_cur_mbuf->m_nextpkt = NULL;;
		    if (first_free)
			last_free->m_nextpkt = p_cur_mbuf;
		    else
			first_free = p_cur_mbuf;
		    last_free = p_cur_mbuf;
			
		}
		else {
		    /*
		     * Transmit error. Call hardware error recovery
		     * function. If this is the first packet,
		     * return error. Otherwise, free the rest packets
		     * and return no error. 
		     */
		    if (first) {
					
  			BUSMEM_DET(bus);
			unlock_enable(ipri, &TX_LOCK);

		        p_ndd->ndd_genstats.ndd_oerrors++;
			/*
			 * if this is not running at interrupt level, 
			 * reset the card.
			 */
			if (getpid() != -1) {
				en3com_hard_err(p_dev_ctl, FALSE, FALSE, 
					NDD_PIO_FAIL);
			}

  			TRACE_BOTH(HKWD_EN3COM_ERR, "Ttx5", ENETDOWN, 0, 0);
			return(ENETDOWN);
		    }
		    else {
  			BUSMEM_DET(bus);
			unlock_enable(ipri, &TX_LOCK);

			/* increment the error counter */
			while (p_cur_mbuf = p_mbuf) {
		        	p_ndd->ndd_genstats.ndd_oerrors++;
				p_mbuf = p_mbuf->m_nextpkt;
				p_cur_mbuf->m_nextpkt = NULL;
				m_freem(p_cur_mbuf);
			}

			/* free the free list */
			while (p_cur_mbuf = first_free) {
				first_free = first_free->m_nextpkt;
				p_cur_mbuf->m_nextpkt = NULL;
				m_freem(p_cur_mbuf);
			}

			/*
			 * if this is not running at interrupt level, 
			 * reset the card.
			 */
			if (getpid() != -1) {
				en3com_hard_err(p_dev_ctl, FALSE, FALSE, 
					NDD_PIO_FAIL);
			}

  			TRACE_BOTH(HKWD_EN3COM_ERR, "Ttx6", 0, 0, 0);
  			return(0);

		    }
		}
	}    /* if there is txd available */
	else {
		buf_tofree = en3com_txq_put(p_dev_ctl, p_cur_mbuf);
  		BUSMEM_DET(bus);
		unlock_enable(ipri, &TX_LOCK);
		
		while (p_cur_mbuf = buf_tofree) {
			p_ndd->ndd_genstats.ndd_xmitque_ovf++;
			p_ndd->ndd_genstats.ndd_opackets_drop++;
			buf_tofree = buf_tofree->m_nextpkt;
			p_cur_mbuf->m_nextpkt = NULL;
			m_freem(p_cur_mbuf);
		}
		/* free the free list */
		while (p_cur_mbuf = first_free) {
			first_free = first_free->m_nextpkt;
			p_cur_mbuf->m_nextpkt = NULL;
			m_freem(p_cur_mbuf);
		}
  		TRACE_SYS(HKWD_EN3COM_XMIT, "Ttx7", 0, 0, 0);
		return(0);

	}

  }    /* while */

  BUSMEM_DET(bus);

  unlock_enable(ipri, &TX_LOCK);

  /* free the free list */
  while (p_cur_mbuf = first_free) {
	first_free = first_free->m_nextpkt;
	p_cur_mbuf->m_nextpkt = NULL;
	m_freem(p_cur_mbuf);
  }
  TRACE_SYS(HKWD_EN3COM_XMIT, "TtxE", 0, 0, 0);
  return(0);

}

/*****************************************************************************/
/*
 * NAME:     en3com_xmit
 *
 * FUNCTION: transmit packets to the adapter.
 *
 * EXECUTION ENVIRONMENT: process and interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_output
 *	en3com_tx_intr
 *
 * INPUT:
 *	p_dev_ctl	- pointer to the device control area
 *	p_mbuf		- pointer to a packet in mbuf
 *	bus		- handle for accessing I/O bus
 *
 * RETURNS:  
 *	0 - OK
 * 	-1 - error occurred during transmit
 */
/*****************************************************************************/
en3com_xmit(
  en3com_dev_ctl_t  *p_dev_ctl,	/* pointer to the device control area */
  struct mbuf	*p_mbuf,	/* pointer to the packet in mbuf */
  int		bus)		/* handle for I/O bus accessing */

{

  	ndd_t   *p_ndd = &(NDD);
	en3com_bdc_t	*p_txd = WRK.txd_avail;
	int count;
	int offset;
	int rc;
	int pio_rc = 0;



  	TRACE_SYS(HKWD_EN3COM_OTHER, "TxmB", (ulong)p_dev_ctl, (ulong)p_mbuf, 
		bus);

	/*
	 * Call ndd_trace if it is enabled
	 */
	if (p_ndd->ndd_trace) {
		(*(p_ndd->ndd_trace))(p_ndd, p_mbuf, 
			p_mbuf->m_data, p_ndd->ndd_trace_arg);
	}

	/*
	 * Use the first available txd to transmit the packet
	 */
	
	count = (p_mbuf->m_pkthdr.len <= (DMA_PSIZE / 2)) ? 
		p_mbuf->m_pkthdr.len : (DMA_PSIZE / 2);
	p_txd->flags |= BDC_IN_USE;

	/* increment the tx_pending count */
	p_dev_ctl->tx_pending++;

	/*
	 * copy data into tranmit buffer and do processor cache flush
         */
        m_copydata(p_mbuf, 0, count, p_txd->buf);

	/*
	 * Pad short packet with garbage
	 */
	if (count < ENT_MIN_MTU)
		count = ENT_MIN_MTU;
	p_txd->tx_len = count;

	/*
	 * We require the M_BCAST and M_MCAST flags set for broadcast or
	 * multicast packets before this functionis called. Now save
	 * the flag in the txd.
	 */
	if (p_mbuf->m_flags & M_BCAST) 
		p_txd->flags |= BDC_BCAST;
	if (p_mbuf->m_flags & M_MCAST) 
		p_txd->flags |= BDC_MCAST;

 	vm_cflush(p_txd->buf, count);

	/* 
	 * tell the adapter how many bytes to send
	 * clear the status and set the EOP and EL bit.
         */
        offset = bus + p_txd->bd_off;
        ENT_PUTSRX((short *)(offset + offsetof(BUFDESC, count)), count);
        ENT_PUTSX((short *)(offset + offsetof(BUFDESC, status)),
		(short)(EOP_BIT_MASK | EL_BIT_MASK));
	/*
	 * clear the EL bit in the previous last txd in the list.
	 */
        ENT_PUTCX((char *)(bus + WRK.txd_last->bd_off +
                offsetof(BUFDESC, control)), (char)EOP_BIT_MASK);

	/*
	 * update the txd pointers
	 */
	WRK.txd_last = WRK.txd_avail;
	WRK.txd_avail = WRK.txd_avail->next;

	if (pio_rc) {
  	  	TRACE_SYS(HKWD_EN3COM_ERR, "Txm2", (long)-1, pio_rc, 0);
		return(-1);
	}
		
        /* start watchdog timer */
        w_start(&(TXWDT));

	/* For netpmon performance tool */
  	TRACE_SYS(HKWD_EN3COM_XMIT, TRC_WEND, p_dev_ctl->seq_number, 
		(int)p_mbuf, count);

  	TRACE_SYS(HKWD_EN3COM_OTHER, "TxmE", 0, 0, 0);
	return(0);



}

/*****************************************************************************/
/*
 * NAME:     en3com_txq_put
 *
 * FUNCTION: put packets onto the transmit queue.
 *
 * EXECUTION ENVIRONMENT: process and interrupt level.
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_output
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area
 *	p_mbuf		- pointer to a mbuf chain
 *
 * RETURNS:  
 *	NULL - all mbufs are queued.
 *	mbuf pointer - point to a mbuf chain which contains packets
 *		       that overflows the transmit queue.
 */
/*****************************************************************************/
struct mbuf *
en3com_txq_put(
	en3com_dev_ctl_t  *p_dev_ctl,	/* pointer to the device control area */
	struct mbuf	*p_mbuf)	/* pointer to a mbuf chain */

{

	int room;
	int pkt_cnt;
	struct mbuf *p_last, *p_over;



  	TRACE_SYS(HKWD_EN3COM_OTHER, "TqpB", (ulong)p_dev_ctl, 
		(ulong)p_mbuf, 0);

	pkt_cnt = 0;
	room = DDS.xmt_que_size - p_dev_ctl->txq_len;
	if (room > 0) {
		p_last = p_mbuf;
		room--;
		pkt_cnt++;
		while (room && p_last->m_nextpkt) {
			/* For netpmon performance tool */
  			TRACE_SYS(HKWD_EN3COM_XMIT, TRC_WQUE, 
				p_dev_ctl->seq_number, (int)p_last, 
				p_last->m_pkthdr.len);
			room--;
			pkt_cnt++;
			p_last = p_last->m_nextpkt;
		}
		/* For netpmon performance tool */
  		TRACE_SYS(HKWD_EN3COM_XMIT, TRC_WQUE, p_dev_ctl->seq_number, 
			(int)p_last, p_last->m_pkthdr.len);
		p_over = p_last->m_nextpkt;  /* overflow packets */
		
		if (p_dev_ctl->txq_first) {
			p_dev_ctl->txq_last->m_nextpkt = p_mbuf;
			p_dev_ctl->txq_last = p_last;
		}
		else {
			p_dev_ctl->txq_first = p_mbuf;
			p_dev_ctl->txq_last = p_last;
		}
		p_last->m_nextpkt = NULL;
		p_dev_ctl->txq_len += pkt_cnt;
		if (NDD.ndd_genstats.ndd_xmitque_max < p_dev_ctl->txq_len)
			NDD.ndd_genstats.ndd_xmitque_max = p_dev_ctl->txq_len;
	}
	else {
		p_over = p_mbuf;
	}

  	TRACE_SYS(HKWD_EN3COM_OTHER, "TqpE", (ulong)p_over, 0, 0);

	return(p_over);


}

/*****************************************************************************/
/*
 * NAME:     en3com_txq_get
 *
 * FUNCTION: get a packet off the transmit queue.
 *
 * EXECUTION ENVIRONMENT: interrupt level.
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_tx_intr
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area
 *
 * RETURNS:  
 *	NULL - queue is empty
 *	mbuf pointer - point to a mbuf chain which contains a packet.
 */
/*****************************************************************************/
struct mbuf *
en3com_txq_get(
  en3com_dev_ctl_t	*p_dev_ctl)	/* pointer to the device control area */

{

	struct mbuf *p_first = NULL;

  	TRACE_SYS(HKWD_EN3COM_OTHER, "TqgB", (ulong)p_dev_ctl, 0, 0);

	if (p_dev_ctl->txq_first) {
		p_first = p_dev_ctl->txq_first;
		p_dev_ctl->txq_first = p_first->m_nextpkt;
		p_first->m_nextpkt = NULL;
		p_dev_ctl->txq_len--;
	}
  	TRACE_SYS(HKWD_EN3COM_OTHER, "TqgE", (ulong)p_first, 0, 0);

	return(p_first);

}
