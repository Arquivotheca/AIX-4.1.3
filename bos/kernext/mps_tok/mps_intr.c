static char sccsid[] = "@(#)68  1.10.1.5  src/bos/kernext/mps_tok/mps_intr.c, sysxmps, bos41J, 9520B_all 5/18/95 16:11:52";
/*
 *   COMPONENT_NAME: sysxmps
 *
 *   FUNCTIONS: mps_intr
 *		mps_mac_recv
 *		rw_intr
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
 * NAME:     mps_intr
 *
 * FUNCTION: Wildwood driver interrupt routine.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      The FLIH
 *
 * INPUT:
 *      p_ihs           - point to the interrupt structure.
 *
 * RETURNS:
 *      INTR_SUCC - our interrupt
 *      INTR_FAIL - not our interrupt
 */
/*****************************************************************************/
mps_intr(
struct intr *p_ihs)  /* This also points to device control area */
{
  mps_dev_ctl_t   *p_dev_ctl = (mps_dev_ctl_t *)p_ihs;
  ndd_statblk_t    stat_blk;   /* status block */
  ndd_t           *p_ndd = &(NDD);
  ushort           sisr_status_reg;
  ushort           cmd_reg, bctl;
  int              rc = INTR_FAIL;             /* Return code            */
  int              ioa;
  int              ipri;
  int              fr_len;
  int              buf_ptr;
  ushort           data[10],i;
  ushort           lan_status;

  TRACE_DBG(MPS_OTHER, "SinB", p_dev_ctl, (ulong)p_ihs,p_dev_ctl->device_state);

  ipri = disable_lock(PL_IMP, &SLIH_LOCK);
  if ((p_dev_ctl->device_state == CLOSED) |
  	(p_dev_ctl->device_state == DEAD)) {
                  unlock_enable(ipri, &SLIH_LOCK);
                  TRACE_BOTH(MPS_ERR, "Sin1", p_dev_ctl, INTR_FAIL, 0);
                  return(INTR_FAIL);
  }

  /*
   * Gets access to the I/O bus to access I/O registers                 
   */
  ioa = (int)BUSIO_ATT(DDS.bus_id, DDS.io_base_addr);
  /* 
   * Gets the system interrupt status register for this card            
   */
  PIO_GETSRX(ioa + SISR, &sisr_status_reg);
  if (WRK.pio_rc) {
        mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0,TRUE,FALSE,FALSE);
        TRACE_BOTH(MPS_ERR, "Sin1", p_dev_ctl, WRK.pio_rc, 0);
	sisr_status_reg = 0;
  }

  /* 
   * Processes interrupt from the adapter 
   */
  while (sisr_status_reg & SISR_MSK) {

	/*
         * Valid adapter interrupt, sets retcode to success           
	 */
        rc = INTR_SUCC;

        /*********************************************************************/
        /* 
	 * We have a Master interrupt status register intr :   
  	 * when one or more of the interrupt status condition bits in the Bus 
	 * Master Interrupt Status Register is a ONE.
         */
        /*********************************************************************/

        if (sisr_status_reg & MISR_INT) {
                PIO_GETSRX(ioa + MISR, &cmd_reg);
  		if (WRK.pio_rc) {
        		mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
							TRUE,FALSE,FALSE);
        		TRACE_BOTH(MPS_ERR, "Sin2", p_dev_ctl, WRK.pio_rc, 0);
			break;
  		}

		/*
		 * To clear the MISR interrupt bit in the System Interrupt 
		 * Status Register, we must clear the Bus Master Interrupt 
		 * Status Register.
                 */
                PIO_PUTSRX( ioa + MISR, ~cmd_reg);
  		if (WRK.pio_rc) {
        		mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
							TRUE, FALSE, FALSE);
        		TRACE_BOTH(MPS_ERR, "Sin3", p_dev_ctl, WRK.pio_rc, 0);
			break;
  		}

                if ((p_dev_ctl->device_state == OPENED) ||
                    (p_dev_ctl->device_state == CLOSE_PENDING)) {
			/* Handle the Tx or Rx related interrupt */
                        if (rw_intr(p_dev_ctl, cmd_reg, ioa) == FALSE) {
				break;
			}
                }

        }

        /*********************************************************************/
        /* 
	 * SRB response interrupt : The Adapter has recognized a SRB request 
         * and has set the return code in the SRB
         */
        /*********************************************************************/
        else if (sisr_status_reg & SRB_RSP) {

                if ((p_dev_ctl->device_state == OPEN_SETUP) |
                	(p_dev_ctl->device_state == LIMBO)) {
			/*
                         * Gets offset for SRB  
			 */
                        PIO_GETSRX(ioa + LAPWWO, &WRK.srb_address);
  			if (WRK.pio_rc) {
        			mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, 
					   NDD_PIO_FAIL, 0, TRUE, FALSE, FALSE);
        			TRACE_BOTH(MPS_ERR," Sin4", p_dev_ctl, 
							WRK.pio_rc, 0);
				break;
  			}

			/* 
			 * Cleanup the first buffer in the Rx list which has 
			 * been used by the adapter for doing the adapter self
			 * test during initialize process.
			 */
                        one_buf_undo(p_dev_ctl);
                }
                srb_response(p_dev_ctl, ioa);
                PIO_PUTSRX(ioa + SISR_RUM, ~SRB_RSP); /*RUM.SISR.bit5*/
  		if (WRK.pio_rc) {
        		TRACE_BOTH(MPS_ERR, "Sin5", p_dev_ctl, WRK.pio_rc, 0);
        		mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
							TRUE, FALSE, FALSE);
			break;
  		}
        }

        /*****************************************************/
        /* LAP access violation error interrupt              */
        /*****************************************************/
        else if (sisr_status_reg & LAP_ACC) {
                mps_logerr(p_dev_ctl, ERRID_MPS_BUS_ERR, __LINE__,
                              __FILE__, NDD_BUS_ERROR, sisr_status_reg, 0);
                enter_limbo(p_dev_ctl, TRUE, TRUE, 0, NDD_BUS_ERROR, 
				sisr_status_reg, 0);
        }

        /*****************************************************/
        /* LAP data parity error interrupt                   */
        /*****************************************************/
        else if ((sisr_status_reg & LAP_PRTY) |
        /*****************************************************/
        /* Micro channel parity error interrupt              */
        /*****************************************************/
        	(sisr_status_reg & MC_PRTY)) {
                /*
                 * Resets the channel check bit in POS register
                 */
                mps_logerr(p_dev_ctl,ERRID_MPS_BUS_ERR, __LINE__,
                              __FILE__, NDD_BUS_ERROR, sisr_status_reg, 0);
                PIO_GETSRX(ioa + BCtl, &bctl);
                bctl |= CHCK_BIT;
                PIO_PUTSRX( ioa + BCtl, bctl);
  		if (WRK.pio_rc) {	
        		mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
							TRUE, FALSE, FALSE);
        		TRACE_BOTH(MPS_ERR, "Sin6", p_dev_ctl, WRK.pio_rc, 0);
			break;
  		}
                enter_limbo(p_dev_ctl, TRUE, TRUE, 0, NDD_BUS_ERROR, 
				sisr_status_reg, 0);
        }

        /*****************************************************/
        /* Adapter check interrupt                           */
        /*****************************************************/
        else if (sisr_status_reg & ADAPT_CHK) {
		ushort data[4];
		ushort lapwwc; 

                PIO_GETSRX(ioa + LAPWWC, &lapwwc);
  		PIO_PUTSRX(ioa + LAPE, 0x00);
  		PIO_PUTSRX(ioa + LAPA, lapwwc); 
  		for (i=0; i < 4; i++) {
        		PIO_GETSX(ioa + LAPD_I, &data[i]);
  		}

                mps_logerr(p_dev_ctl, ERRID_MPS_ADAP_CHECK, __LINE__,
                        __FILE__, data[0], data[1], data[2]);
  		if (!WRK.pio_rc) {
                	PIO_PUTSRX(ioa + SISR_RUM, ~ADAPT_CHK);/*RUM.SISR.bit8*/
  		}
  		if (WRK.pio_rc) {
        		mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
						TRUE, FALSE, FALSE);
                	TRACE_BOTH(MPS_ERR,"Sin8",p_dev_ctl, WRK.pio_rc, 0);
			break;
  		}

                enter_limbo(p_dev_ctl, TRUE, TRUE, 0, NDD_ADAP_CHECK, 
					sisr_status_reg, 0);

        }
        /**********************************************************************/
        /* 
	 * ASB Free interrupt : The adapter has read the ARB response provided 
	 * in the  ASB, and ASB is available for other response
         */
        /**********************************************************************/
        else if (sisr_status_reg & ASB_FREE) {
                PIO_PUTSRX(ioa + SISR_RUM, ~ASB_FREE);/*RUM.SISR.bit4*/
  		if (WRK.pio_rc) {	
        		mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
							TRUE, FALSE, FALSE);
                	TRACE_BOTH(MPS_ERR,"Sin9",p_dev_ctl, WRK.pio_rc, 0);
			break;
  		}
        }
        /*********************************************************************/
        /* 
	 * ARB command interrupt : The ARB comtains a command for the drivers
         * to act on
         */
        /*********************************************************************/
        else if (sisr_status_reg & ARB_CMD) {
                ushort  data[6], i, command;

                PIO_PUTSRX(ioa + LAPE, 0x00);
                PIO_PUTSRX(ioa + LAPA, WRK.arb_address);
                for (i = 0; i < 6; i++)
		{
                        PIO_GETSX(ioa + LAPD_I, &data[i]);
		}
  		if (!WRK.pio_rc) {
                	PIO_PUTCX(ioa + SISR_RUM, ~ARB_CMD); /*SUM.SISR.bit3*/
  		}
  		if (WRK.pio_rc) {
        		mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
							TRUE, FALSE, FALSE);
                	TRACE_BOTH(MPS_ERR,"Sina",p_dev_ctl, WRK.pio_rc, 0);
			break;
  		}
                command = data[0] >> 8;

                if (command == RECEIVE_DATA) {
			fr_len = data[5];
			buf_ptr = data[3];
                        mps_mac_recv(p_dev_ctl, fr_len, buf_ptr);
                } else if (command == LAN_STATUS_CHANGE) {
			
			lan_status = data[3];
                        if (lan_status & LOBE_WIRE_FAULT) {
                        	if (!(NDD.ndd_flags & NDD_LIMBO)) {
                        		mps_logerr(p_dev_ctl, 
						   ERRID_MPS_WIRE_FAULT, 
						   __LINE__, __FILE__, 
						   lan_status, 0, 0);
				}
				w_stop (&(LANWDT));
                                TOKSTATS.lobewires++;
                                MIB.Token_ring_mib.Dot5Entry.ring_status
                                                 = TR_MIB_LOBEWIREFAULT;

                        	enter_limbo(p_dev_ctl, TRUE, TRUE, 30, 
						TOK_WIRE_FAULT, 0, lan_status);
               			TRACE_BOTH(MPS_ERR,"Sinc",p_dev_ctl, lan_status,
						 0);
				break;
                        }

                        if (lan_status & CABLE_NOT_CONNECTED){
                        	mps_logerr(p_dev_ctl, ERRID_MPS_WIRE_FAULT, 
					__LINE__, __FILE__, lan_status, 0, 0);
				w_stop (&(LANWDT));

                        	enter_limbo(p_dev_ctl, TRUE, TRUE, 30, 
						TOK_WIRE_FAULT, 0, lan_status);
               			TRACE_BOTH(MPS_ERR,"Sind",p_dev_ctl, lan_status,
						 0);
				break;
                        }

                        if (lan_status & AUTO_REMOVAL_ERROR) {
                        	mps_logerr(p_dev_ctl, ERRID_MPS_AUTO_RMV, 
					__LINE__, __FILE__, lan_status, 0, 0);
				w_stop (&(LANWDT));
                                TOKSTATS.removes++;
                                MIB.Token_ring_mib.Dot5Entry.ring_status                                                 = TR_MIB_AUTOREMOVAL_ERR;

				if (WRK.ndd_limbo) {
					mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, 
						0, 0, TRUE, FALSE, FALSE);
                                	WRK.ndd_limbo = FALSE;
				} else {
                                	WRK.ndd_limbo = TRUE;
                        		enter_limbo(p_dev_ctl, TRUE, TRUE, 30, 
						TOK_BEACONING, 0, lan_status);
				}
               			TRACE_BOTH(MPS_ERR,"Sing",p_dev_ctl, lan_status,
						 0);
				break;
                        }

                        if (lan_status & REMOVE_RECEIVE) {
                        /*
                         * We have been kicked off the ring by the LAN manager.
                         */
                        	mps_logerr(p_dev_ctl, ERRID_MPS_RMV_ADAP, 
					__LINE__, __FILE__, lan_status, 0, 0);
				w_stop (&(LANWDT));
                                MIB.Token_ring_mib.Dot5Entry.ring_status
                                                 = TR_MIB_REMOVE_RX;

				if (WRK.ndd_limbo) {
					mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, 
						0, 0, TRUE, FALSE, FALSE);
                                	WRK.ndd_limbo = FALSE;
				} else {
                                	WRK.ndd_limbo = TRUE;
                        		enter_limbo(p_dev_ctl, TRUE, TRUE, 30, 
						TOK_BEACONING, 0, lan_status);
				}
               			TRACE_BOTH(MPS_ERR,"Sinh",p_dev_ctl, lan_status,
						 0);
				break;
                        }

                        if (lan_status & SIGNAL_LOSS) {
                                TOKSTATS.signal_loss++;
                                MIB.Token_ring_mib.Dot5Entry.ring_status
                                                 = TR_MIB_SIGNAL_LOSS;
                        }

                        if (lan_status & HARD_ERROR) {
                                TOKSTATS.hard_errs++;
                                MIB.Token_ring_mib.Dot5Entry.ring_status
                                                 = TR_MIB_HARD_ERROR;

				if (!WRK.hard_err) {
        				/*
       			 		 * Pass the status block to demuxer
       					 */
       					 stat_blk.code = NDD_STATUS;
       					 stat_blk.option[0] = TOK_BEACONING;
       					 stat_blk.option[1] = 0;
       					 stat_blk.option[2] = lan_status;
       					 (*(NDD.nd_status))(p_ndd, &stat_blk);
					WRK.hard_err = TRUE;
				}

               			TRACE_BOTH(MPS_ERR,"Sine",p_dev_ctl, lan_status,
						 0);
                        }

                        if (lan_status & TRANSMIT_BEACON) {
                        	mps_logerr(p_dev_ctl, ERRID_MPS_WIRE_FAULT, 
					__LINE__, __FILE__, lan_status, 0, 0);
				w_stop (&(LANWDT));
                                TOKSTATS.tx_beacons++;
                                MIB.Token_ring_mib.Dot5Entry.ring_status
                                                 = TR_MIB_TX_BEACON;

                        }

                        if (lan_status & SINGLE_STATION) {
                                TOKSTATS.singles++;
                                MIB.Token_ring_mib.Dot5Entry.ring_status
                                         = TR_MIB_SINGLESTATION;
                        }

                        if (lan_status & RING_RECOVERY) {
                                TOKSTATS.recoverys++;
                                MIB.Token_ring_mib.Dot5Entry.ring_status
                                         = TR_MIB_RINGRECOVERY;
				WRK.hard_err = FALSE;
                        }

                        if (lan_status & COUNTER_OVERFLOW){
                                read_adapter_log(p_dev_ctl, TRUE);
                        }

                        if (lan_status & SR_BRIDGE_COUNTER_OVERFLOW) {
                                read_adapter_log(p_dev_ctl, TRUE);
                        }

                        if (lan_status & SOFT_ERROR) {
                                TOKSTATS.soft_errs++;
                                MIB.Token_ring_mib.Dot5Entry.ring_status
                                                 = TR_MIB_SOFT_ERR;
                        }
                }

                PIO_PUTCX(ioa + LISR_SUM, ARB_FREE); /*SUM.LISR.bit1*/
  		if (WRK.pio_rc) {
        		mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
							TRUE, 0, 0);
                	TRACE_BOTH(MPS_ERR,"Sini", p_dev_ctl, WRK.pio_rc, 0);
			break;
  		}

        }
        /*****************************************************/
        /* 
         * TRB response interrupt : The adapter has recognized a transmit 
	 * request and set the return code in the TRB
         */
        /*****************************************************/
        else if (sisr_status_reg & TRB_RSP) {
                PIO_PUTSRX(ioa + LAPE, 0x00);
                PIO_PUTSRX(ioa + LAPA, WRK.trb_address);
                for (i=0; i < 4; i++) {
                        PIO_GETSX(ioa + LAPD_I, &data[i]);
                }
  		if (!WRK.pio_rc) {
                	PIO_PUTCX(ioa + SISR_RUM, ~TRB_RSP);/* RUM.SISR.bit2 */
  		}
  		if (WRK.pio_rc) {
        		mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
							TRUE, 0, 0);
                	TRACE_BOTH(MPS_ERR,"Sinj", p_dev_ctl, WRK.pio_rc, 0);
			break;
  		}

        }

        /* 
	 * Gets the system interrupt status register for this card    
	 */
        PIO_GETSRX(ioa + SISR, &sisr_status_reg);
  	if (WRK.pio_rc) {
        	mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
							TRUE, 0, 0);
               	TRACE_BOTH(MPS_ERR,"Sink", p_dev_ctl, WRK.pio_rc, 0);
		break;
  	}
  } /* end while adapter is still interrupting                         */

  BUSIO_DET(ioa);                 /* restore I/O Bus                   */
  TRACE_DBG(MPS_OTHER, "SinE", p_dev_ctl, rc, 0);
  unlock_enable(ipri, &SLIH_LOCK);
  return(rc);

} /* end mps_intr                                                            */


/*****************************************************************************/
/*
 * NAME:     rw_intr
 *
 * FUNCTION: Wildwood driver read/write interrupt routine.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 *    Input:
 *  	p_dev_ctl - pointer to device control structure.
 *      cmd_reg   - value in the interrupt register.
 *      ioa	  - BUSIO_ATT return value
 *
 *    CALLED FROM:
 *      mps_intr
 *
 *    CALLED TO:
 *      mps_recv
 *      mps_tx1_done
 *      mps_tx2_done
 *
*/
/*****************************************************************************/
int rw_intr (
mps_dev_ctl_t  *p_dev_ctl,
int        	cmd_reg,
int		ioa)
{
  register int       i, rc, index;
  volatile tx_list_t xmitlist;
  xmit_elem_t        *xelm;         /* transmit element structure */
  ushort             bmctl;
  ulong              bda, fda, stat;
  volatile rx_list_t recvlist;

  /*********************************************/
  /* Test if a receive interrupt occurred      */
  /*********************************************/
  TRACE_DBG(MPS_OTHER, "SrwB", p_dev_ctl, cmd_reg, 0);
  if ((cmd_reg & RECEIVE_MSK) && (p_dev_ctl->device_state == OPENED)) {
  /* Receive interrupt occurred - process it */
        /* 
	 * Increments counters for receive interrupts processed 
	 */
        if (NDD.ndd_genstats.ndd_recvintr_lsw == ULONG_MAX) {
                NDD.ndd_genstats.ndd_recvintr_msw++;
	}
        NDD.ndd_genstats.ndd_recvintr_lsw++;

        if (cmd_reg & (Rx_HALT | Rx_NBA)) {

                /* 
		 * Unmasks the adapter MISR interrupt register 
	 	 */
                PIO_PUTSRX( ioa + MISRMask_RUM, ~RV_UNMASK);
  		if (WRK.pio_rc) {	
        		mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
							TRUE, 0, 0);
  			TRACE_BOTH(MPS_ERR, "Srw1", p_dev_ctl, WRK.pio_rc, 0);
			return (FALSE);
  		}

                /*
                 * Resets the Rx channel disable in BMCTL only when it is set
                 */
                for (i=0; i < 5; i++) {
                        PIO_GETSRX(ioa + BMCtl_sum, &bmctl);
                        if (bmctl & RX_DISABLE) {
                                PIO_PUTSRX(ioa+BMCtl_rum, ~RX_DISABLE);
				break;
                        }
			io_delay(1000);
                }
  		if (WRK.pio_rc) {
  			TRACE_BOTH(MPS_ERR,"Srw3",p_dev_ctl, WRK.pio_rc,0);
        		mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
							TRUE, 0, 0);
			return (FALSE);
  		}
        }

        /* 
	 * Receives a frame with no status posted 
	 */
        if (cmd_reg & Rx_NOSTA) {
                /*
                 * Checks if any status in the RxStat register, and
                 * update the status field in the buffer descriptor
                 */
                PIO_GETLX(ioa + RxStat_L,  &stat);
  		if (WRK.pio_rc) {
  			TRACE_BOTH(MPS_ERR, "Srw4", p_dev_ctl, WRK.pio_rc, 0);
        		mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
							TRUE, 0, 0);
			return (FALSE);
  		}
                if (stat) {
                        PIO_GETLRX(ioa + RxBDA_L,  &bda);
  			if (WRK.pio_rc) {
  				TRACE_BOTH(MPS_ERR, "Srw5", p_dev_ctl,
							WRK.pio_rc, 0);
        			mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, 
						NDD_PIO_FAIL, 0, TRUE, 0, 0);
				return (FALSE);
  			}

                        for (i=0; i < MAX_RX_LIST; i++) {
                                if (bda == (int)WRK.recv_list[i]) {
                                        break;
				}
                        } /* end of for */

                        stat |= BUF_PROCES; /* Set BUF PROCESS bit*/

                        /*  Updates the receive dma image of the list by
                         *  d_moving it through the IOCC cache into
                         *  system memory.
                         */
			if (WRK.iocc) {
                        	d_kmove (&stat, &WRK.recv_list[i]->recv_status,
					 4, WRK.dma_channel, DDS.bus_id,
                                         DMA_WRITE_ONLY);
			} else {
                                WRK.recv_vadr[i]->recv_status = stat;
			}

                }
        } /* end of Rx_NOSTA */

	/*
         * Receives a frame 
	 */
        if (cmd_reg & (Rx_EOF | Rx_EOB)) {
                /* Process the receive frames */
                mps_recv(p_dev_ctl, cmd_reg, ioa);
        }

        /* 
	 * Receives channel halt/receive channel with no buffer available
         * in the system
        */
        if (cmd_reg & (Rx_HALT | Rx_NBA)) {
		/* 
	 	 * Frees any mbuf that have allocated for the receive frame but
		 * have not yet passed up to the higher layer.  The driver 
		 * process frame based on EOB & EOF interrupt.  The driver 
		 * will pass the frame to the higher layer only when it 
		 * detected an EOF.
		 */
                if (WRK.mhead) {
                        m_freem(WRK.mhead);
                        WRK.mhead = WRK.mtail  = NULL;
                }

  		/*
  		 *  Updates the receive dma image of the list by d_moving it
  		 *  through the IOCC cache into system memory.
  		 */
  		if (WRK.iocc) {
        		/*
        		 * Setup the receive list such that the adapter
			 *  will begin writing receive data after the 
			 * configured data offset.
        		 */
			for (i = 0; i < MAX_RX_LIST; i++) {
        			recvlist.fw_pointer   = 
					toendianL((ulong)WRK.recv_list[(i+1) %
								MAX_RX_LIST]);
        			recvlist.recv_status  = 0;
        			recvlist.data_pointer = 
					toendianL((ulong)WRK.recv_addr[i]);
        			recvlist.fr_len       = 0;
				/* 4K buf len (byte swapped) */
        			recvlist.data_len     = 0x0010;   

        			d_kmove (&recvlist, WRK.recv_list[i], 
					(uint)RX_LIST_SIZE, WRK.dma_channel, 
					DDS.bus_id, DMA_WRITE_ONLY);
			}
  		} else {
        		/*
        		 * Setup the receive list such that the 
			 * adapter will begin writing receive data 
			 * after the configured data offset.
        		 */
			for (i = 0; i < MAX_RX_LIST; i++) {
        			WRK.recv_vadr[i]->fw_pointer   = 
					toendianL((ulong)WRK.recv_list[(i+1) %
							 MAX_RX_LIST]);
        			WRK.recv_vadr[i]->recv_status  = 0;
        			WRK.recv_vadr[i]->data_pointer = 
					toendianL((ulong)WRK.recv_addr[i]);
        			WRK.recv_vadr[i]->fr_len       = 0;
				/* 4K buf len(byte swapped) */
        			WRK.recv_vadr[i]->data_len     = 0x0010; 
  			}
		}

	       /*
		* Send the first and last receive buffer descriptors to the 
		* adapter and then try to adjust to the receive descriptor 
		* which the adapter first received into it.
		*/
                PIO_PUTLRX(ioa + RxBDA_L, WRK.recv_list[0]);
                PIO_PUTLRX(ioa + RxLBDA_L,WRK.recv_list[MAX_RX_LIST-1]);
                PIO_GETLRX(ioa + RxBDA_L, &bda);
  		if (WRK.pio_rc) {
  			TRACE_BOTH(MPS_ERR, "Srw8",p_dev_ctl, WRK.pio_rc, 0);
        		mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
							TRUE, 0, 0);
			return (FALSE);
  		}
                for (WRK.read_index = 0; WRK.read_index < MAX_RX_LIST;
                        WRK.read_index++) {
                          if (bda == (int)WRK.recv_list[WRK.read_index]) {
                                  break;
			  }

                } /* end of for */
                WRK.read_index = (WRK.read_index) % MAX_RX_LIST;

                /* 
	 	 * Allows the adapter generate the HALT/NBA intr again 
		 */
                PIO_PUTSRX( ioa + MISRMask, RV_UNMASK);
  		if (WRK.pio_rc) {
  			TRACE_BOTH(MPS_ERR, "Srw9",p_dev_ctl, WRK.pio_rc, 0);
        		mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
							TRUE, 0, 0);
			return (FALSE);
  		}
        }

  } /* endif for RECEIVE  interrupt             */

  /*****************************************************/
  /* Adapter Tx interrupt has occurred - channel 1     */
  /*****************************************************/
  /* Test if a transmit 1 interrupt occurred             */
  if (cmd_reg & XMIT_DONE_MSK_1) {
  /* 
   * Tx done interrupt occurred - process it increment RAS counters for
   * transmit interrupts processed
   */

        if (NDD.ndd_genstats.ndd_xmitintr_lsw == ULONG_MAX) {
                NDD.ndd_genstats.ndd_xmitintr_msw++;
	}

        NDD.ndd_genstats.ndd_xmitintr_lsw++;

        if (cmd_reg & Tx1_NOST) {
		/*
		 * First gets the current transmit status and current 
		 * transmit descriptor. Then finds the index of current 
		 * transmit descriptor in the Tx descriptor list and update 
		 * the status field.
		 */
                PIO_GETLX(ioa + Tx1Stat_L,  &stat);
                PIO_GETLRX(ioa + Tx1FDA_L,  &fda);
  		if (WRK.pio_rc) {
  			TRACE_BOTH(MPS_ERR, "Srwb",p_dev_ctl, WRK.pio_rc, 0);
        		mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
							TRUE, 0, 0);
			return(FALSE);
  		}

                for (i=0; i < MAX_TX_LIST; i++) {
                        if (fda == (int)WRK.tx1_list[i]) {
                        	break;
			}
                } /* end of for */


                /*  
		 *  Updates the receive dma image of the list by d_moving 
                 *  it through the IOCC cache into system memory.
                 */
                stat |= BUF_PROCES; /* Set BUF PROCESS bit*/

		if (WRK.iocc) {
                	d_kmove (&stat, &WRK.tx1_list[i]->xmit_status, 4,
                		WRK.dma_channel, DDS.bus_id, DMA_WRITE_ONLY);
		} else {
                        WRK.tx1_vadr[i]->xmit_status = stat;
		}

        } /* end of Tx1_NOST */

        mps_tx1_done (p_dev_ctl, ioa, cmd_reg);

  } /* endif for transmit 1 interrupt               */

  /*****************************************************/
  /* Adapter Tx interrupt has occurred - channel 2     */
  /*****************************************************/
  /* Test if a transmit 2 interrupt occurred           */
  if (cmd_reg & XMIT_DONE_MSK_2) {
        /* 
	 * Tx done interrupt occurred - process it increment RAS
         * counters for transmit interrupts processed
         */
        if (NDD.ndd_genstats.ndd_xmitintr_lsw == ULONG_MAX) {
                NDD.ndd_genstats.ndd_xmitintr_msw++;
	}

        NDD.ndd_genstats.ndd_xmitintr_lsw++;

        if (cmd_reg & Tx2_NOST) {
		/*
		 * First gets the current transmit status and current 
		 * transmit descriptor. Then finds the index of current 
		 * transmit descriptor in the Tx descriptor list and update 
		 * the status field.
		 */
                PIO_GETLX(ioa + Tx2Stat_L,  &stat);
                PIO_GETLRX(ioa + Tx2FDA_L,  &fda);
  		if (WRK.pio_rc) {
  			TRACE_BOTH(MPS_ERR, "Srwd",p_dev_ctl, WRK.pio_rc, 0);
        		mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
							TRUE, 0, 0);
			return(FALSE);
  		}

                for (i=0; i < MAX_TX_LIST; i++) {
                        if (fda == (int)WRK.tx2_list[i]) {
                        	break;
			}
                } /* end of for */

                /*  
		 *  Updates the receive dma image of the list by d_moving 
                 *  it through the IOCC cache into system memory.
                 */
                stat |= BUF_PROCES; /* Set BUF PROCESS bit*/

		if (WRK.iocc) {
                	d_kmove (&stat, &WRK.tx2_list[i]->xmit_status, 4,
                		WRK.dma_channel, DDS.bus_id, DMA_WRITE_ONLY);
		} else {
                        WRK.tx2_vadr[i]->xmit_status = stat;
		}

        } /* end of Tx2_NOST */

        mps_tx2_done (p_dev_ctl, ioa, cmd_reg);

  } /* endif for transmit 2 interrupt               */

  return (TRUE);
  TRACE_DBG(MPS_OTHER, "SrwE", p_dev_ctl, cmd_reg, 0);

}

/*****************************************************************************/
/*
* NAME: mps_mac_recv
*
* FUNCTION:  Receive the packets from the adapter.
*
* EXECUTION ENVIRONMENT: interrupt only
*
* NOTES:
*    Input:
*       p_dev_ctl - pointer to device control structure.
*       fr_len    - length of frame.
*       buf_ptr   - data pointer.
*
*    Called From:
*               mps_intr
*
*/
/*****************************************************************************/
int mps_mac_recv (
mps_dev_ctl_t *p_dev_ctl,     /* point to the device control area */
uint          fr_len,
uint          buf_ptr)
{
  typedef       struct {
        uchar        cmd;
        uchar        rsv;
        uchar        retcode;
        uchar        rsv3[3];
        ushort       buf_ptr;
  } o_parm_t;

  o_parm_t      o_parm;
  ushort        *parm = (ushort *)&o_parm;
  struct mbuf   *m;
  ushort        *mem, id;
  ushort        recv_fs, buf_len, next_buf;
  ndd_t         *p_ndd = (ndd_t *)&(NDD);
  uint          lapd;
  uint          count = 0;
  int           ioa, i, x;
  uchar bcast1_addr[CTOK_NADR_LENGTH] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
  uchar bcast2_addr[CTOK_NADR_LENGTH] = { 0xC0,0x00,0xFF,0xFF,0xFF,0xFF};

  TRACE_DBG(MPS_OTHER, "RmrB", (ulong)p_dev_ctl, fr_len, buf_ptr);
  parm  = (ushort * )&o_parm.cmd;

  /* 
   * Increments counters for receive interrupts processed 
   */
  if (NDD.ndd_genstats.ndd_recvintr_lsw == ULONG_MAX) {
        NDD.ndd_genstats.ndd_recvintr_msw++;
  }
  NDD.ndd_genstats.ndd_recvintr_lsw++;

  /*
   * Gets access to the I/O bus to access I/O registers                 
   */
  ioa = (int)BUSIO_ATT(DDS.bus_id, DDS.io_base_addr);

  /*
   * If the data is less than 256 bytes then get a small mbuf instead of
   * a cluster.
   */
  if (fr_len <= (MHLEN)) {
        m = m_gethdr(M_DONTWAIT, MT_HEADER);
  } else {
        m = m_getclust(M_DONTWAIT, MT_HEADER);
  }

  if (m == NULL) {
        NDD.ndd_genstats.ndd_nobufs++;
  } else {

        /* 
	 * Copys data from adapter buffer to mbuf  
	 */
  	m->m_flags |= M_PKTHDR;
  	m->m_nextpkt = NULL;
        m->m_len = fr_len;
        mem = MTOD( m, ushort * );
        next_buf = buf_ptr;
        while (next_buf)
        {
                /* (WRITE).LAPA = Addr of first adapter receive buf */
                PIO_PUTSRX(ioa + LAPA, next_buf);
                lapd = ioa + LAPD_I;

                PIO_GETSX(lapd, &next_buf);
                PIO_GETSX(lapd, &recv_fs);
                PIO_GETSX(lapd, &buf_len);
                for (i=0; i < buf_len; i++)
                {
                        PIO_GETSX(lapd, mem++);

                }
  		if (WRK.pio_rc)
  		{
  			TRACE_BOTH(MPS_ERR,"Rmr5", p_dev_ctl, WRK.pio_rc, 0);
  			BUSIO_DET(ioa);  /* restore I/O Bus  */
        		mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
							TRUE, 0, 0);
			return(FALSE);
  		}
                count += buf_len;
        }

        if (count != fr_len) {
  		BUSIO_DET(ioa);  /* restore I/O Bus  */
                m_freem(m);
                return (fr_len - count);
        }
        m->m_pkthdr.len = m->m_len = count;

        /* Checks if broadcast or multicast */
        if (*(mtod(m, caddr_t) + 2) & MULTI_BIT_MASK) {
                if ((SAME_NADR((mtod(m, caddr_t) + 2), bcast1_addr)) |
                    (SAME_NADR((mtod(m, caddr_t) + 2), bcast2_addr)))
                {
                        TOKSTATS.bcast_recv++;
                        m->m_flags |= M_BCAST;
                } else {
                        TOKSTATS.mcast_recv++;
                        m->m_flags |= M_MCAST;
                }
        }

        /* 
	 * Sends the frame up 
         */
        (*(p_ndd->nd_receive))(p_ndd, m);

        /* 
	 * Updates status counter 
	 */
        if (p_ndd->ndd_genstats.ndd_ipackets_lsw == ULONG_MAX)
                p_ndd->ndd_genstats.ndd_ipackets_msw++;
        p_ndd->ndd_genstats.ndd_ipackets_lsw++;

        if ((ULONG_MAX - fr_len) < p_ndd->ndd_genstats.ndd_ibytes_lsw)
                p_ndd->ndd_genstats.ndd_ibytes_msw++;
        p_ndd->ndd_genstats.ndd_ibytes_lsw += fr_len;

  }

  /* 
   * Builds an adapter status block (ASB) to respond to the ARB  
   */
  o_parm.cmd     = RECEIVE_DATA;
  o_parm.retcode = 0;
  o_parm.buf_ptr = buf_ptr;

  PIO_PUTSRX(ioa + LAPE, 0x00);
  PIO_PUTSRX(ioa + LAPA, WRK.srb_address);
  for (i = 0; i < 4; i++) {
          PIO_PUTSX(ioa + LAPD_I, *(parm + i));
  }
  if (WRK.pio_rc)
  {
 	TRACE_BOTH(MPS_ERR,"Rmr6", p_dev_ctl, WRK.pio_rc, 0);
  	BUSIO_DET(ioa);  /* restore I/O Bus  */
	return;
  }

  /* 
   * Sets under Mask LISR.bit4 
   */
  PIO_PUTCX(ioa + LISR_SUM, ASB_RSP); /* SUM.LISR.bit.4 */

  BUSIO_DET(ioa);                 /* restore I/O Bus                   */
  TRACE_DBG(MPS_OTHER, "RmrE", (ulong)p_dev_ctl, fr_len, buf_ptr);
} /* end mps_mac_recv routine                                                */

