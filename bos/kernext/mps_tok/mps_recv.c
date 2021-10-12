static char sccsid[] = "@(#)72    1.16  src/bos/kernext/mps_tok/mps_recv.c, sysxmps, bos41J, 9520B_all 5/18/95 11:24:48";
/*
 *   COMPONENT_NAME: sysxmps
 *
 *   FUNCTIONS: TRACE_DBG
 *		arm_recv_list
 *		discard_packet
 *		mps_recv
 *		one_buf_undo
 *		recv_buf_setup
 *		recv_buf_undo
 *		recv_cleanup
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
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/sleep.h>
#include <sys/trchkid.h>
#include <sys/err_rec.h>
#include <sys/mbuf.h>
#include <sys/dump.h>
#include <sys/ndd.h>
#include <sys/cdli.h>

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

/*****************************************************************************/
/* 
* NAME: recv_buf_setup                                                          
*                                                                           
* FUNCTION: Initialize the receive list buffer descriptor indexes.          
*                                                                           
* EXECUTION ENVIRONMENT:                                                    
*                                                                           
*      This routine runs only under the process thread.                     
*                                                                           
* NOTES:                                                                    
*                                                                           
*    Input: 
*       p_dev_ctl       - point to the device control area
*                                                                           
*    Output: 
*  receive list variables initialized.                            
*                                                                        
*    Called From: 
*  srb_response                                                 
*                                                                          
* RETURN:  0 - Successful completion                                        
*          ENOBUFS - No Bus Address space available                         
*/
/*****************************************************************************/
int  recv_buf_setup (
mps_dev_ctl_t   *p_dev_ctl,  /* pointer to the device control area */
int		resetup)

{
  volatile rx_list_t    recvlist;
  uchar   		*data_p;
  uchar   		*addr_p;
  caddr_t 		busmem;
  int			rc, ioa;
  int			i, j;        /* Loop Counter                        */

  TRACE_SYS(MPS_RV, "RstB", p_dev_ctl, 0, 0);

  WRK.read_index = 0;

  if (!resetup) {
  	/*
   	 * Creates the receive chain by initializing the pointer to each      
   	 * receive list.  Create both DMA and virtual address lists.  
  	 * Allocated 256 bytes for each Rx decriptor.       
   	 */
  	WRK.recv_list[0] = (rx_list_t * )((int)DDS.dma_base_addr);
  	WRK.recv_vadr[0] = (rx_list_t * )((int)WRK.rx_p_mem_block);
  	for (i = 1; i < MAX_RX_LIST; i++) {
  		WRK.recv_list[i] = (rx_list_t *)((int)WRK.recv_list[i - 1]+256);
  		WRK.recv_vadr[i] = (rx_list_t *)((int)WRK.recv_vadr[i - 1]+256);
  	}
  }

  /* 
   * Initializes the dma & mbuf pointers arrays                         
   */
  addr_p = (uchar *)(DDS.dma_base_addr + RX_BUF_OFFSET);
  for (i = 0; i < MAX_RX_LIST; i++) {

  	if (!resetup) {
  		/* 
		 * Gets page size chunk of memory 
		 */
  		if ((WRK.recv_mbuf[i] = m_getclust(M_WAIT, MT_HEADER)) == 
								NULL) {
  			NDD.ndd_genstats.ndd_nobufs++;
  			recv_buf_undo (p_dev_ctl, i);
  			TRACE_BOTH(MPS_ERR, "Rst1", p_dev_ctl, i, 0);
  			return (ENOBUFS);
  		}

  		WRK.recv_addr[i]      = addr_p;                /* DMA  addr  */
  		addr_p += (int)PAGESIZE;
	} else {
        	rc = d_complete (WRK.dma_channel, DMA_READ | DMA_NOHIDE,
                        MTOD(WRK.recv_mbuf[i], uchar * ),
                        WRK.recv_mbuf[i]->m_len, &WRK.rx_xmemp[i],
                        WRK.recv_addr[i]);
        	if (rc != DMA_SUCC) {
                	mps_logerr(p_dev_ctl, ERRID_MPS_DMAFAIL, __LINE__, 
					__FILE__, WRK.dma_channel, 
					WRK.recv_mbuf[i], rc);
        	}


        }

  } /* end of for */

  for (i = 0; i < MAX_RX_LIST; i++) {
  	WRK.recv_mbuf[i]->m_len = PAGESIZE;
  	/* 
	 * Set up cross memory descriptor                      
         */
  	WRK.rx_xmemp[i].aspace_id  = XMEM_GLOBAL;
  	WRK.rx_xmemp[i].subspace_id = NULL;
  	/* 
	 * Set up the DMA channel for block mode DMA transfer       
	 */
  	d_master(WRK.dma_channel, DMA_READ | DMA_NOHIDE,  
  	    	 		mtod(WRK.recv_mbuf[i], uchar * ), 
  	    	 		WRK.recv_mbuf[i]->m_len, &WRK.rx_xmemp[i], 
				WRK.recv_addr[i]);
  	/* 
	 * Set up buffer table entry                                 
  	 */
	if (WRK.iocc) {
  		recvlist.fw_pointer   = toendianL((ulong)WRK.recv_list[i]);
  		recvlist.recv_status  = 0;
  		recvlist.data_pointer = toendianL((ulong)WRK.recv_addr[i]);
  		recvlist.data_len  = toendianW((ushort)WRK.recv_mbuf[i]->m_len);
  		recvlist.fr_len       = 0;

  		/*  
	 	 * Updates the receive dma image of the list by d_moving 
  	 	 * it through the IOCC cache into system memory.               
	 	 */
  		rc = d_kmove (&recvlist, WRK.recv_list[i], (uint)RX_LIST_SIZE, 
  	    			WRK.dma_channel, DDS.bus_id, DMA_WRITE_ONLY);

		/*
  	 	 * IOCC is NOT buffered.  We are running at a cach consistent
		 * machine, set WRK.iocc = FALSE.
  	 	 */
  		if (rc == EINVAL) {
  			bcopy(&recvlist, WRK.recv_vadr[i], (uint)RX_LIST_SIZE);
			WRK.iocc = FALSE;
  		}
	} else {
  		WRK.recv_vadr[i]->fw_pointer   = toendianL((ulong)
							WRK.recv_list[i]);
  		WRK.recv_vadr[i]->recv_status  = 0;
  		WRK.recv_vadr[i]->data_pointer = toendianL((ulong)
							WRK.recv_addr[i]);
  		WRK.recv_vadr[i]->data_len     = toendianW((ushort)
						       WRK.recv_mbuf[i]->m_len);
  		WRK.recv_vadr[i]->fr_len       = 0;
        }
  } /* end of for */

  /* 
   * Gives the buffer descriptor address to adapter                     
   */
  ioa = (int)BUSIO_ATT( DDS.bus_id, DDS.io_base_addr);
  PIO_PUTLRX(ioa + RxBDA_L, WRK.recv_list[0]);
  if (WRK.pio_rc) {
  	BUSIO_DET(ioa);                /* restore I/O Bus              */
        mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL,0,FALSE,FALSE,FALSE);
  	recv_buf_undo (p_dev_ctl, MAX_RX_LIST);
  	TRACE_BOTH(MPS_ERR, "Rst2", p_dev_ctl, WRK.pio_rc, 0);
	return (ENETDOWN);
  }
  BUSIO_DET(ioa);                      /* restore I/O Bus              */

  TRACE_SYS(MPS_RV, "RstE", p_dev_ctl, WRK.recv_mbuf[0], 0);
  return(0);
}


/*****************************************************************************/
/*                                                                           
* NAME: one_buf_undo                                                       
*
* FUNCTION:  Cleanup the first buffer in the Rx list which has been used by the 
*	     adapter for doing the adapter self test during initialize process.
* 	     And re_initialize the receive list buffer descriptor indexes.          
*
* EXECUTION ENVIRONMENT: process & interrupt 
*
* NOTES:
*    Input: 
*  p_dev_ctl - pointer to device control structure.
*
*    Called From:
*               mps_mps_intr
*               mps_setup
*               mps_act
*
*    Calls to:
*               arm_recv_list
*/
/*****************************************************************************/
one_buf_undo ( 
mps_dev_ctl_t       *p_dev_ctl)  /* pointer to the device control area */
{
  uint                  rc, i;
  volatile rx_list_t   	recvlist;
  uchar   		*addr_p;

  TRACE_SYS(MPS_RV, "RosB", p_dev_ctl, 0, 0);

  rc = d_complete (WRK.dma_channel, DMA_READ | DMA_NOHIDE, 
      		   MTOD(WRK.recv_mbuf[0], uchar * ),  
      		   WRK.recv_mbuf[0]->m_len, &WRK.rx_xmemp[0],
      		   WRK.recv_addr[0]);
  if (rc != DMA_SUCC) {
  	mps_logerr(p_dev_ctl, ERRID_MPS_DMAFAIL, __LINE__, __FILE__,
        		WRK.dma_channel, WRK.recv_mbuf[0], rc);
  }

  /* 
   * Initializes the dma & mbuf pointers arrays                         
   */
  addr_p = (uchar *)(DDS.dma_base_addr + RX_BUF_OFFSET);
  for (i = 0; i < MAX_RX_LIST; i++) {
  	/* 
	 * Set up buffer table entry                                 
  	 */
	if (WRK.iocc) {
  		recvlist.fw_pointer   = toendianL((ulong)WRK.recv_list[(i+1) % 
                                        MAX_RX_LIST]);
  		recvlist.recv_status  = 0;
  		recvlist.data_pointer = toendianL((ulong)addr_p);
  		recvlist.data_len  = toendianW((ushort)WRK.recv_mbuf[i]->m_len);
  		recvlist.fr_len       = 0;

  		/*  
	 	 * Updates the receive dma image of the list by d_moving 
  	 	 * it through the IOCC cache into system memory.               
	 	 */
  		d_kmove (&recvlist, WRK.recv_list[i], (uint)RX_LIST_SIZE, 
  	    			WRK.dma_channel, DDS.bus_id, DMA_WRITE_ONLY);

	} else { /* IOCC is NOT buffered */                                     
  		WRK.recv_vadr[i]->fw_pointer   = toendianL((ulong)
							WRK.recv_list[(i+1) % 
							MAX_RX_LIST]);
  		WRK.recv_vadr[i]->recv_status  = 0;
  		WRK.recv_vadr[i]->data_pointer = toendianL((ulong)addr_p);
  		WRK.recv_vadr[i]->data_len     = toendianW((ushort)
						       WRK.recv_mbuf[i]->m_len);
  		WRK.recv_vadr[i]->fr_len       = 0;
        }

  	addr_p += (int)PAGESIZE;
  } /* end of for */
  TRACE_SYS(MPS_RV, "RosE", p_dev_ctl, 0, 0);

} /* end one_buf_undo                                                       */


/*****************************************************************************
*                                                                           
* NAME: recv_buf_undo                                                       
*                                                                           
* FUNCTION: Undoes the effects of MPS_RECV_SETUP.        
*           Frees the mbufs allocated & frees the region.                   
*                                                                           
* EXECUTION ENVIRONMENT:                                                    
*                                                                           
*      This routine runs under both the process thread and the off level    
*      interrupt handler.                                                   
*                                                                           
* NOTES:                                                                    
*                                                                           
*    Input: 
*  p_dev_ctl - pointer to device control structure.
*                                                                           
*    Called From:
*               mps_setup
*                                                                           
*****************************************************************************/
recv_buf_undo ( 
mps_dev_ctl_t       *p_dev_ctl,  /* pointer to the device control area */
int                  num_entry)
{
  struct mbuf 		*m;
  volatile rx_list_t    recvlist;
  register int		x, i;
  uint                  rc;

  TRACE_SYS(MPS_RV, "RouB", p_dev_ctl, 0, 0);
  /* 
   *  Updates the receive dma image of the list by d_moving  
   *  it through the IOCC cache into system memory.               
   */
  WRK.read_index = 0;
  bzero(&recvlist, RX_LIST_SIZE);              /* clear receive list   */

  /*
   * Frees up the DMA buffer resources
   */
  for (i = 0; i < num_entry; i++) {
	if (WRK.iocc) {
  		d_kmove (&recvlist, WRK.recv_list[i], (uint)RX_LIST_SIZE, 
  	    			WRK.dma_channel, DDS.bus_id, DMA_WRITE_ONLY);

	} else { /* IOCC is NOT buffered */                                     
  		bcopy(&recvlist, WRK.recv_vadr[i], (uint)RX_LIST_SIZE);
        }

  	/*  
	 *  If an mbuf is set up for this receive list, call         
  	 *  d_complete to "un_dma" it, then return it.              
	 */
  	if (m = WRK.recv_mbuf[i]) {
  		rc = d_complete (WRK.dma_channel, DMA_READ, 
  		    MTOD(m, uchar * ), PAGESIZE, 
  		    &WRK.rx_xmemp[i], WRK.recv_addr[i]);
        	if (rc != DMA_SUCC) {
                	mps_logerr(p_dev_ctl, ERRID_MPS_DMAFAIL, __LINE__, 
				   __FILE__, WRK.dma_channel, m, rc);
        	}


  		m_freem(m);
  		WRK.recv_mbuf[i] = NULL;
  	}
  } /* end of for */

  TRACE_SYS(MPS_RV, "RouE", p_dev_ctl, WRK.recv_mbuf[0], 0);

} /* end recv_buf_undo                                                       */


/*****************************************************************************
*                                                                           
* NAME: recv_cleanup                                                       
*                                                                           
* FUNCTION: Undoes the effects of MPS_RECV_SETUP.        
*           Frees the mbufs allocated & frees the region.                   
*                                                                           
* EXECUTION ENVIRONMENT:                                                    
*                                                                           
*      This routine runs under both the process thread and the off level    
*      interrupt handler.                                                   
*                                                                           
* NOTES:                                                                    
*                                                                           
*    Input: 
*  p_dev_ctl - pointer to device control structure.
*                                                                           
*    Called From:
*               mps_setup
*                                                                           
*****************************************************************************/
recv_cleanup ( 
mps_dev_ctl_t       *p_dev_ctl,  /* pointer to the device control area */
int                  num_entry)
{
  struct mbuf 		*m;
  volatile rx_list_t    recvlist;
  register int		x, i;
  uint                  rc;

  TRACE_SYS(MPS_RV, "RrcB", p_dev_ctl, 0, 0);

  recv_buf_undo(p_dev_ctl, MAX_RX_LIST);

  rc = d_complete(WRK.dma_channel, DMA_READ | DMA_NOHIDE, WRK.rx_p_mem_block, 
      	 RX_DESCRIPTOR_SIZE, &(WRK.rx_mem_block), (uchar *)DDS.dma_base_addr);
  if (rc != DMA_SUCC) {
       	mps_logerr(p_dev_ctl, ERRID_MPS_DMAFAIL, __LINE__, 
		   __FILE__, WRK.dma_channel, WRK.rx_p_mem_block, rc);
  }

  /* 
   * Undo adapter control area                                          
   */
  xmfree(WRK.rx_p_mem_block, pinned_heap);
  TRACE_SYS(MPS_RV, "RrcE", p_dev_ctl, WRK.recv_mbuf[0], 0);

} /* end recv_cleanup                                                       */


/*****************************************************************************/
/*                                                                           
* NAME: mps_recv                                                            
*
* FUNCTION:  Receive the packets from the adapter. 
*
* EXECUTION ENVIRONMENT: interrupt only
*
* NOTES:
*    Input: 
*  	p_dev_ctl - pointer to device control structure.
*       cmd_reg   - value in the status register.
*	ioa	  - BUSIO_ATT return value
*
*    Called From:
*               mps_intr
*
*    Calls to:
*               arm_recv_list
*/
/*****************************************************************************/
void mps_recv ( 
mps_dev_ctl_t *p_dev_ctl,     /* point to the device control area */
uchar         cmd_reg,        /* value in the status register    */
int	      ioa)
{
  volatile rx_list_t    recvlist;
  struct mbuf 		*recv_mbuf, *new_mbuf, *p_mbuf;
  int			rc, i, len;
  ushort  		bctl;
  ndd_statblk_t  	stat_blk;   /* status block */
  ndd_t   		*p_ndd = (ndd_t *)&(NDD);
  uchar bcast1_addr[CTOK_NADR_LENGTH] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
  uchar bcast2_addr[CTOK_NADR_LENGTH] = { 0xC0,0x00,0xFF,0xFF,0xFF,0xFF};
  int	bda;

  TRACE_SYS(MPS_RV, "RrvB", (ulong)p_dev_ctl, (ulong)cmd_reg, 0);

  while (TRUE) {
  	i = WRK.read_index;               /* start at next recv list */

  	/*  
	 *  D_move the receive list image from the IOCC cache to main 
  	 *  memory (local image) pointed to by rlptr. 
         */
	if (WRK.iocc) {
  		d_kmove (&recvlist, WRK.recv_list[i], (uint)RX_LIST_SIZE, 
      	    			WRK.dma_channel, DDS.bus_id, DMA_READ);

	} else { /* IOCC is NOT buffered */                                     
  		bcopy(WRK.recv_vadr[i], &recvlist, (uint)RX_LIST_SIZE);
        }

  	recvlist.recv_status  = toendianL(recvlist.recv_status);

  	/* 
	 * Checks if any more buffer is being process
	 */
  	if (!(recvlist.recv_status & BUF_PROCESS)) {
  		break;
        }

        /* 
         * The hardware have problem to update the Received Address Descriptor
         * This is the code path for that problem (see appendix B in
         * Maunakea Functional Specification)
         */ 
        PIO_GETLRX(ioa + RxBDA_L, &bda);
        if (WRK.pio_rc) {
                TRACE_BOTH(MPS_ERR, "Srw8",p_dev_ctl, WRK.pio_rc, 0);
                mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0,
                                                        TRUE, 0, 0);
                break;
        }
        if (bda == (int)WRK.recv_list[WRK.read_index]) {
                  break;
        }

  	recv_mbuf = WRK.recv_mbuf[i];
  	recv_mbuf->m_len = toendianW(recvlist.data_len); /* buffer len */
	WRK.t_len += recv_mbuf->m_len;

	/*
  	 * Checks the status of the receive buffer 
	 */
        if (recvlist.recv_status & RX_ERR) {
	 	/*
  		 * Bad frame with micro channel side error 
		 */

		/*
  		 * Resets the channel check bit in POS register 
		 */
        	if (recvlist.recv_status & CHCK_ERR) {
                	/*
                	 * Resets the channel check bit in POS register
                	 */
                          PIO_GETSRX(ioa + BCtl, &bctl);
                          bctl |= CHCK_BIT;
                          PIO_PUTSRX( ioa + BCtl, bctl);
  			  if (WRK.pio_rc) {
        			mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, 
					  NDD_PIO_FAIL, 0, FALSE, FALSE, FALSE);
  			       TRACE_BOTH(MPS_ERR,"Rst2",p_dev_ctl,WRK.pio_rc,0);
  			  }
  		}

  		TRACE_DBG(MPS_RV, "Rrv1", p_dev_ctl, recvlist.recv_status, 0);
  		rc =  d_complete(WRK.dma_channel, DMA_READ | DMA_NOHIDE,
  	    			 MTOD( recv_mbuf, uchar *), recv_mbuf->m_len, 
  	    			 &WRK.rx_xmemp[i], WRK.recv_addr[i]);
  		if (rc != DMA_SUCC) {
       			mps_logerr(p_dev_ctl, ERRID_MPS_DMAFAIL, __LINE__, 
		   		   __FILE__, WRK.dma_channel, recv_mbuf, rc);
  		}

                DEVSTATS.rx_frame_err++;
  		if (discard_packet(p_dev_ctl)) {
			break;
		}

        } else if ((((recvlist.recv_status & PROTOCOL_ERR1) &&
  		    (!WRK.promiscuous_count))) |
        	  (((recvlist.recv_status & PROTOCOL_ERR2) &&
  		    (WRK.promiscuous_count)))) {
  		/* 
		 *  bad frame with receive overrun/Protocol chip error 
  		 *  with promiscuous mode off/promiscuous mode on
  		 */
  		TRACE_BOTH(MPS_ERR, "Rrv3", p_dev_ctl, recvlist.recv_status, 0);
                if (recvlist.recv_status & RECV_OVERRUN)  {
  			DEVSTATS.recv_overrun++;
		} else {
                	DEVSTATS.rx_frame_err++;
		}
  		if (discard_packet(p_dev_ctl)) {
			break;
		}

  	} else { /* good frame receive */

                /* For netpmon performance tool */
                TRACE_SYS(MPS_RV, TRC_RDAT, p_dev_ctl->seq_number, 
				WRK.read_index, recv_mbuf->m_len);


  		/* Gets the mbuf for receive frame */
  		if (recv_mbuf->m_len <= (MHLEN)) {
  			new_mbuf = m_gethdr(M_DONTWAIT, MT_HEADER);
  		} else { 
  			new_mbuf = m_getclust(M_DONTWAIT, MT_HEADER);
		}

  		if (new_mbuf == NULL) {
  			NDD.ndd_genstats.ndd_nobufs++;
  			NDD.ndd_genstats.ndd_ipackets_drop++;
  			discard_packet(p_dev_ctl);
			WRK.t_len = 0;
  		} else {

			if (recv_mbuf->m_len < PAGESIZE/2) {
  				/*
			 	 * Copys data from d_master'ed mbuf to the new 
				 * one
			 	 */
  				bcopy(mtod(recv_mbuf, caddr_t), 
				      mtod(new_mbuf, caddr_t),
				      recv_mbuf->m_len);

  				new_mbuf->m_len = recv_mbuf->m_len;
  				cache_inval(mtod(recv_mbuf, caddr_t), 
						 recv_mbuf->m_len);

  				/* 
			 	 * Re-arm the receive list and pass the existing
				 * mbuf to be used again to avoid doing another
				 * d_master.  
                         	 */
  				if (arm_recv_list(p_dev_ctl, i, 
						      FALSE, recv_mbuf)) {
					break;
				}
			} else {
  				if (arm_recv_list(p_dev_ctl, i, 
							TRUE, new_mbuf)) {
					break;
				}
				new_mbuf = recv_mbuf;
			}

  			/*  
			 *  See if we are at the beginning of a new frame;  
			 *  if so, begin a new linked list of mbufs. If we 
			 *  are not at the start of a frame, simply add this 
			 *  mbuf to the end of the current list.    
                   	 */
  			if (WRK.mhead == NULL) {
  				WRK.mhead = WRK.mtail = p_mbuf = new_mbuf;
  				WRK.mhead->m_flags |= M_PKTHDR;
  				WRK.mhead->m_nextpkt = NULL;

  				/* Checks if broadcast or multicast */
  				if (*(mtod(new_mbuf, caddr_t) + 2) & 
					MULTI_BIT_MASK) {
  				      if((SAME_NADR((mtod(new_mbuf,caddr_t)+2),
  						bcast1_addr)) |
  				    	 (SAME_NADR((mtod(new_mbuf,caddr_t)+2), 
						bcast2_addr)))
  					{
  						TOKSTATS.bcast_recv++;
  						new_mbuf->m_flags |= M_BCAST;
  					} else {
  						TOKSTATS.mcast_recv++;
  						new_mbuf->m_flags |= M_MCAST;
  					}
  				}
  			} else {
				p_mbuf = WRK.mtail;
  				WRK.mtail->m_next = new_mbuf;
  				WRK.mtail = new_mbuf;
  			}

  		/*  
		 *  If this is the end of the frame, submit this list 
                 *  of mbufs to the receive handler.   
                 */
  		if (recvlist.recv_status & END_OF_FRAME) {

			/* removes the 4 bytes hardware CRC */
			if (WRK.mtail->m_len > 4) {
  				WRK.mtail->m_len -= 4;
			} else {
				len = WRK.mtail->m_len - 4;
				WRK.mtail = p_mbuf;	
  				WRK.mtail->m_next = NULL;
				WRK.mtail->m_len += len;
				m_freem(new_mbuf);
			}
  			WRK.mhead->m_pkthdr.len = toendianW(recvlist.fr_len)-4;

			if (WRK.mhead->m_pkthdr.len != (WRK.t_len - 4)) {
  				NDD.ndd_genstats.ndd_ierrors++;
     				m_freem(WRK.mhead);
			} else {
  				if (NDD.ndd_genstats.ndd_ipackets_lsw == 
					ULONG_MAX) {
  					NDD.ndd_genstats.ndd_ipackets_msw++;
				}
  				NDD.ndd_genstats.ndd_ipackets_lsw++;

  				if ((ULONG_MAX - WRK.mhead->m_pkthdr.len) <
  			    	NDD.ndd_genstats.ndd_ibytes_lsw) {
  					NDD.ndd_genstats.ndd_ibytes_msw++;
				}

  				NDD.ndd_genstats.ndd_ibytes_lsw += 
  					WRK.mhead->m_pkthdr.len;

                        	/* For netpmon performance tool */
                        	TRACE_SYS(MPS_RV, TRC_RNOT, 
					  p_dev_ctl->seq_number, 
					  WRK.mhead, WRK.mhead->m_pkthdr.len);
  				(*(NDD.nd_receive))(p_ndd, WRK.mhead);

                        	/* For netpmon performance tool */
                        	TRACE_SYS(MPS_RV, TRC_REND, 
					  p_dev_ctl->seq_number, 
					  WRK.read_index, 0); 

			}
  			WRK.mhead = WRK.mtail  = NULL;
			WRK.t_len = 0;
  		}
                WRK.read_index = (WRK.read_index + 1) % MAX_RX_LIST;
  	}
  } /* end of check status */
} /* end of while */
TRACE_SYS(MPS_RV, "RrvE", (ulong)p_dev_ctl, (ulong)cmd_reg, 0);

} /* end mps_recv routine                                                    */


/*****************************************************************************/
/* NAME : discard_packet
*
* FUNCTION: discard the bad packet. 
*
* EXECUTION ENVIRONMENT: process only
*
* NOTES:
*    Input: p_dev_ctl - pointer to device control structure.
*
*    Called From:
*               mps_recv
*
*    Calls to:
*               arm_recv_list
*/
/*****************************************************************************/
int discard_packet (
mps_dev_ctl_t       *p_dev_ctl)  /* pointer to the device control area */
{

  volatile rx_list_t    recvlist;
  struct mbuf 		*m, *m0;
  int			i;

  TRACE_DBG(MPS_RV, "PdpB", (ulong)p_dev_ctl, WRK.read_index, 0);
  NDD.ndd_genstats.ndd_ierrors++;

  /*
   * Frees any mbuf that have allocated for the receive frame but have not yet
   * passed up to the higher layer.  The driver process frame based on EOB & EOF
   * interrupt.  The driver will pass the frame to the higher layer only when it
   * detected an EOF.
   */
  if (WRK.mhead) {
	WRK.t_len = 0;
  	m_freem(WRK.mhead);
  	WRK.mhead = WRK.mtail  = NULL;
  }

  while (TRUE) {

  	i = WRK.read_index;               /* start at next recv list */
  	cache_inval(mtod(WRK.recv_mbuf[i], caddr_t), WRK.recv_mbuf[i]->m_len);

  	/*  
    	 * D_move the receive list image from the IOCC cache to main
   	 *  memory (local image) pointed to by rlptr.
   	 */
	if (WRK.iocc) {
  		d_kmove (&recvlist, WRK.recv_list[i], (uint)RX_LIST_SIZE,
  	 	      WRK.dma_channel, DDS.bus_id, DMA_READ);
        } else {
  		bcopy(WRK.recv_vadr[i], &recvlist, (uint)RX_LIST_SIZE);
  	}

  	recvlist.recv_status  = toendianL(recvlist.recv_status);

  	if (!(recvlist.recv_status & BUF_PROCESS)) {
  		break;
	}

  	if (arm_recv_list(p_dev_ctl, i, FALSE, WRK.recv_mbuf[i])) {
		return(i);
	}

  	WRK.read_index = (WRK.read_index + 1) % MAX_RX_LIST;

  	if (recvlist.recv_status & END_OF_FRAME) {
  		break;
        }

  } /* end of while */

  return(0);
  TRACE_DBG(MPS_RV, "PdpE", (ulong)p_dev_ctl, WRK.read_index, 0);
} /* end of discard_packet */

/*****************************************************************************/
/*                                                                           */
/*  NAME: arm_recv_list                                                      */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    Rearms the receive list indexed by i and d_kmoves it thru the IOCC     */
/*    cache back to the receive chain.  Uses the mbuf passed in unless the   */
/*    mbuf is NULL.  If the mbuf is NULL, then it gets a new mbuf and        */
/*    d_masters it.  If it can't get a new MBUF then the receive list is set */
/*    with a zero data count and its valid flag is turned off to cause the   */
/*    adapter to wait at this receive list until an mbuf becomes available.  */
/*                                                                           */
/*  EXECUTION ENVIRONMENT:                                                   */
/*      This routine can be executed by both the interrupt and process call  */
/*      threads of the driver.                                               */
/*                                                                           */
/*  DATA STRUCTURES:                                                         */
/*      Allocates an mbuf for the recv_mbuf indexed by i (if possible);      */
/*      d_master's the mbuf to the associated recv_addr location. Modifies   */
/*      the associated recv_list.                                            */
/*                                                                           */
/*  RETURNS:                                                                 */
/*      0               Completed successfully.                              */
/*      -1              Error (not enough mbufs).                            */
/*                                                                           */
/*  EXTERNAL PROCEDURES CALLED:  m_getclust,                                 */
/*                               d_master, d_kmove.                          */
/*                                                                           */
/*****************************************************************************/

int  arm_recv_list (
mps_dev_ctl_t *p_dev_ctl,  /* pointer to the device control area */
int            i,          /* which receive list */
int            new_mbuf,  /* new mbuf */
struct mbuf    *m)         /* MBUF to rearm      */
{
  volatile rx_list_t      recvlist;
  int	ioa, rc;

  TRACE_DBG(MPS_RV, "PalB", (ulong)p_dev_ctl, i, 0);
  /*
   *  get a new mbuf then d_master it.
   */
  if (new_mbuf) {
	/*
  	 * Set up cross memory descriptor                            
	 */
  	WRK.rx_xmemp[i].aspace_id  = XMEM_GLOBAL;
  	WRK.rx_xmemp[i].subspace_id = NULL;
  	d_master (WRK.dma_channel, DMA_READ | DMA_NOHIDE, 
  	    	  MTOD( m, uchar * ), PAGESIZE, &WRK.rx_xmemp[i] , 
  	    	  WRK.recv_addr[i]);
  	WRK.recv_mbuf[i] = m;
  }

  /*  
   *  Updates the receive dma image of the list by d_moving it     
   *  through the IOCC cache into system memory.                  
   */
  if (WRK.iocc) {                                                              
  	/*                                                              
   	 * Setup the receive list such that the adapter will begin     
   	 * writing receive data after the configured data offset.      
   	 */
  	recvlist.fw_pointer   = toendianL((ulong)WRK.recv_list[(i+1) % 
								MAX_RX_LIST]);
  	recvlist.recv_status  = 0;
  	recvlist.data_pointer = toendianL((ulong)WRK.recv_addr[i]);
  	recvlist.fr_len       = 0;
  	recvlist.data_len     = 0x0010;   /* 4K buf len (byte swapped) */

  	d_kmove (&recvlist, WRK.recv_list[i], (uint)RX_LIST_SIZE, 
  	   	WRK.dma_channel, DDS.bus_id, DMA_WRITE_ONLY);
  } else {
  	/*                                                              
   	 * Setup the receive list such that the adapter will begin     
   	 * writing receive data after the configured data offset.      
   	 */
  	WRK.recv_vadr[i]->fw_pointer   = toendianL((ulong)WRK.recv_list[(i+1) % 
								MAX_RX_LIST]);
  	WRK.recv_vadr[i]->recv_status  = 0;
  	WRK.recv_vadr[i]->data_pointer = toendianL((ulong)WRK.recv_addr[i]);
  	WRK.recv_vadr[i]->fr_len       = 0;
  	WRK.recv_vadr[i]->data_len     = 0x0010; /* 4K buf len(byte swapped) */
  }

  ioa = (int)BUSIO_ATT( DDS.bus_id, DDS.io_base_addr);
  PIO_PUTLRX(ioa + RxLBDA_L, WRK.recv_list[i]);
  if (WRK.pio_rc) {
  	BUSIO_DET(ioa);                /* restore I/O Bus              */
        mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL,0,FALSE,FALSE,FALSE);
  	TRACE_BOTH(MPS_ERR, "Ral1", p_dev_ctl, WRK.pio_rc, 0);
	return (ENETDOWN);
  }
  BUSIO_DET(ioa);                      /* restore I/O Bus              */
  TRACE_DBG(MPS_RV, "PalE", (ulong)p_dev_ctl,WRK.recv_mbuf[i],WRK.recv_mbuf[0]);

  return(0);                /* return 0 if mbuf */
}



