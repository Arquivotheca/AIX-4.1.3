static char sccsid[] = "@(#)98	1.17.1.8  src/bos/kernext/entdiag/entrw.c, diagddent, bos411, 9428A410j 10/28/93 15:56:57";
/*
 * COMPONENT_NAME: DIAGDDENT
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <net/spl.h>
#include <sys/cblock.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/adspace.h>
#include <sys/errno.h>
#include "ent_comio_errids.h"
#include <sys/except.h>
#include <sys/file.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/ioctl.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/pin.h>
#include <sys/poll.h>
#include <sys/sleep.h>
#include <sys/timer.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/watchdog.h>
#include <sys/xmem.h>
#include <sys/trcmacros.h>
#include <sys/trchkid.h>
#include <stddef.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/uio.h>
#include <sys/entuser.h>
#include "entddi.h"
#include "cioddi.h"

#include "cioddhi.h"
#include "entdshi.h"
#include "ciodds.h"
#include "cioddlo.h"
#include "entdslo.h"

#ifdef KOFF
/*
 * This stuff is only for the KOFF kernel debugger.  Unless your kernel has
 * KOFF running in it, or the KOFF kernel extension is loaded before
 * this kernel extension, you will not be able to use it.
 */
#include "../../sys/koff/db_trace.h"
#define TR_EN	0x80000000
#else
#define db_trace(flag, exp)
#endif

extern dd_ctrl_t dd_ctrl;

void ent_recv (dds_t *dds_ptr, uchar cmd_reg);
void ent_tx_off_lvl(dds_t *dds_ptr, uchar cmd_reg);
void ent_xmit_done(dds_t *dds_ptr, xmit_elem_t *xelm, int status);
void ent_xmit (dds_t *dds_ptr, char *bus);

/*
* table for translating adapter receive error codes to ethernet error codes
*/
static ENT_EXCEPT rx_err_tab[7] = {
	  CIO_OK, ENT_RX_CRC_ERROR, ENT_RX_OVERRUN, ENT_RX_ALIGN_ERROR,
	  ENT_RX_NO_RESOURSE, ENT_RX_TOO_SMALL, ENT_RX_TOO_LARGE };
		
/*****************************************************************************/
/*                                                                           */
/* NAME: xxx_intr                                                            */
/*                                                                           */
/* FUNCTION: Test if this adapter caused an interrupt & if so, process it.   */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the interrupt thread.                   */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*                                                                           */
/*    Output: Interrupt process or off level processing queued.              */
/*                                                                           */
/* RETURN:  INTR_SUCC = Interrupt for this adapter & it was processed.       */
/*          INTR_FAIL = Interrupt not caused by this adapter.                */
/*                                                                           */
/*****************************************************************************/

int
xxx_intr (
	struct intr *ihsptr)	/* This is also the DDS pointer */
{
	dds_t *dds_ptr;
	char status_reg;
	char command_reg;
	int ioa, bus, sched = 0;
	int rc;
	int state;
	offlevel_que_t *offl_ptr;
	int tempndx;
	enum pend pending = pend_none;

	dds_ptr = (dds_t *)ihsptr;
	offl_ptr = &(OFL);

	DTRACE2("FLI1", dds_ptr);

	state = WRK.adpt_start_state;
	if (state == NOT_STARTED)
	{
		return(INTR_FAIL);
	}

	ioa = (int)BUSIO_ATT((ulong)WRK.bus_id, WRK.io_port);
	if (CIO.mode == 'D')
	{
		rc = diag_intr(dds_ptr);
		goto int_out;
	}

	/* read the status register, if status register can't be read
	 * then assume not our interrupt
	 */
	if (rc = BUS_GETCX((char *)(ioa + STATUS_REG), &status_reg))
	{
		if (pio_retry(dds_ptr, rc, GETC, (char *)(ioa + STATUS_REG),
							 (ulong)&status_reg))
		{
			rc = INTR_FAIL;
			goto int_out;
		}
	}

	if (status_reg & CWR_MSK) {
		/*
		 * this is ours. disable further reporting, and schedule the
		 * off level handler (sigh).
		 * Unfortunately, this is getting hacked too much.  The queue
		 * stuff is needed unless I rewrite too much of the driver
		 * to get rid of it.  The adapter doesn't handle `turn off
		 * attentions' correctly (at least the lowest revision doesn't)
		 * it generates an interrupt when attention is turned back on!
		 * Also, I seem to get multiple interrupts anyway.  I have
		 * decided to keep pending interrupt status between the
		 * hardware level interrupt handler and the off-level one.
		 * We'll see how this turns out.
		 * Basically, the normal case is that of good xmits or good
		 * receives.  These are reflected in the pending bits.
		 * Errors in xmit or errors in recv are also reflected in the
		 * pending bits.
		 * The abnormal cases are queued as before.
		 * When the driver state is ramping up, we must queue all
		 * the interrupts also.
		 */
		int queue;

		rc    = INTR_SUCC;
		sched = 1;

		PIO_GETCX((char *)(ioa + COMMAND_REG), &command_reg);

		if (command_reg & RECEIVE_MSK)
			++RAS.ds.recv_intr_cnt;

		if (command_reg & XMIT_MSK)
			++RAS.ds.xmit_intr_cnt;

		if (state >= STARTED && WRK.adpt_state == normal) {
			queue = 0;

			switch (command_reg & RECEIVE_MSK) {
			    case RX_NOP:
				break;

			    case RX_P_RCVD:
				pending |= pend_rx;
				break;

			    case RX_ERROR:
				pending |= pend_rx_err;
				break;

			    case RX_ABORT:
				queue = 1;
				break;
			}


			switch (command_reg & XMIT_MSK) {
			    case TX_NOP:
				break;

			    case TX_P_SENT:
				pending |= pend_tx;
				break;

			    case TX_ERROR:
				pending |= pend_tx_err;
				break;

			    case TX_ABORT:
				queue = 1;
			}
		} else
			queue = 1;	/* always queue when starting	*/


		db_trace(TR_EN
			 , (ihsptr
			    , ">>> intr: en cmd %x pend %x pending %x queue %d"
			    , command_reg, pending, offl_ptr->pending, queue));

		offl_ptr->pending |= pending;

		if (queue || (command_reg & EXECUTE_MSK)) {
			if ((tempndx = offl_ptr->next_in+1) >= MAX_OFFL_QUEUED)
				tempndx = 0;

			if (tempndx == offl_ptr->next_out) {
				printf("lost!\n");
				++RAS.ds.intr_lost;
				goto int_out;
			}

			/*
			 * queue up work for offlevel
			 */
			offl_ptr->offl_elem[offl_ptr->next_in].who_queued =
				 (state < STARTED) ? OFFL_START : OFFL_INTR;
			offl_ptr->offl_elem[offl_ptr->next_in].cmd_reg
				= command_reg;

                  	offl_ptr->next_in = tempndx;
		}
	} else
		rc = INTR_FAIL;

	/* on AT form factor card the status register will not reflect
	 * parity errors
	 */
	if (((status_reg & PARITY_MSK) || WRK.card_type == ACT_10)
	    && parity_error(dds_ptr, command_reg, status_reg))
		sched = 1;

	if (sched && !offl_ptr->scheduled) {
		/* only call i_sched if the offlevel handler has not
		 * been scheduled or is not running
		 */
		i_sched ((struct intr *)offl_ptr);
		offl_ptr->scheduled = TRUE;
	}

int_out:
	/* if this card generated the interrupt then reset interrupt
	 */
	if (rc == INTR_SUCC)
		i_reset(ihsptr);
	BUSIO_DET(ioa);

	return(rc);
}
/*****************************************************************************/
/*                                                                           */
/* NAME: xxx_offl                                                            */
/*                                                                           */
/* FUNCTION: Process the off level interrupt queue and call the off level    */
/*           interrupt handler.                                             */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the interrupt thread.                   */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: Offlevel Queue pointer - tells which adapter                    */
/*                                                                           */
/*    Output: Offlevel interrupt handler called and additional off level     */
/*            scheduling is inhibited.                                       */
/*                                                                           */
/*    Called From: i_sched                                                   */
/*                                                                           */
/*    Calls To: xxxoffl                                                      */
/*                                                                           */
/* RETURN:  0 (always)                                                       */
/*                                                                           */
/*****************************************************************************/

int
xxx_offl (
	offlevel_que_t *ofl_ptr)	/* pionter to offlevel que struct */
{
	dds_t		*dds_ptr;
	uchar		cmd_reg;


	db_trace(TR_EN, (ofl_ptr, "xxx_offl: enter"));

	DTRACE2 ("offB",(ulong)ofl_ptr);


	/* Calculate which dds (which card) caused the interrupt
	 */
	dds_ptr = (dds_t *)(((ulong)ofl_ptr) - offsetof(dds_t, ofl_section));

	do {
		enum pend pend, get_pending();

		ASSERT(OFL.pending || (OFL.next_out != OFL.next_in));

		switch (WRK.adpt_state) {
		    case normal:
		    case restart:
			break;

		    case error:
			/*
			 * interrupts from the adapter are disabled.
			 * flush all pending and queued, start cleanup.
			 */
			(void) get_pending(&OFL.pending);
			OFL.next_out = OFL.next_in;

			WRK.adpt_state = cleanup;
			ent_cleanup(dds_ptr);
			continue;

		    case cleanup:
			ASSERT(!get_pending(&OFL.pending));
			break;

		    case broken:
			return;
		}
		/*
		 * process all pending first
		 */
		if (pend = get_pending(&OFL.pending)) {
			db_trace(TR_EN
				 , (ofl_ptr
				    , "offl: pend %x", pend));

			if (pend & pend_rx)
				ent_recv(dds_ptr, RX_P_RCVD);
			if (pend & pend_tx)
				ent_tx_off_lvl(dds_ptr, TX_P_SENT);
			if (pend & (pend_rx_err | pend_tx_err)) {
				if (pend & pend_rx_err)
					ent_recv(dds_ptr, RX_ERROR);
				if (pend & pend_tx_err)
					ent_tx_off_lvl(dds_ptr, TX_ERROR);
			}
		}

		/*
		 * if there are none queued (not checked atomically) jump down
		 * to the `test_set()' atomic check.
		 */
		if (OFL.next_out == OFL.next_in)
			continue;

		if (OFL.offl_elem[OFL.next_out].who_queued == OFFL_INTR) {
			/*
			 * process queued requests.
			 */
			cmd_reg =  OFL.offl_elem[OFL.next_out].cmd_reg;

			db_trace(TR_EN
				 , (ofl_ptr
				    , "offl: cmd %x nxt_out %x nxt_in %x"
				    , cmd_reg, OFL.next_out, OFL.next_in));

			if (cmd_reg & RECEIVE_MSK)
				ent_recv(dds_ptr, cmd_reg & RECEIVE_MSK);
			if (cmd_reg & XMIT_MSK)
				ent_tx_off_lvl(dds_ptr, cmd_reg & XMIT_MSK);

			if (cmd_reg & (EXECUTE_MSK|OFLOW_MSK)) {
				if (cmd_reg & EXECUTE_MSK)
					entexecdque(dds_ptr);
				if (cmd_reg & OFLOW_MSK)
					entrdcounts(dds_ptr);
			}
		} else {
			db_trace(TR_EN, (ofl_ptr, "offl: non interrupt"));
			xxxoffl(dds_ptr, &OFL.offl_elem[OFL.next_out]);
		}

		/* increment out pointer, then check if the queue is empty.
		 * test_set will clear the scheduled word atomicly with
		 * the test for a empty queue
		 */
		if (OFL.next_out == (MAX_OFFL_QUEUED - 1))
			OFL.next_out = 0;
		else
			OFL.next_out++;
	} while (test_set(&OFL.pending
			  , &OFL.next_out, &OFL.next_in, &OFL.scheduled));

	return(0);
}
/*****************************************************************************/
/*                                                                           */
/* NAME: ent_recv                                                            */
/*                                                                           */
/* FUNCTION: Process the receive interrupts, fill the receive list with new  */
/*           buffers, & restart the adapter receive processing if needed.    */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the off level interrupt handler thread. */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*           Restart flag - tells if recv needs to be restarted on the adptr */
/*                                                                           */
/*    Output: Received data buffer returned to common code to be passed to   */
/*            the user process, adapter given new buffer for receive, and    */
/*            adapter receive is restarted if needed (this is doen in the    */
/*            case when the adapter returns a receive interrupt with         */
/*            "no resources".                                                */
/*                                                                           */
/*    Called From: xxx_offl                                                  */
/*                                                                           */
/*    Calls To: d_complete, cio_proc_recv, m_free, MTOD, AIXTRACE, M_XMEMD   */
/*                                                                           */
/* RETURN:  0 - Successful completion                                        */
/*                                                                           */
/*****************************************************************************/

void
ent_recv (
	dds_t   *dds_ptr,		/* DDS pointer - tells which adapter */
        uchar	cmd_reg)		/* Command register */
{
	int	restart;        	/* restart adapter receive flag      */
	open_elem_t *open_elem;     	/* open element                      */
	netid_t netid;          	/* Net Id data type                  */
	ushort  netid_length;   	/* Length of netid                   */
	ulong	netid_index;    	/* index into netid table            */
	rec_elem_t recv_elem;      	/* receive element                   */
	int     ioa;            	/* Attachment to adapter I/O         */
	int     bus;            	/* Attachment to adapter memory      */
	int     mbox;           	/* Receive mail box.                 */
	ulong   i, j, l;
	ulong   found;
	uchar   temp_byte;
	short	count;
	recv_des_t *rd;
	struct mbuf *mbufp;
	int bufoff;
	cio_read_ext_t rd_ext;
	ulong broad, mcast;
	ulong throwaway;

	DTRACE3 ("rcvB",(ulong)dds_ptr,(ulong)cmd_reg);

	switch (cmd_reg)
	{

		case RX_P_RCVD:    /* Packet Received */
         		/* Process the receive interrupt
			 */
         		restart = FALSE;
         		break;

		case RX_ERROR:     /* Out of Resources */
			/* Process the recv interrupt with "receive restart"
			 * flag
			 */
         		restart = TRUE;
         		break;

		case RX_ABORT:     /* Reception Aborted */
         		TRACE1 ("reca");

			/* Test if transmit abort already received
			 */
         		if (WRK.adpt_start_state == X_ABORT_1)
			{
				TRACE2 ("slpE",(ulong)&(WRK.close_event));

				/* Wake up the sleeping process - finish
				 * the close
				 */
				e_wakeup((int *)&WRK.close_event);
         		}

			/* Test is starting to close the adapter
			 */
         		if (WRK.adpt_start_state == SLEEPING)
         		{
            			/* Update the adapter state
				 */
            			WRK.adpt_start_state = X_ABORT_1;
         		}

         		TRACE1 ("rcvE");
         		return;

      		default:
			/* Reserved - adapter error  0x08, 0x18 or 0x28
			 */
         		TRACE1 ("rcvE");
			return;
	}

        if(WRK.rstart_flg == TRUE) {
                restart=TRUE;
                WRK.rstart_flg=FALSE;
        }

	/* start processing the receive list
	 */

	/* Get access to the adapter RAM
	 */
	bus = (int)BUSMEM_ATT( (ulong)WRK.bus_id, WRK.bus_mem_addr );


	/* Calculate host addresses to mailbox
	 */
	mbox = bus + WRK.recv_mail_box;


	while (TRUE)
      	{
		/* acvive head of receive list, zero temp_byte so we will
		 * break out if PIO fails
		 */
		rd = &WRK.recv_list[WRK.Recv_Index];
		bufoff = bus + rd->rbufd;
		temp_byte = 0;
		throwaway = FALSE;
		broad = mcast = FALSE;
		PIO_GETCX((char *)(bufoff + offsetof(BUFDESC, status)),
								&temp_byte);

		/* exit if no more packets to process
		 */
         	if (!(temp_byte & COMPLETE))
            		break;


		/* Check dma status
		 */
		if (d_complete (WRK.dma_channel, DMA_READ|DMA_NOHIDE,
			rd->buf, ent_MAX_PACKET, (struct xmem *)&WRK.rbuf_xd,
			rd->rdma))
		{
			ent_logerr(dds_ptr, ERRID_ENT_ERR5, EFAULT,
							(ulong)cmd_reg, 0);
		}
		/*
		* Is this a good packet or bad packet? 
		*/
		
		if (!(temp_byte &= ERRCODE_MASK)) { /* no error, good packet */

			PIO_GETSRX((short *)(bufoff + offsetof(BUFDESC, count)),
									&count);

			/* Performance group trace entry point
		 	*/
			AIXTRACE (TRC_RDAT, CIO.devno, 0, 0, count);

			if ((count >= ent_MIN_PACKET) && 
			  (count <= ent_MAX_PACKET)) {

			  RAS.cc.rx_frame_lcnt++;
			  if (RAS.cc.rx_frame_lcnt == ULONG_MAX)
				RAS.cc.rx_frame_mcnt++;
			  if ((ULONG_MAX - count) <  RAS.cc.rx_byte_lcnt)
				RAS.cc.rx_byte_mcnt++;
       	                  RAS.cc.rx_byte_lcnt += count;

			/* check if broadcast or multicast */
			  if (rd->buf[0] & MULTI_BIT_MASK == MULTI_BIT_MASK)
			  {
				for (j=0; j<ent_NADR_LENGTH && 
					rd->buf[j] == 0xFF; j++);
				if (j >= ent_NADR_LENGTH)
					broad = TRUE;
				else
					mcast = TRUE;
			  }
			}
			else 
			  throwaway = TRUE;
		/*
		* give a copy to promiscuous mode channels
		*/
			if (!throwaway && (i = CIO.promiscuous_count)) {
				for (j = 0; j < MAX_OPENS; j++) {
				  if ((open_elem = CIO.open_ptr[j]) &&
				   	open_elem->prom_on_cnt) {
                        		if (count <= MHLEN - WRK.rdto) {
						DTRACE2("gsmb",count);	
                                		mbufp = m_gethdr(M_DONTWAIT, MT_DATA);
					}
                        		else {
						DTRACE2("gcls",count);	
                                		mbufp = m_getclust(M_DONTWAIT,MT_DATA);
					}
					if (mbufp) {
					  mbufp->m_data += WRK.rdto;
					  mbufp->m_len = count;
					  bcopy(rd->buf, MTOD(mbufp, char *), count);
					  if (broad)
						mbufp->m_flags |= M_BCAST;
					  if (mcast)
						mbufp->m_flags |= M_MCAST;

					  rd_ext.status = CIO_OK;
       		                    	  DTRACE1("KRFc");
       		                    	  (*(open_elem->rec_fn)) (open_elem->open_id,
       		                             		&rd_ext, mbufp);
       		                    	  DTRACE1("KRFr");
       		                    	  AIXTRACE(TRC_RNOT,
							open_elem->devno,
							open_elem->chan,
							mbufp, mbufp->m_len);
                        		}
					else
					  RAS.ds.rec_no_mbuf++;

					if ((i -= open_elem->prom_on_cnt) == 0)
					  break;
				  }
				}   /*  for loop */
			}
		/*
		* Screen the multicast packets
		*/
			if (!throwaway) {

			/* This code added because adapter doesn't always
			   filter multicast addresses properly */
			/* Check the multicast list */
			    if (mcast)
			    {
				found = FALSE;
				for (i=0; i < WRK.multi_count && !found; i++) {
					for (j = 0; j < ent_NADR_LENGTH; j++)
					  if(WRK.multi_list[i][j] != rd->buf[j])
						break;
					if (j >= ent_NADR_LENGTH)
					  found = TRUE;
				}
			/* If not a multicast address, throw away the data */
				if (!found) throwaway = TRUE;
			    }
			    else {
				if (!broad) {   
			/* this is a specific address, is it mine? */
					for (j = 0; j < ent_NADR_LENGTH; j++)
					  if(WRK.ent_addr[j] != rd->buf[j])
						break;
					if (j < ent_NADR_LENGTH)
					  throwaway = TRUE;
				}
			    }
					
			}  /* if not throwaway */

			if (!throwaway)
			{
				netid = GET_NETID(rd->buf, DDI.ds.type_field_off);

            			/* Test if netid is a one or two byte net id
			 	*/
				if (netid < 0x0600)
            			{
					/* IEEE 802.3 packet - length is 1 byte
					   & offset is 14, Set up length of the
					   netid
				 	*/
					netid_length = 0x0001;

              				/* Get the 802.3 netid from the packet
				 	*/
			
					netid = GET_NETID(rd->buf,
							DDI.ds.net_id_offset);

					/* Set the net id byte in the LSB of the
				 	* ushort
				 	*/
					netid = ((netid >> 8) & 0x00FF);

				}
            			else
            			{
					/* Standard ethernet packet - Length is
					 * 2 bytes & offset is 12. Set up length
					 * of net id
				 	*/
					netid_length = 0x0002;
				}

				/* Search the netid table as to who owns the
					packet
			 	* Initialize flag before starting the search
			 	*/
				found = FALSE;

				/* Test if this packet belongs to same owner as
				 *  last packet
				 */
				if ((netid == WRK.last_netid) && 
					(netid_length == WRK.last_netid_length))
				{

					/* Packet netid valid in net ID table
				 	 */
					found = TRUE;

					/* Set up which netid index this packet
					 * is for
					 */
					netid_index = WRK.last_netid_index;

				}
				else
				{
					/* Search through the netid table for a
					 * valid match
					 */
					for (j = 0; j < CIO.num_netids; j++)
					{
 						/* Test if the net id matches
						 */
                  				if ((CIO.netid_table_ptr[j].netid == netid) && (CIO.netid_table_ptr[j].length == netid_length))
                  				{ /* A match was found */

							/* Mark as found and
							 * save for fast path
							 * next time
							 */
							found = TRUE;
                     					WRK.last_netid = netid;
                     					WRK.last_netid_length =
								 netid_length;
                     					WRK.last_netid_index=j;

                     					netid_index  = j;

                     					break;

	                  			}

       	        			}
				}

				if (found && (open_elem = CIO.open_ptr[CIO.netid_table_ptr[netid_index].chan-1]) && !open_elem->prom_on_cnt) {
                        		if (count <= MHLEN - WRK.rdto) {
						DTRACE2("gsmb",count);	
                                		mbufp = m_gethdr(M_DONTWAIT, MT_DATA);
				  	}
                        	  	else {
						DTRACE2("gcls",count);	
                                		mbufp = m_getclust(M_DONTWAIT,MT_DATA);
					}

					if (mbufp) {
						mbufp->m_data += WRK.rdto;
						mbufp->m_len = count;
						bcopy(rd->buf, MTOD(mbufp, char *), count);
       	                			if (open_elem->devflag & DKERNEL)
       	                			{
					  	       if (broad)
							 mbufp->m_flags |= M_BCAST;
					  	       if (mcast)
							 mbufp->m_flags |= M_MCAST;
							rd_ext.status = CIO_OK;
       		                    			DTRACE1("KRFc");
       		                    			(*(open_elem->rec_fn))
							  (open_elem->open_id,
       		                             		  &rd_ext, mbufp);
       		                    			DTRACE1("KRFr");
       		                    			AIXTRACE(TRC_RNOT,
							  open_elem->devno,
							  open_elem->chan,
							  mbufp, mbufp->m_len);
                        			}
                        			else
                        			{
                            				recv_elem.mbufp = mbufp;
                            				recv_elem.bytes = count;
							recv_elem.rd_ext.status = CIO_OK;
                           				cio_proc_recv(dds_ptr,
							  open_elem, &recv_elem);
						}
					}  /* if get an mbuf */
					else 
						RAS.ds.rec_no_mbuf++;

				}  /* if need to pass the packet up */

			}   /* if not throwaway */
		}  /* if good packet */
		else {    /* this is a bad packet */

    			cio_stat_blk_t  stat_blk;   /* new status block */


			PIO_GETSRX((short *)(bufoff + offsetof(BUFDESC, count)),
									&count);
		/*
		* got a bad frame.  put it in a mbuf,
		* create a status block for it and inform all the
		* channels enabled the save bad frame mode.
		*/
			if (i = CIO.badframe_count) {
                        	if (count <= MHLEN - WRK.rdto) {
					DTRACE2("gsmb",count);	
                                	mbufp = m_gethdr(M_DONTWAIT, MT_DATA);
				}
                        	else {
					DTRACE2("gcls",count);	
                                	mbufp = m_getclust(M_DONTWAIT,MT_DATA);
				}
				if (mbufp) {
					mbufp->m_data += WRK.rdto;
					mbufp->m_len = count;
					bcopy(rd->buf, MTOD(mbufp, char *), count);
    					stat_blk.code = CIO_ASYNC_STATUS;
    					stat_blk.option[0] = ENT_BAD_FRAME;
    					stat_blk.option[1] = rx_err_tab[temp_byte];
    					stat_blk.option[2] = (ulong)mbufp;
    					stat_blk.option[3] = 0;

					for (j = 0; j < MAX_OPENS; j++) {
				  	  if ((open_elem = CIO.open_ptr[j]) &&
				   	    open_elem->badframe_on_cnt) {
      						/* notify kernel process */
						TRACE1 ("KSFc"); 
      						(*(open_elem->sta_fn)) 
						  (open_elem->open_id, &stat_blk);
      						TRACE1 ("KSFr"); 
					    if ((i -= open_elem->badframe_on_cnt) == 0)
					      break;
                        		  }
					}   /*  for loop */

					/* we own the mbuf, so we free it */
					m_free(mbufp);   
				}
				else 
					RAS.ds.rec_no_mbuf++;

			}


		}

		/* invalidate processor cache, d_master has allready been
		* called.  Note that only the range that was touched by
		* the above bcopy needs to be invalidated.
		*/
		cache_inval(rd->buf, count);

		/* shift EL bit from last receive buffer descriptor to new
		* one and clear the status byte
		*/
		DASSERT(PIO_GETC(bus + WRK.Recv_El_off +
			offsetof(BUFDESC, control)) & EL);

		PIO_PUTSX((short *)(bufoff), (short)EL);
		PIO_PUTCX((char *)(bus + WRK.Recv_El_off +
			offsetof(BUFDESC, control)), (char)EOP);

		/* save the new end of list index, and update receive buffer
		* index
		*/
		WRK.Recv_El_off = rd->rbufd;
		WRK.Recv_Index++;
		if (WRK.Recv_Index >= WRK.recv_tcw_cnt)
			WRK.Recv_Index = 0;
	}

	if (restart == TRUE)
	{
		db_trace(TR_EN, (0, "ent_recv: restarting!"));

		PIO_PUTSR( mbox + offsetof(RECVMBOX, status), 0 );
		PIO_PUTSR( mbox + offsetof(RECVMBOX, recv_list),
                             WRK.recv_list[WRK.Recv_Index].rbufd );
         	ioa = (int)BUSIO_ATT(  (ulong)DDI.cc.bus_id, DDI.ds.io_port );
		PIO_PUTC( ioa + COMMAND_REG, RX_START );
		BUSIO_DET( ioa );
		/* Check the mailbox status/error bit */
		WRK.mbox_status = recv;
		mail_check_status(dds_ptr);
	}

	BUSMEM_DET( bus );

	DTRACE1 ("rcvE");
	return;

} /* end ent_recv routine                                                    */

/*****************************************************************************/
/*                                                                           */
/* NAME: ent_tx_off_lvl                                                      */
/*                                                                           */
/* FUNCTION: Acknowledge transmit completed elements                         */
/*           Que up more elements to be transmitted                          */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the offlevel interrupt handler.         */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*           Command register - tells if the transmit completed without error*/
/*                                                                           */
/*    Output: Transmits acknowledged and new transmits started.              */
/*                                                                           */
/*    Called From: xxx_offl                                                  */
/*                                                                           */
/*    Calls To: d_complete, ent_xmit_done, ent_xmit, M_XMEMD                 */
/*                                                                           */
/* RETURN:	None							     */
/*                                                                           */
/*****************************************************************************/

void
ent_tx_off_lvl (
	dds_t *dds_ptr,	/* data area for this adapter */
	uchar cmd_reg)	/* type of command interrupt */
{
	uint bus;
	struct xmit_elem *xel;
	uchar status;
	int i;
	int bdrc;
	int rc;
	/* table for translating adapter error codes to ethernet error
	 * codes
	 */
	static ENT_EXCEPT err_tab[7] = {
	  /* TE_OK */ CIO_OK, /* TE_COLLISION */ ENT_TX_MAX_COLLNS,
	  /* TE_UNDERRUN */ ENT_TX_UNDERRUN, /* TE_CARRIER */ ENT_TX_CD_LOST,
	  /* TE_CTS_LOST */ ENT_TX_CTS_LOST, /* TE_TIMEOUT */ ENT_TX_TIMEOUT,
	  /* TE_SIZE */ ENT_TX_BAD_SIZE };
		
	DTRACE3("txoB", dds_ptr, cmd_reg);

	/* Turn of the watch dog timer, and reset the reason why it was set
	 */
	if (WRK.wdt_setter == WDT_XMIT) {
		w_stop(&(WDT));
		WRK.wdt_setter = WDT_INACTIVE;
	}

	/* get addressability to adapter memory
	 */
	bus = (int)BUSMEM_ATT((ulong)WRK.bus_id, WRK.bus_mem_addr);

	if ((cmd_reg == TX_P_SENT) || (cmd_reg == TX_ERROR))
	{
		while (TRUE)
		{
			xel = &WRK.xmit_queue[WRK.tx_list_next_out];
			if (xel->in_use == FALSE)
				break;

			/* get status byte from buffer descriptor
			 */
			PIO_GETCX((char *)(bus + xel->tx_ds->des_offset),
						&status);

			/* check for transmit complete and no error
			 */
			if ((status & (CMPLT_BIT_MASK|BD_ERR_CD_MASK))
							== CMPLT_BIT_MASK)
			{
				bdrc = CIO_OK;

			}

			/* if the transmit complete bit is not set then
			 * break out of loop
			 */
			else if (!(status & CMPLT_BIT_MASK))
			{
				break;
			}
			else
			{
				/* xmit completed with error do
				 * the error loggin
				 */
				if ((status & BD_ERR_CD_MASK) > TE_SIZE)
				{
					ASSERT(0);
					bdrc = CIO_OK;
				}
				else
				{
					bdrc = err_tab[status&BD_ERR_CD_MASK];
				}
				ent_logerr(dds_ptr, ERRID_ENT_ERR2, bdrc,
					cmd_reg, 0);
				if (bdrc == ENT_TX_CD_LOST)
					ent_logerr(dds_ptr, ERRID_ENT_ERR6,
						bdrc, cmd_reg, 0);
			}

			/* check dma status
			 */
			rc = d_complete(WRK.dma_channel, DMA_WRITE_ONLY,
				xel->tx_ds->sys_addr, (size_t)xel->bytes,
				(struct xmem *)&WRK.xbuf_xd,
				(char *)xel->tx_ds->io_addr);
			ASSERT(rc == DMA_SUCC);
			if (rc != DMA_SUCC)
				ent_logerr(dds_ptr, ERRID_ENT_ERR5, EFAULT,
						cmd_reg, 0);

			/* pass transmit to cio code
			 */
			ent_xmit_done(dds_ptr, xel, bdrc);

			/* mark transmit element as free
			 */
			WRK.tx_tcw_use_count--;
			xel->in_use = FALSE;
			XMITQ_INC(WRK.tx_list_next_out);

		}
	}
	else if (cmd_reg == TX_ABORT)
	{
		TRACE1("tx_a");
		if (WRK.adpt_start_state == X_ABORT_1)
		{
			TRACE2("slpE", &(WRK.close_event));
			e_wakeup((int *)&(WRK.close_event));
		}
		if (WRK.adpt_start_state == SLEEPING) {
			WRK.adpt_start_state = X_ABORT_1;
		}
	}
	else
	{
		TRACE2("tx_un", (ulong)cmd_reg);
	}


	 /* check if there is room on the adapter to transmit another
	  * packet
	  */
	if ((WRK.tx_list_next_in != WRK.tx_list_next_buf) &&
				((WRK.xmit_tcw_cnt - 1) > WRK.tx_tcw_use_count))
		ent_xmit(dds_ptr, (char *)bus);

	/* restart watchdog timer if there are more tranmissions
	 */
	if (WRK.tx_tcw_use_count)
	{
		WRK.wdt_setter = WDT_XMIT;
		w_start(&(WDT));
	}

	BUSMEM_DET(bus);

	DTRACE1("txoC");

}


/*****************************************************************************/
/*
 * NAME: ent_xmit_done
 *
 * FUNCTION: process a completed transmission
 *
 * EXECUTION ENVIRONMENT:
 *	Called by offlevel interrup handler
 *
 * RETURNS: None
 *
 */
/*****************************************************************************/

void
ent_xmit_done (
	dds_t	*dds_ptr,	/* dds structure for adapter */
	xmit_elem_t *xelm,	/* transmit element structure */
	int	status)		/* status of transmit */
{
	chan_t chan;
	chan_state_t chan_state;
	open_elem_t *open_ptr;
	cio_stat_blk_t stat_blk;

	DTRACE4("XMTb", dds_ptr, xelm, status);

	if (status == CIO_OK)
	{
		/* update the standard counter
		 */
		if (ULONG_MAX == RAS.cc.tx_frame_lcnt)
			RAS.cc.tx_frame_mcnt++;
		RAS.cc.tx_frame_lcnt++;
		if ((ULONG_MAX - xelm->mbufp->m_len) < RAS.cc.tx_byte_lcnt)
			RAS.cc.tx_byte_mcnt++;
		RAS.cc.tx_byte_lcnt += xelm->mbufp->m_len;

		/*
		* give a copy to promiscuous mode channels
		*/
		if (CIO.promiscuous_count) {

			int i = CIO.promiscuous_count;
			int j;
			int count = xelm->mbufp->m_len;
			struct mbuf *mbufp;
			cio_read_ext_t rd_ext;
			

			RAS.cc.rx_frame_lcnt++;
			if (RAS.cc.rx_frame_lcnt == ULONG_MAX)
				RAS.cc.rx_frame_mcnt++;
			if ((ULONG_MAX - count) <  RAS.cc.rx_byte_lcnt)
				RAS.cc.rx_byte_mcnt++;
       	                  RAS.cc.rx_byte_lcnt += count;

			for (j = 0; j < MAX_OPENS; j++) {
				if ((open_ptr = CIO.open_ptr[j]) &&
				   	open_ptr->prom_on_cnt) {
                        		if (count <= MHLEN - WRK.rdto) {
                                		mbufp = m_gethdr(M_DONTWAIT, MT_DATA);
					}
                        		else {
                                		mbufp = m_getclust(M_DONTWAIT,MT_DATA);
					}
					if (mbufp) {
					  mbufp->m_data += WRK.rdto;
					  mbufp->m_len = count;
					  bcopy(MTOD(xelm->mbufp, char *), MTOD(mbufp, char *), count);
					  if (xelm->mbufp->m_flags & M_BCAST) {
						mbufp->m_flags |= M_BCAST;
					  }
					  if (xelm->mbufp->m_flags & M_MCAST) {
						mbufp->m_flags |= M_MCAST;
					  }

					  rd_ext.status = CIO_OK;
       		                    	  (*(open_ptr->rec_fn)) (open_ptr->open_id,
       		                             		&rd_ext, mbufp);
                        		}
					else 
                        		  RAS.ds.rec_no_mbuf++;

					if ((i -= open_ptr->prom_on_cnt) == 0)
					  break;
				}
			}   /*  for loop */
		}  /* if there are promiscuous users */
	}

	open_ptr = CIO.open_ptr[xelm->chan - 1];
	chan_state = CIO.chan_state[xelm->chan - 1];

	if (open_ptr != NULL)
	{
		AIXTRACE(TRC_WEND, open_ptr->devno, open_ptr->chan,
			xelm->mbufp, xelm->mbufp->m_len);

		if ((xelm->wr_ext.flag & CIO_ACK_TX_DONE) &&
					(chan_state == CHAN_OPENED))
		{
			stat_blk.code = CIO_TX_DONE;
			stat_blk.option[0] = status;
			stat_blk.option[1] = xelm->wr_ext.write_id;
			stat_blk.option[2] = (ulong)xelm->mbufp;
			stat_blk.option[3] = 0;

			cio_report_status(dds_ptr, open_ptr, &stat_blk);
		}
	}

	/* free mbuf if needed
	 */
	if (xelm->free)
		m_freem(xelm->mbufp);


	/* check if anyone needs to be notified of a transmit function, if
	 * flag is set check each open and call wakeup function
	 */
	if (CIO.xmt_fn_needed)
	{
		CIO.xmt_fn_needed = FALSE;
		for (chan = 0; chan < MAX_OPENS; chan++)
		{
			if ((CIO.chan_state[chan] == CHAN_OPENED) &&
			((open_ptr = CIO.open_ptr[chan]) != NULL) &&
			(open_ptr->xmt_fn_needed)                    )
			{
				DTRACE1("XDN1");
				open_ptr->xmt_fn_needed = FALSE;
				(*(open_ptr->xmt_fn)) (open_ptr->open_id);
				DTRACE1("XDN2");
			}
		}
	}

	/* check if anyone is blocked on a transmit, if they are check
	 * each open and do a wakeup if needed
	 */
	if (CIO.xmit_event)
	{
		CIO.xmit_event = FALSE;
		for (chan = 0; chan < MAX_OPENS; chan++)
		{
			if ((CIO.chan_state[chan] == CHAN_OPENED) &&
				((open_ptr = CIO.open_ptr[chan]) != NULL) &&
				(open_ptr->xmt_event != EVENT_NULL))
			{
				e_wakeup((int *)&open_ptr->xmt_event);
			}
		}
	}

	DTRACE1("XMTe");
	return;
}

/*****************************************************************************/
/*
 * NAME: ent_fastwrt
 *
 * FUNCTION: fast write function for kernel
 *
 * EXECUTION ENVIORNMENT:
 *	Can be called from interrupt or process level
 *
 * RETURNS:
 *	0 if successful
 *	errno value on failure
 */
/*****************************************************************************/
int
ent_fastwrt (
	dev_t		devno,		/* major minor number */
	struct mbuf *mp)
{
	dds_t *dds_ptr;
	int ipri;
	xmit_elem_t *xlm;
	int bus;
	int total_bytes;


	dds_ptr = dd_ctrl.dds_ptr[minor(devno)];
	if (dds_ptr == NULL)
		return(ENODEV);


	ipri = i_disable(INTCLASS2); 
/*	ipri = i_disable(INTOFFL1); */

	if (XMITQ_FULL || WRK.adpt_start_state != STARTED)

	{
		i_enable(ipri);
		return(EAGAIN);
	}

	xlm = &WRK.xmit_queue[WRK.tx_list_next_buf];
	XMITQ_INC(WRK.tx_list_next_buf);

	/* update the number of transmits queued, and update if it is
	 * greater than the high water mark.
	 */
	WRK.xmits_queued++;
	if (WRK.xmits_queued > RAS.cc.xmt_que_high)
		RAS.cc.xmt_que_high = WRK.xmits_queued;

	xlm->mbufp = mp;
	xlm->wr_ext.flag = 0;
	xlm->free = TRUE;
	xlm->chan = 1; /* safe value for xmit done */
	/* calculate the number of bytes in mbuf chain */
	total_bytes = 0;
        do
        {
                total_bytes += mp->m_len;
                mp = mp->m_next;
        } while(mp != NULL);
        xlm->bytes = total_bytes;
	AIXTRACE(TRC_WQUE, devno, -1, xlm->mbufp, total_bytes);

        if ((WRK.xmit_tcw_cnt - 1) > WRK.tx_tcw_use_count)
	{
		bus = BUSMEM_ATT(WRK.bus_id, WRK.bus_mem_addr);
		ent_xmit(dds_ptr, bus);
		BUSMEM_DET(bus);
		/* start watchdog timer */
		WRK.wdt_setter = WDT_XMIT;
		w_start(&(WDT));
	}

	i_enable(ipri);

	return(0);

}

/*****************************************************************************/
/*
 * NAME: xmit_sleep
 *
 * FUCNTION: wait on room int transmit queue
 *
 * EXECUTION ENVIORNMENT:
 *	Called under process level, can be called on interrupt level with
 *	NDELAY set
 *
 *	Caller must be disabled to offlevel
 *
 * RETURNS:
 *	0 if successful
 *	errno value on failure
 */
/*****************************************************************************/

int
xmit_sleep (
	dds_t *dds_ptr,
	open_elem_t *open_ptr,
	int fmode,
	cio_write_ext_t *wr_ext)
{

	/* check if we should block based on NDELAY
	 */
	if (fmode & DNDELAY)
	{
		/* if caller is from kernel, then set up to so xmt_fn gets
		 * called when there is room
		 */
		if (open_ptr->devflag & DKERNEL)
		{
			CIO.xmt_fn_needed = TRUE;
			open_ptr->xmt_fn_needed = TRUE;
		}

		/* update status and set error code
		 */
		wr_ext->status = CIO_TX_FULL;
		TRACE2("SLP1", EAGAIN);
		return(EAGAIN);
	}
	else
	{
		/* set flag so offlevel will know there is someone waiting
		 * on room in the queue
		 */
		CIO.xmit_event = TRUE;
		if (e_sleep((int *)&open_ptr->xmt_event, EVENT_SIGRET) !=
								EVENT_SUCC)
		{
			TRACE2("SLP2", EINTR);
			return(EINTR);
		}
	}
	return(0);

}

/*****************************************************************************/
/*
 * NAME: get_user
 *
 * FUNCTION: copy user data into an mbuf
 *
 * EXECUTION ENVIORNMENT:
 *	Called from entwrite.  Runs only under a process
 *
 * RETURNS:
 *	0 if successful
 *	-1 on failure
 */
/*****************************************************************************/

int
get_user (
	dds_t *dds_ptr,
	struct mbuf **mpp,		/* mbuf pointer returned here */
	struct uio *uiop,		/* uio struct for user data */
	open_elem_t *open_ptr,
	cio_write_ext_t *wr_ext)
	
{
	int total_bytes;
	struct mbuf *mbufp;
	int ipri;
	int rc;

	/* check for invalid transfer size
	 */
	total_bytes = uiop->uio_resid;
	if ((total_bytes < ent_MIN_PACKET) || (total_bytes > ent_MAX_PACKET))
	{
		TRACE2("WGU1", EINVAL);
		return(EINVAL);
	}

	/* check that there is room on queue before using an mbuf
	 */
	ipri = i_disable(INTOFFL1);
	while (XMITQ_FULL)
	{
		rc = xmit_sleep(dds_ptr, open_ptr, uiop->uio_fmode, wr_ext);
		if (rc != 0)
		{
			i_enable(ipri);
			return(rc);
		}
	}
	i_enable(ipri);


	/* get an mbuf to copy data into
	 */
	mbufp = m_get(M_WAIT, MT_DATA);
	if (mbufp == NULL)
	{
		TRACE2("WGU2", ENOMEM);
		return(ENOMEM);
	}

	/* get a cluster if one is needed
	 */
	if (total_bytes > MLEN)
	{
		m_clget(mbufp);
		if (!M_HASCL(mbufp))
		{
			TRACE3("WGU3", ENOMEM, mbufp->m_len);
			m_free(mbufp);
			return(ENOMEM);
		}
	}

	/* move data from user space to mbuf
	 */
	if (uiomove(MTOD(mbufp, uchar *), total_bytes, UIO_WRITE, uiop))
	{
		TRACE2("WGU4", EFAULT);
		m_free(mbufp);
		return(EFAULT);
	}
	mbufp->m_len = total_bytes;
	*mpp = mbufp;

	return(0);
}

/*****************************************************************************/
/*
 * NAME:     entwrite
 *
 * FUNCTION: write entry point from kernel
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * RETURNS:  0 or errno
 *           if successful, also returns indirectly the number of bytes
 *           written through the updating of uiop->uio_resid by uiomove
 *
 */
/*****************************************************************************/
int entwrite (
	dev_t            devno,  /* major and minor number */
	struct uio      *uiop,   /* pointer to uio structure */
	chan_t           chan,   /* channel number */
	cio_write_ext_t *extptr) /* pointer to write extension structure */
{
	dds_t		*dds_ptr;
	int		adap;
	open_elem_t     *open_ptr;
	struct mbuf     *mbufp;
	struct mbuf	*mp;
	int             rc;
	int             ndx;
	cio_write_ext_t wr_ext;
	int		ipri;
	int		bus;
	char		free;
	short		total_bytes;

	/* this shouldn't fail if kernel and device driver are working
	 * correctly
	 */
	adap = minor(devno);

	if ((adap >= MAX_ADAPTERS_INDEX) || (chan <= 0))
		return(ENXIO);
	if (chan > MAX_OPENS)
		return(EBUSY);
	if ((dds_ptr = dd_ctrl.dds_ptr[adap]) == NULL)
		return(ENODEV);
	if ((CIO.chan_state[chan-1] != CHAN_OPENED)  ||
	    ((open_ptr = CIO.open_ptr[chan-1]) == NULL)    )
		return(ENXIO);

	/* don't process transmits for diagnostics
	 */
	if (CIO.mode == 'D')
		return(0);

	/* don't allow writes unless a connection has been completed
	 * in the past
	 */
	if (CIO.device_state != DEVICE_CONNECTED)
	{

		TRACE2 ("WRI3", (ulong)ENOCONNECT);
		return (ENOCONNECT);
	}

	/* handle the extended parameter
	 */
	if (extptr == NULL)
	{
		/* 
		 * build our own with defaults
		 */
		wr_ext.status = (ulong)CIO_OK;
		wr_ext.flag = 0; /* don't notify complete, free mbuf  */
		wr_ext.write_id = 0; /* doesn't matter -no notification */
		wr_ext.netid = 0;
	}
	else
	{
		/* copy the write extension pointer into the transmit element
		 */
		if (open_ptr->devflag & DKERNEL)
		{
			wr_ext = *extptr;
		}
		else
		{
			rc = copyin(extptr, &wr_ext, sizeof(wr_ext));
			if (rc != 0)
			{
				TRACE2 ("WRI2", EFAULT);
				return(EFAULT);
			}

			if (wr_ext.flag & CIO_NOFREE_MBUF)
			{
				TRACE2 ("WRI3", EINVAL);
				return(EINVAL);
			}

		}
	}

	if (open_ptr->devflag & DKERNEL)
	{
		mbufp = (struct mbuf *)(uiop->uio_iov->iov_base);
		free = !(wr_ext.flag & CIO_NOFREE_MBUF);
	}
	else
	{
		/* copy the user data into an mbuf
		 */
		rc = get_user(dds_ptr, &mbufp, uiop, open_ptr, &wr_ext);
		if (rc != 0)
		{
			copyout(&wr_ext, extptr, sizeof(wr_ext));
			return(rc);
		}
		free = TRUE;
	}

	/* caclulate the number of bytes in mbuf chain
	 */
	mp = mbufp;
	total_bytes = 0;
	do
	{
		total_bytes += mp->m_len;
		mp = mp->m_next;
	} while(mp != NULL);

	/* check for valid mbuf length
	 */
	if ((total_bytes < ent_MIN_PACKET) || (total_bytes > ent_MAX_PACKET))
	{
		TRACE2("WRI9", EINVAL);
		return(EINVAL);
	}

	AIXTRACE (TRC_WQUE, devno, chan, mbufp, mbufp->m_len);

	/* serialize access to adapter transmit lists
	 */
	ipri = i_disable(INTOFFL1);

	/* while there is no room on transmit queue block
	 */
	while (WRK.adpt_start_state == STARTED && XMITQ_FULL)
	{
		rc = xmit_sleep(dds_ptr, open_ptr, uiop->uio_fmode, &wr_ext);
		if (rc != 0)
		{
			i_enable(ipri);
			COPYOUT(open_ptr->devflag, &wr_ext, extptr,
					sizeof(wr_ext));
			if (!(open_ptr->devflag & DKERNEL))
				m_free(mbufp);
			return(rc);
		}
	}

	if (WRK.adpt_start_state != STARTED) {
		/*
		 * protect against mucking with the transmit queues
		 * unless we are fully up and running.
		 */
		i_enable(ipri);
		if (!(open_ptr->devflag & DKERNEL))
			m_free(mbufp);

		return EAGAIN;
	}


	/* copy transmit element into queue, and advance next in pointer
	 */
	WRK.xmit_queue[WRK.tx_list_next_buf].mbufp = mbufp;
	WRK.xmit_queue[WRK.tx_list_next_buf].chan = chan;
	WRK.xmit_queue[WRK.tx_list_next_buf].wr_ext = wr_ext;
	WRK.xmit_queue[WRK.tx_list_next_buf].free = free;
	WRK.xmit_queue[WRK.tx_list_next_buf].bytes = total_bytes;

	XMITQ_INC(WRK.tx_list_next_buf);

	/* update the number of transmits queued, and update if it is
	 * greater than the high water mark.
	 */
	WRK.xmits_queued++;
	if (WRK.xmits_queued > RAS.cc.xmt_que_high)
		RAS.cc.xmt_que_high = WRK.xmits_queued;

	/* if there are no tranmit buffers available then return
	 */
	if ((WRK.xmit_tcw_cnt - 1) > WRK.tx_tcw_use_count)
	{
		bus = (int)BUSMEM_ATT((ulong)WRK.bus_id, WRK.bus_mem_addr);
		ent_xmit(dds_ptr, (char *)bus);
		BUSMEM_DET(bus);

		/* start watchdog timer
		 */
		WRK.wdt_setter = WDT_XMIT;
		w_start(&(WDT));
	}


	i_enable(ipri);
	return(0);

}

/*****************************************************************************/
/*
 * NAME: ent_xmit
 *
 * FUNCTION: add packet to adapter transmit list, and start it transmitting
 *
 * EXECUTION ENVIORNMENT:
 *	Called under process and interrupt level
 *	Caller must be disabled to INTOFFL1
 *
 * NOTES:
 *	Caller must insure there is room on the transmit queue and there
 *	is a packet to transmit
 *
 * RETURNS:
 *	NONE
 */
/*****************************************************************************/

void
ent_xmit (
	dds_t *dds_ptr,		/* adapter to transmit */
	char *bus)		/* address of IO bus */
{
	xmit_elem_t *xel;
	xmit_des_t *xd;
	uint buf_off;
	uint el_offset;
	ushort xmit_next_off;
	int rc, rc1;
	
	if (WRK.adpt_state != normal)
		return;

	/* while there are more tramit elements on software queue then
	 * start a tranmission
	 */
	while(TRUE)
	{
		/* sanity checks
		 */
		ASSERT(WRK.tx_list_next_in >= 0);
		ASSERT(WRK.tx_list_next_buf >= 0);
		ASSERT(WRK.tx_list_next_out >= 0)
		ASSERT(WRK.tx_list_next_in < DDI.cc.xmt_que_size);
		ASSERT(WRK.tx_list_next_buf < DDI.cc.xmt_que_size);
		ASSERT(WRK.tx_list_next_out < DDI.cc.xmt_que_size);
		ASSERT(WRK.tx_des_next_in >= 0);
		ASSERT(WRK.tx_des_next_in <= WRK.xmit_tcw_cnt);

		xel = &WRK.xmit_queue[WRK.tx_list_next_in];
		xd = &WRK.tx_buf_des[WRK.tx_des_next_in];

		xel->tx_ds = xd;
		xel->in_use = (char)TRUE;

		buf_off = (uint)bus + xd->des_offset;

		/* tell the adapter how many bytes to send
		 */
		PIO_PUTSRX((short *)(buf_off + offsetof(BUFDESC, count)),
								 xel->bytes);

		/* copy data into tranmit buffer and do processor cache
		 * flush
		 */
		m_copydata(xel->mbufp, 0, xel->bytes, xd->sys_addr);
		rc = vm_cflush(xd->sys_addr, xel->bytes);
		ASSERT(rc == 0);

		/* increment the buffer use count, and advance the next
		 * transmit element in counter, also reduce count on
		 * transmits queued
		 */
		WRK.xmits_queued--;
		WRK.tx_tcw_use_count++;
		XMITQ_INC(WRK.tx_list_next_in);
		WRK.tx_des_next_in++;
		if (WRK.tx_des_next_in >= WRK.xmit_tcw_cnt)
			WRK.tx_des_next_in = 0;

		/* if all the tranmit buffers are in use then exit
		 */
		if ((WRK.tx_tcw_use_count == (WRK.xmit_tcw_cnt - 1)) ||
			(WRK.tx_list_next_in == WRK.tx_list_next_buf))
			break;

		/* there is another tranmit element put onto the adapter
		 * trasmit list so only set the end op packet bit, and clear
		 * the status field
		 */
		PIO_PUTSX((short *)(buf_off + offsetof(BUFDESC, status)),
							(short)EOP_BIT_MASK);
	}

	rc1 = PIO_GETC((char *)(bus + WRK.xmit_el_off->des_offset +
		offsetof(BUFDESC, control)));

	if ( !(rc1 & EL_BIT_MASK))
	   {
		TRACE3("outs", rc, EL_BIT_MASK);
		ent_logerr(dds_ptr, ERRID_ENT_ERR1, rc, EL_BIT_MASK, 99);
	   }

	/* move the end of list to last tramit element added to adapter
	 * queue
	 */
	PIO_PUTSX((short *)(buf_off + offsetof(BUFDESC, status)),
					(short)(EL_BIT_MASK+EOP_BIT_MASK));
	PIO_PUTCX((char *)(bus + WRK.xmit_el_off->des_offset +
		offsetof(BUFDESC, control)), (char)EOP_BIT_MASK);

	/* save buffer descrpitor that now contains end of list bit
	 */
	WRK.xmit_el_off = xd;

}
