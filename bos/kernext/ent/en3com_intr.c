static char sccsid[] = "@(#)42  1.14  src/bos/kernext/ent/en3com_intr.c, sysxent, bos41B, 9506A 1/26/95 11:59:27";
/*
 * COMPONENT_NAME: sysxent -- High Performance Ethernet Device Driver
 *
 * FUNCTIONS: 
 *		en3com_intr
 *		en3com_rv_intr
 *		en3com_tx_intr
 *		en3com_exec_intr
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
#include "en3com_errids.h"

uchar ent_broad_adr[ENT_NADR_LENGTH] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void en3com_rv_intr();
void en3com_tx_intr();
void en3com_exec_intr();
struct mbuf *en3com_txq_get();

/*****************************************************************************/
/*
 * NAME:     en3com_intr
 *
 * FUNCTION: Ethernet driver interrupt routine.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      The FLIH
 *
 * INPUT:
 *      p_ihs		- point to the interrupt structure.
 *
 * RETURNS:  
 *	INTR_SUCC - our interrupt
 *	INTR_FAIL - not our interrupt
 */
/*****************************************************************************/
en3com_intr(
	struct intr *p_ihs)	/* This also points to device control area */

{
  	en3com_dev_ctl_t	*p_dev_ctl = (en3com_dev_ctl_t *)p_ihs;
	uchar status_reg;
	uchar command_reg;
	int ioa; 
	int bus;
	int rc;
	int parity_rc;
	int ipri;
	int pio_rc = 0;


	
	TRACE_SYS(HKWD_EN3COM_OTHER, "SinB", (ulong)p_ihs, 0, 0);

	ipri = disable_lock(PL_IMP, &SLIH_LOCK);

	if (p_dev_ctl->device_state < LIMBO) {
			unlock_enable(ipri, &SLIH_LOCK);
			TRACE_BOTH(HKWD_EN3COM_ERR, "Sin2", INTR_FAIL, 
				p_dev_ctl->device_state, 0);
			return(INTR_FAIL);
	}

	ioa = (int)BUSIO_ATT((ulong)DDS.bus_id, DDS.io_port);

	/*
	 * read the status register, if status register can't be read
	 * then assume not our interrupt
	 */
	ENT_GETCX((char *)(ioa + STATUS_REG), &status_reg);
	if (pio_rc) {
		BUSIO_DET(ioa);

		en3com_hard_err(p_dev_ctl, TRUE, FALSE, NDD_PIO_FAIL);

		unlock_enable(ipri, &SLIH_LOCK);
		TRACE_BOTH(HKWD_EN3COM_ERR, "Sin3", INTR_FAIL, pio_rc, 0);
		return(INTR_FAIL);
	}

	TRACE_DBG(HKWD_EN3COM_OTHER, "Sin4", (ulong)status_reg, 0, 0);
	
	if (status_reg & CWR_MSK) {

		ENT_GETCX((char *)(ioa + COMMAND_REG), &command_reg);
		if (pio_rc) {
			BUSIO_DET(ioa);

			en3com_hard_err(p_dev_ctl, TRUE, FALSE, NDD_PIO_FAIL);

			unlock_enable(ipri, &SLIH_LOCK);
			TRACE_BOTH(HKWD_EN3COM_ERR, "Sin5", INTR_FAIL, 
				pio_rc, 0);
			return(INTR_FAIL);
		}

		rc = INTR_SUCC;
		TRACE_DBG(HKWD_EN3COM_OTHER, "Sin6", (ulong)command_reg, 0, 0);

		/*
	 	* get addressability to adapter memory
	 	*/
		bus = (int)BUSMEM_ATT(DDS.bus_id, DDS.bus_mem_addr);

		if ((command_reg & RECEIVE_MSK) && 
			(p_dev_ctl->device_state == OPENED)) {
			if (NDD.ndd_genstats.ndd_recvintr_lsw == ULONG_MAX)
				NDD.ndd_genstats.ndd_recvintr_msw++;
			NDD.ndd_genstats.ndd_recvintr_lsw++;
			en3com_rv_intr(p_dev_ctl, command_reg & RECEIVE_MSK, 
				bus, ioa);
		}

		if (command_reg & XMIT_MSK) {
			if (NDD.ndd_genstats.ndd_xmitintr_lsw == ULONG_MAX)
				NDD.ndd_genstats.ndd_xmitintr_msw++;
			NDD.ndd_genstats.ndd_xmitintr_lsw++;
			en3com_tx_intr(p_dev_ctl, command_reg & XMIT_MSK, bus);
		}

		if (command_reg & EXECUTE_MSK) {
			en3com_exec_intr(p_dev_ctl, command_reg & EXECUTE_MSK, 
				bus);
		}

		BUSMEM_DET(bus);

	}
	else {		/* not our interrupt */
		rc = INTR_FAIL;
		TRACE_SYS(HKWD_EN3COM_ERR, "Sin7", INTR_FAIL, status_reg, 0);
	}
		

	/*
	 * on AT form factor card the status register will not reflect
	 * parity errors, need more verification.
	 */
	if (((status_reg & PARITY_MSK) || (WRK.card_type == ADPT_10))
	    && (parity_rc = en3com_parity_err(p_dev_ctl, ioa))) {
		rc = INTR_SUCC;
		TRACE_BOTH(HKWD_EN3COM_ERR, "Sin8", INTR_SUCC, status_reg, 0);
		en3com_logerr(p_dev_ctl, ERRID_EN3COM_PARITY, __LINE__, 
			__FILE__, status_reg, parity_rc, 0);
		en3com_hard_err(p_dev_ctl, TRUE, TRUE, ENT_PARITY_ERR);
	}

	BUSIO_DET(ioa);

	unlock_enable(ipri, &SLIH_LOCK);

	TRACE_SYS(HKWD_EN3COM_OTHER, "SinE", rc, 0, 0);
	return(rc);


}

/*****************************************************************************/
/*
 * NAME:     en3com_rv_intr
 *
 * FUNCTION: Ethernet driver receive interrupt routine.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_intr
 *
 * INPUT:
 *      p_dev_ctl	- point to the device control area
 *	command_reg	- value from command register
 *	bus		- bus memory access handle 
 *	ioa		- io address access handle
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
void
en3com_rv_intr(
  en3com_dev_ctl_t	*p_dev_ctl,	/* point to the device control area */
  uchar		command_reg,	/* value in the command register */
  int 		bus,		/* bus memory access handel */
  int		ioa)		/* io address access handle */

{

	int rc;
	int pio_rc = 0;
	uchar bd_stat;
	en3com_bdc_t *p_rd;
	int bufoff;
	short count;
	int broad, mcast;
	ndd_t	*p_ndd = (ndd_t *)&(NDD);
	struct mbuf  *p_mbuf;



	TRACE_SYS(HKWD_EN3COM_RECV, "SrvB", (ulong)p_dev_ctl, 
		(ulong)command_reg, 0);

	/*
         * Process the receive interrupt
	 * start processing the receive list
	 */
	if (command_reg == RX_P_RCVD || command_reg == RX_ERROR) {

	  while (TRUE) {

		bd_stat = 0;
		p_rd = WRK.rvd_first;
		bufoff = bus + p_rd->bd_off;
		broad = mcast = FALSE;
		ENT_GETCX((char *)(bufoff + offsetof(BUFDESC, status)),
			&bd_stat);

		if (pio_rc) {
			en3com_hard_err(p_dev_ctl, TRUE, FALSE, NDD_PIO_FAIL);
			TRACE_BOTH(HKWD_EN3COM_ERR, "Srv1", pio_rc, 0, 0);
			return;
		}

		/*
		 * exit if no more packets to process
		 */
         	if (!(bd_stat & CMPLT_BIT_MASK))
            		break;


		/*
		 * Check dma status
		 */
		if ((rc = d_complete(WRK.dma_channel, DMA_READ|DMA_NOHIDE,
			p_rd->buf, ENT_MAX_MTU, &WRK.rvbuf_xmem,
			p_rd->dma_io)) != DMA_SUCC) {
		
			en3com_logerr(p_dev_ctl, ERRID_EN3COM_DMAFAIL, 
				__LINE__, __FILE__, WRK.dma_channel, p_rd->buf, 
				ENT_MAX_MTU);
			TRACE_BOTH(HKWD_EN3COM_ERR, "Srv2", rc, 0, 0);
		}
		
		/*
		 * Is this a good packet or bad packet? 
		 */
		
		if (!(bd_stat &= BD_ERR_MASK)) { /* no error, good packet */

			ENT_GETSRX((short *)(bufoff + offsetof(BUFDESC, count)),
				&count);
			if (pio_rc) {
				en3com_hard_err(p_dev_ctl, TRUE, FALSE, 
					NDD_PIO_FAIL);
				TRACE_BOTH(HKWD_EN3COM_ERR, "Srv3", pio_rc, 
					0, 0);
				return;
			}

			/* For netpmon performance tool */
			TRACE_SYS(HKWD_EN3COM_RECV, TRC_RDAT, 
				p_dev_ctl->seq_number, count, 0);

			if (p_ndd->ndd_genstats.ndd_ipackets_lsw == ULONG_MAX)
			  p_ndd->ndd_genstats.ndd_ipackets_msw++;
			p_ndd->ndd_genstats.ndd_ipackets_lsw++;
			if ((ULONG_MAX - count) < 
			  p_ndd->ndd_genstats.ndd_ibytes_lsw)
			  p_ndd->ndd_genstats.ndd_ibytes_msw++;
			p_ndd->ndd_genstats.ndd_ibytes_lsw += count;

			/* check if broadcast or multicast */
			if (p_rd->buf[0] & MULTI_BIT_MASK) {
			  if (SAME_NADR(p_rd->buf, ent_broad_adr)) {
				broad = TRUE;
				ENTSTATS.bcast_rx_ok++;
			  }
			  else {
				mcast = TRUE;
 				ENTSTATS.mcast_rx_ok++;

			  }
			}

			if (count <= MHLEN) {
				p_mbuf = m_gethdr(M_DONTWAIT, MT_HEADER);
			}
                        else {
                                p_mbuf = m_getclustm(M_DONTWAIT, MT_HEADER,
					count);
			}
			if (p_mbuf) {
				p_mbuf->m_len = count;
				p_mbuf->m_pkthdr.len = count;
				p_mbuf->m_flags |= M_PKTHDR;
				bcopy(p_rd->buf, MTOD(p_mbuf, char *), count);
				if (broad)
					p_mbuf->m_flags |= M_BCAST;
				if (mcast)
					p_mbuf->m_flags |= M_MCAST;

				/* For netpmon performance tool */
				TRACE_SYS(HKWD_EN3COM_RECV, TRC_RNOT, 
					p_dev_ctl->seq_number, p_mbuf, count);

       		                (*(p_ndd->nd_receive))(p_ndd, p_mbuf);

 				TRACE_SYS(HKWD_EN3COM_RECV, TRC_REND, p_mbuf, 
					0, 0);

			}
			else {
				p_ndd->ndd_genstats.ndd_nobufs++;
				p_ndd->ndd_genstats.ndd_ipackets_drop++;
			}

		}  /* if good packet */
		else {    /* this is a bad packet */

    			ndd_statblk_t  stat_blk;   /* new status block */


		/*
		* Got a bad frame,  put it in a mbuf,
		* create a status block and pass it to the demuxer.
		*/


			ENT_GETSRX(bufoff + offsetof(BUFDESC, count), &count);

			if (pio_rc) {
				en3com_hard_err(p_dev_ctl, TRUE, FALSE, 
					NDD_PIO_FAIL);
				TRACE_BOTH(HKWD_EN3COM_ERR, "Srv4", pio_rc, 
					0, 0);
				return;
			}

			/* increment bad packet counter */
			p_ndd->ndd_genstats.ndd_ibadpackets++;

			if (count <= MHLEN) {
                                  p_mbuf = m_gethdr(M_DONTWAIT, MT_HEADER);
			}
                        else {
                                  p_mbuf = m_getclustm(M_DONTWAIT, MT_HEADER,
					count);
			}
			if (p_mbuf) {
				  p_mbuf->m_len = count;
				  p_mbuf->m_pkthdr.len = count;
				  p_mbuf->m_flags |= M_PKTHDR;
				  bcopy(p_rd->buf, MTOD(p_mbuf, char *), count);

				  bzero(&stat_blk, sizeof(ndd_statblk_t));
    				  stat_blk.code = NDD_BAD_PKTS;
    				  stat_blk.option[0] = (ulong)bd_stat;
    				  stat_blk.option[1] = (ulong)p_mbuf;

      				  (*(p_ndd->nd_status))(p_ndd, &stat_blk);

				  /* we own the mbuf, so we free it */
				  m_free(p_mbuf);   
			}
			else {
				  p_ndd->ndd_genstats.ndd_nobufs++;
			  	  p_ndd->ndd_genstats.ndd_ipackets_drop++;
			}


		}

		/*
		 * Invalidate processor cache.
		 * Note that only the range that was touched by
		 * the above bcopy needs to be invalidated.
		 */
		cache_inval(p_rd->buf, count);

		/*
		 * shift EL bit from last receive buffer descriptor to this
		 * one and clear the status byte
		 */

		ENT_PUTSX(bufoff, (short)EL_BIT_MASK);
		ENT_PUTCX(bus + WRK.rvd_last->bd_off +
			offsetof(BUFDESC, control), (uchar)EOP_BIT_MASK);

		/*
		 * update the receive descriptors list pointers.
		 */
		WRK.rvd_first = WRK.rvd_first->next;
		WRK.rvd_last = p_rd;

	  }  /* while */

	  if (command_reg == RX_ERROR) {
		if (en3com_rv_start(p_dev_ctl, bus, ioa, FALSE)) {
			en3com_hard_err(p_dev_ctl, TRUE, FALSE, NDD_PIO_FAIL);
			TRACE_BOTH(HKWD_EN3COM_ERR, "Srv5", 0, 0, 0);
			return;
		}
	  }

	}
	else {     /* interrupt that should not happen */
	  en3com_logerr(p_dev_ctl, ERRID_EN3COM_FAIL, __LINE__, __FILE__,
          	(ulong)command_reg, 0, 0);
	  en3com_hard_err(p_dev_ctl, TRUE, TRUE, NDD_UCODE_FAIL);
          TRACE_BOTH(HKWD_EN3COM_ERR, "Srv6", 0, 0, 0);
          return;
	}
  
	TRACE_SYS(HKWD_EN3COM_RECV, "SrvE", 0, 0, 0);


}

/*****************************************************************************/
/*
 * NAME:     en3com_tx_intr
 *
 * FUNCTION: Ethernet driver transmit interrupt routine.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_intr
 *
 * INPUT:
 *      p_dev_ctl	- point to the device control area
 *	command_reg	- value from command register
 *	bus		- bus memory access handle
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
void
en3com_tx_intr(
  en3com_dev_ctl_t	*p_dev_ctl,	/* point to the device control area */
  uchar		command_reg,	/* value in the command register */
  int		bus)		/* bus memory access handle */

{
	
	int rc;
	int restart_timer = FALSE;
	en3com_bdc_t  *p_td;
	ndd_t	*p_ndd = (ndd_t *)&(NDD);
	uchar status;
	struct mbuf *p_mbuf;
	int count;
	int pio_rc = 0;
	int ipri;



	TRACE_SYS(HKWD_EN3COM_XMIT, "StxB", (ulong)p_dev_ctl, 
		(ulong)command_reg, bus);

	ipri = disable_lock(PL_IMP, &TX_LOCK);


	if ((command_reg == TX_P_SENT) || (command_reg == TX_ERROR)) {

		/*
		 * Stop the watchdog timer
		 */
		w_stop(&TXWDT);
		restart_timer = TRUE;

		while (TRUE) {
			p_td = WRK.txd_first;
			if (!(p_td->flags & BDC_IN_USE))
				break;

			/*
			 * get status byte from buffer descriptor
			 */
			ENT_GETCX((char *)(bus + p_td->bd_off),
						&status);

			if (pio_rc) {
				unlock_enable(ipri, &TX_LOCK);
				en3com_hard_err(p_dev_ctl, TRUE, FALSE, 
					NDD_PIO_FAIL);
				TRACE_BOTH(HKWD_EN3COM_ERR, "Stx1", pio_rc, 
					0, 0);
				return;
			}

			if (!(status & CMPLT_BIT_MASK)) {
				break;
			}

			/* 
			 * if xmit completed with error or not
			 */
			count = p_td->tx_len;
			if (status & BD_ERR_MASK)  {
				p_ndd->ndd_genstats.ndd_oerrors++;
			}
			else {
	  		  if (p_ndd->ndd_genstats.ndd_opackets_lsw == 
				ULONG_MAX) 
	    			p_ndd->ndd_genstats.ndd_opackets_msw++;
	    		  p_ndd->ndd_genstats.ndd_opackets_lsw++;

	  		  if ((ULONG_MAX - count) < 
				p_ndd->ndd_genstats.ndd_obytes_lsw)
	    			p_ndd->ndd_genstats.ndd_obytes_msw++;
	    		  p_ndd->ndd_genstats.ndd_obytes_lsw += count;

			  if (p_td->flags & BDC_BCAST) {
 				ENTSTATS.bcast_tx_ok++;
			  }
			  if (p_td->flags & BDC_MCAST) {
				ENTSTATS.mcast_tx_ok++;
			  }
				
			}
			/*
			 * Check dma status
			 */
			if ((rc = d_complete(WRK.dma_channel, DMA_WRITE_ONLY,
				p_td->buf, (size_t)count,
				(struct xmem *)&WRK.txbuf_xmem,
				(char *)p_td->dma_io)) != DMA_SUCC) {

				en3com_logerr(p_dev_ctl, ERRID_EN3COM_DMAFAIL, 
					__LINE__, __FILE__, WRK.dma_channel, 
					p_td->buf, count);

				TRACE_BOTH(HKWD_EN3COM_ERR, "Stx2", rc, 0, 0);
			}


			/*
			 * mark transmit element as free
			 */
			p_td->flags = BDC_INITED;
			WRK.txd_first = p_td->next;
			p_dev_ctl->tx_pending--;

		}
	}
        else {     /* interrupt that should not happen */
          en3com_logerr(p_dev_ctl, ERRID_EN3COM_FAIL, __LINE__, __FILE__,
                (ulong)command_reg, 0, 0);
	  unlock_enable(ipri, &TX_LOCK);
          en3com_hard_err(p_dev_ctl, TRUE, TRUE, NDD_UCODE_FAIL);
          TRACE_BOTH(HKWD_EN3COM_ERR, "Stx3", 0, 0, 0);
          return;
        }


	/* 
	 * if there is packets on the transmit queue to be transmitted now.
	 */
	while (p_dev_ctl->txq_len && !(WRK.txd_avail->flags & BDC_IN_USE)) { 
		if (p_mbuf = en3com_txq_get(p_dev_ctl)) {
			if (!en3com_xmit(p_dev_ctl, p_mbuf, bus)) {
				/* 
				 * Transmit OK, free the packet 
				 */
				m_freem(p_mbuf);
				restart_timer = FALSE;
			}
			else {
				/*
				 * Transmit error. free the packet.
				 */
				NDD.ndd_genstats.ndd_oerrors++;
				m_freem(p_mbuf);

				unlock_enable(ipri, &TX_LOCK);
				en3com_hard_err(p_dev_ctl, TRUE, FALSE, 
					NDD_PIO_FAIL);
				TRACE_BOTH(HKWD_EN3COM_ERR, "Stx4", 0, 0, 0);
				return;
			}
		}
		else 
			break;
	}
	
	/* re-arm the watchdog timer */
	if (restart_timer && p_dev_ctl->tx_pending) {
		w_start(&(TXWDT));
	}
				
	unlock_enable(ipri, &TX_LOCK);

	TRACE_SYS(HKWD_EN3COM_XMIT, "StxE", 0, 0, 0);

}

/*****************************************************************************/
/*
 * NAME:     en3com_exec_intr
 *
 * FUNCTION: Ethernet driver execute command interrupt routine.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_intr
 *
 * INPUT:
 *      p_dev_ctl	- point to the device control area
 *	command_reg	- value from command register
 *	bus		- bus memory access handle
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
void
en3com_exec_intr(
  en3com_dev_ctl_t	*p_dev_ctl,	/* point to the device control area */
  uchar		command_reg,	/* value in the command register */
  int		bus)		/* bus memory access handle */

{

  short tmp_stat;
  int pio_rc = 0;
  int ipri;
  int rc;
  ndd_statblk_t  stat_blk;   /* status block */




	TRACE_SYS(HKWD_EN3COM_OTHER, "SexB", (ulong)p_dev_ctl, 
		(ulong)command_reg, bus);


	/* commands completed during non-limbo mode */

	if (p_dev_ctl->device_state != LIMBO) {

	  ipri = disable_lock(PL_IMP, &CMD_LOCK);
	  /*
	   * Stop the watchdog timer
	   */
	  w_stop(&(CTLWDT));

          ENT_GETSRX( bus + WRK.exec_mail_box, &tmp_stat);
	  if (pio_rc) {
		unlock_enable(ipri, &CMD_LOCK);
		en3com_hard_err(p_dev_ctl, TRUE, FALSE, NDD_PIO_FAIL);
		TRACE_BOTH(HKWD_EN3COM_ERR, "Sex1", pio_rc, 0, 0);
		return;
	  }

	  if ((tmp_stat & DONE_MSK) && !(tmp_stat & ERROR_MSK)) 
		p_dev_ctl->ctl_status = 0;     /* no error */
	  else {
		TRACE_SYS(HKWD_EN3COM_ERR, "Sex2", tmp_stat, 0, 0);
	  }

	  /* wakeup the ioctl event */
	  e_wakeup((int *)&p_dev_ctl->ctl_event);

	  unlock_enable(ipri, &CMD_LOCK);
	}
	else {
	  /* for commands executed during the limbo (error recovery) mode */

 	  /*
           * Stop the system timer
           */
          while(tstop(p_dev_ctl->systimer)) {
		unlock_enable(PL_IMP, &SLIH_LOCK);
		disable_lock(PL_IMP, &SLIH_LOCK);
	  }
	
          ENT_GETSRX( bus + WRK.exec_mail_box, &tmp_stat);
	  if (pio_rc) {
		en3com_hard_err(p_dev_ctl, TRUE, FALSE, NDD_PIO_FAIL);
		TRACE_BOTH(HKWD_EN3COM_ERR, "Sex3", pio_rc, 0, 0);
		return;
	  }

	  if ((tmp_stat & DONE_MSK) && !(tmp_stat & ERROR_MSK)) {

		/* command has no error, go through the limbo state */

		switch (p_dev_ctl->limbo_state) {
		  case LIMBO_SET_EN:
  			/* Turn off AL-LOC to enable back-to-back 
			   receive frames if new microcode */
  			if (WRK.vpd_hex_rosl >= 0xC) {
			  p_dev_ctl->limbo_state = LIMBO_AL_LOC_OFF;
        		  if (en3com_cmd(p_dev_ctl, AL_LOC_OFF, TRUE)) {
                		TRACE_SYS(HKWD_EN3COM_ERR, "Sex4", ENOCONNECT,
                        	  AL_LOC_OFF, 0);
                		break;
			  }
			}
			else {
			  p_dev_ctl->limbo_state = LIMBO_CONFIGURE;
  			  /* Set up the Receive Packet Filter */
  			  if (en3com_cmd(p_dev_ctl, CONFIGURE, TRUE)) {
        			TRACE_SYS(HKWD_EN3COM_ERR, "Sex5", ENOCONNECT, 
				  CONFIGURE, 0);
				break;
			  }
			}

                        /* set system timer to delay 10 ms. */
			STIMER_MS(10);

                        break;

		  case LIMBO_AL_LOC_OFF:
			p_dev_ctl->limbo_state = LIMBO_CONFIGURE;
  			/* Set up the Receive Packet Filter */
  			if (en3com_cmd(p_dev_ctl, CONFIGURE, TRUE)) {
        			TRACE_SYS(HKWD_EN3COM_ERR, "Sex6", ENOCONNECT, 
				  CONFIGURE, 0);
				break;
			}

                        /* set system timer to delay 10 ms. */
			STIMER_MS(10);

                        break;

		  case LIMBO_CONFIGURE:
			p_dev_ctl->limbo_state = LIMBO_SET_ADDR;
  			/* Set up adapter with Network Address  */
  			if (en3com_cmd(p_dev_ctl, SET_ADDRESS, TRUE)) {
        			TRACE_SYS(HKWD_EN3COM_ERR, "Sex7", ENOCONNECT, 
				  CONFIGURE, 0);
				break;
			}

                        /* set system timer to delay 10 ms. */
			STIMER_MS(10);

                        break;

		  case LIMBO_SET_ADDR:
			p_dev_ctl->limbo_state = LIMBO_CONFIG_LIST;
  			/* Set up adapter the number of transmit & receive 
			 * list to use. Need to do another report_Config to 
			 * know where the list are.
			 */
  			if (en3com_cmd(p_dev_ctl, CONFIG_LIST, TRUE)) {
        			TRACE_SYS(HKWD_EN3COM_ERR, "Sex8", ENOCONNECT, 
				  CONFIG_LIST, 0);
				break;
			}

                        /* set system timer to delay 10 ms. */
			STIMER_MS(10);

                        break;

		  case LIMBO_CONFIG_LIST:
			p_dev_ctl->limbo_state = LIMBO_REPORT_CONFIG;
  			/* get the current adapter configuration */
  			if (en3com_cmd(p_dev_ctl, REPORT_CONFIG, TRUE)) {
        			TRACE_SYS(HKWD_EN3COM_ERR, "Sex9", ENOCONNECT, 
				  REPORT_CONFIG, 0);
				break;
			}

                        /* set system timer to delay 10 ms. */
			STIMER_MS(10);

                        break;

		  case LIMBO_REPORT_CONFIG:
  			if (en3com_getcfg(p_dev_ctl, bus)) {
        			TRACE_SYS(HKWD_EN3COM_ERR, "Sexa", ENOCONNECT, 
				  0, 0);
				break;
			}

			p_dev_ctl->limbo_state = LIMBO_NO_FILTER;
  			/* Set up adapter with no filters */
  			if (en3com_cmd(p_dev_ctl, SET_TYPE_NULL, TRUE)) {
        			TRACE_SYS(HKWD_EN3COM_ERR, "Sexb", ENOCONNECT, 
				  SET_TYPE_NULL, 0);
				break;
			}

                        /* set system timer to delay 10 ms. */
			STIMER_MS(10);

                        break;

		  case LIMBO_NO_FILTER:
			p_dev_ctl->limbo_state = LIMBO_MULTI;

			/* 
			 * Set up multicast filter if there is one.
			 * If there is no need for this, fall through to
			 * the next step.
			 */
			if (WRK.multi_count && WRK.multi_count <= MAX_MULTI) {
			  if (en3com_cmd(p_dev_ctl, SET_MULTICAST, TRUE)) {
        			TRACE_SYS(HKWD_EN3COM_ERR, "Sexc", ENOCONNECT, 
				  SET_MULTICAST, 0);
				break;
			  }

                          /* set system timer to delay 10 ms. */
			  STIMER_MS(10);

                          break;
			}
				
		  case LIMBO_MULTI:

  			/*
   			 * Set up for transmit buffer descriptors
   			 */
  			if (rc = en3com_tx_setup(p_dev_ctl)) {
      				TRACE_SYS(HKWD_EN3COM_ERR, "Sexc", rc, 0, 0);
      				break;
  			}

  			/*
   			 * Set up for receive buffer descriptor
   			 */
  			if (rc = en3com_rv_setup(p_dev_ctl, TRUE)) {
      				TRACE_SYS(HKWD_EN3COM_ERR, "Sexd", rc, 0, 0);
      				break;
			}
			
  			/* pass a status block to demuxer */
  			bzero(&stat_blk, sizeof(ndd_statblk_t));
  			stat_blk.code = NDD_LIMBO_EXIT;
  			(*(NDD.nd_status))(&NDD, &stat_blk);
			
			p_dev_ctl->device_state = OPENED;
			p_dev_ctl->limbo_state = NO_LIMBO;
			NDD.ndd_flags &= ~NDD_LIMBO;
			NDD.ndd_flags |= NDD_RUNNING;
			break;

		  default:
			break;

		}

	  }

	}



	TRACE_SYS(HKWD_EN3COM_OTHER, "SexE", 0, 0, 0);

}
