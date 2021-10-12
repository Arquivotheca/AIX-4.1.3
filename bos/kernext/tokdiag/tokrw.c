static char sccsid[] = "@(#)31	1.11  src/bos/kernext/tokdiag/tokrw.c, diagddtok, bos411, 9428A410j 10/26/93 16:24:10";
/*
 *   COMPONENT_NAME: SYSXTOK
 *
 *   FUNCTIONS: get_user
 *		list_error
 *		pio_retry
 *		tok_receive
 *		tok_xmit
 *		tok_xmit_done
 *		tokfastwrt
 *		tokwrite
 *		tx_oflv_pro
 *		xmit_sleep
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1991
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/cblock.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/uio.h>
#include <sys/dump.h>
#include <sys/errno.h>
#include <sys/err_rec.h>
#include "tok_comio_errids.h"
#include <sys/except.h>
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
#include <stddef.h>
#include <sys/timer.h>
#include <sys/types.h>
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

extern tracetable_t    tracetable;
extern cdt_t   ciocdt;
extern dd_ctrl_t   dd_ctrl;


/*----------------------  T O K _ R E C E I V E  -----------------------*/
/*                                                                      */
/*  NAME: tok_receive                                                   */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Processes receive interrupts; reads frames from the adapter,    */
/*      replenishes mbufs, and restarts the adapter as needed.          */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Called by the offlevel interrupt handler.                       */
/*      Must be called disabled INTCLASS2			 	*/
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Modifies the mbuf list, read address list, and read index.      */
/*                                                                      */
/*  RETURNS:                                                            */
/*      0               Completed successfully.                         */
/*      -1              Receive processing is down.                     */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: read_recv_chain, load_recv_chain        */
/*                                                                      */
/*----------------------------------------------------------------------*/

tok_receive (
        dds_t           *p_dds,
        offl_elem_t     *owe)
{
        recv_list_t    *last = NULL;

        DEBUGTRACE2 ("recB",(unsigned long) owe->stat2);
        p_dds->ras.ds.recv_intr_cnt++;          /* increment statistic */
        if (!p_dds->wrk.recv_mode)              /* ignore if not recv mode */
        {
            TRACE2("FooT", RCV_TRCV_0);
            return;
        }

        if (owe->stat0 & RECEIVE_SUSPENDED)
        {
                TRACE4("FooT", (ulong)RCV_TRCV_1, (ulong)owe->stat0,
			(ulong)( (owe->stat1 << 16) | (owe->stat2) ) );
                /*
                 *  NOTE:
                 *      We have an adapter command failure.
                 *      This is a limbo entry condition.
                 */
		++p_dds->ras.cc.rx_err_cnt;
		p_dds->wrk.limbo_owe = *owe;

		if ( p_dds->wrk.limbo == PARADISE )
                	(void)enter_limbo(p_dds, TOK_CMD_FAIL, owe->stat0);
		else if ( !(p_dds->wrk.limbo == NO_OP_STATE) )
			(void) cycle_limbo(p_dds);
                return;
        }
        last = (recv_list_t *)((owe->stat1 << 16) | owe->stat2);

        if (last == NULL) {      /* check for invalid ending point */
           return;
	}

        read_recv_chain(p_dds, last);           /* read frames from adapter */
        DEBUGTRACE2 ("recE",(unsigned long) owe->stat2);
        return(0);
}

/*****************************************************************************/
/*
 * NAME:     tokwrite
 *
 * FUNCTION: write entry point from kernel
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *           if successful, also returns indirectly the number of bytes
 *           written through the updating of uiop->uio_resid by uiomove
 *
 */
/*****************************************************************************/
int 
tokwrite (
	dev_t            devno,  /* major and minor number */
	struct uio      *uiop,   /* pointer to uio structure */
	chan_t           chan,   /* channel number */
	cio_write_ext_t *extptr) /* pointer to write extension structure */
{
	dds_t		*p_dds;
	int		adap;
	open_elem_t     *open_ptr;
	struct mbuf     *mbufp;
	struct mbuf	*mp;
	int             rc;
	cio_write_ext_t wr_ext;
	int		ipri;
	int		bus;
	char		free;
	short		total_bytes;

	/* this shouldn't fail if kernel and device driver are working
	 * correctly
	 */
	adap = minor(devno);

	if ((adap >= MAX_ADAPTERS) || (chan <= 0))
		return(ENXIO);
	if (chan > MAX_OPENS)
		return(EBUSY);
	if ((p_dds = dd_ctrl.p_dds[adap]) == NULL)
		return(ENODEV);
	if ((CIO.chan_state[chan-1] != CHAN_OPENED)  ||
	    ((open_ptr = CIO.open_ptr[chan-1]) == NULL)    )
		return(ENXIO);

	/* don't allow writes unless a connection has been completed
	 * in the past
	 */
	if (CIO.device_state != DEVICE_CONNECTED) {

		TRACE2 ("WRI3", (ulong)ENOCONNECT);
		return (ENOCONNECT);
	}

	/*
	 *   See if we are in limbo mode or if we are dead.
	 *   If so, reject the write request.
	 */
	if (p_dds->wrk.limbo == NO_OP_STATE)
		return(ENETDOWN);
	if (p_dds->wrk.limbo != PARADISE)
		return(ENETUNREACH);


	/* handle the extended parameter
	 */
	if (extptr == NULL) {
		/* 
		 * build our own with defaults
		 */
		wr_ext.flag = 0; /* don't notify complete, free mbuf  */
		wr_ext.write_id = 0; /* doesn't matter -no notification */
		wr_ext.netid = 0;
	} else {
		/* copy the write extension pointer into the transmit element
		 */
		if (open_ptr->devflag & DKERNEL) {
			wr_ext = *extptr;
		} else {
			rc = copyin(extptr, &wr_ext, sizeof(wr_ext));
			if (rc != 0) {
				TRACE2 ("WRI2", EFAULT);
				return(EFAULT);
			}

			if (wr_ext.flag & CIO_NOFREE_MBUF) {
				TRACE2 ("WRI3", EINVAL);
				return(EINVAL);
			}
		}
	}
	wr_ext.status = (ulong)CIO_OK;

	if (open_ptr->devflag & DKERNEL) {
		mbufp = (struct mbuf *)(uiop->uio_iov->iov_base);
		free = !(wr_ext.flag & CIO_NOFREE_MBUF);
	} else {
		/* copy the user data into an mbuf
		 */
		rc = get_user(p_dds, &mbufp, uiop, open_ptr, &wr_ext);
		if (rc != 0) {
			copyout(&wr_ext, extptr, sizeof(wr_ext));
			return(rc);
		}
		free = TRUE;
	}

	/* caclulate the number of bytes in mbuf chain
	 */
	mp = mbufp;
	total_bytes = 0;
	do {
		total_bytes += mp->m_len;
		mp = mp->m_next;
	} while(mp != NULL);

	/* check for valid mbuf length
	 */
	if ((total_bytes < WRK.min_packet_len) || 
	    (total_bytes > WRK.max_packet_len)) {
		TRACE2("WRI9", EINVAL);
		return(EINVAL);
	}

	AIXTRACE (TRC_WQUE, devno, chan, mbufp, total_bytes);

	/* serialize access to adapter transmit lists
	 */
	ipri = i_disable(INTCLASS2);

	/* while there is no room on transmit queue block
	 */
	while (XMITQ_FULL) {
		rc = xmit_sleep(p_dds, open_ptr, uiop->uio_fmode, &wr_ext);
		if (rc != 0) {
			i_enable(ipri);
			COPYOUT(open_ptr->devflag, &wr_ext, extptr,
					sizeof(wr_ext));
			if (!(open_ptr->devflag & DKERNEL))
				m_free(mbufp);
			return(rc);
		}
	}

	/* copy transmit element into queue, and advance next in pointer
	 */
	WRK.xmit_queue[WRK.tx_list_next_buf].mbufp = mbufp;
	WRK.xmit_queue[WRK.tx_list_next_buf].chan = chan;
	WRK.xmit_queue[WRK.tx_list_next_buf].wr_ext = wr_ext;
	WRK.xmit_queue[WRK.tx_list_next_buf].free = free;
	WRK.xmit_queue[WRK.tx_list_next_buf].bytes = total_bytes;

	XMITQ_INC(WRK.tx_list_next_buf);

	/* 
	 * update the number of transmits queued, and update if it is
	 * greater than the high water mark.
	 */
	WRK.xmits_queued++;
	if (WRK.xmits_queued > RAS.cc.xmt_que_high)
		RAS.cc.xmt_que_high = WRK.xmits_queued;

	/* 
	 *  if there are no tranmit buffers available then return
	 */
	if (WRK.tx_tcw_use_count < MAX_TX_TCW ) {
		bus = BUSIO_ATT(p_dds->ddi.bus_id, p_dds->ddi.bus_io_addr);
		tok_xmit(p_dds, bus);
		BUSIO_DET(bus);

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
 * NAME: tok_xmit
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

tok_xmit (
	dds_t *p_dds,		/* adapter to transmit */
	int	bus)		/* address of IO bus */
{
	xmt_elem_t *p_tx_elem;
	xmit_des_t *xd;
	t_tx_list tmp_tx_list;
	t_tx_list *start_tx_list;
	int rc;
        volatile t_scb          scb;            /* system command block */
	
	/* while there are more tramit elements on software queue then
	 * start a tranmission
	 */
	start_tx_list = p_dds->wrk.p_d_tx_next_avail;
	while(TRUE) {
		/* sanity checks
		 */
		ASSERT(WRK.tx_list_next_in >= 0);
		ASSERT(WRK.tx_list_next_buf >= 0);
		ASSERT(WRK.tx_list_next_out >= 0)
		ASSERT(WRK.tx_list_next_in < DDI.xmt_que_size);
		ASSERT(WRK.tx_list_next_buf < DDI.xmt_que_size);
		ASSERT(WRK.tx_list_next_out < DDI.xmt_que_size);
		ASSERT(WRK.tx_des_next_in >= 0);
		ASSERT(WRK.tx_des_next_in <= MAX_TX_TCW);

		p_tx_elem = &WRK.xmit_queue[WRK.tx_list_next_in];
		xd = &WRK.tx_buf_des[WRK.tx_des_next_in];

		p_tx_elem->tx_ds = xd;
		p_tx_elem->in_use = (char)TRUE;

		/* copy data into tranmit buffer and do processor cache
		 * flush
		 */
		MCOPY_TO_BUF(p_tx_elem->mbufp, xd->sys_addr);
		rc = vm_cflush(xd->sys_addr, p_tx_elem->bytes);
		ASSERT(rc == 0);

		/* increment the buffer use count, and advance the next
		 * transmit element in counter, also reduce count on
		 * transmits queued
		 */
		WRK.xmits_queued--;
		WRK.tx_tcw_use_count++;
		XMITQ_INC(WRK.tx_list_next_in);
		WRK.tx_des_next_in++;
		if (WRK.tx_des_next_in >= MAX_TX_TCW)
			WRK.tx_des_next_in = 0;

		/* set up the temp. TX List Chain element */
		bzero(&tmp_tx_list, sizeof(tmp_tx_list) );
		tmp_tx_list.gb[0].cnt   = (unsigned short)p_tx_elem->bytes;
		tmp_tx_list.gb[0].addr_hi = (ushort)ADDR_HI(xd->io_addr);
		tmp_tx_list.gb[0].addr_lo = (ushort)ADDR_LO(xd->io_addr);
		tmp_tx_list.p_d_addr[0] = (unsigned short *)(xd->io_addr);
		tmp_tx_list.frame_size = (unsigned short)p_tx_elem->bytes;
		tmp_tx_list.p_tx_elem = p_tx_elem; /* save the TX Q element */
		/*
		*  set CSTAT so adapter will know that this TX List Chain
		*  element is to be transmitted
		*/
		tmp_tx_list.tx_cstat = ( TX_START_OF_FRAME | TX_VALID_CHAIN_EL
				       | TX_END_OF_FRAME | TX_FRAME_INTERRUPT );

		/* move TX List Chain element into the Adapter Control Area */
		rc = move_tx_list( p_dds, &tmp_tx_list,
			   p_dds->wrk.p_tx_next_avail, 
			   p_dds->wrk.p_d_tx_next_avail, DMA_WRITE_ONLY);
		NEXT_TX_AVAIL(p_dds->wrk.p_tx_next_avail, p_dds);
		NEXT_D_TX_AVAIL(p_dds->wrk.p_d_tx_next_avail, p_dds);
		p_dds->wrk.tx_chain_empty = FALSE;

		/* 
		 * if all the tranmit buffers are in use then exit
		 */
		if ((WRK.tx_tcw_use_count == MAX_TX_TCW) ||
			(WRK.tx_list_next_in == WRK.tx_list_next_buf))
			break;

	}

	if (p_dds->wrk.issue_tx_cmd) {
		p_dds->wrk.issue_tx_cmd = FALSE;
		scb.adap_cmd    = ADAP_XMIT_CMD;
		scb.addr_field1 = ADDR_HI( start_tx_list );
		scb.addr_field2 = ADDR_LO( start_tx_list );
		rc = d_kmove(&scb, p_dds->wrk.p_d_scb, sizeof(scb),
			WRK.dma_chnl_id, p_dds->ddi.bus_id, DMA_WRITE_ONLY);

		if (rc == EINVAL) 	/* IOCC is NOT buffered */
		   bcopy(&scb, p_dds->wrk.p_scb, sizeof(scb));

		PIO_PUTSRX(bus + REG_OFFSET(COMMAND_REG), EXECUTE);
	} else
		PIO_PUTSRX(bus + REG_OFFSET(COMMAND_REG), TX_VALID);
}


/*****************************************************************************/
/*
 * NAME: tok_xmit_done
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
tok_xmit_done (
	dds_t	*p_dds,		/* dds structure for adapter */
	xmt_elem_t *p_tx_elem,	/* transmit element structure */
	ulong	status,		/* status word for status block option[0] */
	ulong	stat2)      /* status word for status block option[3] */
{
	chan_t chan;
	chan_state_t chan_state;
	open_elem_t *open_ptr;
	cio_stat_blk_t stat_blk;
        int  len;
        struct mbuf *mp;

	TRACE4("XMTb", p_dds, p_tx_elem, status);
	AIXTRACE(TRC_WEND, 0, 0,
		p_tx_elem->mbufp, p_tx_elem->mbufp->m_len);

	if (status == CIO_OK) {
		/* update the standard counter
		 */
                 mp = p_tx_elem->mbufp;
                 len = 0;
                 do {
                     len += mp->m_len;
                     mp = mp->m_next;
                } while ( mp != NULL);

		if (ULONG_MAX == RAS.cc.tx_frame_lcnt)
			RAS.cc.tx_frame_mcnt++;
		RAS.cc.tx_frame_lcnt++;
		if ((ULONG_MAX - len) < RAS.cc.tx_byte_lcnt)
			RAS.cc.tx_byte_mcnt++;
		RAS.cc.tx_byte_lcnt += len;
	}

	/* fast path for default actions */
	if (p_tx_elem->wr_ext.flag == 0) {
		m_freem(p_tx_elem->mbufp);
		TRACE1("XMTe");
	}else {

	open_ptr = CIO.open_ptr[p_tx_elem->chan - 1];
	chan_state = CIO.chan_state[p_tx_elem->chan - 1];

	if (open_ptr != NULL) {
		/*
		 * this trace hook is commented out until we figure out
		 * why open_ptr is now NULL
		 * AIXTRACE(TRC_WEND, open_ptr->devno, open_ptr->chan,
		 *	p_tx_elem->mbufp, p_tx_elem->mbufp->m_len);
		 */

		if ((p_tx_elem->wr_ext.flag & CIO_ACK_TX_DONE) &&
					(chan_state == CHAN_OPENED))
		{
			stat_blk.code = CIO_TX_DONE;
			stat_blk.option[0] = status;
			stat_blk.option[1] = p_tx_elem->wr_ext.write_id;
			stat_blk.option[2] = (ulong)p_tx_elem->mbufp;
			stat_blk.option[3] = stat2;

			report_status(p_dds, open_ptr, &stat_blk);
		}
	}

	/* free mbuf if needed
	 */
	if (p_tx_elem->free)
		m_freem(p_tx_elem->mbufp);

        } /* end of else */

	/* check if anyone needs to be notified of a transmit function, if
	 * flag is set check each open and call wakeup function
	 */
	if (CIO.xmt_fn_needed) {
		CIO.xmt_fn_needed = FALSE;
		for (chan = 0; chan < MAX_OPENS; chan++) {
			if ((CIO.chan_state[chan] == CHAN_OPENED) &&
			((open_ptr = CIO.open_ptr[chan]) != NULL) &&
			(open_ptr->xmt_fn_needed)                    )
			{
				TRACE1("XDN1");
				open_ptr->xmt_fn_needed = FALSE;
				(*(open_ptr->xmt_fn)) (open_ptr->open_id);
				TRACE1("XDN2");
			}
		}
	}

	/* check if anyone is blocked on a transmit, if they are check
	 * each open and do a wakeup if needed
	 */
	if (CIO.xmit_event) {
		CIO.xmit_event = FALSE;
		for (chan = 0; chan < MAX_OPENS; chan++) {
			if ((CIO.chan_state[chan] == CHAN_OPENED) &&
				((open_ptr = CIO.open_ptr[chan]) != NULL) &&
				(open_ptr->xmt_event != EVENT_NULL))
			{
				e_wakeup((int *)&open_ptr->xmt_event);
			}
		}
	}

	TRACE1("XMTe");
	return;
}

/*****************************************************************************/
/*
 * NAME: tokfastwrt
 *
 * FUNCTION: fast write entry point for kernel
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
tokfastwrt (
	dev_t		devno,		/* major minor number */
	struct mbuf 	*p_mbuf)
{
	dds_t *p_dds;
	int ipri;
	xmt_elem_t *xlm;
	int bus;
	register int len;
	struct mbuf	*mp;

	p_dds = dd_ctrl.p_dds[minor(devno)];
	if (p_dds == NULL)
		return(ENODEV);

	mp = p_mbuf;
	len = 0;
	do {
		len += mp->m_len;
		mp = mp->m_next;
	} while(mp != NULL);

	AIXTRACE(TRC_WQUE, devno, -1, p_mbuf, len);

	if (p_dds->wrk.limbo != PARADISE)
		return(ENETUNREACH);

	ipri = i_disable(INTCLASS2);

	if (XMITQ_FULL) {
		i_enable(ipri);
		return(EAGAIN);
	}

	xlm = &WRK.xmit_queue[WRK.tx_list_next_buf];
	XMITQ_INC(WRK.tx_list_next_buf);
	WRK.xmits_queued++;

	xlm->mbufp = p_mbuf;
	xlm->bytes = len;
	xlm->wr_ext.flag = 0;
	xlm->free = TRUE;

	/* 
	 *  if there are tranmit buffers available then process transmit request
	 */
	if (WRK.tx_tcw_use_count < MAX_TX_TCW ) {
		bus = BUSIO_ATT(p_dds->ddi.bus_id, p_dds->ddi.bus_io_addr);
		tok_xmit(p_dds, bus);
		BUSIO_DET(bus);

		/*
		 *  start watchdog timer
		 */
		WRK.wdt_setter = WDT_XMIT;
		w_start(&(WDT));
	}

	i_enable(ipri);
	return(0);
} /* End of tokfastwrt */


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
	dds_t *p_dds,
	open_elem_t *open_ptr,
	int fmode,
	cio_write_ext_t *wr_ext)
{

	/* check if we should block based on NDELAY
	 */
	if (fmode & DNDELAY) {
		/* if caller is from kernel, then set up to so xmt_fn gets
		 * called when there is room
		 */
		if (open_ptr->devflag & DKERNEL) {
			CIO.xmt_fn_needed = TRUE;
			open_ptr->xmt_fn_needed = TRUE;
		}

		/* update status and set error code
		 */
		wr_ext->status = CIO_TX_FULL;
		TRACE2("SLP1", EAGAIN);
		return(EAGAIN);
	} else {
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
 *	Called from tokwrite.  Runs only under a process
 *
 * RETURNS:
 *	0 if successful
 *	-1 on failure
 */
/*****************************************************************************/

int
get_user (
	dds_t *p_dds,
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
	if ((total_bytes < WRK.min_packet_len) || 
	    (total_bytes > WRK.max_packet_len)) {
		TRACE2("WGU1", EINVAL);
		return(EINVAL);
	}

	/* check that there is room on queue before using an mbuf
	 */
	ipri = i_disable(INTCLASS2);
	while (XMITQ_FULL) {
		rc = xmit_sleep(p_dds, open_ptr, uiop->uio_fmode, wr_ext);
		if (rc != 0) {
			i_enable(ipri);
			return(rc);
		}
	}
	i_enable(ipri);


	/* get an mbuf to copy data into
	 */
	mbufp = m_get(M_WAIT, MT_DATA);
	if (mbufp == NULL) {
		TRACE2("WGU2", ENOMEM);
		return(ENOMEM);
	}

	/* get a cluster if one is needed
	 */
	if (total_bytes > MLEN) {
		m_clget(mbufp);
		if (!M_HASCL(mbufp)) {
			TRACE3("WGU3", ENOMEM, mbufp->m_len);
			m_free(mbufp);
			return(ENOMEM);
		}
	}

	/* move data from user space to mbuf
	 */
	if (uiomove(MTOD(mbufp, uchar *), total_bytes, UIO_WRITE, uiop)) {
		TRACE2("WGU4", EFAULT);
		m_free(mbufp);
		return(EFAULT);
	}
	mbufp->m_len = total_bytes;
	*mpp = mbufp;

	return(0);
}

/*--------------------------------------------------------------------*/
/*                                                                    */
/*************                  Transmit Pro            ***************/
/*                                                                    */
/*--------------------------------------------------------------------*/
/*
 *  This routine handles a transmit interrupt. The interrupt to be processed
 *  resides in the off-level work elemnt.  The off-level element is as follows:
 *
 *          p_oflv->int_reason = Interrupt code returned by the adapter
 *          p_oflv->cmd = ADAP_XMIT_CMD
 *          p_oflv->stat0 = The SSB transmit completion code
 *          p_oflv->stat1 = The high 2 bytes of a TX Chain element
 *          p_oflv->stat2 = The low 2 bytes of a TX Chain element
 *
 *  First the stat0 field is check for a transmit command complete.
 *      if so, the issue_tx_cmd flag is set to true.  This is done, so that
 *      when another write request is issued, a new ADAP_XMIT_CMD will be
 *      issued.  Otherwise just a TX_VALID interrupt would be issued.  
 *
 *  If the TX command has not completed, then the stat0 is checked for a
 *  LIST ERROR.
 *      if so, Network Recovery Mode is entered.
 *
 *  If neither of the above 2 events occured, the processing of the TX chain
 *  is begun.  Processing of the TX chain continues until A TX chain element
 *  is found that has a tx_cstat set to TX_VALID_CHAIN_EL or TX_START_OF_FRAME
 *  or to 0 (a Zero indicates that this element has nothing in it).
 *
 *      Processing of the TX chain begins with p_tx_1st_update.  When
 *      processing has completed, the p_tx_1st_update is set to the current
 *      TX chain element index.
 *
 *  The TX chain is then repopulated with transmit requests from the TX queue.
 *  The TX chain is repopulated via the tok_transmit routine.  After the
 *  TX chain is repopulated, either a transmit command or a TX_VALID interrupt
 *  is issued to the adapter.
 */

int 
tx_oflv_pro(register dds_t *p_dds,
            register offl_elem_t *p_owe) /* off level work que element */
{
	t_tx_list   *p_tx_tmp, *p_d_tx_tmp;
	t_tx_list   tmp_tx_list;		/* temp Transmit List Element */
	int 	rc;
	int     bus;

	struct mbuf     *p_mbuf;
	unsigned short  *p_mbuf_data;
	unsigned int    txrc=CIO_OK, done=FALSE;

	DEBUGTRACE2 ("txoB",(unsigned long) p_dds->wrk.p_tx_next_avail);

	/*
	*   If we are in limbo mode AND we come into the
	*   transmit OFLV processing routine, this OFLV work element
	*   will have already have been processed via the clean_tx()
	*   routine that is called when we enter limbo mode.
	*
	*   FUTURE FIX:
	*       Research that the above statement will
	*       actually be the case.
	*/
	if(p_dds->wrk.limbo != PARADISE) {
		TRACE4("FooT", TX_TXOP_6, (ulong)p_dds,(ulong)p_dds->wrk.limbo);
		return(ENOMSG);
	}

	w_stop(&(WDT));
	WRK.wdt_setter = WDT_INACTIVE;

	/*
	*  Get current location of TX chain element to update.
	*  Get both virtual and BUS address of element.
	*/
	p_tx_tmp = p_dds->wrk.p_tx_1st_update;
	p_d_tx_tmp = p_dds->wrk.p_d_tx_1st_update;

	if ((p_owe->stat0 & 0xff00) == LIST_ERROR)
		return(list_error(p_dds, p_owe));

	if ((p_owe->stat0 & 0xff00) == TX_CMD_COMPLETE) {
	       /*   The adapter transmit command has commpleted
	        *   Another transmit command must be issued to restart
	        *   transmission.
	        */
		TRACE2("FooT", TX_TXOP_0);
	        p_dds->wrk.issue_tx_cmd = TRUE;
	} else {
		while (!p_dds->wrk.tx_chain_empty && !done) {
			rc = move_tx_list( p_dds, &tmp_tx_list, 
				   p_tx_tmp, p_d_tx_tmp, DMA_READ);

		        if ( (tmp_tx_list.tx_cstat & TX_VALID_CHAIN_EL)  ||
			       (tmp_tx_list.tx_cstat == 0) ) {
				done = TRUE;
				break;
			}

			++p_dds->ras.ds.pkt_trx_cnt;
			p_dds->wrk.tx_cstat = tmp_tx_list.tx_cstat; 

			if (tmp_tx_list.tx_cstat & TX_ERROR) {
				TRACE3("FooT", TX_TXOP_4, 
					(ulong)tmp_tx_list.tx_cstat );
				txrc=TOK_TX_ERROR;
				++p_dds->ras.ds.tx_err_cnt;
				++p_dds->ras.cc.tx_err_cnt;
			}

			/*
			*   issue the d_complete for the transmit data area
			*   and free the bus address space.
			*/
			rc = d_complete(p_dds->wrk.dma_chnl_id, DMA_WRITE_ONLY,
				       tmp_tx_list.p_tx_elem->tx_ds->sys_addr,
				       tmp_tx_list.p_tx_elem->bytes,
				       (struct xmem *)&WRK.xbuf_xd,
				       tmp_tx_list.p_tx_elem->tx_ds->io_addr);
			ASSERT(rc == DMA_SUCC);

			tok_xmit_done(p_dds, tmp_tx_list.p_tx_elem, txrc,
						((p_owe->stat0 << 16) | tmp_tx_list.tx_cstat));
			WRK.tx_tcw_use_count--;
			tmp_tx_list.p_tx_elem->in_use = FALSE;
			XMITQ_INC(WRK.tx_list_next_out);

			txrc=CIO_OK;
			/*
			*   At this point we know that there will be at least
			*   one Tx Chain element free to hold another packet
			*   for transmission.  Set the tx_chain_full flag
			*   to signal this event.
			*/
			p_dds->wrk.tx_chain_full = FALSE;

			/*
			* Zero out the CSTAT, data count, and address fields.
			* Put the change back to the Adapter Control Area
			*/
			tmp_tx_list.tx_cstat = 0;
			tmp_tx_list.frame_size = 0;
			tmp_tx_list.p_tx_elem = NULL;
			bzero(&tmp_tx_list.gb, sizeof(tmp_tx_list.gb));

		        /* Clear out the working TX chain element */
			rc = move_tx_list( p_dds, &tmp_tx_list, 
					   p_tx_tmp, 
					   p_d_tx_tmp, 
					   DMA_WRITE_ONLY);

			NEXT_TX_AVAIL(p_tx_tmp, p_dds);
			NEXT_D_TX_AVAIL(p_d_tx_tmp, p_dds);

			if (p_tx_tmp == p_dds->wrk.p_tx_next_avail) {
		                /* The Transmit Chain is empty */
				p_dds->wrk.tx_chain_empty = TRUE;
				break;
			}

		} /* endwhile */
	} /* end else if !TX error */

	/* update the 1st TX Chain element to update pointer */
	p_dds->wrk.p_tx_1st_update = p_tx_tmp;
	p_dds->wrk.p_d_tx_1st_update = p_d_tx_tmp;

	if ((WRK.tx_list_next_in != WRK.tx_list_next_buf) &&
			(WRK.tx_tcw_use_count < MAX_TX_TCW))
	{
		bus = BUSIO_ATT(p_dds->ddi.bus_id, p_dds->ddi.bus_io_addr);
		tok_xmit(p_dds, bus);
		BUSIO_DET(bus);
	}

	if (WRK.tx_tcw_use_count) {
		WRK.wdt_setter = WDT_XMIT;
		w_start(&(WDT));
	}

	DEBUGTRACE2 ("txoE",(unsigned long) p_dds->wrk.p_tx_next_avail);
	return(TRUE);
} /* end function tx_oflv_pro */

int 
list_error(register dds_t *p_dds,
            register offl_elem_t *p_owe) /* off level work que element */
{
	int	rc;

	/*
	 *   The adapter detected one of the following TX List errors:
	 *       - Illegal Frame size
	 *       - Transmit Threshold
	 *       - Odd Address - (Enter Limbo condition)
	 *       - Start of Frame - (Enter Limbo condition)
	 *       - Unauthorized Access Priority
	 *       - Unauthorized Mac Frame
	 *       - Illegal Frame Format
	 *
	 * NOTE:
	 *	It was originally intended that Limbo would
	 *	only be entered for Odd address and unexpected
	 *	Start of Frame.  Due to time constraints, the additional
	 *	logic required to clean up after the other errors
	 *	could not be coded.  It was then decided to enter
	 *	Limbo for all conditions.
	 */

	TRACE2("FooT", TX_TXOP_1);
	++p_dds->ras.ds.tx_err_cnt;
	++p_dds->ras.cc.tx_err_cnt;

	/*
	 * log the error
	 */
	logerr( p_dds, ERRID_TOK_TX_ERR );
	p_dds->wrk.limbo_owe = *p_owe;

	/*
	*   We have an enter limbo condition.
	*   Check the current state of limbo.
	*/
	if (p_dds->wrk.limbo == PARADISE ) {
		TRACE4("FooT", TX_TXOP_3, (ulong) p_owe->stat0,
		(ulong)((p_owe->stat1 << 16) | (p_owe->stat2)));
		rc = enter_limbo(p_dds, TOK_CMD_FAIL, p_owe->stat0);
		return(rc);
	} else {
		/*
		*   FUTURE FIX:
		*       Check this logic.  We should NEVER
		*       have the condition where we would
		*       want to cycle limbo while processing
		*       transmit interrupts.
		*/
		TRACE4("FooT", TX_TXOP_2, (ulong) p_owe->stat0,
			(ulong)((p_owe->stat1 << 16) | (p_owe->stat2)));
		rc = cycle_limbo(p_dds);
		return(rc);
	}
}

/*****************************************************************************/
/*
 * NAME pio_retry
 *
 * FUNCTION: This routine is called when a pio rotine returns an
 *	exception.  It will retry the the PIO and do error logging
 *
 * EXECUTION ENVIRONMENT:
 *	Called by interrupt and processes level
 *	This routine is invoked by the PIO_xxxX routines
 *
 * RETURNS:
 *	0 - excetion was retried successfully
 *	exception code of last failure if not successful
 */
/*****************************************************************************/

int
pio_retry(
	dds_t	*p_dds,	/* tells which adapter this came from	*/
	int 	excpt_code,	/* exception code from original PIO	*/
	enum pio_func iofunc,	/* io function to retry			*/
	void 	*ioaddr,	/* io address of the exception		*/
	long	ioparam)	/* parameter to PIO routine		*/

{
	int	retry_count;		/* retry count 	*/

	TRACE5("pior", (ulong)p_dds, (ulong)excpt_code, (ulong)iofunc,
			(ulong)ioaddr);

	retry_count = PIO_RETRY_COUNT;

	while(1) {
		/* trap if not an io exception
		 */
		assert(excpt_code == EXCEPT_IO);

		/* chech if out of retries, and do error logging
		 */
		if (retry_count <= 0) {
			logerr(p_dds, (ulong)ERRID_TOK_PIO_ERR);
			return(excpt_code);
		} else
			retry_count--;

		/* retry the pio function, return if successful
		 */
		switch (iofunc) {
			case PUTC:
				excpt_code = BUS_PUTCX((char *)ioaddr,
							(char)ioparam);
				break;
			case PUTS:
				excpt_code = BUS_PUTSX((short *)ioaddr,
							(short)ioparam);
				break;
			case PUTSR:
				excpt_code = BUS_PUTSRX((short *)ioaddr,
							(short)ioparam);
				break;
			case PUTL:
				excpt_code = BUS_PUTLX((long *)ioaddr,
							(long)ioparam);
				break;
			case PUTLR:
				excpt_code = BUS_PUTLRX((long *)ioaddr,
							(long)ioparam);
				break;
			case GETC:
				excpt_code = BUS_GETCX((char *)ioaddr,
							(char *)ioparam);
				break;
			case GETS:
				excpt_code = BUS_GETSX((short *)ioaddr,
							(short *)ioparam);
				break;
			case GETSR:
				excpt_code = BUS_GETSRX((short *)ioaddr,
							(short *)ioparam);
				break;
			case GETL:
				excpt_code = BUS_GETLX((long *)ioaddr,
							(long *)ioparam);
				break;
			case GETLR:
				excpt_code = BUS_GETLRX((long *)ioaddr,
							(long *)ioparam);
				break;
			defalut:
				ASSERT(0);
		}

		if (excpt_code == 0)
			return(0);
	}
}
