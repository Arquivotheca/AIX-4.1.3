static char sccsid[] = "@(#)38	1.30.1.3  src/bos/kernext/tokdiag/tokrecv.c, diagddtok, bos411, 9428A410j 10/26/93 16:23:48";
/*
 * COMPONENT_NAME: (SYSXTOK) - Token-Ring device handler
 *
 * FUNCTIONS:  tokread, proc_recv, tok_recv_start, tok_recv_setup,
 *             tok_recv_undo, tok_recv_init, tok_recv_frame,
 *             load_recv_chain, read_recv_chain, clear_recv_chain,
 *             arm_recv_list, read_recv_list, clear_recv_list,
 *             setup_recv_tcws, free_recv_tcws, recv_mbuf_timer,
 *             setup_mbuf_timer, start_mbuf_timer, free_mbuf_timer
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/cblock.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/dump.h>
#include <sys/errno.h>
#include <sys/err_rec.h>
#include "tok_comio_errids.h"
#include <sys/file.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/sleep.h>
#include <sys/timer.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <sys/watchdog.h>
#include <sys/xmem.h>
#include <sys/comio.h>
#include <sys/trchkid.h>
#include <sys/tokuser.h>
#include <sys/adspace.h>

#include "tokddi.h"
#include "tokdshi.h"
#include "tokdds.h"
#include "tokdslo.h"
#include "tokproto.h"
#include "tokprim.h"

extern tracetable_t     tracetable;
extern cdt_t            ciocdt;
extern dd_ctrl_t        dd_ctrl;

/*--------------------------  T O K R E A D  ---------------------------*/
/*                                                                      */
/*  NAME: tokread                                                       */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Read entry point to Token Ring driver from kernel (this is      */
/*      used by user processes only).                                   */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can only be called on a user process thread.                    */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Unlinks and deallocates receive elements from the read queue.   */
/*                                                                      */
/*  RETURNS: 0 or errno                                                 */
/*      If successful, also returns indirectly the number of bytes      */
/*      read through the updating of uiop->uio_resid.                   */
/*                                                                      */
/*----------------------------------------------------------------------*/

int tokread (
   dev_t           devno,  /* major and minor number */
   struct uio     *uiop,   /* pointer to uio structure */
   chan_t          chan,   /* channel number */
   cio_read_ext_t *extptr) /* optional pointer to read extension structure */
{
   register dds_t *p_dds;
   register int    adap;
   open_elem_t    *open_ptr;
   rec_elem_t     *rec_ptr;
   cio_read_ext_t  local_rd_ext;
   int             rc;
   int             saved_intr_level;
   int             ndx;
   struct mbuf    *tempmbufp;
   int             total_bytes;
   int             bytes_to_move;

   DEBUGTRACE5 ("REAb", (ulong)devno, (ulong)uiop, (ulong)chan,
      (ulong)extptr); /* tokread begin */
   /* this shouldn't fail if kernel and device driver are working correctly */
   adap=minor(devno);
   if ((chan <= 0)                                 ||
       (chan > MAX_OPENS)                          ||
       ((p_dds = dd_ctrl.p_dds[adap]) == NULL) ||
       (CIO.chan_state[chan-1] != CHAN_OPENED)     ||
       ((open_ptr = CIO.open_ptr[chan-1]) == NULL)    )
   {
      TRACE2 ("REA1", (ulong)ENXIO); /* tokread end (bad device or channel) */
      return (ENXIO);
   }
   if (open_ptr->devflag & DKERNEL) /* illegal call from kernel process */
   {
      TRACE2 ("REA2", (ulong)EACCES); /* tokread end (call from kernel proc) */
      return (EACCES);
   }
   /* don't allow reads unless a connection has been completed in the past */
   if (CIO.device_state != DEVICE_CONNECTED)
   {
      TRACE2 ("REA3", (ulong)ENOCONNECT); /* tokread end (not connected) */
      return (ENOCONNECT);
   }
   /* get receive element if there is any data to be read */
   DISABLE_INTERRUPTS (saved_intr_level);
   while ((rec_ptr = (rec_elem_t *) sll_unlink_first (
      (s_link_list_t *)(&(open_ptr->rec_que)))) == NULL)
   {
      /* there's no data -- maybe because there's no netid in table for us */
      rc = EIO;
      for (ndx = 0; ndx < TOK_MAX_NETIDS; ndx++)
      {
         if ( (CIO.netid_table[ndx].chan == chan) && 
	      (CIO.netid_table[ndx].inuse) )
         {
            rc = 0;
            break;
         }
      }
      if (rc == EIO)
      {
         /* handle problem of read without start currently in effect */
         ENABLE_INTERRUPTS (saved_intr_level);
         local_rd_ext.status = (ulong)CIO_NOT_STARTED;
         (void)COPYOUT (open_ptr->devflag, &local_rd_ext, extptr,
                         sizeof(local_rd_ext));
         TRACE2 ("REA4", (ulong)EIO); /* tokread end (no active netid) */
         return (EIO);
      }
      /* well, there's simply no data so action depends on DNDELAY */
      if (uiop->uio_fmode & DNDELAY) /* user doesn't want to block */
      {
         ENABLE_INTERRUPTS (saved_intr_level);
         TRACE2 ("REA5", (ulong)0); /* tokread end (no data with DNDELAY) */
         return (0); /* user will see 0 bytes read */
      }
      /* block by sleeping until there's data (wakeup in proc_recv) */
      if (SLEEP (&open_ptr->rec_event) != EVENT_SUCC)
      {
         ENABLE_INTERRUPTS (saved_intr_level);
         TRACE2 ("REA6",
            (ulong)EINTR); /* tokread end (blocking sleep interrupted) */
         return (EINTR);
      }
      /* now we'll loop back up to while and try again to get rec element */
   }
   ENABLE_INTERRUPTS (saved_intr_level);
   /* at this point, we have data for user and will pass ds rd_ext */
   rc = 0;
   /* calculate total bytes available */
   for (tempmbufp = rec_ptr->mbufp, total_bytes = 0;
        tempmbufp != NULL;
        tempmbufp = tempmbufp->m_next)
   {
      total_bytes += tempmbufp->m_len;
   }
   /* if we have too much data, action depends on existence of extptr */
   if (total_bytes > uiop->uio_resid)
   {
      if (extptr == NULL) /* there's no other way to report error */
         rc = EMSGSIZE;
      else /* we can supply part of data and report overflow in *extptr */
      {
         rec_ptr->rd_ext.status = (ulong)CIO_BUF_OVFLW;
         total_bytes = uiop->uio_resid;
      }
   }
   /* if user has room or a rd_ext was provided for status then move the data*/
   for (tempmbufp = rec_ptr->mbufp;
        (rc == 0) && (total_bytes > 0);
        total_bytes -= bytes_to_move, tempmbufp = tempmbufp->m_next)
   {
      bytes_to_move = tempmbufp->m_len;
      if (bytes_to_move > total_bytes) /* limit data from this mbuf */
         bytes_to_move = total_bytes;
      if (uiomove (MTOD(tempmbufp,uchar *), bytes_to_move, UIO_READ, uiop))
         rc = EFAULT;
   }
   AIXTRACE (TRC_REND, devno, chan, rec_ptr->mbufp, rec_ptr->bytes);
   /* free the mbuf(s) */
   m_freem (rec_ptr->mbufp);
   /* if no error, update caller's read extension if one is provided */
   if (rc == 0)
      (void)COPYOUT (open_ptr->devflag, &(rec_ptr->rd_ext), extptr,
                      sizeof(rec_ptr->rd_ext));
   /* free the rec element */
   sll_free_elem ((s_link_list_t *)(&(open_ptr->rec_que)),
      (sll_elem_ptr_t)rec_ptr);
   DEBUGTRACE2 ("REAe", (ulong)rc); /* tokread end */
   return (rc);
}

/*--------------------  C I O _ P R O C _ R E C V  ---------------------*/
/*                                                                      */
/*  NAME: proc_recv                                                     */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Process a receive element.                                      */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Called by the offlevel interrupt handler.                       */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Allocates a receive element and adds it the receive element     */
/*      queue.                                                          */
/*                                                                      */
/*  RETURNS: Nothing                                                    */
/*                                                                      */
/*----------------------------------------------------------------------*/

void proc_recv (
   register dds_t       *p_dds,  /* pointer to dds structure */
   register open_elem_t *open_ptr, /* pointer to open structure */
   register rec_elem_t  *rec_ptr)  /* pointer to receive element structure */
{ /* proc_recv begin */
   rec_elem_t     *new_rec_ptr;
   cio_stat_blk_t  stat_blk;

   DEBUGTRACE4 ("RECb", (ulong)p_dds, (ulong)open_ptr, (ulong)rec_ptr);
   /* update the standard counters */
   if (ULONG_MAX == RAS.cc.rx_frame_lcnt)
      RAS.cc.rx_frame_mcnt++;           /* record overflow in msh of counter */
   RAS.cc.rx_frame_lcnt++;
   if ((ULONG_MAX - rec_ptr->bytes) < RAS.cc.rx_byte_lcnt)
      RAS.cc.rx_byte_mcnt++;            /* record overflow in msh of counter */
   RAS.cc.rx_byte_lcnt += rec_ptr->bytes;

   /*
    *  NOTE:
    *   If we are not yet out of Limbo, we must
    *   reject this receive data because our users may not be
    *   ready for it since we have not yet notified them that we
    *   are out of Limbo.
    */
   if ( p_dds->wrk.limbo != PARADISE )
   {
         AIXTRACE (TRC_ROVR, open_ptr->devno, open_ptr->chan,
            rec_ptr->mbufp, rec_ptr->bytes);
         m_freem (rec_ptr->mbufp); /* free the mbuf -- lose data */
         return;
   }

   if (open_ptr->devflag & DKERNEL) /* data is for a kernel process */
   {
      /* call kernel process read_data_available function (will free mbuf) */
      DEBUGTRACE1 ("KRFc"); /* kernel proc read function call */
      (*(open_ptr->rec_fn)) (open_ptr->open_id,
                             &(rec_ptr->rd_ext),
                             rec_ptr->mbufp);
      DEBUGTRACE1 ("KRFr"); /* kernel proc read function returned */
      AIXTRACE (TRC_RNOT, open_ptr->devno, open_ptr->chan,
         rec_ptr->mbufp, rec_ptr->bytes);
   }
   else /* data is for a user process */
   {
      /* actions depend on whether there is room in receive que */
      new_rec_ptr =
       (rec_elem_t *) sll_alloc_elem ((s_link_list_t *)(&(open_ptr->rec_que)));
      if (new_rec_ptr != NULL)
      {
         *new_rec_ptr = *rec_ptr;
         /* add the element to the receive queue */
         sll_link_last ((s_link_list_t *)(&(open_ptr->rec_que)),
            (sll_elem_ptr_t)new_rec_ptr);
         AIXTRACE (TRC_RQUE, open_ptr->devno, open_ptr->chan,
            rec_ptr->mbufp, rec_ptr->bytes);
         /* notify any waiting user process there is data available */
         if (open_ptr->selectreq & POLLIN)
         {
            open_ptr->selectreq &= ~POLLIN;
            selnotify (open_ptr->devno, open_ptr->chan, POLLIN);
         }
         /* if user is blocked on read, wake him up */
         if (open_ptr->rec_event != EVENT_NULL)
         {
            WAKEUP (&open_ptr->rec_event);
         }
      }
      else
      {
         AIXTRACE (TRC_ROVR, open_ptr->devno, open_ptr->chan,
            rec_ptr->mbufp, rec_ptr->bytes);
         TRACE2("REC1",(ulong)p_dds);/* proc_recv ERROR (recv que full)*/
         m_freem (rec_ptr->mbufp); /* free the mbuf -- lose data */
         RAS.ds.rec_que_overflow++;

         /* now attempt to que a status element to report the overflow */
         stat_blk.code = (ulong)CIO_ASYNC_STATUS;
         stat_blk.option[0] = (ulong)CIO_LOST_DATA;
         que_stat_blk (p_dds, open_ptr, &stat_blk);
      }
   }
   DEBUGTRACE1 ("RECe"); /* proc_recv */
   return;
}

/*-------------------  T O K _ R E C V _ S E T U P  --------------------*/
/*                                                                      */
/*  NAME: tok_recv_setup                                                */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Initializes the receive portion of the driver for handling      */
/*      receive data from the adapter.                                  */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      This is a top-half routine in this driver and can only be       */
/*      executed under a user process.                                  */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Clears all receive data structures to init state; including     */
/*      the read index, mbuf list, and read address lists.              */
/*                                                                      */
/*  RETURNS:                                                            */
/*      0               Completed Successfully.                         */
/*      ENOMEM          Not enough memory.                              */
/*      ENOBUFS         Not enough mbufs.                               */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: tok_recv_init, setup_recv_tcws,         */
/*                              load_recv_chain, clear_recv_chain,      */
/*                              free_recv_tcws, setup_mbuf_timer        */
/*                                                                      */
/*----------------------------------------------------------------------*/

tok_recv_setup (
        dds_t   *p_dds)
{
        /*  Register expected mbuf usage with the mbuf services.        */
        /*  Initially, we need enough mbufs to fill the receive chain.  */
        /*                                                              */
        p_dds->wrk.mbreq.low_mbuf      = RCV_CHAIN_SIZE;
        p_dds->wrk.mbreq.low_clust     = RCV_CHAIN_SIZE;
        p_dds->wrk.mbreq.initial_mbuf  = RCV_CHAIN_SIZE;
        p_dds->wrk.mbreq.initial_clust = RCV_CHAIN_SIZE;
        m_reg(&p_dds->wrk.mbreq);
        tok_recv_init(p_dds);           /* init data structures */
        if (setup_recv_tcws(p_dds))     /* get recv TCW's */
            return(ENOMEM);             /* return if error */
        if (load_recv_chain(p_dds))     /* get all recv mbuf's */
        {
            clear_recv_chain(p_dds);    /* if error, free up any left, */
            free_recv_tcws(p_dds);      /* free up receive TCW's, */
            return(ENOBUFS);            /* and return error */
        }
        setup_mbuf_timer(p_dds);        /* get timer for mbufs */
        return(0);                      /* made it */
}

/*-------------------  T O K _ R E C V _ U N D O   ---------------------*/
/*                                                                      */
/*  NAME: tok_recv_undo                                                 */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Stops receive processing, frees all receive mbufs and receive   */
/*      TCW's, and changes the receive state.                           */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      This is a top-half routine in this driver and can only be       */
/*      executed under a user process.                                  */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Clears all receive data structures to init state; including     */
/*      the read index, mbuf list, and read address lists.              */
/*                                                                      */
/*  RETURNS:                                                            */
/*      0               Completed successfully.                         */
/*      -1              Memory deallocation error.                      */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: free_mbuf_timer, clear_recv_chain,      */
/*                              free_recv_tcws                          */
/*                                                                      */
/*----------------------------------------------------------------------*/

tok_recv_undo(
        dds_t   *p_dds)
{
        p_dds->wrk.recv_mode = FALSE;   /* change receive state */
        free_mbuf_timer(p_dds);         /* remove mbuf timer */
        clear_recv_chain(p_dds);        /* free mbufs, halt adapter */
        m_dereg(&p_dds->wrk.mbreq);     /* deregister mbuf usage */
        if (free_recv_tcws(p_dds))      /* free up receive TCW's */
            return(-1);
        return(0);
}

/*---------------------  T O K _ R E C V _ I N I T  --------------------*/
/*                                                                      */
/*  NAME: tok_recv_init                                                 */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Initializes the receive data structures.                        */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      This is a top-half routine in this driver and can only be       */
/*      executed under a user process.   Assumes that the ACA has       */
/*      been allocated and d_mastered.                                  */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Clears all receive data structures to init state; including     */
/*      the read index, mbuf list, read list, and address lists.        */
/*                                                                      */
/*  RETURNS:  Nothing                                                   */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: None                                    */
/*                                                                      */
/*----------------------------------------------------------------------*/

static
tok_recv_init (
        dds_t   *p_dds)
{
        register int    i;

        /*  Set all receive variables to initial state:                 */
        /*                                                              */
        p_dds->wrk.recv_mode     = FALSE;
        p_dds->wrk.read_index    = 0;
        p_dds->wrk.recv_tcw_list = NULL;
        p_dds->wrk.recv_tcw_base = NULL;
        p_dds->wrk.mbuf_timer    = NULL;
        p_dds->wrk.mbuf_timer_on = FALSE;
        for (i = 0; i < RCV_CHAIN_SIZE; i++)
        {
            p_dds->wrk.recv_addr[i] = NULL;
            p_dds->wrk.recv_mbuf[i] = NULL;
        }
        /*  Create the receive chain by initializing the pointer to     */
        /*  each receive list.  Create both DMA and virtual address     */
        /*  lists.                                                      */
        /*                                                              */
 
        p_dds->wrk.recv_list[0] =
            (recv_list_t *)( (int)p_dds->wrk.p_d_mem_block
                                                + ACA_RCV_CHAIN_BASE);
        p_dds->wrk.recv_vadr[0] =
            (recv_list_t *)( (int)p_dds->wrk.p_mem_block
                                                + ACA_RCV_CHAIN_BASE);
        for (i = 1; i < RCV_CHAIN_SIZE; i++)
	{
            p_dds->wrk.recv_list[ i ]
                        = p_dds->wrk.recv_list[ i-1 ] + 1;
            p_dds->wrk.recv_vadr[ i ]
                        = p_dds->wrk.recv_vadr[ i-1 ] + 1;
	}
}

/*--------------------  T O K _ R E C V _ S T A R T  -------------------*/
/*                                                                      */
/*  NAME: tok_recv_start                                                */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Issues a receive start command to the Token Ring adapter.       */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called by either a process or interrupt offlevel.        */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Changes the receive state to running.                           */
/*                                                                      */
/*  RETURNS:  Nothing                                                   */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: issue_scb_command                       */
/*                                                                      */
/*----------------------------------------------------------------------*/

tok_recv_start (
        dds_t   *p_dds)
{
	DEBUGTRACE1("strv");
        p_dds->wrk.recv_mode = TRUE;    /* change state */
        issue_scb_command               /* start adapter recv */
            (p_dds, ADAP_RCV_CMD, p_dds->wrk.recv_list[0]);
}

/*-------------------- T O K _ R E C V _ F R A M E  --------------------*/
/*                                                                      */
/*  NAME: tok_recv_frame                                                */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Checks the size and type of the frame before looking up its     */
/*      netid in the netid table; if the netid is found, it gets the    */
/*      open element associated with it and creates a receive element   */
/*      that is to be handed up to the receive interface for placement  */
/*      in the receive queue.  If the netid is not found, the size is   */
/*      out of bounds, or it is a MAC frame that cannot be accepted,    */
/*      the frame is dropped by freeing the mbuf(s).                    */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Called by the offlevel interrupt handler.                       */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Modifies the receive queue in the receive interface.            */
/*                                                                      */
/*  RETURNS:                                                            */
/*      0               Frame accepted.                                 */
/*      -1              Frame rejected.                                 */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  m_free, proc_recv                      */
/*                                                                      */
/*----------------------------------------------------------------------*/

static
tok_recv_frame (
        dds_t           *p_dds,
        struct mbuf     *m,
        unsigned int    size)
{
        netid_t         netid;
        open_elem_t     *open_elem;
        rec_elem_t      recv_elem;
        in_test_packet *in_packet;
        out_test_packet_hdr *out_packet_hdr;
        out_test_packet_data *out_packet_data;
        int     control;
        int     route_len;
        int     data_len;
        int     hdr_len;
        struct mbuf *mbufp;
        uchar iee_xid [] = { 0x81, 0x01, 0x00 };



        /*  If the frame size is out of bounds, or the frame is a MAC      */
        /*  frame (and we don't accept MAC frames), drop it.               */
        /*                                                                 */
        if ((size < p_dds->wrk.min_packet_len) ||
            (size > p_dds->wrk.max_packet_len))
        {
            TRACE3("FooT", (ulong)RCV_TRF_0, (ulong)size);
            if (size > p_dds->wrk.max_packet_len)
                ++p_dds->ras.ds.ovflo_pkt_cnt;

            ++p_dds->ras.cc.rx_err_cnt;
            m_freem(m);
            return(-1);
        }
        if (MAC_FRAME( MTOD( m, unsigned char *)))
        {
            if (!p_dds->wrk.mac_frame_active)
            {
                DEBUGTRACE2("FooT", RCV_TRF_1);
                m_freem(m);
                return(-1);
            }
            netid = TOK_MAC_FRAME_NETID;
        } else {
            netid = GET_NETID( MTOD( m, unsigned char *));
        }
        /*  Scan the netid table until a match is found.  If this netid is */
        /*  found, put the pointer to the mbuf into the receive element    */
        /*  structure along with its size.  Set the status to CIO_OK, then */
        /*  get the open element associated with this netid.  Call the CIO */
        /*  receive processing routine with pointers to the open element   */
        /*  and the receive element.  Return TRUE to indicate that the     */
        /*  frame was accepted.                                            */
        /*                                                                 */
        if (p_dds->cio.netid_table[netid].inuse)
        {
            recv_elem.mbufp         = m;
            recv_elem.bytes         = size;
            recv_elem.rd_ext.status = CIO_OK;
            open_elem =
                p_dds->cio.open_ptr
                    [p_dds->cio.netid_table[netid].chan - 1];
            proc_recv(p_dds, open_elem, &recv_elem);
            p_dds->ras.ds.pkt_acc_cnt++;
            return(0);
        }
        /*  The netid could not be found in the table, so free the mbuf   */
        /*  and return -1 to indicate that the frame was not accepted.    */
        /*                                                                */
        /*                                                                */
        /* if netid is 0x0000 then see if xid or test                     */
        if (netid == 0x0000) {
          TRACE2 ("NET0", m);
          in_packet=MTOD (m, in_test_packet *);
          control=(in_packet->ctl & CONTROL_MASK);
          if ((control == XID_FRAME) || (control == TEST_FRAME)) {
          /* first lets see if we are trying to answer ourself */
            if (bcmp (&in_packet->src_addr[1], &WRK.tok_addr[1], 5) !=0) {
          /* get cluster  */
            if ((mbufp=m_getclust (M_DONTWAIT, MT_DATA)) != NULL ) {
              out_packet_hdr=MTOD (mbufp, out_test_packet_hdr *);
              /* see if there is a chain */
              if (m->m_next != NULL) {
                mbufp->m_next=m->m_next;
                m->m_next=NULL;
              }
              /* need to see if there is any routing info */
              /* check high bit in first byte of source address */
              if ((in_packet->src_addr[0] & ROUTE_PRESENT_BIT)) {
                /* yes there is routing info */
                /* first lets check the broadcast indicators */
                if ((in_packet->ctl1 & ROUTE_BCAST_MASK)==SINGLE_ROUTE_BCAST) {
                  /* need to make it an all route broadcast */
                  route_len=2;                        /* len=2bytes of RI */
                  out_packet_hdr->ctl1=ALL_ROUTE_CTL1;
                  out_packet_hdr->ctl2=in_packet->ctl2;
                } else {
                  route_len=in_packet->ctl1 & ROUTE_LEN_MASK;
                  out_packet_hdr->ctl1=in_packet->ctl1 ;
                  out_packet_hdr->ctl2=in_packet->ctl2 ^ ROUTE_DIR_BIT;
                }
                if (route_len-2 > 0) {  /* len-2 is route info less RI */
                  bcopy (&in_packet->route_info, &out_packet_hdr->route_info,
                          route_len-2);
                }
                out_packet_data=(out_test_packet_data *)(&out_packet_hdr->ctl1
			+route_len);
                /* header length of 14 = ac+fc+src_addr+dest_addr */
                hdr_len=14+route_len;
                /* lets copy over source and dest addresses */
                bcopy (&in_packet->src_addr, out_packet_hdr->src_addr,
                       TOK_NADR_LENGTH);
                bcopy (&WRK.tok_addr, &out_packet_hdr->dest_addr,
                       TOK_NADR_LENGTH);
                /* need to turn off ROUTE_PRESENT_BIT in dest */
                out_packet_hdr->src_addr[0]=out_packet_hdr->src_addr[0] &
                                            (~ROUTE_PRESENT_BIT);
                /* need to to ON ROUTE_PRESENT_BIT in src address */
                out_packet_hdr->dest_addr[0]=out_packet_hdr->dest_addr[0] |
                                            ROUTE_PRESENT_BIT;
              } else {
                /* no routing information */
                /* header length of 14 = ac+fc+src_addr+dest_addr */
                hdr_len=14;
                out_packet_data=
			(out_test_packet_data *)(&out_packet_hdr->ctl1);
                /* lets copy over source and dest addresses */
                bcopy (&in_packet->src_addr, out_packet_hdr->src_addr,
                       TOK_NADR_LENGTH);
                bcopy (&WRK.tok_addr, &out_packet_hdr->dest_addr,
                       TOK_NADR_LENGTH);
              }


             /* now copy over lsap and rsap and control byes */
             out_packet_data->ctl=in_packet->ctl | UFMT_PF_BIT;
             out_packet_data->lsap=in_packet->lsap | RESP_ON_BIT;
             out_packet_data->rsap=in_packet->rsap;
             out_packet_hdr->ac=XID_TEST_AC;
             out_packet_hdr->fc=XID_TEST_FC;

             /* now copy over either xid or test data */

             if (control == XID_FRAME) {
               bzero (out_packet_data->data, WRK.min_packet_len);
               bcopy (iee_xid, &out_packet_data->data, 3);
               mbufp->m_len=hdr_len+6;
             } else {
               /* 35 = ac+fc+src_addr+dest+addr+route_info+lsap+rsap+control */
               data_len=m->m_len-35;
               if (data_len < 0) {
                 mbufp->m_len=hdr_len+3;
               } else {
                 bcopy (&in_packet->data, out_packet_data->data, data_len);
                 mbufp->m_len=hdr_len+3+data_len;
               }
             }
             /* now send it off */
             if (tokfastwrt (p_dds->cio.devno, mbufp) != 0) {
               m_freem (mbufp);
             }
           }
           }
         }
         m_freem(m);
         return (0);
       }  else {
        /* netid not found and it is not 0x0000 just forget it            */
        m_freem(m);
        p_dds->ras.ds.ctr_pkt_rej_cnt++;
        return(-1);
      }
}

/*-------------------  L O A D _ R E C V _ C H A I N  ------------------*/
/*                                                                      */
/*  NAME: load_recv_chain                                               */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Loads the receive chain with mbufs where needed.  If a new      */
/*      mbuf is added and receive processing has started, a receive     */
/*      valid interrupt is issued.  If not enough mbufs are available,  */
/*      the mbuf timer will be started or an error will be returned     */
/*      depending on the state of receive processing.                   */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      This routine can be executed by both the interrupt and          */
/*      process call threads of the driver.                             */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Fills the recv_mbuf chain if possible; d_master's recv_addr     */
/*      array elements for dma where mbufs are added.                   */
/*                                                                      */
/*  RETURNS:                                                            */
/*      0               Completed successfully.                         */
/*      -1              Failed (Not enough mbufs)                       */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  i_disable, i_enable, arm_recv_list,    */
/*                               start_mbuf_timer, attach_bus,          */
/*                               pio_write, detach_bus.                 */
/*                                                                      */
/*----------------------------------------------------------------------*/

load_recv_chain (
        dds_t   *p_dds)
{
        int     x, i, rc = 0, mbuf_count = 0;
        int     bus;

        DEBUGTRACE1("mbrB");
        /*                                                               */
        /*  Starting at the next receive list to read, walk through the  */
        /*  receive chain and fill in any needed mbufs until either the  */
        /*  chain is full or we run out of mbufs; in the latter case,    */
        /*  either quit or start the mbuf timer depending on the state.  */
        /*  If a new mbuf was added, a receive valid flag was set; issue */
        /*  receive valid interrupt only if already up and running.      */
        /*                                                               */
        i = p_dds->wrk.read_index;              /* next to read */
        while (TRUE)
        {
            if (!p_dds->wrk.recv_mbuf[i])            /* need one here? */
            {                                        /* then load it - */
		/*
		 *  Arm the receive list and try to get an
		 *  mbuf for it.
		 */
                if (arm_recv_list(p_dds, i, NULL))   /* out of mbufs? */
                {
                    if (p_dds->wrk.recv_mode)   /* if running */
                        start_mbuf_timer(p_dds);/* start mbuf timer */
                    else
                    {
                        TRACE2("FooT", RCV_LRC_0);
                        rc = -1;                /* else, error */
                    }
                    break;                      /* break from loop */
                } else
                    mbuf_count++;               /* got an mbuf */
            }
            i = (i + 1) % RCV_CHAIN_SIZE;       /* next recv list */
            if (i == p_dds->wrk.read_index)     /* last one? */
                break;                          /* exit loop */
        }
        /*  If no errors and new mbufs were added, issue the recv       */
        /*  valid interrupt to the adapter.                             */
        /*                                                              */
	x = i_disable(INTCLASS2);
        if ( (!rc && mbuf_count && p_dds->wrk.recv_mode) ) {
	    bus = BUSIO_ATT(p_dds->ddi.bus_id, p_dds->ddi.bus_io_addr);
	    PIO_PUTSRX(bus + REG_OFFSET(COMMAND_REG), RCV_VALID);
	    BUSIO_DET(bus);
        }
	i_enable(x);
        DEBUGTRACE1("mbrE");
        return(rc);
}

/*-------------------  R E A D _ R E C V _ C H A I N  ------------------*/
/*                                                                      */
/*  NAME: read_recv_chain                                               */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Reads completed mbufs from the receive chain and queues         */
/*      them as receive frames via tok_recv_frame.  This routine        */
/*      does not replace mbufs as removed from the receive chain,       */
/*      nor does it issue receive valid interrupts to the adapter.      */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      This routine can be executed by both the interrupt and          */
/*      process call threads of the driver.                             */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Updates the read index to the next read list to be read.        */
/*      Modifies the mbuf array and receive lists.                      */
/*      Must be called INTCLASS2 		                        */
/*                                                                      */
/*  RETURNS: Nothing                                                    */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  i_disable, i_enable, read_recv_list,   */
/*                               and tok_recv_frame.                    */
/*                                                                      */
/*----------------------------------------------------------------------*/

read_recv_chain (
        dds_t           *p_dds,
        recv_list_t     *last)
{
        int                     size, worklen, x, i, sof = FALSE, mof = FALSE;
        int                     eof = FALSE;
        volatile recv_list_t    recvlist;
        struct mbuf             *m, *m0, *mhead, *mtail;
	int			rx_chain_valid = TRUE;
	int			bus;
	int			rc;

        mhead = mtail = NULL;
        while (TRUE)
        {
            i = WRK.read_index;          /* start at next recv list */
            read_recv_list( p_dds, &recvlist, i );
            /*                                                              */
            /*  If the valid bit is still set, and not start of frame,      */
            /*  then nothing has been received into this mbuf (so we didn't */
            /*  get anything).  If the start of frame and VALID = TRUE,     */
            /*  this is a frame that crosses several receive lists (the     */
            /*  middle recv list has valid set).                            */
            /*                                                              */
            if ((recvlist.status & VALID) && !sof)
                break;

            /*
             *  If an mbuf is set up for this receive list, call
             *  d_complete.  Otherwise, no more data so break. 
             */
            if (m = WRK.recv_mbuf[i]) {
		    rc =  d_complete (WRK.dma_chnl_id, DMA_READ,
				    MTOD( m, unsigned short *), m->m_len,
				    M_XMEMD(m), WRK.recv_addr[i]);

		    if ( rc != DMA_SUCC ) {
			    TRACE3("FooT", (ulong)RCV_RRL_0, (ulong)rc);
			    WRK.mcerr = rc;	/* save the error */
			    logerr( p_dds, ERRID_TOK_MC_ERR );
		    }
	    } else
		break;

           /*
            *  Performace group trace entry point
            */
           AIXTRACE(TRC_RDAT, CIO.devno, 0,0, recvlist.frame_size);

            /*
             *  See if we are at the beginning of a new frame; if so,
             *  initialize the working length to the total size of the
             *  frame.
             */
            if (recvlist.status & START_OF_FRAME) {
                sof   = TRUE;
                eof   = FALSE;
                size  = worklen = recvlist.frame_size;
            } else
		mof = TRUE;

            m->m_len = MIN( m->m_len, worklen );
            worklen      = MAX( 0, worklen - m->m_len );

	    /* If the number of received bytes is below our threshold
	     * copy into a new mbuf and leave the old one mapped. This
	     * avoids another d_master call.  Also, if the data is less
	     * than 256 bytes then get a small mbuf instead of a cluster.
	     */
	    if (m->m_len <= (CLBYTES / 2)) {
	    	if (m->m_len <= (MHLEN - DDI.rdto))
			m0 = m_gethdr(M_DONTWAIT, MT_DATA);
		else
			m0 = m_getclust(M_DONTWAIT, MT_DATA);
	    } else
		m0 = NULL;

	    /*
	     *  Copy data from d_master'ed mbuf to the new one
	     */
	    if ( m0 ) {
		m0->m_data += DDI.rdto;
		bcopy(mtod(m, caddr_t) + DDI.rdto, 
			mtod(m0, caddr_t), m->m_len);
		m0->m_len = m->m_len;
		cache_inval(mtod(m, caddr_t) + DDI.rdto, 
			m->m_len);
		/*
		 *  Re-arm the receive list and pass the existing
		 *  mbuf to be used again to avoid doing another 
		 *  d_master.
		 */
		arm_recv_list(p_dds, i, m);
		m = m0;
	    } else {
		m->m_data += DDI.rdto;
		WRK.recv_mbuf[i] = NULL;
		/*
		 *  Re-arm the receive list and try to get a new
		 *  mbuf to put in it.
		 */
		if (arm_recv_list(p_dds, i, NULL)) {
			if (WRK.recv_mode)   /* if running */
				start_mbuf_timer(p_dds);
			else
				rx_chain_valid = FALSE;
		}
	    }

            /*                                                           */
            /*  See if we are at the beginning of a new frame; if so,    */
            /*  begin a new linked list of mbufs. If we are not at the   */
            /*  start of a frame, simply add this mbuf to the end of the */
            /*  current list.                                            */
            /*                                                           */
	    if (sof && (!mof)) 
		mhead = mtail = m;
	    else if (mtail) {
		mtail->m_next = m;
		mtail = m;
	    }

            /*                                                           */
            /*  If this is the end of the frame, submit this list of     */
            /*  mbufs to the receive handler.                            */
            /*                                                           */
            if (recvlist.status & END_OF_FRAME) {
		if (mhead)
                    tok_recv_frame(p_dds, mhead, size);
                mhead = mtail = NULL;
                eof   = TRUE;
                sof   = FALSE;
                mof   = FALSE;
            }
            WRK.read_index = (WRK.read_index + 1) % RCV_CHAIN_SIZE;
            if (WRK.recv_list[i] == last) {
                if (!eof)
                    if ( mhead )
                        m_freem( mhead );
                break;                  /* quit if this was the last one */
            }
        }
	if (rx_chain_valid) {
		bus = BUSIO_ATT(DDI.bus_id, DDI.bus_io_addr);
		PIO_PUTSRX(bus + REG_OFFSET(COMMAND_REG), RCV_VALID);
		BUSIO_DET(bus);
	}
}

/*-----------------  C L E A R _ R E C V _ C H A I N  ------------------*/
/*                                                                      */
/*  NAME: clear_recv_chain                                              */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      D_completes and frees all remaining mbufs in the receive        */
/*      chain and clears all receive list elements.                     */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      This is a top-half routine in this driver and can only be       */
/*      executed under a user process.                                  */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Clears the recv_mbuf array and clears receive list elements     */
/*      of the receive chain.                                           */
/*                                                                      */
/*  RETURNS: Nothing                                                    */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  clear_recv_list                        */
/*                                                                      */
/*----------------------------------------------------------------------*/

clear_recv_chain (
        dds_t    *p_dds)
{
        register int   i, x;

        /*  Clear each receive list in the chain.                       */
        /*                                                              */
        x = i_disable(INTCLASS2);
        for (i = 0; i < RCV_CHAIN_SIZE; i++)
            clear_recv_list(p_dds, i);

        p_dds->wrk.read_index    = 0;
        i_enable(x);
}

/*--------------------  A R M _ R E C V _ L I S T  ---------------------*/
/*                                                                      */
/*  NAME: arm_recv_list                                                 */
/*                                                                      */
/*  FUNCTION:                                                           */
/*    Rearms the receive list indexed by i and d_kmoves it thru the     */ 
/*    IOCC cache back to the receive chain.  Uses the mbuf passed in    */
/*    unless the mbuf is NULL.  If the mbuf is NULL, then it gets a     */
/*    new mbuf and d_masters it.  If it can't get a new MBUF then       */
/*    the receive list is set with a zero data count and its valid      */
/*    flag is turned off to cause the adapter to wait at this receive   */
/*    list until an mbuf becomes available.                             */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      This routine can be executed by both the interrupt and          */
/*      process call threads of the driver.                             */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Allocates an mbuf for the recv_mbuf indexed by i (if            */
/*      possible);  d_master's the mbuf to the associated recv_addr     */
/*      location. Modifies the associated recv_list.                    */
/*                                                                      */
/*  RETURNS:                                                            */
/*      0               Completed successfully.                         */
/*      -1              Error (not enough mbufs).                       */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  m_getclust,                            */
/*                               d_master, d_kmove.                     */
/*                                                                      */
/*----------------------------------------------------------------------*/

arm_recv_list (
        dds_t          *p_dds,                  /* pointer to dds     */
        int            i,                       /* which receive list */
        struct mbuf    *m)                      /* MBUF to rearm      */
{
        volatile recv_list_t	recvlist;
        unsigned char		*recvaddr;
	int			rc, reload = FALSE;

	/*
	 *  No mbuf is passed in so try to get a new one.  If can't
	 *  get a new mbuf then update the ras counter.
	 */
	if (!m) {
            if ( m = m_getclust(M_DONTWAIT, MT_DATA) )
		reload = TRUE;
            else
                p_dds->ras.ds.rec_no_mbuf_ext++;
	}
        /*                                                              */
        /*  Setup the receive list such that the adapter will begin     */
        /*  writing receive data after the configured data offset.      */
        /*                                                              */
        recvaddr = WRK.recv_addr[i] + DDI.rdto;
        recvlist.addr_hi = ADDR_HI( recvaddr );
        recvlist.addr_lo = ADDR_LO( recvaddr );
        recvlist.next    = WRK.recv_list [ (i + 1) % RCV_CHAIN_SIZE ];
        recvlist.frame_size = 0;

	/*
	 *  Have an mbuf to put in this receive list.
	 */
        if ( m ) {

	    /*
	     *  Set up the length of the mbuf.
	     */
            m->m_len = CLBYTES - DDI.rdto;

            /*
	     *  Had a NULL mbuf pointer passed in and got a new mbuf so
             *  need to d_master the new mbuf.
             */
	    if (reload)
            	d_master (WRK.dma_chnl_id, DMA_READ|DMA_NOHIDE,
                	MTOD( m, unsigned short *), m->m_len,
                	M_XMEMD(m) ,WRK.recv_addr[i]);

            recvlist.count  = m->m_len;
            recvlist.status = (FRAME_INTERRUPT | VALID);
            WRK.recv_mbuf[i] = m;
	/*
	 *  If no mbuf is available, turn off flags, causing the
	 *  adapter to stop at this "dead" receive list.
	 */
        } else {
            recvlist.count  = 0;
            recvlist.status = 0;
        }
        /*  Update the receive dma image of the list by d_moving it     */
        /*  through the IOCC cache into system memory.                  */
        /*                                                              */
        rc = d_kmove (&recvlist, 
                 WRK.recv_list[i], 
                 sizeof(recvlist),
                 WRK.dma_chnl_id, 
                 DDI.bus_id, 
                 DMA_WRITE_ONLY);

	if (rc == EINVAL) 	/* IOCC is NOT buffered */
           bcopy (&recvlist, 
              WRK.recv_vadr[i], 
              sizeof(recvlist));

        return ( (m) ? 0 : -1 );                /* return 0 if mbuf */
}

/*--------------------  R E A D _ R E C V _ L I S T  -------------------*/
/*                                                                      */
/*  NAME: read_recv_list                                                */
/*                                                                      */
/*  FUNCTION:  Reads in the recv_list from IOCC cache.                  */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      This routine can be executed by both the interrupt and          */
/*      process call threads of the driver.                             */
/*      Must be called disabled to INTCLASS2.                           */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Recv_list_t is read in from IOCC cache to main memory.          */
/*                                                                      */
/*  RETURNS:                                                            */
/*      NULL            Nothing was read from the receive list.         */
/*      non-NULL        Pointer to the mbuf from this read list.        */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  d_kmove, d_complete.                   */
/*                                                                      */
/*----------------------------------------------------------------------*/

void
read_recv_list (
        dds_t           *p_dds,
        recv_list_t     *rlptr,
        int             i)
{
	int		rc;

        /*                                                              */
        /*  D_move the receive list image from the IOCC cache to main   */
        /*  memory (local image) pointed to by rlptr.                   */
        /*                                                              */
        rc = d_kmove (rlptr,
                 p_dds->wrk.recv_list[i],
                 sizeof(recv_list_t),
                 p_dds->wrk.dma_chnl_id,
                 p_dds->ddi.bus_id,
                 DMA_READ);

	if (rc == EINVAL) 	/* IOCC is NOT buffered */
           bcopy (p_dds->wrk.recv_vadr[i],
              rlptr,
              sizeof(recv_list_t));

        return;
}

/*-------------------  C L E A R _ R E C V _ L I S T  ------------------*/
/*                                                                      */
/*  NAME: clear_recv_list                                               */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Turns off the VALID bit, clears the address fields, and         */
/*      d_completes/frees the mbuf for the receive list indexed         */
/*      by i.                                                           */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      This routine can be executed by both the interrupt and          */
/*      process call threads of the driver.                             */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      D_completes the mbuf associated with the recv_addr indexed      */
/*      by i, clears the associated recv_list, and frees the            */
/*      associated mbuf.                                                */
/*                                                                      */
/*  RETURNS: Nothing                                                    */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  i_disable, i_enable, bzero, d_kmove,   */
/*                               d_complete.                            */
/*                                                                      */
/*----------------------------------------------------------------------*/

static
clear_recv_list (
        dds_t           *p_dds,
        int             i)
{
        struct mbuf     *m;
        volatile recv_list_t    recvlist;
        register int    x;
	unsigned int	rc;

        bzero(&recvlist, sizeof(recvlist));     /* clear receive list */
        /*                                                              */
        /*  Update the receive dma image of the list by d_moving it     */
        /*  through the IOCC cache into system memory.                  */
        /*                                                              */
        x = i_disable(INTCLASS2);
        rc = d_kmove (&recvlist,
                 p_dds->wrk.recv_list[i],
                 sizeof(recvlist),
                 p_dds->wrk.dma_chnl_id,
                 p_dds->ddi.bus_id,
                 DMA_WRITE_ONLY);

	if (rc == EINVAL) 	/* IOCC is NOT buffered */
           bcopy (&recvlist,
              p_dds->wrk.recv_vadr[i],
              sizeof(recvlist));
        /*                                                              */
        /*  If an mbuf is set up for this receive list, call            */
        /*  d_complete to "un_dma" it, then return it.                  */
        /*                                                              */
        if (m = p_dds->wrk.recv_mbuf[i])
        {
            rc = d_complete (p_dds->wrk.dma_chnl_id, DMA_READ,
                	MTOD( m, unsigned short *), m->m_len,
                	M_XMEMD(m),
                	p_dds->wrk.recv_addr[i]);

	   if ( rc != DMA_SUCC )
	   {
		TRACE3("FooT", (ulong)RCV_CRL_0, (ulong)rc);
		p_dds->wrk.mcerr = rc;	/* save the error */
		logerr( p_dds, ERRID_TOK_MC_ERR );
	   }
            m_freem(m);
            p_dds->wrk.recv_mbuf[i] = NULL;
        }
        i_enable(x);
}

/*------------------  S E T U P _ R E C V _ T C W S  -------------------*/
/*                                                                      */
/*  NAME: setup_recv_tcws                                               */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Allocates and sets up TCW's for the receive chain.              */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Called from the top half of the driver (process thread).        */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Initializes the receive TCW base, allocates the receive TCW     */
/*      list, and assigns a receive TCW to each element of the receive  */
/*      address array.                                                  */
/*                                                                      */
/*  RETURNS:                                                            */
/*      0               Completed successfully.                         */
/*      -1              Failed.                                         */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  reg_alloc, reg_init, reg_free,         */
/*                               reg_release.                           */
/*                                                                      */
/*----------------------------------------------------------------------*/

static
setup_recv_tcws (
        dds_t   *p_dds)
{
        register int    i, j;

        /*  Initialize the receive TCW list and allocate a TCW for      */
        /*  each receive page.  If an error occurs, free the receive    */
        /*  TCW's and return.                                           */
        /*                                                              */
        p_dds->wrk.recv_tcw_base =
                p_dds->ddi.dma_base_addr + RECV_AREA_OFFSET;
        if (!(p_dds->wrk.recv_tcw_list =
                reg_init (p_dds->wrk.recv_tcw_base,
                                                RCV_TCW, DMA_L2PSIZE)))
            return (-1);                        /* no recv TCW's */
        for (i = 0; i < RCV_CHAIN_SIZE; i++)
        {
            if (!(p_dds->wrk.recv_addr[i] =
                    reg_alloc(p_dds->wrk.recv_tcw_list, CLBYTES)))
            {
                for (j = 0; j < i; j++)
                    reg_free(p_dds->wrk.recv_tcw_list,
                                CLBYTES, p_dds->wrk.recv_addr[j]);
                reg_release(p_dds->wrk.recv_tcw_list);
                return(-1);                     /* not enough regions */
            }
        }
        return(0);                              /* succeeded */
}

/*-------------------  F R E E _ R E C V _ T C W S  --------------------*/
/*                                                                      */
/*  NAME: free_recv_tcws                                                */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Deallocates and frees TCW's from the receive chain.             */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Called from the top half of the driver (process thread).        */
/*      This cannot be called from interrupt level since it frees       */
/*      system memory.                                                  */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Clears the receive address array and receive TCW list.          */
/*                                                                      */
/*  RETURNS:                                                            */
/*      0               Completed successfully.                         */
/*      -1              Failed.                                         */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  i_disable, i_enable, reg_free,         */
/*                               reg_release                            */
/*                                                                      */
/*----------------------------------------------------------------------*/

/* static */
free_recv_tcws (
        dds_t   *p_dds)
{
        register int    i;

        /*  Free each TCW from the receive chain:                       */
        /*                                                              */
        for (i = 0; i < RCV_CHAIN_SIZE; i++)
            if (p_dds->wrk.recv_addr[i])
            {
                if (reg_free (p_dds->wrk.recv_tcw_list, CLBYTES,
                            p_dds->wrk.recv_addr[i]))
                    return(-1);
                p_dds->wrk.recv_addr[i] = NULL;
            }
        /*                                                              */
        /*  Then release the receive TCW List:                          */
        /*                                                              */
        if (p_dds->wrk.recv_tcw_list)
        {
            if (reg_release(p_dds->wrk.recv_tcw_list))
                return(-1);
            p_dds->wrk.recv_tcw_list = NULL;
        }
        return(0);
}

/*-------------------  R E C V _ M B U F _ T I M E R  ------------------*/
/*                                                                      */
/*  NAME: recv_mbuf_timer                                               */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Gets the p_dds from the timer struct and creates an offlevel    */
/*      work element -- submits it to the driver offlevel interrupt     */
/*      queue to call load_recv_chain (see tokintr.c).                  */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Executed at interrupt level by timer services.                  */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Modifies the DDS mbuf timer variable mbuf_timer_on.             */
/*                                                                      */
/*  RETURNS: Nothing                                                    */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  que_oflv                           */
/*                                                                      */
/*----------------------------------------------------------------------*/

static
recv_mbuf_timer (
        struct trb      *timer)
{
        offl_elem_t     owe;
        dds_t           *p_dds;

        p_dds = (dds_t *)timer->func_data;
        owe.who_queued = OFFL_MBUF_TIMER;
        owe.int_reason = 0;
        owe.cmd        = 0;
        owe.stat0      = 0;
        owe.stat1      = 0;
        owe.stat2      = 0;
        if (que_oflv( &(p_dds->ofl), &owe))
	{
		TRACE2("FooT", (ulong)TIME_0);
            p_dds->ras.ds.timo_lost++;
	}
        p_dds->wrk.mbuf_timer_on = FALSE;
}

/*-------------------  S E T U P _ M B U F _ T I M E R  ----------------*/
/*                                                                      */
/*  NAME: setup_mbuf_timer                                              */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Allocates and initializes a timer for mbufs.                    */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Executed by the top half of the driver.                         */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Modifies the DDS mbuf timer variables mbuf_timer and            */
/*      mbuf_timer_on.                                                  */
/*                                                                      */
/*  RETURNS: Nothing                                                    */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  t_alloc                                */
/*                                                                      */
/*----------------------------------------------------------------------*/

static
setup_mbuf_timer (
        dds_t   *p_dds)
{
        if (p_dds->wrk.mbuf_timer == NULL)
        {
            p_dds->wrk.mbuf_timer = (struct trb *)talloc();
            p_dds->wrk.mbuf_timer->flags &= ~(T_ABSOLUTE);
            p_dds->wrk.mbuf_timer->func
                                = (void (*)())recv_mbuf_timer;
            p_dds->wrk.mbuf_timer->func_data
                                = (unsigned long)p_dds;
            p_dds->wrk.mbuf_timer->ipri
                                = INTCLASS2;
            p_dds->wrk.mbuf_timer_on = FALSE;
        }
}

/*-------------------  S T A R T _ M B U F _ T I M E R  ----------------*/
/*                                                                      */
/*  NAME: start_mbuf_timer                                              */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Initiates the mbuf timer -- after MBUF_MS_WAIT milliseconds,    */
/*      an mbuf interrupt is queued to retry loading the receive        */
/*      chain with mbufs.                                               */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Called from the offlevel interrupt processing of the driver     */
/*      when there are not enough mbufs to replenish the receive        */
/*      chain.                                                          */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Modifies the DDS mbuf timer variable mbuf_timer_on.             */
/*                                                                      */
/*  RETURNS: Nothing                                                    */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  t_start                                */
/*                                                                      */
/*----------------------------------------------------------------------*/

static
start_mbuf_timer (
        dds_t   *p_dds)
{
        /*  If the timer exists and is not already running, set up the  */
        /*  mbuf milliseconds wait, mark the timer as running, then     */
        /*  start it.                                                   */
        /*                                                              */
        if (p_dds->wrk.mbuf_timer)
            if (p_dds->wrk.mbuf_timer_on == FALSE)
            {
                p_dds->wrk.mbuf_timer->timeout.it_value.tv_sec
                   = MBUF_MS_WAIT / 1000;
                p_dds->wrk.mbuf_timer->timeout.it_value.tv_nsec
                   = (MBUF_MS_WAIT % 1000) * 1000000;
                p_dds->wrk.mbuf_timer_on  = TRUE;
                tstart(p_dds->wrk.mbuf_timer);
            }
        return;
}

/*-------------------  F R E E _ M B U F _ T I M E R  ------------------*/
/*                                                                      */
/*  NAME: free_mbuf_timer                                               */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Disables and releases the mbuf timer.                           */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Executed by the top half of the driver.                         */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Modifies the DDS mbuf timer variables mbuf_timer and            */
/*      mbuf_timer_on.                                                  */
/*                                                                      */
/*  RETURNS: Nothing                                                    */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  t_stop, t_free                         */
/*                                                                      */
/*----------------------------------------------------------------------*/

/* static */
free_mbuf_timer (
        dds_t   *p_dds)
{
        /*                                                              */
        /*  If the mbuf timer exists, free it (if it is running, stop   */
        /*  it first).  Clear mbuf timer variables.                     */
        /*                                                              */
        if (p_dds->wrk.mbuf_timer)
        {
           if (p_dds->wrk.mbuf_timer_on)
                tstop(p_dds->wrk.mbuf_timer);
            tfree(p_dds->wrk.mbuf_timer);
        }
        p_dds->wrk.mbuf_timer    = NULL;
        p_dds->wrk.mbuf_timer_on = FALSE;
}

#ifdef TOKDEBUG

/*------------------------  P R I N T _ B U F F E R  ------------------------*/
/*                                                                           */
/*  Prints the contents of BUFFER to screen a la the kernel debugger. LENGTH */
/*  is the number of bytes to show, and OFFSET is the starting address to be */
/*  printed on the left hand side.                                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static
print_buffer (
        char            *buffer,
        unsigned int    length,
        unsigned int    offset)
{
        int     i, j;

        for (i = 0; i < length; i = i + 16) {
            printf("\n\t  %04x: ", (offset + i));
            printf("%02x%02x%02x%02x %02x%02x%02x%02x",
                buffer[i+0],  buffer[i+1],  buffer[i+2],  buffer[i+3],
                buffer[i+4],  buffer[i+5],  buffer[i+6],  buffer[i+7]);
            printf(" %02x%02x%02x%02x %02x%02x%02x%02x  ",
                buffer[i+8],  buffer[i+9],  buffer[i+10], buffer[i+11],
                buffer[i+12], buffer[i+13], buffer[i+14], buffer[i+15]);
            for (j = 0; j < 16; j++) {
                if (buffer[ i + j ] < 0x20 || buffer[ i + j ] > 0x7e)
                    printf(".");
                else
                    printf("%c", buffer[ i + j ]);
            }
        }
}

#endif
