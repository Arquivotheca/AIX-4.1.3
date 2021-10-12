static char sccsid[] = "@(#)64  1.5  src/bos/kernext/mps_tok/mps_dump.c, sysxmps, bos41J, 9520B_all 5/18/95 11:24:41";
/*
 *   COMPONENT_NAME: sysxmps
 *
 *   FUNCTIONS: mps_dump
 *		mps_dump_read
 *		mps_dump_recv
 *		mps_dump_wrt
 *		mps_tx2_resetup
 *		ms2time
 *		recv_re_setup
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

#include <sys/cblock.h>
#include <sys/poll.h>
#include <sys/systemcfg.h>
#include <sys/user.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/if_802_5.h>
#include <netinet/ip.h>
#include <netinet/in.h>

#include "mps_dslo.h"
#include "mps_mac.h"
#include "mps_dds.h"
#include "mps_dd.h"
#include "tr_mps_errids.h"
#ifdef KTD_DEBUG
#include "intercept_functions.h"
#endif

#define TX_DUMP_TIMEOUT  1000          /* Dump write timeout value         */
#define ERR_RC           -1          

#define FCF_MASK        0xC0           /* mask for frame type field        */
#define MAC_TYPE        0x00           /* Medium Access Control Frame type */
/*-------------------------------------------------------------------------*/
/*                              MACROS                                     */
/*-------------------------------------------------------------------------*/


/*
 * Converts micro-second to rtcl_t time value.
 */
#define MSEC_PER_SEC    (NS_PER_SEC / NS_PER_MSEC)
ms2time(msec, tp)
int                     msec;    /*  Input timeout value         */
struct timestruc_t      *tp;     /*  Output converted time value */
{
  tp->tv_sec = msec / MSEC_PER_SEC;
  tp->tv_nsec = (msec % MSEC_PER_SEC) * NS_PER_MSEC;
} /* End of ms2time() */


/*--------------------- M P S _ D U M P _ R E A D ----------------------*/
/*                                                                      */
/*  NAME: mps_read_dump                                                 */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Processes a dump read request                                   */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Called by the dump entry point.                                 */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*                                                                      */
/*  RETURNS: a -1 if there was a failure and 0 upon success.            */
/*                                                                      */
/*----------------------------------------------------------------------*/

mps_dump_read(
mps_dev_ctl_t  *p_dev_ctl,
caddr_t          arg)
{
  struct dump_read    dump_parms;
  struct mbuf         *dump_mbuf;     /* Dump mbuf pointer             */
  ushort              reason;         /* Interrupt reason              */
  ushort              type,           /* Type of frame                 */
  		      rc;             /* General return code           */
  struct timestruc_t  current_time,   /* Keeps the current time        */
  timeout_time,                       /* Time out value                */
  temp_time;
  int                 ioa;
  ushort              sisr_status_reg = 0;/* interrupt status register */
  ushort              misr_reg = 0;  /* MISR interrupt status register */

  TRACE_SYS(MPS_RV, "drdB", p_dev_ctl, 0, 0);

  bcopy (arg, &dump_parms, sizeof(dump_parms));
  dump_mbuf = dump_parms.dump_bufread;
  dump_mbuf->m_next = NULL;
  dump_mbuf->m_flags |= M_PKTHDR;

  /*
   *  Set up time information
   */
  ms2time(dump_parms.wait_time, &temp_time);
  curtime(&current_time);
  ntimeradd(current_time, temp_time, timeout_time);

  /*
   *  Checks if adapter has set the receive interrupt bit
   */
  if (WRK.dump_MISR) {
        /*
         *  Validates that it is a receive interrupt
         */
        if ((WRK.dump_MISR & Rx_EOF) | (WRK.dump_MISR & Rx_EOB)) {
                WRK.dump_MISR = 0;
		/*
                 * Processes the receive interrupt 
		 */
                rc = mps_dump_recv(p_dev_ctl, dump_mbuf);
        }

        if (!rc) {
                return(0);
        }
  }
  /*
   *  Gets the bus attachment.
   */
  ioa = (int)BUSIO_ATT( DDS.bus_id, DDS.io_base_addr);


  /*
   *  Polls the adapter until timed out or interrupt is detected
   */
  for (;;) {

 	/*
         * Reads in the interrupt reason from adapter 
	 */
        PIO_GETSRX(ioa + SISR, &sisr_status_reg);
        if (WRK.pio_rc) {
                TRACE_BOTH(MPS_ERR, "drd3", p_dev_ctl, WRK.pio_rc, 0);
                BUSIO_DET(ioa);
                return(ERR_RC);
        }

        /*
         *  Checks if the adapter have set the system interrupt bit
         */
        if (sisr_status_reg & MISR_INT) {
                PIO_GETSRX(ioa + MISR, &misr_reg);
                /*
                 *  Validate that it is a receive interrupt
                 */
                if ((misr_reg & Rx_EOF) | (misr_reg & Rx_EOB)) {
                        /* Process the receive inter */
                        rc = mps_dump_recv(p_dev_ctl, dump_mbuf);
                }

		/*
                 * Resets the SISR & MISR interrupt 
		 */
                PIO_PUTSRX( ioa + MISR, ~misr_reg); 
                PIO_PUTSRX( ioa + SISR, ~sisr_status_reg);
                if (WRK.pio_rc) {
                        TRACE_BOTH(MPS_ERR, "drd7", p_dev_ctl, WRK.pio_rc, 0);
                        BUSIO_DET(ioa);
                        return(ERR_RC);
                }
                if (!rc) {
                        BUSIO_DET(ioa);
                        return(0);
                }
        }

        /*
         *  Checks if timed out has been reached.
         */
        if (ntimercmp(current_time, timeout_time, >)) {
                TRACE_BOTH(MPS_ERR, "drdE", p_dev_ctl, ETIMEDOUT, 0);
                BUSIO_DET(ioa);
                return (ETIMEDOUT);
        }
        curtime(&current_time);
  }

} /* End of mps_dump_read() */

/*****************************************************************************/
/*
 * NAME: mps_dump_recv
 *
 *  FUNCTION:                                                           
 *      Copy data to dump user mbuf - if it's a good frame.
 *                                                                      
 * RETURNS:
 *      TRUE  if successful
 *      FALSE on failure
*/
/*****************************************************************************/
int mps_dump_recv (
mps_dev_ctl_t  *p_dev_ctl,
struct mbuf   *dump_mbuf)                 /* Dump mbuf pointer            */
{
  volatile rx_list_t      recvlist;
  struct mbuf             *m, *m0;
  int                     i;
  uchar                   dsap;           /* DSAP                         */
  uchar                   *p_dsap;        /* DSAP address                 */
  ushort                  type,           /* Type of frame                */
                          bad_frame=FALSE,/* Invalid dump frame indicator */
                          rc,             /* General return code          */
                          macsize;        /* size of mac header           */
  struct ip               *p_ip;          /* Pointer to IP header         */
  struct ie2_llc_snaphdr  *p_llc;         /* Pointer to LLC header        */
  struct ie5_mac_hdr      *p_mac;         /* Pointer to MAC header        */
  struct ie5_arp          *p_arp;         /* Pointer to ARP information   */

  TRACE_SYS(MPS_RV, "drdB", p_dev_ctl, 0, 0);

while (TRUE) {
  	i = WRK.read_index;             /* start at next receive list   */

  	if (WRK.iocc) {
  		/*
  		 *  D_move the receive list image from the IOCC cache to main
   		 *  memory (local image) pointed to by rlptr.
   		 */
  		d_kmove (&recvlist, WRK.recv_list[i], (uint)RX_LIST_SIZE,
      			WRK.dma_channel, DDS.bus_id, DMA_READ); 
  	} else {
       		bcopy(WRK.recv_vadr[i], &recvlist, (uint)RX_LIST_SIZE);
  	}

  	recvlist.recv_status  = toendianL(recvlist.recv_status);
  	recvlist.fr_len       = toendianW(recvlist.fr_len);

  	/*
  	 * Checks if any more buffer is being process
  	 */
  	if (!(recvlist.recv_status & BUF_PROCESS)) {
          	break;
  	}

  	/*
  	 * Checks if this is a good frame to pass on
  	 */
  	m = WRK.recv_mbuf[i];
  	m->m_len = toendianW(recvlist.data_len); /* buffer len */
  	bad_frame = FALSE;
  	if ((recvlist.recv_status & RX_ERR & PROTOCOL_ERR1) ||
      		(recvlist.recv_status & PROTOCOL_ERR1) ||
      		(recvlist.recv_status & PROTOCOL_ERR2)) {

		/*
         	* Bad data 
	 	*/
        	TRACE_BOTH(MPS_ERR, "ddr1", p_dev_ctl, recvlist.recv_status, 0);
        	rc =  d_complete(WRK.dma_channel, DMA_READ, MTOD( m, uchar *),
               	          m->m_len, &WRK.rx_xmemp[i], WRK.recv_addr[i]);
        	if (rc != DMA_SUCC) {
                	mps_logerr(p_dev_ctl, ERRID_MPS_DMAFAIL, __LINE__, 
					__FILE__, WRK.dma_channel, m, rc);
        	}

        	discard_packet(p_dev_ctl);
        	bad_frame = TRUE;

  	} else {
		/*
        	 * Good data.  Copys data from d_master'ed mbuf to the new one    
		 *
        	 *  Copy the data and update the index into the dump
        	 *  mbuf data area.
        	 *
        	 *  NOTE: normally the cache_inval would be done after
        	 *  the bcopy.  The dump will include everything which
        	 *  is in memory INCLUDING the TR DD receive buffers.
        	 *  If the cache_inval is done after the bcopy, when
        	 *  the dump DD copies the TR DD receive buffer to its
        	 *  transmit mbuf that action caused the buffer to be
        	 *  read into the memory cache.  The TR adapter will
        	 *  update system memory, but not the memory cache.
        	 *  When the TR DD reads the receive buffer, it will
        	 *  get the data from the PREVIOUS reception instead of
        	 *  the current data.
        	 */
       	 	cache_inval(mtod(m, caddr_t), m->m_len);
        	bcopy(mtod(m, caddr_t), mtod(dump_mbuf, caddr_t), m->m_len);
        	dump_mbuf->m_len = m->m_len;

        	/*
        	 * Re-arm the receive list and pass the existing mbuf 
        	 * to be used again to avoid doing another d_master.
        	 */
       	 	arm_recv_list(p_dev_ctl, i, FALSE, m);
        	WRK.read_index = (WRK.read_index + 1) % MAX_RX_LIST;

        	/*
        	 *  If frame is NOT to an individual address, it is a bad frame
        	 */
       	 	if (*(uchar *)((dump_mbuf->m_data)+2) & MULTI_BIT_MASK) {
                	bad_frame = TRUE;
		}

      	 	/*
       		 *  If frame is a mac frame then mark it as a bad frame
       		 */
      		 p_mac = MTOD(dump_mbuf, struct ie5_mac_hdr *);
      		 if ((p_mac->mac_fcf & FCF_MASK) == MAC_TYPE) {
          	 	bad_frame = TRUE;
       		 }
       		macsize = mac_size(p_mac);

       		/*
       		 *  Filter off all frames that are not to AA SAP
        	 */
       		p_dsap = MTOD(dump_mbuf, uchar *);
       		dsap = *(uchar *)(p_dsap + macsize);
       		if (dsap != 0xaa) {
           		bad_frame = TRUE;
       		}

       		TRACE_DBG(MPS_OTHER, "drd2", dump_mbuf, bad_frame, dsap);
       		/*
        	 *  If the frame was bad then drop frame and read in
       		 *     next frame, otherwise process frame.
       		 */
       		if (!bad_frame) {
           		/*
           		 * Skips MAC hdr, LLC hdr, and SNAP hdr
           		 */
           		dump_mbuf->m_len = dump_mbuf->m_len - (macsize +
                        		sizeof(struct ie2_llc_snaphdr) );
           		dump_mbuf->m_pkthdr.len = dump_mbuf->m_len;
           		dump_mbuf->m_data = (caddr_t)p_mac + macsize +
                        	   	sizeof(struct ie2_llc_snaphdr);

  			TRACE_BOTH(MPS_ERR, "drdE", p_dev_ctl, bad_frame, 0);
  			return (0);
        	} else {
  			TRACE_BOTH(MPS_ERR, "drd3", p_dev_ctl, bad_frame, 0);
		}

  	}
   } /* end of while */
} /* end of mps_dump_recv */

/*****************************************************************************/
/*
 * NAME: mps_dump_wrt
 *
 * FUNCTION: write function for kernel
 *
 * RETURNS:
 *      0 if successful
 *      errno value on failure
 */
/*****************************************************************************/
int mps_dump_wrt ( ndd_t          *p_ndd,
                   struct mbuf    *p_mbuf)
{
  mps_dev_ctl_t         *p_dev_ctl;
  int                   i,rc,ioa;
  int                   count = 0;
  int                   room = 0;
  volatile   tx_list_t  xmitlist;
  struct     mbuf       *buf_to_free;
  struct     mbuf       *mbufp;
  struct     mbuf       *p_last;
  int                   ipri;
  ushort                sisr_status_reg =0; /* interrupt status register    */
  ushort                Stat;
  ulong                 DBA;
  ulong                 BDA;
  ulong                 FDA;
  ushort                misr_reg = 0;       /*MISR interrupt status register*/
  struct timestruc_t    current_time,       /* Keeps the current time       */
                        timeout_time,       /* Timed out value              */
                        temp_time;

  TRACE_SYS(MPS_TX, "ddwB", p_dev_ctl, 0, 0);

    /*
     *  Get adapter structure
     */
    p_dev_ctl = (mps_dev_ctl_t *) (((unsigned long) p_ndd) - 
						offsetof(mps_dev_ctl_t, ndd));


  i = WRK.tx2_elem_next_in;

  /*
   * Copys data into tranmit buffer and do processor cache flush 
   */
  MBUF_TO_MEM(p_mbuf, WRK.tx2_buf[i]);
  rc = vm_cflush(WRK.tx2_buf[i], p_mbuf->m_len);

  /*
   * Set up buffer table entry                                  
   */
  xmitlist.fw_pointer   = toendianL((ulong)WRK.tx2_list[(i + 1) % MAX_TX_LIST]);
  xmitlist.xmit_status  = 0;
  xmitlist.buf_count    = 0x0100;
  xmitlist.frame_len    = 0;
  xmitlist.xbuf[0].data_pointer = toendianL((ulong)WRK.tx2_addr[i]);
  xmitlist.xbuf[0].buf_len      = toendianW((ushort)p_mbuf->m_len);

  if (WRK.iocc) {
  	d_kmove(&xmitlist, WRK.tx2_list[i], tx_desc_len[1],
      		WRK.dma_channel, DDS.bus_id, DMA_WRITE_ONLY);
  } else {
        bcopy(&xmitlist, WRK.tx2_vadr[i], tx_desc_len[1]);
  }

  ioa = (int)BUSIO_ATT( DDS.bus_id, DDS.io_base_addr);
  PIO_PUTLRX(ioa + Tx2LFDA_L, (int)WRK.tx2_list[i]);
  if (WRK.pio_rc) {
          TRACE_BOTH(MPS_ERR, "ddw1", p_dev_ctl, WRK.pio_rc, 0);
          BUSIO_DET(ioa);
          return(ERR_RC);
  }

  XMITQ_INC(WRK.tx2_elem_next_in);

  ms2time(TX_DUMP_TIMEOUT, &temp_time);
  curtime(&current_time);
  ntimeradd(current_time, temp_time, timeout_time);

  /*
   *  Polls the adapter until timed out or interrupt is detected
   */
  for (;;) {
        PIO_GETSRX(ioa + SISR, &sisr_status_reg);
        if (WRK.pio_rc) {
                TRACE_BOTH(MPS_ERR, "ddw2", p_dev_ctl, WRK.pio_rc, 0);
                BUSIO_DET(ioa);
                return(ERR_RC);
        }

        /*
         *  Checks if the adapter has set the system interrupt bit
         */
        if (sisr_status_reg & MISR_INT) {
                PIO_GETSRX(ioa + MISR, &misr_reg);
                /*****************************************************/
                /* Adapter Tx interrupt has occurred - channel 2     */
                /*****************************************************/
                /* Checks if a transmit 2 interrupt have occurred    */
                if (misr_reg & XMIT_DONE_MSK_2) {

                        if (misr_reg & 0xFF) {
                                WRK.dump_MISR |= misr_reg & 0xFF;
			}

               		PIO_PUTSRX( ioa + MISR, ~misr_reg);
                	PIO_PUTSRX( ioa + SISR, ~sisr_status_reg)
                        BUSIO_DET(ioa);
                        return(0);

                } /* endif for transmit 2 interrupt               */

                if (misr_reg & 0xFF) {
                        WRK.dump_MISR |= misr_reg & 0xFF;
		}

		/*
                 * Resets the MISR & SISR interrupt 
		 */
               	PIO_PUTSRX( ioa + MISR, ~misr_reg);
        } /*  end of if (sisr_status_reg & MISR_INT) */

        PIO_PUTSRX( ioa + SISR, ~sisr_status_reg)
        if (WRK.pio_rc) {
               	TRACE_BOTH(MPS_ERR,"ddw4",p_dev_ctl,WRK.pio_rc,0);
               	BUSIO_DET(ioa);
               	return(ERR_RC);
        }

        /*
         *  Checks if timed out has been reached.
         */
        if (ntimercmp(current_time, timeout_time, >)) {
                BUSIO_DET(ioa);       /* restore I/O Bus             */
                TRACE_BOTH(MPS_ERR, "ddwE", p_dev_ctl, 0, 0);
                return (ETIMEDOUT);
        }
        curtime(&current_time);

  } /* End of for(;;) */
}

/************************************************************************/
/*                                                                      */
/* NAME:        mps_dump                                                */
/*                                                                      */
/* FUNCTION:    Adapter Driver Dump Routine                             */
/*                                                                      */
/*      This routine handles requests for dumping data to a previously  */
/*      opened device.                                                  */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine is called when there is certain to be limited      */
/*      functionality available in the system.  However, system         */
/*      dma kernel services are available.  The passed data is already  */
/*      pinned by the caller.  There are no interrupt or timer kernel   */
/*      services available.  This routine should run at INTMAX level.   */
/*                                                                      */
/* NOTES:                                                               */
/*      The driver should ignore extra dumpinit and dumpstart calls.    */
/*      The driver must check for the availability of internal          */
/*      resources, as these are not guaranteed to be available          */
/*      when the dump routine is called, but the system will have       */
/*      tried to quiesce itself before making the first dump call.      */
/*      Any lack of resources, or error in attempting to run any        */
/*      command, is considered fatal in the context of the dump.        */
/*      It is assumed that normal operation will not continue once      */
/*      this routine has been executed.  Once the DUMPWRITE logic has   */
/*      been executed, in fact, it will be impossible to use the        */
/*      driver's normal path successfully.                              */
/*                                                                      */
/*                                                                      */
/* INPUTS:                                                              */
/*      p_ndd   - pointer to the ndd in the dev_ctl area                */
/*      cmd     - parameter specifying the dump operation               */
/*      arg     - pointer to command specific structure                 */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      A zero will be returned on successful completion, otherwise,    */
/*      one of the errno values listed below will be given.             */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      0       - successful completion                                 */
/*      ENXIO   - kernel service failure, not opened, or general        */
/*                failure running DUMPWRITE option.                     */
/*      EACCES  - user is not a trusted user                            */
/*      EINVAL  - invalid request                                       */
/*      ETIMEDOUT - the DUMPREAD option timed-out                       */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      i_disable       i_enable                                        */
/*                                                                      */
/************************************************************************/
int
mps_dump(
ndd_t         *p_ndd,         /* pointer to the ndd in the dev_ctl area */
int           cmd,            /* control command */
int           arg)            /* address of parameter block */
{
  mps_dev_ctl_t         *p_dev_ctl = (mps_dev_ctl_t *)(p_ndd->ndd_correlator);
  int                   rc = 0;  /* Exit code              */
  int                   pio_attachment;/* PIO attachment         */
  struct dmp_query      dump_ptr;      /* Dump info. structure   */
  struct dump_read      dump_readp;    /* Dump info. structure   */
  int                   mps_dump_wrt();
  int                   dump_pri, ioa;

  TRACE_SYS(MPS_OTHER, "ddpB", p_dev_ctl, cmd, 0);

  /*
   *  Processes dump command
   */
  switch (cmd) {

  case DUMPINIT:
        break;

  case DUMPQUERY:
        dump_ptr.min_tsize = CTOK_MIN_PACKET;
        dump_ptr.max_tsize = H_PAGE;
        bcopy(&dump_ptr,arg,sizeof(struct dmp_query));
        break;

  case DUMPSTART:
  	/*
  	 * Set up variables for Transmit list 
  	 */
  	WRK.tx2_elem_next_out = 0;
  	WRK.tx2_elem_next_in  = 0;
  	WRK.tx2_frame_pending = 0;

        NDD.ndd_output = mps_dump_wrt;

        ioa = (int)BUSIO_ATT( DDS.bus_id, DDS.io_base_addr);
        PIO_PUTSRX( ioa + MISR, 0);
        PIO_PUTSRX( ioa + SISR, 0);
	if (WRK.pio_rc) {
       		TRACE_BOTH(MPS_ERR, "ddp3", p_dev_ctl, WRK.pio_rc, 0);
               	BUSIO_DET(ioa);
               	return(EINVAL);
	}
        BUSIO_DET(ioa);
        WRK.dump_started   = TRUE;
        break;

  case DUMPREAD:
        /*
         *  Calls internal routine to execute the command
         */
        if (!WRK.dump_started) {
                rc = EINVAL;
        } else {
                rc = 0;
                rc = (int)mps_dump_read(p_dev_ctl, (uchar *)arg);
                if (rc) {
                        return(EIO);
		}
        }
        break;

  case DUMPEND:
        if (!WRK.dump_started) {
                rc = EINVAL;
	} else {
                WRK.dump_started = FALSE;
        	p_dev_ctl->device_state = DEAD;
	}
        break;

  case DUMPTERM:
        break;

  default:
        rc = EINVAL;
        break;

  }     /* end switch (cmd) */

  TRACE_SYS(MPS_OTHER, "ddpE", p_dev_ctl, rc, 0);
  return (rc);

}  /* End of mps_dump */

