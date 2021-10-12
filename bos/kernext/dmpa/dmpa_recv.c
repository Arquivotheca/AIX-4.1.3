static char sccsid[] = "@(#)76        1.1  src/bos/kernext/dmpa/dmpa_recv.c, sysxdmpa, bos411, 9428A410j 4/30/93 12:49:17";
/*
 *   COMPONENT_NAME: (SYSXDMPA) MP/A DIAGNOSTICS DEVICE DRIVER
 *
 *   FUNCTIONS: HANDLE_EXT
 *		mparead
 *		startrecv
 *		stoprecv
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


#include <sys/ddtrace.h>
#include <sys/device.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/malloc.h>
#include <sys/dmpauser.h>
#include <sys/dmpadd.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/trchkid.h>
#include <sys/uio.h>
#include <sys/xmem.h>
#include <sys/mbuf.h>
#include <sys/dma.h>
#include <sys/errids.h>


#define HANDLE_EXT(extstatus)   { \
	if (ext_p) { \
		ext.status = extstatus; \
		copyout(&ext, ext_p, sizeof(cio_read_ext_t)); \
	} \
}


/*******************************************************************
 *      Global declarations for the MPA Device Driver             *
 *******************************************************************/
/*
 * NAME: mparead
 *
 * FUNCTION: mparead is called to read from the mutliprotocol quad
 *           port adapter
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Preemptable        : Yes
 *      VMM Critical Region: Yes
 *      Runs on Fixed Stack: Yes
 *      May Page Fault     : Yes
 *      May Backtrack      : Yes
 *
 * NOTES: We set up structs, channel info, etc.  Then we thread through the
 *        the receive buffer for new data.  We then deque things and return
 *
 * DATA STRUCTURES: uio, read_ext, struct acb,  mbuf,
 *
 * RETURN VALUE DESCRIPTION: Return the packet contents to the user.
 */

int mparead (dev_t                      dev,
	     struct uio                 *uiop,
	     chan_t                     chan,
	     cio_read_ext_t             *ext_p)

{
	struct acb      *acb;         /* pointer to adapter control block */
	recv_elem_t     *recvp;
	recv_elem_t     *tmp_recvp;
	struct mbuf     *mbufp;
	cio_read_ext_t  ext;

	int             total_bytes,
			bytes_to_move,
			already_locked,
			rc,
			ext_rc,
			found,
			fpl,
			spl,
			i,
			pkt_length,
			cnt=0;


	/* log trace hook */

	DDHKWD4(HKWD_DD_MPADD, DD_ENTRY_READ, 0, dev, chan,
			uiop->uio_iov->iov_base, uiop->uio_resid);

	if(uiop->uio_iov->iov_base == NULL) {
	      DDHKWD2(HKWD_DD_MPADD, DD_EXIT_READ, EINVAL, dev,0xC0);
	      return EINVAL;
	}

	rc = 0;
	ext_rc = CIO_OK;

	if ( ((acb = get_acb(minor(dev))) == NULL) ||
		!(OPENP.op_flags & OP_OPENED)      ||
		!(acb->flags & STARTED_CIO)          ) {

		DDHKWD2(HKWD_DD_MPADD, DD_EXIT_READ, ENXIO, dev,0xC2);
		HANDLE_EXT(CIO_OK);
		return ENXIO;
	}


	if /* a kernel process has possession of this channel, read should */
	/* never receive control....*/
	( OPENP.op_mode & DKERNEL )
	{
		DDHKWD2(HKWD_DD_MPADD, DD_EXIT_READ, EINVAL, dev,0xC3);
		HANDLE_EXT(CIO_OK);
		return (EINVAL);
	}

	if(acb->flags&MPADEAD) {
		DDHKWD2(HKWD_DD_MPADD, DD_EXIT_READ, ENODEV, dev,0xC4);
		HANDLE_EXT(CIO_HARD_FAIL);
		return ENODEV;
	}

	/* disable interrupts so we can single thread */

	DISABLE_INTERRUPTS(fpl);


	while( ((recvp = acb->act_recv_head) == NULL)
	|| (recvp->rc_state != RC_COMPLETE)) {
		while( recvp ) {
			if( recvp->rc_state == RC_COMPLETE )
				goto foundit;
			recvp = recvp->rc_next;
		}
		/*
		** no data available
		*/
		if (OPENP.op_mode & DNDELAY) {
			DDHKWD2(HKWD_DD_MPADD, DD_EXIT_READ, 0, acb->dev,0xC5);
			HANDLE_EXT(CIO_OK);
			ENABLE_INTERRUPTS( fpl );
			return 0;
		}
		/*
		** sleep waiting for a receive event
		*/
		if (SLEEP(&acb->op_rcv_event) != EVENT_SUCC) {
			DDHKWD2(HKWD_DD_MPADD, DD_EXIT_READ,EINTR, acb->dev,0xC6);
			HANDLE_EXT(CIO_OK);
			ENABLE_INTERRUPTS( fpl );
			return EINTR;
		}
	}

foundit:

	/*
	** Take an element from the "active" list.
	*/
	recvp = acb->act_recv_head;
	acb->act_recv_head =recvp->rc_next;
	recvp->rc_next = NULL;


	/*
	** Inform user of source of read data.
	*/
	if (ext_p) {
		ext.netid = chan;
	}

	/*
	** return notification in the extension (if supplied)
	** if the data received is larger than the user's buffer
	*/


	if (recvp->rc_count > uiop->uio_resid) {
		ext_rc = CIO_BUF_OVFLW;
		total_bytes = uiop->uio_resid;
	} else {
		total_bytes = recvp->rc_count;
	}

	if( mbufp = recvp->rc_mbuf_head ) {
		mbufp->m_len = total_bytes;
		pkt_length = mbufp->m_len;

		while( total_bytes > 0 && mbufp ) {
			bytes_to_move = mbufp->m_len;
			if (bytes_to_move > total_bytes)        /* limit data from this mbuf */
				bytes_to_move = total_bytes;
			if( rc =
			uiomove(MTOD(mbufp,caddr_t), bytes_to_move, UIO_READ, uiop) ) {
				DISABLE_INTERRUPTS( spl );
				m_freem(recvp->rc_mbuf_head);
				recvp->rc_mbuf_head = NULL;
				if( acb->mbuf_event != EVENT_NULL )
					e_wakeup(&acb->mbuf_event);
				ENABLE_INTERRUPTS( spl );
				DDHKWD2(HKWD_DD_MPADD, DD_EXIT_READ, rc, acb->dev,0xC7);
				ext_rc = CIO_LOST_DATA;
				goto mparead_exit;
			}
			total_bytes -= bytes_to_move;
			mbufp = mbufp->m_next;
		}
		DISABLE_INTERRUPTS( spl );
		m_freem(recvp->rc_mbuf_head);
		recvp->rc_mbuf_head = NULL;
		if( acb->mbuf_event != EVENT_NULL )
			e_wakeup(&acb->mbuf_event);
		ENABLE_INTERRUPTS( spl );
	}


mparead_exit:

	/*
	** Add this element to the "free" list.
	*/
	if (acb->recv_free == NULL)
	   acb->recv_free = recvp;
	else {
	   tmp_recvp = acb->recv_free;
	   while(tmp_recvp->rc_next != NULL) tmp_recvp=tmp_recvp->rc_next;
	   tmp_recvp->rc_next = recvp;
	}
	if (acb->op_rcv_event != EVENT_NULL)
		e_wakeup(&acb->op_rcv_event);

	bzero(recvp,sizeof(recv_elem_t));

	ENABLE_INTERRUPTS( fpl );
	DDHKWD2(HKWD_DD_MPADD, DD_EXIT_READ, rc, dev,0);
	HANDLE_EXT(ext_rc);
	return(rc);
}  /* mparead() */

int startrecv (struct acb *acb)
{

    int                         spl,rc;
    dma_elem_t                  *dmap;
    recv_elem_t                 *recvp;
    struct mbuf                 *mbufp=NULL;

    DISABLE_INTERRUPTS(spl);

    if(acb->flags & MPADEAD) {
	ENABLE_INTERRUPTS(spl);
	return 0;
    }

    /*
    ** If there is a recv dma held in acb->hold_recv then
    ** I already have memory and other resources allocated for
    ** the recv so just put the recv dma element back on the q
    ** only if the active dma q is empty (no pending xmits).
    */
    if(acb->hold_recv == NULL) {
	 /*     Allocate resources for receive....
	 ** We always build a packet header for kernel
	 ** users since they always get an mbuf chain...
	 ** ...don't bother for non-kernel processes because
	 ** the mbuf chain gets copied into the user-space
	 ** buffer eventually anyway.
	 */
	 if (OPENP.op_mode & DKERNEL) {
	     if ((mbufp = m_gethdr(M_DONTWAIT, MT_HEADER)) == NULL) {
		 ENABLE_INTERRUPTS(spl);
		 return ENOMEM;
	     }
	     mbufp->m_len = 0;
	     mbufp->m_data = (caddr_t)0;
	 }

	 /*
	 ** Take a recv element from the "free" list.
	 */
	 recvp = acb->recv_free;
	 if(recvp == NULL) {
	      ENABLE_INTERRUPTS(spl);
	      return ENOMEM;
	 }
	 acb->recv_free=recvp->rc_next;
	 bzero(recvp,sizeof(recv_elem_t));

	 /*
	  * Put this element on the active recv q for this adapter
	 */
	 if(acb->act_recv_head==NULL) {  /* its first one */
	     acb->act_recv_head=recvp;
	     acb->act_recv_tail=recvp;
	 }
	 else {        /* its going on the end of a chain */
	     acb->act_recv_tail->rc_next=recvp;
	     acb->act_recv_tail=recvp;
	 }

	 /*
	 ** Initialize the receive element.
	 */
	 recvp->rc_mbuf_head = mbufp;    /* null if ~kernel user */
	 recvp->rc_mbuf_tail = mbufp;    /* null if ~kernel user */
	 recvp->rc_state |= RC_ACTIVE;

	 /*
	 ** If we can't get an mbuf, bail out.
	 */
	 if ((mbufp = m_get(M_DONTWAIT, MT_DATA)) == NULL) {
	     free_recv_elem(acb,recvp);
	     ENABLE_INTERRUPTS(spl);
	     return ENOMEM;
	 }

	 /*
	 ** Set the transfer parameters, get cluster every time
	 */
	 if (!m_clget(mbufp)) {
	     free_recv_elem(acb,recvp);
	     ENABLE_INTERRUPTS(spl);
	     return ENOMEM;
	 }
	 /*
	 **  Default recevie lenght is 4k (CLBYTES).
	 */
	 mbufp->m_len = (CLBYTES);

	 /*
	 ** Add this mbuf to the end of
	 ** the current mbuf chain.  this is a chain of two now.
	 */
	 if (recvp->rc_mbuf_head == NULL) {
		/*
		** first on chain, there is no kernal header.
		*/
		 recvp->rc_mbuf_head = mbufp;
		 recvp->rc_mbuf_tail = mbufp;
	 } else {
		 recvp->rc_mbuf_tail->m_next = mbufp;
		 recvp->rc_mbuf_tail = mbufp;
	 }
	 if(acb->flags&PIO_MODE) {
	     /*
	     ** In PIO_MODE I just need to mark that resources have
	     ** been aquried for a recv. This value is not used as dmap.
	     ** Next I just kick off a recv command on the adatper.
	     */
	     acb->hold_recv = 1;
	     if(!(acb->flags & RECEIVER_ENABLED)) {
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
		if( (rc=que_command(acb)) ) {
		  free_recv_elem(acb,recvp);
		  ENABLE_INTERRUPTS(spl);
		  return rc;
		}
		acb->flags |= RECEIVER_ENABLED;
		DDHKWD2(HKWD_DD_MPADD, MPA_RECV_ENAB, 0, acb->dev,0x01);
	     }
	     recvp->rc_data = MTOD(mbufp, caddr_t);
	     recvp->rc_flags |= USE_PIO;
	 }
	 else {
	     /*
	     ** Take a dma element from the "free" list.
	     */
	     dmap = acb->dma_free;
	     if(dmap == NULL) {
		  free_recv_elem(acb,recvp);
		  ENABLE_INTERRUPTS(spl);
		  return ENOMEM;
	     }
	     acb->dma_free=dmap->dm_next;
	     bzero(dmap,sizeof(dma_elem_t));

	     /*
	     ** Set up the DMA element.
	     */
	     dmap->p.recv_ptr = recvp;
	     dmap->dm_req_type = DM_RECV;
	     dmap->dm_buffer = MTOD(mbufp, caddr_t);
	     dmap->dm_xmem = M_XMEMD(mbufp);
	     dmap->dm_length = mbufp->m_len;
	     dmap->dm_flags = DMA_READ | DMA_NOHIDE;
	     dmap->dm_state = DM_READY;
	     /*
	     **  Set up the hold_recv pointer so I know I have
	     **  all the resources for a recv. This pointer is set
	     **  back to NULL after a valid receive completes.
	     */
	     acb->hold_recv = dmap;
	 }

    }             /* end if recv_dma element does not exits */
    else {
	 /*
	 ** else there is already a recv dma elem so just go to
	 ** the hold_recv variable and get the dma element ptr.
	 */
	 if(acb->flags&PIO_MODE) {     /* handle the PIO_MODE case */
	    /*
	    ** check to see if the receiver is enabled, if not start it.
	    */
	     if(!(acb->flags & RECEIVER_ENABLED)) {
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
		if( (rc=que_command(acb)) ) {
		  free_recv_elem(acb,recvp);
		  ENABLE_INTERRUPTS(spl);
		  return rc;
		}
		DDHKWD2(HKWD_DD_MPADD, MPA_RECV_ENAB, 0, acb->dev,0x02);
		acb->flags |= RECEIVER_ENABLED;
	     }
	 }
	 else dmap= acb->hold_recv;
    }

    /*
    **  Kick off the dma...if I just put a recv on there it will
    **  start a recv dma, else it will start the next xmit dma on
    **  the q.
    */
    if(!(acb->flags&PIO_MODE)) rc=dma_request(acb,dmap);
    else rc = 0;

    ENABLE_INTERRUPTS(spl);
    return(rc);
}   /* startrecv() */

int stoprecv (struct acb *acb)
{

    int                         spl, rc=0;
    dma_elem_t                  *dmap;
    recv_elem_t                 *recvp;
    struct mbuf                 *mbufp;



    DISABLE_INTERRUPTS(spl);

    if(acb->flags & MPADEAD) {
	ENABLE_INTERRUPTS(spl);
	return EIO;
    }

    if( acb->flags & RECV_DMA_ON_Q ) {

	/*
	** The dma element on the head of the q will have the recv
	** dma I need to put on hold, So get the dmap and check to
	** make sure it is the current hold_recv element.
	*/
	if((dmap= acb->act_dma_head) == NULL ) {
	   ENABLE_INTERRUPTS(spl);
	   return ENXIO;
	}
	if( acb->hold_recv != dmap) {
	   ENABLE_INTERRUPTS(spl);
	   return ENXIO;
	}


	/*
	** Take this read dma elem off the active q...but do not
	** put it on the free q, it goes to hold_recv.
	*/
	acb->act_dma_head =dmap->dm_next; /* Null if last on q */
	acb->hold_recv = dmap;
	dmap->dm_next = NULL;

	/*
	** Abort the pending recv dam
	*/
	d_complete(acb->dma_channel, dmap->dm_flags,
		   dmap->dm_buffer, dmap->dm_length,
		   dmap->dm_xmem, NULL);

	dmap->dm_state = DM_READY;
	acb->flags &= ~RECV_DMA_ON_Q;
	DDHKWD2(HKWD_DD_MPADD, MPA_RECV_D_Q, 0, acb->dev,0x00);

	/*
	** Issue a recv disable command to the adapter, to prevent and
	** receive irpts during xmit.
	*/
	acb->cmd_parms.cmd=DISABLE_RECV_CMD;
	acb->cmd_parms.parm_count=0;
	if( !(rc = que_command(acb)) ) {
	      DDHKWD2(HKWD_DD_MPADD, MPA_RECV_DISAB, 0, acb->dev,0x00);
	      acb->flags &= ~RECEIVER_ENABLED;
	}
    }
    ENABLE_INTERRUPTS(spl);
    return(rc);
}   /* stoprecv() */

