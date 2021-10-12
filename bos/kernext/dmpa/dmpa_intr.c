static char sccsid[] = "@(#)72        1.1  src/bos/kernext/dmpa/dmpa_intr.c, sysxdmpa, bos411, 9428A410j 4/30/93 12:49:01";
/*
 *   COMPONENT_NAME: (SYSXDMPA) MP/A DIAGNOSTICS DEVICE DRIVER
 *
 *   FUNCTIONS: get_rx_result
 *		mpa_offlvl
 *		mpaintr
 *		q_irpt_results
 *		
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

#include <errno.h>
#include <sys/adspace.h>
#include <sys/errids.h>
#include <sys/device.h>
#include <sys/dma.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/malloc.h>
#include <sys/dmpauser.h>
#include <sys/dmpadd.h>
#include <sys/poll.h>
#include <sys/mbuf.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/xmem.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>


/*
 * NAME: mpaintr
 *
 * FUNCTION: This procedure is the interrupt handler.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment-Specific aspects, such as -
 *      Preemptable        : No
 *      VMM Critical Region: Yes
 *      Runs on Fixed Stack: Yes
 *      May Page Fault     : Yes
 *      May Backtrack      : Yes
 *
 * NOTES: This procedure checks the level and task registers to verify that
 *        this is our interrupt and processes it
 *
 * DATA STRUCTURES: external struct acb
 *
 * RETURN VALUE DESCRIPTION: Either INTR_SUCC (interrupt handled successfully)
 *                           or INTR_FAIL (interrput was not ours
 */

int mpaintr ( struct acb *acb )

{
   int             statreg=0;
   recv_elem_t     *recvp;
   xmit_elem_t     *xmitp;
   int             rc,cntr=0,count,i;
   uchar           wait_code=0;
   ulong           pos;
   int             byte_xmit = 0;
   int             byte_recv = 0;
   uchar           byte_sent;
   uchar           byte_read;

   /* check the status reg for this interrupts card. */
   while ( ++cntr < 2) {
      if( (statreg = PIO_GETC(acb, RD_STAT_OFFSET) ) == -1) {
	 /* check the pos2 settings to make sure they are correct */
	 /* there seems to be a problem with setting pos regs, the */
	 /* value is not always stored as it should be, so if this */
	 /* pio fails retry setting the pos reg and try the pio    */
	 /* again.                                                 */

	 pos = MPA_IOCC_ATT;

	 /* try to put pos reg back right */
	 BUS_PUTC( pos + POS2, acb->pos2 );

	 IOCC_DET(pos);
      }
      else break;
   }
   if(cntr==2) {
	 /*
	 ** If it gets to here you are hung anyway, so stop the machine.
	 */
	 panic("MPA pos reg 2 cannot be set correctly");
   }

   cntr = 0;
   DDHKWD3 (HKWD_DD_MPADD, DD_ENTRY_INTR,0,acb->dev,statreg,acb->caih_struct.level);

   if ( statreg&IRPT_PENDING ) {
      /* There are two possible causes for the irpt
       * 1. Receive needs service.
       *    a. results from previous receive available
       *    b. In PIO MODE receiver has data for me
       * 2. Transmit needs service
       *    a. results from previous transmit available
       *    b. In PIO MODE xmitter wants data from me
       *
       *   I will read the results and store them in struct
       * then attach the struct to a chain that is processed
       * by the off level handler. This will free up the
       * card an prevent irpt overruns and still get me out
       * of this irpt handler in acceptable time.
       *   When PIO is used for xfer, and a transfer is required
       * I must do it withing this handler to get the cards irpt
       * line to drop, otherwise I get in a irpt loop.
      */

      ++acb->stats.ds.total_intr;   /* increment int. count */

      /*
       * Check recv irpt bit first every time, if its active,
       * handle the recv irpt and wait for the recv irpt bit to
       * drop on the card before exiting irpt handler. If there is
       * no recv irpt check for xmit irpt and handle it. If there
       * are no irpts log error and exit fail.
      */
      if(statreg & (wait_code=RX_IRPT_ACTIVE) ) {
	  ++acb->stats.ds.recv_intr_cnt;

	  if( (statreg & RX_RESULT_READY) ) {
	      q_irpt_results(acb,RECV_RESULT);
	  }
	  else {

	      if(acb->flags&PIO_MODE) {
		 /*
		 ** read the next byte from the receiver if there is
		 ** a valid recv element.
		 */
		 if( (recvp = acb->act_recv_head) != NULL) {
		     if( (rc=PIO_GETC( acb, RD_DATA_OFFSET)) == -1) {
			  acb->flags |= RC_DISCARD;
		     }
		     byte_read = (uchar) rc;
		     byte_recv = 1;
		     recvp->rc_data[recvp->rc_index] = (uchar) rc;
		     ++acb->stats.ds.recv_pio_byte;
		     ++recvp->rc_index;
		 }
	      }
	      else {
		   ++acb->stats.ds.recv_irpt_error;
	      }
	  }
      }
      /*
       * If xmit irpt is active and there are no results ready ,
       * this is an error, unless I am in PIO MODE.
       * log it and continue on to check the  recvp irpt or handle
       * the reqired PIO xfer.
      */
      else {
	  wait_code=TX_IRPT_ACTIVE;
	  ++acb->stats.ds.xmit_intr_cnt;

	  if( (statreg & TX_RESULT_READY) ) {
	      q_irpt_results(acb,XMIT_RESULT);
	  }
	  else {
	      if(acb->flags&PIO_MODE) {
		 /*
		 ** write the next byte to transmitter if there is
		 ** a valid xmit element on the active q.
		 */
		 if( (xmitp = acb->act_xmit_head) != NULL) {
		      if((rc=PIO_PUTC( acb, WR_DATA_OFFSET,
			   (uchar) xmitp->xm_data[xmitp->xm_index])) == -1) {
			   acb->flags |= XM_DISCARD;
		      }
		      byte_sent = (uchar) xmitp->xm_data[xmitp->xm_index];
		      byte_xmit = 1;
		      ++xmitp->xm_index;
		      ++acb->stats.ds.xmit_pio_byte;
		 }
	      }
	      else {
		  ++acb->stats.ds.xmit_irpt_error;
	      }
	  }
      }

      /*
      ** It seems the MPA hardware is a bit slow in dropping the
      ** irpt and status lines, there for I am putting in a loop
      ** to wait for all lines to drop before I clear this irpt.
      ** This should insure that I don't get invalid interrupts.
      */

      statreg = 0x0F;
      while( (statreg&wait_code) && (++cntr<10) ) {
	  if( (statreg=PIO_GETC( acb, RD_STAT_OFFSET )) == -1)  {
	       panic("MPA irpt handler pio read statreg failed");
	  }
	  /*
	  **  If the count exceeds 2 an iprt is hung so try to
	  **  reset it here. There is a problem with the 8273
	  **  that can cuase the xmit irpt to hang so do dummy
	  **  write to data out and dummy read from TX result.
	  */
	  if ( (cntr==4) && (wait_code==TX_IRPT_ACTIVE) ) {
	      if(PIO_PUTC(acb, WR_DATA_OFFSET,0x00) == -1) {
		panic("MPA irpt handler pio write data failed");
	      }
	      if(PIO_GETC( acb, RD_TX_IR_OFFSET ) == -1) {
		panic("MPA irpt handler pio read tx result failed");
	      }
	  }
	  /*
	  ** When the irpt was a receive irpt there is no
	  ** need to wait as long. It could be a valid receive irpt
	  ** waiting to be taken.
	  */
	  else if ( (cntr==4) && (wait_code==RX_IRPT_ACTIVE) ) break;
      }
      if(byte_xmit) {
	 DDHKWD3 (HKWD_DD_MPADD, DD_EXIT_INTR, 2,acb->dev,byte_sent,cntr);
      }
      else if(byte_recv) {
	 DDHKWD3 (HKWD_DD_MPADD, DD_EXIT_INTR, 1,acb->dev,byte_read,cntr);
      }
      else
	 DDHKWD3 (HKWD_DD_MPADD, DD_EXIT_INTR, 0,acb->dev,statreg,cntr);

      if(acb->flags & IRPT_QUEUED) {
	  /*
	  ** schedule an offlevel to handle the queued irpt and
	  ** reset the IRPT_QUEUED flag.
	  */
	  i_sched((struct intr *)&(acb->ofl));
	  acb->flags &= ~IRPT_QUEUED;

      }

      /* reset with irpt struct of card just serviced */
      i_reset(&acb->caih_struct);
      ++acb->stats.ds.irpt_succ;

      return (INTR_SUCC);
   }
   if( !(acb->flags & STARTED_CIO) && (++acb->stats.ds.irpt_fail > 50) ) {
	     /* kill the card, mark it dead and exit */
	     if(PIO_PUTC( acb, PORT_C_8255, 0x0C )==-1) {
		 panic("2. Tried to kill the irpt on the card");
	     }
	     acb->flags |= MPADEAD;
	     i_reset(&acb->caih_struct);
	     DDHKWD3 (HKWD_DD_MPADD, DD_EXIT_INTR, 0,acb->dev,statreg,cntr);
	     return (INTR_SUCC);
   }

   DDHKWD3 (HKWD_DD_MPADD, DD_EXIT_INTR, 1,acb->dev,statreg,0);
   return ( INTR_FAIL ); /* not our interrupt, tell the FLIH */

}            /* mpaintr() */

/*
 * NAME: mpa_offlvl
 *
 * FUNCTION: High level description of what the procedure does
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment-Specific aspects, such as -
 *      Preemptable        : Maybe
 *      VMM Critical Region: Yes
 *      Runs on Fixed Stack: Yes
 *      May Page Fault     : no
 *      May Backtrack      : Yes
 *
 * NOTES: More detailed description of the function, down to
 *          what bits / data structures, etc it manipulates.
 *
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.
 *
 * RETURN VALUE DESCRIPTION: NONE
 */

int mpa_offlvl (struct intr *ofl_ptr)
{
    struct acb              *acb;        /* interrupting adapter's acb */
    int                     rc=0,cnt=0;
    irpt_elem_t             *irptp;
    dma_elem_t              *dmap;
    recv_elem_t             *recvp;
    xmit_elem_t             *xmitp;
    struct mbuf             *mbufp;
    cio_read_ext_t          rd_ext;
    uchar                   result;
    int                     recv_bytes = 0;

    /*
    ** Get pointer acb for interrupting adapter. Since the sturct intr
    ** ofl is the the second struct intr in the acb and the first struct
    ** intr is the begining of the acb, by decrementing the ofl_ptr by
    ** one struct intr I get back to the begining of the address of the
    ** acb.
    */
    acb = (struct acb *)(--ofl_ptr);        /* decrement to acb addr */

    DDHKWD2 (HKWD_DD_MPADD, DD_ENTRY_OFFL,0,acb->dev,0);
    /*
    ** Process all elements on the active irpt q
    */

    while( (irptp=acb->act_irpt_head) != NULL) {
	++cnt;
	switch(irptp->type_flag) {

	   case RECV_RESULT:

		/* check for good rc on recv, if bad report error */
		/* and discard the recv data.                     */
		/* If good set flag WAIT_IDLE, When idle detected */
		/* notify the user of data ready then restart the */
		/* recv command.                                  */

		/*
		** I need to & out bits in last xfer part of the RIC
		** and 0 the flags used. NOTE: RC_WAIT_IDLE is only
		** reset when the idle detect irpt is received.
		*/
		result = (irptp->tp.rcv.RIC & RIC_MASK_LAST_BYTE);

		   /*
		   ** If an error occured in the irpt handler the
		   ** discard flag would be set and this receive is
		   ** already doomed so just throw it away.
		   */
		if(!(acb->flags&RC_DISCARD) && !(acb->flags&IRPT_PIO_ERR) ) {

		   switch(result) {

		     case RECV_GEN_OK:
		     case RECV_SEL_OK:
       /*                 acb->flags |= RC_WAIT_IDLE;  */
			  acb->flags |= RC_GET_PTRS;
			  acb->flags |= RC_SAVE_CNT;
			  acb->flags |= RC_FREE_DMA;
			  acb->flags |= RC_START_RECV;
			  acb->flags |= RC_TAP_USER;
			  ++acb->stats.ds.recv_completes;
			  break;
		     /*
		     ** following is the set of recv results that do
		     ** cause the recv data to be lost.
		     */
		     case RECV_CRC_ERR:
			  ++acb->stats.ds.recv_crc_errors;
			  acb->flags |= RC_DISCARD;
			  acb->flags |= RC_GET_PTRS;
			  acb->flags |= RC_FREE_DMA;
			  acb->flags |= RC_FREE_RECV;
			  acb->flags |= RC_START_RECV;
			  break;
		     case RECV_ABORTED:
			  ++acb->stats.ds.recv_aborts;
			  acb->flags |= RC_DISCARD;
			  acb->flags |= RC_GET_PTRS;
			  acb->flags |= RC_FREE_DMA;
			  acb->flags |= RC_FREE_RECV;
			  acb->flags |= RC_START_RECV;
			  break;
		     case RECV_BAD_FRAME:
			  ++acb->stats.ds.recv_frame_to_small;
			  acb->flags |= RC_DISCARD;
			  acb->flags |= RC_GET_PTRS;
			  acb->flags |= RC_FREE_DMA;
			  acb->flags |= RC_FREE_RECV;
			  acb->flags |= RC_START_RECV;
			  break;

			  /* The receiver is disabled */
			  /* after the following.     */

		     case RECV_DMA_OVERRUN:
			  ++acb->stats.ds.recv_dma_overruns;
			  acb->flags |= RC_DISCARD;
			  acb->flags |= RC_GET_PTRS;
			  acb->flags |= RC_FREE_DMA;
			  acb->flags |= RC_FREE_RECV;
			  acb->flags |= RC_START_RECV;
			  break;
		     case RECV_MEM_OVERFLOW:
			  ++acb->stats.ds.recv_buf_overflow;
			  acb->flags |= RC_DISCARD;
			  acb->flags |= RC_GET_PTRS;
			  acb->flags |= RC_FREE_DMA;
			  acb->flags |= RC_FREE_RECV;
			  acb->flags |= RC_START_RECV;
			  break;
		     case RECV_CARRIER_DOWN:
			  ++acb->stats.ds.recv_cd_failure;
			  acb->flags |= RC_DISCARD;
			  acb->flags |= RC_GET_PTRS;
			  acb->flags |= RC_FREE_DMA;
			  acb->flags |= RC_FREE_RECV;
			  acb->flags |= RC_START_RECV;
			  break;
		     case RECV_PIO_ERROR:
			  ++acb->stats.ds.io_irpt_error;
			  acb->flags |= RC_GET_PTRS;
			  acb->flags |= RC_FREE_DMA;
			  acb->flags |= RC_START_RECV;
			  acb->flags |= RC_TAP_USER;
			  acb->flags &= ~RC_WAIT_IDLE;
			  break;
		     /*
		     ** following is the set of recv results that do not
		     ** cause the recv data to be lost. But the receiver
		     ** is disabled and must be restarted.
		     */
		     case RECV_IDLE:
			  ++acb->stats.ds.recv_idle_detects;
			   if(acb->station_type & PRIMARY) {
			      acb->cmd_parms.cmd=GEN_RECEIVE_CMD;
			      acb->cmd_parms.parm[0]=MAX_FRAME_SIZE;
			      acb->cmd_parms.parm[1]=(MAX_FRAME_SIZE>>8);
			      acb->cmd_parms.parm_count=2;
			   }
			   else {
			      acb->cmd_parms.cmd=SEL_RECEIVE_CMD;
			      acb->cmd_parms.parm[0]=MAX_FRAME_SIZE;
			      acb->cmd_parms.parm[1]=(MAX_FRAME_SIZE>>8);
			      acb->cmd_parms.parm[2]=acb->station_addr;
			      acb->cmd_parms.parm[3]=acb->station_addr;
			      acb->cmd_parms.parm_count=4;
			   }
			   if( (rc=que_command(acb)) ) break;
			  acb->flags |= RECEIVER_ENABLED;
			  DDHKWD2(HKWD_DD_MPADD, MPA_RECV_ENAB, 0, acb->dev,rc);
			  break;
		     case RECV_EOP:
			  ++acb->stats.ds.recv_eop_detects;
			   if(acb->station_type & PRIMARY) {
			      acb->cmd_parms.cmd=GEN_RECEIVE_CMD;
			      acb->cmd_parms.parm[0]=MAX_FRAME_SIZE;
			      acb->cmd_parms.parm[1]=(MAX_FRAME_SIZE>>8);
			      acb->cmd_parms.parm_count=2;
			   }
			   else {
			      acb->cmd_parms.cmd=SEL_RECEIVE_CMD;
			      acb->cmd_parms.parm[0]=MAX_FRAME_SIZE;
			      acb->cmd_parms.parm[1]=(MAX_FRAME_SIZE>>8);
			      acb->cmd_parms.parm[2]=acb->station_addr;
			      acb->cmd_parms.parm[3]=acb->station_addr;
			      acb->cmd_parms.parm_count=4;
			   }
			   if( (rc=que_command(acb)) ) break;
			  acb->flags |= RECEIVER_ENABLED;
			  DDHKWD2(HKWD_DD_MPADD, MPA_RECV_ENAB, 0, acb->dev,rc);
			  break;
		     case RECV_IRPT_OVERRUN:
			  ++acb->stats.ds.recv_irpt_overruns;
			   if(acb->station_type & PRIMARY) {
			      acb->cmd_parms.cmd=GEN_RECEIVE_CMD;
			      acb->cmd_parms.parm[0]=MAX_FRAME_SIZE;
			      acb->cmd_parms.parm[1]=(MAX_FRAME_SIZE>>8);
			      acb->cmd_parms.parm_count=2;
			   }
			   else {
			      acb->cmd_parms.cmd=SEL_RECEIVE_CMD;
			      acb->cmd_parms.parm[0]=MAX_FRAME_SIZE;
			      acb->cmd_parms.parm[1]=(MAX_FRAME_SIZE>>8);
			      acb->cmd_parms.parm[2]=acb->station_addr;
			      acb->cmd_parms.parm[3]=acb->station_addr;
			      acb->cmd_parms.parm_count=4;
			   }
			   if( (rc=que_command(acb)) ) break;
			  acb->flags |= RECEIVER_ENABLED;
			  DDHKWD2(HKWD_DD_MPADD, MPA_RECV_ENAB, 0, acb->dev,rc);
			  break;

		     default:
			  acb->flags |= RC_DISCARD;
			  acb->flags |= RC_GET_PTRS;
			  acb->flags |= RC_FREE_DMA;
			  acb->flags |= RC_FREE_RECV;
			  acb->flags |= RC_START_RECV;
			  break;
		   }   /* end of switch based on result */
		}
		else {
		     acb->flags |= RC_DISCARD;
		     acb->flags |= RC_GET_PTRS;
		     acb->flags |= RC_FREE_DMA;
		     acb->flags |= RC_FREE_RECV;
		     acb->flags |= RC_START_RECV;
		     acb->flags &= ~IRPT_PIO_ERR;
		}
		if(acb->flags&RC_GET_PTRS) {
		       acb->flags &= ~RC_GET_PTRS;
		       if(acb->flags&PIO_MODE) {
			    if( (recvp=acb->act_recv_head) == NULL ) {
			       ++acb->stats.ds.recv_not_handled;
			       acb->flags &= ~RECV_DMA_ON_Q;
			       DDHKWD5 (HKWD_DD_MPADD, DD_EXIT_OFFL,1,acb->dev,RECV_RESULT,
				   result,recv_bytes,0xF1);
			       break;
			    }
		       }
		       else {
			    if( (dmap=acb->act_dma_head) == NULL ) {
			       ++acb->stats.ds.recv_not_handled;
			       acb->flags &= ~RECV_DMA_ON_Q;
			       DDHKWD5 (HKWD_DD_MPADD, DD_EXIT_OFFL,1,acb->dev,RECV_RESULT,
				   result,recv_bytes,0xF2);
			       break;
			    }
			    if( dmap->dm_req_type!=DM_RECV ) {
			       ++acb->stats.ds.recv_not_handled;
			       acb->flags &= ~RECV_DMA_ON_Q;
			       DDHKWD5 (HKWD_DD_MPADD, DD_EXIT_OFFL,1,acb->dev,RECV_RESULT,
				   result,recv_bytes,0xF3);
			       break;
			    }
			    if( (recvp = dmap->p.recv_ptr) == NULL)  {
			       ++acb->stats.ds.recv_not_handled;
			       acb->flags &= ~RECV_DMA_ON_Q;
			       DDHKWD5 (HKWD_DD_MPADD, DD_EXIT_OFFL,1,acb->dev,RECV_RESULT,
				   result,recv_bytes,0xF4);
			       break;
			    }
		       }
		       recv_bytes = recvp->rc_count;
		}
		if(acb->flags&RC_SAVE_CNT) {
		       acb->flags &= ~RC_SAVE_CNT;
		       if(irptp->tp.rcv.R1==0xFF) {
			   async_status(acb, CIO_ASYNC_STATUS,
				   CIO_LOST_DATA,irptp->tp.rcv.RIC,2,0xff);
			   free_recv_elem(acb, recvp);

			   if(acb->flags&PIO_MODE) {
			       acb->flags &= ~RECEIVER_ENABLED;
			       acb->hold_recv = NULL;
			   }
			   else  free_dma_elem(acb,dmap);

			   ++acb->stats.ds.recv_lost_data;
			   DDHKWD5 (HKWD_DD_MPADD, DD_EXIT_OFFL,1,acb->dev,RECV_RESULT,
				   result,recv_bytes,0xF5);
			   break;
		       }
		       /*
		       **  Set the actual length of the recv.
		       */
		       recvp->rc_count = irptp->tp.rcv.R1;
		       recvp->rc_count = (recvp->rc_count<<8)|irptp->tp.rcv.R0;
		       recv_bytes = recvp->rc_count;
		}
		if(acb->flags&RC_DISCARD) {
		       acb->flags &= ~RC_DISCARD;
		       async_status(acb, CIO_ASYNC_STATUS,
				 CIO_LOST_DATA,irptp->tp.rcv.RIC,2, 0);
		       ++acb->stats.ds.recv_lost_data;
		}
		if(acb->flags&RC_FREE_DMA) {
		       acb->flags &= ~RC_FREE_DMA;
		       if(acb->flags&PIO_MODE) {
			   acb->flags &= ~RECEIVER_ENABLED;
			   acb->hold_recv = NULL;
		       }
		       else  free_dma_elem(acb,dmap);
		}
		if(acb->flags&RC_FREE_RECV) {
		       acb->flags &= ~RC_FREE_RECV;
		       free_recv_elem(acb,recvp);
		}
		if(acb->flags&RC_TAP_USER) {
		       acb->flags &= ~RC_TAP_USER;
		       recvp->rc_state = RC_COMPLETE;
		       /*
		       ** Update statistics counters.
		       */
		       if (++acb->stats.cc.rx_frame_lcnt == 0) {
			       acb->stats.cc.rx_frame_mcnt++;
		       }
		       if ((acb->stats.cc.rx_byte_lcnt += recvp->rc_count) <
			       recvp->rc_count) {
			       acb->stats.cc.rx_byte_mcnt++;
		       }
		       if (OPENP.op_mode & DKERNEL) {
			       /*
			       ** Fill in the read extension fields.
			       */
			       rd_ext.status = CIO_OK;

			       /*
			       ** Notify the kernel user of data received.
			       */
			       mbufp = recvp->rc_mbuf_head;
			       /*
			       ** Set the packet length in the header
			       ** (created in pscaxbuf()) so TCP/IP
			       ** doesn't have to calculate it.
			       */
			       mbufp->m_pkthdr.len = recvp->rc_count;
			       recvp->rc_mbuf_head = NULL;

			       (*(OPENP.mpa_kopen.rx_fn))(OPENP.mpa_kopen.open_id,&(rd_ext),mbufp);

		       } else {
			       /*
			       ** User-mode process:
			       ** If user is blocked on read, do a wakeup.
			       */
			       if (acb->op_rcv_event != EVENT_NULL)
				      e_wakeup(&acb->op_rcv_event);

			       /*
			       ** Notify the user vi poll/select mechanism.
			       */
			       if (OPENP.op_select & POLLIN) {
				       selnotify((int)acb->dev,OPENP.op_chan, POLLIN);
			       }

			       /*
			       ** Free the receive element in mparead()...
			       ** unless this is kernel mode.
			       */
			       if (OPENP.op_mode & DKERNEL)  free_recv_elem(acb, recvp);
		       }
		       ++acb->stats.ds.recv_sent;
		}
		if(acb->flags&RC_START_RECV) {
		       acb->flags &= ~RC_START_RECV;
		       acb->flags &= ~RECEIVER_ENABLED;
		       rc=startrecv(acb);
		}
		DDHKWD5 (HKWD_DD_MPADD, DD_EXIT_OFFL,rc,acb->dev,RECV_RESULT,
		   result,recv_bytes,cnt);

		break;

	   case XMIT_RESULT:
		if(acb->flags&PIO_MODE) {
		     if( (xmitp=acb->act_xmit_head) == NULL ) {
			++acb->stats.ds.xmit_not_handled;
			DDHKWD5 (HKWD_DD_MPADD, DD_EXIT_OFFL,rc,acb->dev,XMIT_RESULT,
			      result,0,0xF6);
			break;
		     }
		}
		else {
		     if((dmap=acb->act_dma_head) == NULL) {
			++acb->stats.ds.xmit_not_handled;
			DDHKWD5 (HKWD_DD_MPADD, DD_EXIT_OFFL,rc,acb->dev,XMIT_RESULT,
			      result,0,0xF7);
			break;
		     }
		     if(dmap->dm_req_type!=DM_XMIT) {
			++acb->stats.ds.xmit_not_handled;
			async_status(acb, CIO_ASYNC_STATUS,
				    CIO_LOST_DATA,irptp->tp.TIC,1, 0);
			DDHKWD5 (HKWD_DD_MPADD, DD_EXIT_OFFL,rc,acb->dev,XMIT_RESULT,
			      result,0,0xF8);
			break;
		     }
		     if( (xmitp = dmap->p.xmit_ptr) != acb->act_xmit_head) {
			++acb->stats.ds.xmit_not_handled;
			DDHKWD5 (HKWD_DD_MPADD, DD_EXIT_OFFL,rc,acb->dev,XMIT_RESULT,
			      result,0,0xF9);
			break;
		     }
		}
		result = irptp->tp.TIC;
		switch(result) {

		   case XMIT_EARLY_IRPT:
			++acb->stats.ds.xmit_early_irpts;
			break;
		   case XMIT_COMPLETE:
			++acb->stats.ds.xmit_completes;
			break;
		   case XMIT_DMA_UNDERRUN:
			++acb->stats.ds.xmit_dma_underrun;
			acb->flags |= XM_DISCARD;
			break;
		   case XMIT_CL_TO_SEND_ERR:
			++acb->stats.ds.xmit_cts_errors;
			acb->flags |= XM_DISCARD;
			break;
		   case XMIT_ABORT_DONE:
			++acb->stats.ds.xmit_aborts;
			acb->flags |= XM_DISCARD;
			break;
		   case XMIT_PIO_ERROR:
			++acb->stats.ds.io_irpt_error;
			acb->flags |= XM_DISCARD;
			break;

		   default:
			acb->flags |= XM_DISCARD;
			break;
		}


		if (xmitp->xm_ack & CIO_ACK_TX_DONE) {
		   /* if status not good  give report */

		    if ((acb->flags&XM_DISCARD)||(acb->flags&IRPT_PIO_ERR)) {
			  async_status(acb, CIO_TX_DONE,
				    CIO_LOST_DATA,irptp->tp.TIC,1, 0);
			acb->flags &= ~XM_DISCARD;
		    } else {
			 /*
			 ** Notify, calling process good xmit done.
			 */
			 async_status(acb, CIO_TX_DONE,
				 CIO_OK,irptp->tp.TIC,1, 0);
		    }
		}
		acb->flags &= ~IRPT_PIO_ERR;
		/*
		** Update statistics counters.
		*/
		if (++acb->stats.cc.tx_frame_lcnt == 0) {
			acb->stats.cc.tx_frame_mcnt++;
		}
		if ((acb->stats.cc.tx_byte_lcnt += xmitp->xm_length) <
			xmitp->xm_length) {
			acb->stats.cc.tx_byte_mcnt++;
		}

		/*
		** Wakeup anyone affected by this transfer
		*/
		if (OPENP.op_select & POLLOUT) {
			selnotify((int)acb->dev, OPENP.op_chan, POLLOUT);
		}

		/*
		** If the kernel user wants us to free the mbufs,
		** do so now.
		*/
		if (xmitp->xm_mbuf
			&& (OPENP.op_mode & DKERNEL)
			&& ((xmitp->xm_ack & CIO_NOFREE_MBUF) == 0)) {
			m_freem(xmitp->xm_mbuf);
		}
		/*
		** Note: In PIO_MODE I do not have the recv stopped
		** so don't try to restart it here. Also do not start
		** a recv if the NO_RECV flag is set.
		*/
		if( !(acb->flags&PIO_MODE) ) {
		    free_dma_elem(acb,dmap);
		    if( !(acb->flags&NO_RECV) ) {
			/*
			** Restart the receive
			*/
			rc=startrecv(acb);
		    }
		}
		DDHKWD5 (HKWD_DD_MPADD, DD_EXIT_OFFL,rc,acb->dev,
			 XMIT_RESULT,result,xmitp->xm_length,cnt);
		free_xmit_elem(acb,xmitp);
		break;

	   default:
		DDHKWD5 (HKWD_DD_MPADD,DD_EXIT_OFFL,EINVAL,acb->dev,0,result,0,0);
		break;
	}

	free_irpt_elem(acb,irptp);

    }    /* end of while irpt elements left on the q */


    return 0;
}              /* mpa_offlvl() */

/*
 * NAME: q_irpt_results
 *
 * FUNCTION: Allocates memory for an irpt results structure and
 *           reads the results from the card, then fills in the
 *           sturct values and adds it to the irpt results struct
 *           chain.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment-Specific aspects, such as -
 *      Preemptable        : Maybe
 *      VMM Critical Region: Yes
 *      Runs on Fixed Stack: Yes
 *      May Page Fault     : no
 *      May Backtrack      : no
 *
 * NOTES:
 *
 *
 * DATA STRUCTURES: acb altered.
 *
 * RETURN VALUE DESCRIPTION: NONE
 */

q_irpt_results (
		    struct acb *acb,
		    int type)
{
     irpt_elem_t *irptp;
     int             rc,res,result1;


     /* take element off the free_irpt q. */
     if( (irptp = acb->irpt_free) == NULL) {
	  ++acb->stats.ds.recv_not_handled;
	  return;
     }
     acb->irpt_free=irptp->ip_next;
     irptp->ip_next = NULL;

     /*
      * Put this element on the active irpt q for this adapter
     */
     if(acb->act_irpt_head==NULL) {  /* its first one */
	 acb->act_irpt_head=irptp;
	 acb->act_irpt_tail=irptp;
     }
     else {        /* its going on the end of a chain */
	 acb->act_irpt_tail->ip_next=irptp;
	 acb->act_irpt_tail=irptp;
     }
     irptp->ip_state |= IP_ACTIVE;
     irptp->type_flag = type;

     /*
      * Switch based on type of irpt.
     */
     switch(type) {
	case XMIT_RESULT:
		/* set up bus access and get the io base addr  */
		if((rc=PIO_GETC( acb, RD_TX_IR_OFFSET))==-1) {
		   irptp->tp.TIC = XMIT_PIO_ERROR;
		   return;
		}
		irptp->tp.TIC = (uchar) rc;
		break;
	case RECV_RESULT:
		/*
		 * Read the first result byte from result reg
		*/
		/* set up bus access and get the io base addr  */
		if((rc=PIO_GETC( acb, RD_RX_IR_OFFSET))==-1) {
		   irptp->tp.rcv.RIC = RECV_PIO_ERROR;
		   return;
		}
		irptp->tp.rcv.RIC = (uchar) rc;
		/*
		** If this is an idle detect irpt there are no more results,
		** so don't try to read them.
		*/
		result1 = (irptp->tp.rcv.RIC & RIC_MASK_LAST_BYTE);
		if(result1 != RECV_IDLE) {
		    /*
		    ** If card is in buffered mode there will be 4 more result
		    ** bytes to read otherwise there will be only 2 more bytes.
		    */
		    if(acb->state.oper_mode_8273&SET_BUFFERED_MODE) {
			  if( (res = get_rx_result(acb)) == 300) {
				irptp->tp.rcv.R0 = 0xFF;
				irptp->tp.rcv.R1 = 0xFF;
				break;
			  }
			  else if(res == 301) {
				irptp->tp.rcv.RIC = RECV_PIO_ERROR;
				break;
			  }
			  else irptp->tp.rcv.R0 = (uchar) res;

			  if( (res = get_rx_result(acb)) == 300) break;
			  else if(res == 301) {
				irptp->tp.rcv.RIC = RECV_PIO_ERROR;
				break;
			  }
			  else irptp->tp.rcv.R1 = (uchar) res;

			  if( (res = get_rx_result(acb)) == 300) break;
			  else if(res == 301) {
				irptp->tp.rcv.RIC = RECV_PIO_ERROR;
				break;
			  }
			  else irptp->tp.rcv.ADR = (uchar) res;

			  if( (res = get_rx_result(acb)) == 300) break;
			  else if(res == 301) {
				irptp->tp.rcv.RIC = RECV_PIO_ERROR;
				break;
			  }
			  else irptp->tp.rcv.CNTL = (uchar) res;
		    }
		    else  {
			  if( (res = get_rx_result(acb)) == 300) {
				irptp->tp.rcv.R0 = 0xFF;
				irptp->tp.rcv.R1 = 0xFF;
				break;
			  }
			  else if(res == 301) {
				irptp->tp.rcv.RIC = RECV_PIO_ERROR;
				break;
			  }
			  else irptp->tp.rcv.R0 = (uchar) res;

			  if( (res = get_rx_result(acb)) == 300) break;
			  else if(res == 301) {
				irptp->tp.rcv.RIC = RECV_PIO_ERROR;
				break;
			  }
			  else irptp->tp.rcv.R1 = (uchar) res;
		    }
		}   /* end of if not idle detect irpt */
	     break;

	default:
	     break;
     }
     if (acb->irpt_free == NULL) {
	acb->flags |= NEED_IRPT_ELEM;
     }

     /*
     **  Set the IRPT_QUEUED flag so offlevel will be scheculed.
     */
     acb->flags |= IRPT_QUEUED;

     /* check to see if the result was a good receive, if it was then */
     /* disable the cards receiver so I don;t get the idle detect     */
     if( (type==RECV_RESULT) && ( (result1 == RECV_GEN_OK) ||
	  (result1 == RECV_SEL_OK) ) ) {
	  acb->cmd_parms.cmd=DISABLE_RECV_CMD;
	  acb->cmd_parms.parm_count=0;
	  if( !(rc = que_command(acb)) ) {
		DDHKWD2(HKWD_DD_MPADD, MPA_RECV_DISAB, 0, acb->dev,0x00);
		acb->flags &= ~RECEIVER_ENABLED;
	  }
     }
     return;
}   /* q_irpt_results() */


/*  this function waits for receive results to be available then
**  reads and returns the resutls
**
**  Returns 301 for PIO error
**  Returns 300 for timeout error (waiting for result status)
**  Returns < 256 valid result value.
*/
int get_rx_result (struct acb *acb)
{
    int rc=0,cnt=0;

    /*
    ** loop here waiting for the card to raise recv result read bit.
    */
    while( !(rc&RX_RESULT_READY ) && ++cnt<5) {
	 if((rc=PIO_GETC( acb, RD_STAT_OFFSET))==-1) {
	       return 301;
	 }
    }

    /*
    **  If it takes longer that 5 tries, assume there are no more results
    **  else read and return the next result value.
    */
    if(cnt==5) {
	 rc= 300;
    }
    else  {
	 if((rc=PIO_GETC( acb, RD_RX_IR_OFFSET))==-1) {
	       return 301;
	 }
    }
    return(rc);
}


