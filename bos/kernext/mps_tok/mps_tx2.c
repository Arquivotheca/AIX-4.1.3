static char sccsid[] = "@(#)76  1.15.1.3  src/bos/kernext/mps_tok/mps_tx2.c, sysxmps, bos41J, 9520B_all 5/18/95 11:20:53";
/*
 *   COMPONENT_NAME: sysxmps
 *
 *   FUNCTIONS: mps_output
 *		mps_tx2_done
 *		mps_send2
 *		mps_clean_dma2
 *		mps_tx2_undo
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/lock_def.h>
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/malloc.h>
#include <sys/dump.h>
#include <sys/watchdog.h>
#include <sys/intr.h>
#include <sys/mbuf.h>
#include <sys/err_rec.h>
#include <sys/trcmacros.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/dma.h>
#include <sys/cdli.h>
#include <sys/ndd.h>
#include <unistd.h>

#include <sys/cdli_tokuser.h>
#include <sys/generic_mibs.h>
#include <sys/tokenring_mibs.h>

#include "mps_dslo.h"
#include "mps_mac.h"
#include "mps_dds.h"
#include "mps_dd.h"
#include "tr_mps_errids.h"
#ifdef KTD_DEBUG
#include "intercept_functions.h"
#endif

/****************************************************************************/
/*
* NAME: mps_tx2_undo
*
* FUNCTION: Delete transmit list & reset transmit pointers and flags. 
*
* NOTES:
*
*    Input: 
*  p_dev_ctl - pointer to device control structure
*
*    Output: 
*  Deleted transmit list and flags reset.
*
*    Called From: 
*  mps_act   - on failure
*       ndd_close - normal shutdown
*
* RETURN:  0 - Successful completion
*/
/****************************************************************************/
int mps_tx2_undo (mps_dev_ctl_t *p_dev_ctl)
{
  int         i, rc;          /* Local Loop counter                       */

  TRACE_SYS(MPS_TX, "C2uB", p_dev_ctl, 0, 0);
  for (i = 0; i < MAX_TX_LIST; i++) {
  	rc = d_complete(WRK.dma_channel, DMA_WRITE_ONLY, WRK.tx2_buf[i],
  	    H_PAGE, &WRK.tx2_mem_block, WRK.tx2_addr[i]);
       	if (rc != DMA_SUCC) {
        	mps_logerr(p_dev_ctl, ERRID_MPS_DMAFAIL, __LINE__, __FILE__, 
			   WRK.dma_channel, WRK.tx2_buf[i], rc);
      	}

  	xmfree(WRK.tx2_buf[i], pinned_heap);
  	WRK.tx2_buf[i] = NULL;
  	WRK.tx2_addr[i] = NULL;
  }
  rc = d_complete(WRK.dma_channel, DMA_READ | DMA_NOHIDE, WRK.tx2_p_mem_block,
      		  TX2_DESCRIPTOR_SIZE, &(WRK.tx2_mem_block), 
		  (uchar *)(DDS.dma_base_addr + TX2_DESCRIPTOR_SIZE));
  if (rc != DMA_SUCC) {
       	mps_logerr(p_dev_ctl, ERRID_MPS_DMAFAIL, __LINE__, __FILE__, 
		   WRK.dma_channel, WRK.tx2_p_mem_block, rc);
  }

  /* 
   * Undo adapter control area                                          
   */
  xmfree(WRK.tx2_p_mem_block, pinned_heap);

  TRACE_SYS(MPS_TX, "C2uE", p_dev_ctl, 0, 0);
  return(0);
}  /* end function mps2_tx_undo                                             */

/*****************************************************************************/
/*
 * NAME: mps_clean_dma2
 *
 * FUNCTION: clean dma channel for retransmit
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *    	        p_dev_ctl - pointer to device control structure
 *		mps_tx2_done    
 *
 * INPUT:
 *    	        mbufp - buffer to be clean 
 *
 */
/*****************************************************************************/
void mps_clean_dma2 (
mps_dev_ctl_t       *p_dev_ctl,
struct mbuf       *mbufp)
{

  uint		rc;
  uchar		*dma_addr;
  ushort	buf_out = WRK.tx2_buf_next_out;
  ushort	dma_out = WRK.tx2_dma_next_out;

  TRACE_SYS(MPS_TX, "T2cB", p_dev_ctl, mbufp, 0); 
  if (mbufp->m_pkthdr.len <= H_PAGE) {
        /*
         * Cleans up the DMA channel
         */
        rc = d_complete(WRK.dma_channel, DMA_WRITE_ONLY, WRK.tx2_buf[buf_out], 
                H_PAGE, &WRK.tx2_xmemp[buf_out], WRK.tx2_addr[buf_out]);
        if (rc != DMA_SUCC) {
                mps_logerr(p_dev_ctl, ERRID_MPS_DMAFAIL, __LINE__, __FILE__, 
                        WRK.dma_channel, WRK.tx2_addr[buf_out], rc);
        }

        /*
         * Sets up the DMA channel
         */
        d_master(WRK.dma_channel, DMA_WRITE_ONLY, WRK.tx2_buf[buf_out], 
                H_PAGE, &WRK.tx2_xmemp[buf_out], WRK.tx2_addr[buf_out]);
  } else {
        while (mbufp) {
                if (mbufp->m_len > H_PAGE) {
                        /*
                         * Cleans up the DMA channel
                         */
                        dma_addr = WRK.tx2_dma_addr[dma_out] +
                                        (int)((int)mtod(mbufp, uchar *) &0xFFF);
                        rc = d_complete(WRK.dma_channel, DMA_WRITE_ONLY,
                                        mtod(mbufp, uchar *), mbufp->m_len,
                                        &WRK.tx2_dma_xmemp[dma_out], dma_addr);
                        if (rc != DMA_SUCC) {
                                mps_logerr(p_dev_ctl, ERRID_MPS_DMAFAIL, 
                                        __LINE__, __FILE__, WRK.dma_channel,
                                        WRK.tx2_dma_addr[dma_out], rc);
                        }

                        /*
                         * Sets up the DMA channel
                         */
                        d_master(WRK.dma_channel, DMA_WRITE_ONLY,
                                mtod(mbufp, uchar *), mbufp->m_len,
                                &WRK.tx2_dma_xmemp[dma_out], dma_addr);
                        XMITQ_INC(dma_out);

                } else {
                        /*
                         * Cleans up the DMA channel
                         */
                        rc = d_complete(WRK.dma_channel, DMA_WRITE_ONLY,
                                WRK.tx2_buf[buf_out], H_PAGE,
                                &WRK.tx2_xmemp[buf_out], WRK.tx2_addr[buf_out]);
                        if (rc != DMA_SUCC) {
                                mps_logerr(p_dev_ctl, ERRID_MPS_DMAFAIL, 
                                        __LINE__, __FILE__, WRK.dma_channel,
                                        WRK.tx2_addr[buf_out], rc);
                        }

                        /*
                         * Sets up the DMA channel
                         */
                        d_master(WRK.dma_channel, DMA_WRITE_ONLY,
                                WRK.tx2_buf[buf_out], H_PAGE,
                                &WRK.tx2_xmemp[buf_out], WRK.tx2_addr[buf_out]);
                        XMITQ_INC(buf_out);
                }
                mbufp = mbufp->m_next;
        }
  }

  TRACE_SYS(MPS_TX, "T2cB", p_dev_ctl, mbufp, 0); 
}
/*****************************************************************************/
/*
 * NAME: mps_send2
 *
 * FUNCTION: send the data buffer to the adapter
 *
 * EXECUTION ENVIRONMENT: process and interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *  		mps_output
 *		mps_tx2_done    
 *
 * INPUT:
 *    	        p_dev_ctl - pointer to device control structure
 *	     	ioa       - IO address 
 *
 */
/*****************************************************************************/
void mps_send2 (
mps_dev_ctl_t       *p_dev_ctl,
int                   ioa)
{

  ndd_t			*p_ndd = (ndd_t *)&(NDD);
  struct  mbuf          *mbufp, *txq2_first, *n, *m, *head;
  ushort                buf_count;
  int                   i,j,d, cnt, ct, len, copy_len, no_space = FALSE;
  uchar      	        *dma_addr, *ndata, *odata;

  TRACE_SYS(MPS_TX, "Ts2B", p_dev_ctl, WRK.tx2_elem_next_in, 
						WRK.tx2_elem_next_out); 
  
  /*
   * What DMA address adapter get for transmit are based on the
   * length of the package and the length of each mbuf
   *   o  For package length less than or equal to the 2K bytes.
   *      Copys data into transmit buffer which already setup
   *      for DMA
   *   o  For package length greater than 2K bytes
   *	    - If mbuf len less than or equal to the 2K bytes. 
   *          Copys data into transmit buffer which already setup
   *          for DMA
   *	    - If mbuf length greater than the 2K bytes. 
   *          Setup DMA transfer directly from the mbuf data
   *          to the adapter
   */
  while (p_dev_ctl->txq2_first && !(XMITQ2_FULL))
  {
        i = WRK.tx2_elem_next_in;

        WRK.tx2_queue[i].bytes = p_dev_ctl->txq2_first->m_pkthdr.len;
        WRK.tx2_queue[i].mbufp = p_dev_ctl->txq2_first;
        txq2_first  = p_dev_ctl->txq2_first->m_nextpkt;
        WRK.tx2_queue[i].mbufp->m_nextpkt = NULL;

        /*
         * Set up temp buffer descriptor entry
         */
        WRK.tx2_temp[i].fw_pointer  =
                        toendianL((ulong)WRK.tx2_list[(i + 1) % MAX_TX_LIST]);
        WRK.tx2_temp[i].xmit_status   = 0;
        WRK.tx2_temp[i].frame_len     = 0;

        if (WRK.tx2_queue[i].bytes <= H_PAGE) {
                if (WRK.tx2_buf_use_count < MAX_TX_LIST) {
                	j = WRK.tx2_buf_next_in;
                	/*
                	 * Copys data into transmit buffer which already
			 * setup for DMA and do processor cache flush
                	 */
                        MBUF_TO_MEM(WRK.tx2_queue[i].mbufp, WRK.tx2_buf[j]);
                        vm_cflush(WRK.tx2_buf[j], WRK.tx2_queue[i].bytes);

                        /*
                         * Setup temp buffer descriptor entry and update
                         * transmit counter
                         */
                        WRK.tx2_temp[i].buf_count     = 0x0200;
                        WRK.tx2_temp[i].xbuf[0].data_pointer =
                                        toendianL((ulong)WRK.tx2_addr[j]);
                        WRK.tx2_temp[i].xbuf[0].buf_len =
                                toendianW((ushort)WRK.tx2_queue[i].bytes);
                        WRK.tx2_temp[i].xbuf[1].data_pointer =
                                toendianL((ulong)WRK.tx2_addr[j]);
                        WRK.tx2_temp[i].xbuf[1].buf_len      = 0;

			XMITQ_INC(WRK.tx2_buf_next_in);
                        WRK.tx2_buf_use_count++;
                } else {
  			TRACE_BOTH(MPS_ERR, "Ts21", p_dev_ctl, 
					WRK.tx2_elem_next_in, 
					WRK.tx2_buf_use_count);
			WRK.tx2_queue[i].mbufp->m_nextpkt = txq2_first;
                        return;
                }

        } else {
                cnt = 0;
        	mbufp = WRK.tx2_queue[i].mbufp;
                while (mbufp) {
                        if (cnt > 11) {
                        /* if mbuf chain is more then 11 then collapsing */
                                n = m_getclust(M_DONTWAIT, MT_HEADER);
                                if (!n) {
                                        WRK.tx2_queue[i].mbufp->m_nextpkt = 
								txq2_first;
                                        return;
                                }
                                head = m = n;
                                m->m_flags |= M_PKTHDR;
                                m->m_nextpkt = NULL;
                                n->m_len = 0;
                                ndata = mtod(n, caddr_t);

                                mbufp = WRK.tx2_queue[i].mbufp;
                                while (mbufp) {
                                        odata = mtod(mbufp, caddr_t);
                                        len = mbufp->m_len;
                                        while (len) {
                                                /* 
						 * get new "gather" mbuf if 
						 * data won't fit      
						 */
                                                if (n->m_len >= CLBYTES) {
                                                        n=m_getclust(M_DONTWAIT,
								     MT_DATA);
                                                        if (!n) {
                                                                m_freem(head);
                                             WRK.tx2_queue[i].mbufp->m_nextpkt =
                                                                     txq2_first;
                                                                return;
                                                        }
                                                        m->m_next = n;
                                                        m = n;
                                                        ndata = mtod(n,caddr_t);
                                                        n->m_len = 0;
                                                }

                                                if ((n->m_len + len) > CLBYTES){
                                                        copy_len = CLBYTES - 
								   n->m_len;
                                                } else {
                                                        copy_len = len;
                                                }

                                                bcopy(odata+(mbufp->m_len-len),
                                                      ndata + n->m_len,
                                                      copy_len);

                                                len -= copy_len;
                                                n->m_len += copy_len;
                                        }
                                        mbufp = mbufp->m_next;
                                }
                                if(p_dev_ctl->txq2_first==p_dev_ctl->txq2_last){
                                        p_dev_ctl->txq2_first = 
					p_dev_ctl->txq2_last  = head;
                                } else {
        				p_dev_ctl->txq2_first = head;
				}
        			p_dev_ctl->txq2_first->m_pkthdr.len = 
							WRK.tx2_queue[i].bytes;
                                m_freem(WRK.tx2_queue[i].mbufp);
                                WRK.tx2_queue[i].mbufp = head;

                                break;
                        }
                        mbufp = mbufp->m_next;
                        cnt++;
                }

                cnt = 0;
                mbufp = WRK.tx2_queue[i].mbufp;
                while (mbufp) {
                        j = WRK.tx2_buf_use_count;
			ct = WRK.tx2_dma_use_count;
                        if ((mbufp->m_len <= H_PAGE) && (j < MAX_TX_LIST)) {
                		/*
                		 * Copys data into transmit buffer which already
				 * setup for DMA and do processor cache flush
				 * and then update the transmit counter
                		 */
                        	j = WRK.tx2_buf_next_in;
                                bcopy(mtod(mbufp, caddr_t),
                                           WRK.tx2_buf[j],
                                           mbufp->m_len);
                                 WRK.tx2_temp[i].xbuf[cnt].data_pointer =
                                              toendianL((ulong)WRK.tx2_addr[j]);
                        	 vm_cflush(WRK.tx2_buf[j], mbufp->m_len);

				 XMITQ_INC(WRK.tx2_buf_next_in);
                                 WRK.tx2_buf_use_count++;

                        } else if((mbufp->m_len > H_PAGE)&&(ct < MAX_TX_LIST)) {

                                /*
                                 * Setup the DMA channel for block mode DMA 
				 * transfer directly from the mbuf data
				 * to the adapter
                                 */
				ct = WRK.tx2_dma_next_in;
                                WRK.tx2_dma_xmemp[ct].aspace_id   = XMEM_GLOBAL;
                                WRK.tx2_dma_xmemp[ct].subspace_id = NULL;
				dma_addr = WRK.tx2_dma_addr[ct] + 
					(int)((int)mtod(mbufp, uchar *) &0xFFF);
        			d_master(WRK.dma_channel, DMA_WRITE_ONLY, 
					mtod(mbufp, uchar *), mbufp->m_len, 
					&WRK.tx2_dma_xmemp[ct], dma_addr);
                                WRK.tx2_temp[i].xbuf[cnt].data_pointer =
                                       toendianL((ulong)dma_addr);

				XMITQ_INC(WRK.tx2_dma_next_in);
                                WRK.tx2_dma_use_count++;

                        } else {
  				TRACE_BOTH(MPS_ERR, "Ts22", p_dev_ctl, 
						WRK.tx2_buf_use_count, 
						WRK.tx2_dma_use_count);
                                no_space = TRUE;
                                break;
                        }
                        WRK.tx2_temp[i].xbuf[cnt].buf_len =
                                                toendianW((ushort)mbufp->m_len);
			mbufp = mbufp->m_next;
                        cnt++;
                } /* end of while (mbufp) */

                if(no_space == TRUE) {
                        /*
                         * Not enough tx buffer or DMA address available for
			 * the transmit frame
                         */

			/*
			 * undo the transmit counter
			 */
                        d = cnt;
        		mbufp = WRK.tx2_queue[i].mbufp;
                        while (d != 0) {
                                if (mbufp->m_len > H_PAGE) {
					XMITQ_DEC(WRK.tx2_dma_next_in);
                                        WRK.tx2_dma_use_count--;
                                } else {
					XMITQ_DEC(WRK.tx2_buf_next_in);
                                        WRK.tx2_buf_use_count--;
                                }
                                mbufp = mbufp->m_next;
                                d--;
                        }

			/*
			 * undo the d_master
			 */
			d = WRK.tx2_dma_next_in;
        		mbufp = WRK.tx2_queue[i].mbufp;
                        while (cnt != 0) {
                                if (mbufp->m_len > H_PAGE) {
					dma_addr = WRK.tx2_dma_addr[ct] + 
					   (int)((int)mtod(mbufp,uchar*)&0xFFF);
                                        d_complete(WRK.dma_channel,
                                              	DMA_WRITE_ONLY,
                                              	mtod(mbufp, uchar *),
                                              	mbufp->m_len,
                                              	&WRK.tx2_dma_xmemp[d],
                                              	dma_addr);
					XMITQ_INC(d);
                                }

                                mbufp = mbufp->m_next,
                                cnt--;
                        }
			WRK.tx2_queue[i].mbufp->m_nextpkt = txq2_first;
                        return;
                }
                WRK.tx2_temp[i].xbuf[cnt].data_pointer =
                   	(ulong) WRK.tx2_temp[i].xbuf[cnt - 1].data_pointer;
                WRK.tx2_temp[i].xbuf[cnt].buf_len      = 0;
                WRK.tx2_temp[i].buf_count     = toendianW((ushort) (cnt + 1));
        }

        /*
         * Updates the adapter buffer descriptor
         */
        buf_count = WRK.tx2_temp[i].buf_count >> 8;
        if(WRK.iocc) {
                d_kmove(&WRK.tx2_temp[i],WRK.tx2_list[i],tx_desc_len[buf_count],
                         WRK.dma_channel, DDS.bus_id, DMA_WRITE_ONLY);
        } else {
                bcopy(&WRK.tx2_temp[i],WRK.tx2_vadr[i],tx_desc_len[buf_count]);
        }

        /*
         * Calls ndd_trace if it is enabled
         */
        if (p_ndd->ndd_trace) {
                (*(p_ndd->ndd_trace))(p_ndd, WRK.tx2_queue[i].mbufp,
                         WRK.tx2_queue[i].mbufp->m_data, p_ndd->ndd_trace_arg);
        }

        /*
         * Gives the buffer descriptor address to the adapter
         */
        PIO_PUTLRX(ioa + Tx2LFDA_L, (int)WRK.tx2_list[i]);
        WRK.tx2_frame_pending++;
        p_dev_ctl->txq2_len--;
        p_dev_ctl->txq2_first  = txq2_first;
        XMITQ_INC(WRK.tx2_elem_next_in);

  } /* end of while */
  TRACE_SYS(MPS_TX, "Ts2E", p_dev_ctl, WRK.tx2_elem_next_in, 
						WRK.tx2_elem_next_out);
}

/*****************************************************************************/
/*
 * NAME: mps_tx2_done
 *
 * FUNCTION: process a completed transmission
 *
 * EXECUTION ENVIRONMENT:
 *      Called by interrupt handler
 *
 * NOTES:
 *
 *    Input: p_dev_ctl - pointer to device control structure
 *	     ioa       - IO address 
 *           misr_reg  - MISR register 
 *
 * RETURNS: None
 *
 */
/*****************************************************************************/

int mps_tx2_done (
mps_dev_ctl_t       *p_dev_ctl,
int                  ioa,
int		     misr_reg)
{
  ulong                           status; /* status of transmit         */
  xmit_elem_t                     *xelm;  /* transmit element structure */
  volatile        tx_list_t       xmitlist;
  int                             rc, i, j, out, First, pio_rc;
  int                             ipri;
  ndd_t                           *p_ndd = (ndd_t *)&(NDD);
  int                             first, last;
  ushort                          bmctl, bctl, buf_count;
  ulong                           dbas, dba, lfda, ct;
  uchar                           free_mbuf = TRUE, halt = FALSE, *dma_addr;
  struct  	mbuf    	  *mbufp, *free_buf = NULL, *buf;

  TRACE_SYS(MPS_TX, "T2dB", p_dev_ctl, 0, 0); 
  ipri = disable_lock(PL_IMP, &TX_LOCK);

  /*
   * Stops the watchdog timer 
   */
  w_stop(&TX2WDT);
  WRK.tx2wdt_setter = INACTIVE;
  if (WRK.tx1wdt_setter == TX1) {
  	w_stop(&TX1WDT);
  	WRK.tx1wdt_setter = TX2;
  }

  while (TRUE) {
	if (!WRK.tx2_frame_pending)  {
		break;
	}

        out = WRK.tx2_elem_next_out;
        xelm = &WRK.tx2_queue[out];

        /* 
	 * Gets the TX list element from the adapter control area.     
	 */
        if (WRK.iocc) {
		d_kmove(&xmitlist, WRK.tx2_list[out],
              		TX_LIST_SIZE, WRK.dma_channel, DDS.bus_id, DMA_READ);
        } else {
                  bcopy(WRK.tx2_vadr[out], &xmitlist, TX_LIST_SIZE);
        }

        /* 
	 * Gets the status from the tx buffer descriptor and verify 
         * the status.  (The description of status field are defined
         * in Maunakea Functional Specification )
         */
        status  = toendianL(xmitlist.xmit_status);

	/*
	 * Checks the status of the frame.  If something wrong, cleanup and
         * try to retransmit that frame one more time
         */
        if (status & ERR_STATUS) { 
	/* Something went wrong with the transmit package */
 		NDD.ndd_genstats.ndd_oerrors++;
                if (status & MC_ERR) {
                        if (status & TX_UNDERRUN) { /* Tx under run */
                                  DEVSTATS.xmit_underrun++;
  			} else {

  				TRACE_BOTH(MPS_ERR,"T2d7",p_dev_ctl, status, 0);
                		/*
                 		 * Resets the channel check bit in POS register
                 		 */
                            	if (status & CHCK_ERR) { /* CHCK Error */
                          	   	PIO_GETSRX(ioa + BCtl, &bctl);
                          	   	bctl |= CHCK_BIT;
                          	   	PIO_PUTSRX( ioa + BCtl, bctl);
  			    	}

				mps_clean_dma2 (p_dev_ctl, xelm->mbufp);
  			}

  	  		/* 
			 * Begins of TX HANG hardware work around 
			 */
                        PIO_GETSRX(ioa + BMCtl_sum, &bmctl);
                        if ((status & TX_UNDERRUN)  | /* Tx under run */
  			    (bmctl & Tx2_DISABLE)   | /* Tx2 disable  */  
			    (misr_reg & Tx2_HALT))  { /* Tx2 halt     */

                                PIO_GETLRX(ioa + Tx2DBA_L, &dba);
				i = 0;
                        	do { 
					if (i++ > 20) {
						halt = TRUE;
  						TRACE_BOTH(MPS_ERR, "T2d1", 
							  p_dev_ctl, dba, dbas);
						break;
					}
					dbas = dba;
					io_delay(1000);
                                  	PIO_GETLRX(ioa + Tx2DBA_L, &dba);
                        	} while (dbas != dba);

				PIO_GETLRX(ioa + Tx2LFDA_L, &lfda);
				PIO_GETSRX(ioa + BMCtl_sum, &bmctl);
				i = 0;
				while (!(bmctl & Tx2_DISABLE)) {
					if (i++ > 20) {
						TRACE_BOTH(MPS_ERR, "T2d9", 
							  p_dev_ctl, bmctl, 
							  misr_reg);
						halt = TRUE;
						break;
					}

				        PIO_PUTLRX(ioa + Tx2FDA_L, lfda);
				        PIO_PUTLRX(ioa + Tx2LFDA_L, lfda);
					io_delay(1000);
				       	PIO_GETSRX(ioa + BMCtl_sum, &bmctl);
				};

  		  		PIO_PUTSRX( ioa + MISR, ~Tx2_HALT);
			}
  			/* end of hardware work around */

  		} else if (status & TX_PROTOCOL_ERR) {

  			/* requests Tx2 channel disable */
  			PIO_PUTSRX(ioa + BMCtl_sum, Tx2_DISABLE_R);
			i = 0;
			do {
				if (i++ > 20) {
  					TRACE_BOTH(MPS_ERR, "T2d3", 
						  p_dev_ctl, bmctl, 0);
					halt = TRUE;
					break;
				}

				io_delay(1000);
                        	PIO_GETSRX(ioa + BMCtl_sum, &bmctl);
  			} while (!(bmctl & Tx2_DISABLE));
  		}

		if (halt == TRUE)  {
			/*
			 * something went wrong in TX HANG hardware work around
			 */
  			TRACE_BOTH(MPS_ERR, "T2d8", p_dev_ctl, status, bmctl);
                        mps_logerr(p_dev_ctl, ERRID_MPS_ADAP_CHECK, __LINE__,
                                   __FILE__, NDD_ADAP_CHECK, status, bmctl); 
  			unlock_enable(ipri, &TX_LOCK);
			enter_limbo(p_dev_ctl, TRUE, TRUE, 10, NDD_ADAP_CHECK, 
								status, bmctl);
			return(0);
		}

  		PIO_PUTSRX(ioa + BMCtl_rum, ~Tx2_DISABLE);
		if (WRK.pio_rc)  {
  			TRACE_BOTH(MPS_ERR,"T2d4", p_dev_ctl, WRK.pio_rc,bmctl);
        		mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 
					0, TRUE, TRUE, FALSE);
  			unlock_enable(ipri, &TX_LOCK);
			return(0);
		}


                /* 
		 * Gets the list of packets to retransmit              
		 */
                first = WRK.tx2_elem_next_out;
                last  = WRK.tx2_elem_next_in;

                /* 
		 *Skips retransmit the first packet if it has been retransmitted
                 */
                if (first == WRK.tx2_retry) {
                        XMITQ_INC(first);
                        WRK.tx2_retry = MAX_TX_LIST;
                } else {
                        free_mbuf = FALSE;
  		}

  		/* 
		 * Checks if any packet need to be retransmit 
		 */
                if(first != last) {
                        WRK.tx2_retry = first;
                        i = first;
			j = 0;
                        do { /* Retransmit the packets */
                                /* 
				 * Moves the TX list element into the
                                 * adapter control area
                                 */
	  			buf_count = WRK.tx2_temp[i].buf_count >> 8;
  				if(WRK.iocc) {
					d_kmove(&WRK.tx2_temp[i], 
						 WRK.tx2_list[i], 
	  					 tx_desc_len[buf_count],
						 WRK.dma_channel, 
					         DDS.bus_id, 
						 DMA_WRITE_ONLY);
  				} else {
  					bcopy(&WRK.tx2_temp[i], WRK.tx2_vadr[i],
	  					 tx_desc_len[buf_count]);
  				}
                        	/* 
				 * Gives the buffer descriptor address to the 
				 * adapter 
  			 	 */
                          	PIO_PUTLRX(ioa + Tx2LFDA_L, WRK.tx2_list[i]);
				PIO_GETSRX(ioa + BMCtl_sum, &bmctl);
				if (bmctl & Tx2_DISABLE) {
					PIO_PUTSRX(ioa+BMCtl_rum, ~Tx2_DISABLE);
                        		i = first;
                			TRACE_BOTH(MPS_TX, "T2d5", p_dev_ctl, 
							status, j);
					if (j++ > 5) {
						break;
					}
				} else {
                                	XMITQ_INC(i);
				}
                        } while (i != last);
                }

		halt = TRUE;
                TRACE_SYS(MPS_TX, "T2d6",p_dev_ctl, status, 0);

  	/* 
	 * The transmit frame was processed 
	 */ 
        } else if (status & BUF_PROCESS) {

        	/* For netpmon performance tool */
        	TRACE_SYS(MPS_TX, TRC_WEND, p_dev_ctl->seq_number,
                	(int)p_dev_ctl->txq2_first, WRK.tx2_frame_pending); 

		/*
                 * Updates the standard counter 
		 */
                if (NDD.ndd_genstats.ndd_opackets_lsw == ULONG_MAX) {
                        NDD.ndd_genstats.ndd_opackets_msw++;
		}
                NDD.ndd_genstats.ndd_opackets_lsw++;

                if ((ULONG_MAX-xelm->bytes) <
                        NDD.ndd_genstats.ndd_obytes_lsw) {
                        NDD.ndd_genstats.ndd_obytes_msw++;
		}
                NDD.ndd_genstats.ndd_obytes_lsw += xelm->bytes;

                /*
                 * Determines if it's a broadcast or multicast packet
                 */
  		if (xelm->mbufp->m_flags & M_MCAST)  {
                  	TOKSTATS.mcast_xmit++;
		}

  		if (xelm->mbufp->m_flags & M_BCAST) {
                  	TOKSTATS.bcast_xmit++;
		}

        } else {
                break; /* No final status at all */
        }

	if (free_mbuf != TRUE) {
		break;
	}

	/*
	 * Based on the length of packet or length of each mbuf to update
         * the transmit counter.
         * For packet_len/mbuf_len less than 2K bytes, update the 
         * WRK.tx2_buf_use_count.  Otherwise update the WRK.tx2.dma_use_count
         */
	mbufp = xelm->mbufp;
	if (mbufp->m_pkthdr.len > H_PAGE) {
		while (mbufp) {
			if (mbufp->m_len > H_PAGE) {
				ct = WRK.tx2_dma_next_out;
				dma_addr = WRK.tx2_dma_addr[ct] +
				       (int)((int)mtod(mbufp, uchar *) & 0xFFF);
				rc = d_complete(WRK.dma_channel, 
					      	DMA_WRITE_ONLY,
			      			mtod(mbufp, uchar *), 
			      			mbufp->m_len,
						&WRK.tx2_dma_xmemp[ct],
						dma_addr);
       				if (rc != DMA_SUCC) {
        				mps_logerr(p_dev_ctl, ERRID_MPS_DMAFAIL,
				   		   __LINE__, __FILE__, 
			   	   		  WRK.dma_channel, 
				   		  dma_addr, rc);
      				}

				XMITQ_INC(WRK.tx2_dma_next_out);
				WRK.tx2_dma_use_count--;
			} else {
				XMITQ_INC(WRK.tx2_buf_next_out);
				WRK.tx2_buf_use_count--;
			}
			mbufp = mbufp->m_next;
		}
	} else {
		XMITQ_INC(WRK.tx2_buf_next_out);
		WRK.tx2_buf_use_count--;
	}

	/*
         * Marks transmit element as free 
	 */
	buf = xelm->mbufp;
	while(buf->m_next != NULL) {
		buf = buf->m_next;
	}
	buf->m_next = free_buf;
	free_buf = xelm->mbufp;

        WRK.tx2_frame_pending--;
        XMITQ_INC(WRK.tx2_elem_next_out);

	if (halt == TRUE) {
		break;
	}

  } /* end of while */

  /* 
   * Checks if any package is pending in Tx queue, if they are then try
   * to transmit them
   */
  mps_send2(p_dev_ctl, ioa);

  /* 
   * Checks if need to start the watch dog timer
   */
  if (WRK.tx2_frame_pending) {
       	WRK.tx2wdt_setter = TX2;
       	w_start (&(TX2WDT));
  }

  if (WRK.tx1wdt_setter == TX2) {
       	WRK.tx1wdt_setter = TX1;
       	w_start (&(TX1WDT));
  }

  unlock_enable(ipri, &TX_LOCK);

  if (free_buf) {
        m_freem(free_buf);
  }

  TRACE_SYS(MPS_TX, "T2dE", p_dev_ctl, 0, 0);
  return(0);
}

/*****************************************************************************/
/*
 * NAME: mps_output
 *
 * FUNCTION: write function for kernel
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
 *      p_mbuf          - pointer to a mbuf (chain) for outgoing packets
 *
 * RETURNS:
 *      0 - successful
 *      EAGAIN - transmit queue is full
 *      ENETUNREACH - device is currently unreachable
 *      ENETDOWN - device is down
 */
/*****************************************************************************/
int mps_output ( ndd_t          *p_ndd,
  		 struct mbuf    *p_mbuf)
{
  mps_dev_ctl_t   	*p_dev_ctl = (mps_dev_ctl_t *)(p_ndd->ndd_correlator);
  int             	i,rc,ioa, pio_rc;
  int             	count = 0;
  int             	room = 0;
  struct  mbuf    	*buf_to_free, *buf_to_count, *free_buf;
  struct  mbuf    	*mbufp;
  struct  mbuf    	*p_last;
  int 			ipri;
  ushort	  	buf_count;

  TRACE_SYS(MPS_TX, "Tx2B", (ulong)p_dev_ctl,(ulong)p_mbuf,p_dev_ctl->txq2_len);

  /*
   * Determines if it's a broadcast or multicast packet and update the MIB
   * counter : ifHCOutUcastPkts, ifHCOutMulticastPkts, ifHCOutBroadcastPkts.
   */

  buf_to_count = p_mbuf;
  while (buf_to_count) {
        if (buf_to_count->m_flags & M_MCAST)  {
                if (NDD.ndd_genstats.ndd_ifOutMcastPkts_lsw == ULONG_MAX) {
                        NDD.ndd_genstats.ndd_ifOutMcastPkts_msw++;
                }
                NDD.ndd_genstats.ndd_ifOutMcastPkts_lsw++;
        } else if (buf_to_count->m_flags & M_BCAST) {
                if (NDD.ndd_genstats.ndd_ifOutBcastPkts_lsw == ULONG_MAX) {
                        NDD.ndd_genstats.ndd_ifOutBcastPkts_msw++;
                }
                NDD.ndd_genstats.ndd_ifOutBcastPkts_lsw++;
        } else {
                if (NDD.ndd_genstats.ndd_ifOutUcastPkts_lsw == ULONG_MAX) {
                        NDD.ndd_genstats.ndd_ifOutUcastPkts_msw++;
                }
                NDD.ndd_genstats.ndd_ifOutUcastPkts_lsw++;
        }
        buf_to_count = buf_to_count->m_nextpkt;
  }

  ipri = disable_lock(PL_IMP, &TX_LOCK);

  if (p_dev_ctl->device_state != OPENED) {
  	unlock_enable(ipri, &TX_LOCK);
  	if (p_dev_ctl->device_state == DEAD) {
  		TRACE_BOTH(MPS_ERR, "tx21", ENETDOWN, 0, 0);
  		return(ENETDOWN);
  	} else {
  		TRACE_BOTH(MPS_ERR, "tx22", ENETUNREACH, 0, 0);
  		return(ENETUNREACH);
  	}
  }

  /*
   *  If the txq is full, return EAGAIN. Otherwise, queue as
   *  many packets onto the transmit queue and free the
   *  rest of the packets, return no error.
   */
  if (p_dev_ctl->txq2_len >= DDS.xmt_que_size) {

  	unlock_enable(ipri, &TX_LOCK);
        buf_to_free = p_mbuf;
        while (buf_to_free) {
  		NDD.ndd_genstats.ndd_xmitque_ovf++;
                buf_to_free = buf_to_free->m_nextpkt;
        }
  	TRACE_BOTH(MPS_ERR,"Tx23",p_dev_ctl,NDD.ndd_genstats.ndd_xmitque_ovf,0);
  	return(EAGAIN);
  } else {
  	room = DDS.xmt_que_size - p_dev_ctl->txq2_len;

  	buf_to_free = p_mbuf;
  	while (room && buf_to_free) {
  		p_last = buf_to_free;
  		p_dev_ctl->txq2_len++;
  		room--;
  		buf_to_free = buf_to_free->m_nextpkt;

        	/* For netpmon performance tool */
        	TRACE_SYS(MPS_TX, TRC_WQUE, p_dev_ctl->seq_number, 
				(int)buf_to_free, room); 
  	}

  	if (p_dev_ctl->txq2_first) {
  		p_dev_ctl->txq2_last->m_nextpkt = p_mbuf;
  	} else {
  		p_dev_ctl->txq2_first = p_mbuf;
        }
	
  	p_dev_ctl->txq2_last = p_last;
  	p_dev_ctl->txq2_last->m_nextpkt = NULL;
  }

  ioa = (int)BUSIO_ATT( DDS.bus_id, DDS.io_base_addr);
  mps_send2(p_dev_ctl, ioa);
  BUSIO_DET(ioa);                      /* restore I/O Bus      */

  /*
   * p_dev_ctl->txq2_len == # of packets in software queue 
   * WRK.tx2_frame_pending == # of packets in hardware queue
   */
  if ((p_dev_ctl->txq2_len + WRK.tx2_frame_pending) >
                    NDD.ndd_genstats.ndd_xmitque_max) {
      NDD.ndd_genstats.ndd_xmitque_max =
              p_dev_ctl->txq2_len + WRK.tx2_frame_pending;
  }

  /* 
   * At least one tx buffer is waiting for transfer 
   */
  if ((WRK.tx2_frame_pending) && (WRK.tx2wdt_setter != TX2)) {
  	/* start the watch dog timer */
        WRK.tx2wdt_setter = TX2;
  	w_start (&(TX2WDT));
  }
  unlock_enable(ipri, &TX_LOCK);

  if (buf_to_free) {
  	free_buf = NULL;
  	while (buf_to_free)
  	{
  		NDD.ndd_genstats.ndd_xmitque_ovf++;
        	p_ndd->ndd_genstats.ndd_opackets_drop++;
  		mbufp = buf_to_free;
  		while(mbufp->m_next != NULL) {
			mbufp = mbufp->m_next;
  		}
  		mbufp->m_next = free_buf;
  		free_buf = buf_to_free;

  		buf_to_free = buf_to_free->m_nextpkt;
  	}
  	m_freem(free_buf);
  }

  TRACE_SYS(MPS_TX, "Tx2E", p_dev_ctl, count, 0);
  return(0);
}

