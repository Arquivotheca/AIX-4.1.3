static char sccsid[] = "@(#)94  1.21.1.2  src/bos/kernext/entdiag/entrecv.c, diagddent, bos411, 9428A410j 10/28/93 15:56:43";
/*
 * COMPONENT_NAME: DIAGDDENT - Ethernet device handler
 *
 * FUNCTIONS: ent_recv_setup, ent_recv_start, ent_recv_undo, ent_recv,
 *            Setup_Rbufd
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stddef.h>
#include <sys/types.h>
#include <sys/cblock.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/adspace.h>
#include <sys/errno.h>
#include "ent_comio_errids.h"
#include <sys/file.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/timer.h>
#include <sys/trchkid.h>
#include <sys/user.h>
#include <sys/watchdog.h>
#include <sys/xmem.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/entuser.h>

#include "entddi.h"
#include "cioddi.h"
#include "cioddhi.h"
#include "entdshi.h"
#include "ciodds.h"
#include "cioddlo.h"
#include "entdslo.h"

/*****************************************************************************/
/*                                                                           */
/* The following routines reside in this file:                               */
/*                                                                           */
/*      ent_recv_setup        - Sets up the device driver data structures    */
/*                              for receive; initializes receive tcw's.      */
/*      ent_recv_start        - Sets up the adapter data structures and      */
/*                              issues receive start command to the adapter  */
/*      ent_recv_undo         - Deallocates resources allocated by           */
/*                              ent_recv_setup and ent_recv_start.           */
/*      ent_recv              - Offlevel receive interrupt handler.          */
/*      Setup_Rbufd           - Internal routine used by setup and           */
/*                              recv handler to prepare receive buffer       */
/*                              descriptors for adapter receive dma.         */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/* NAME: ent_recv_setup                                                      */
/*                                                                           */
/* FUNCTION: Set up the TCWs for receive.                                    */
/*           Initialize the receive list buffer descriptor indexes.          */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the process thread.                     */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*                                                                           */
/*    Output: TCWs for receive set up, receive list variables initialized.   */
/*                                                                           */
/*    Called From: xxx_act                                                   */
/*                                                                           */
/*    Calls To:  m_get, m_clget, M_HASCL, m_free                             */
/*                                                                           */
/* RETURN:  0 - Successful completion                                        */
/*          ENOBUFS - No Bus Address space available                         */
/*                                                                           */
/*****************************************************************************/

int ent_recv_setup (
	dds_t *dds_ptr)		/* DDS pointer - tells which adpater */
{
	int i;
	int bsize;

	TRACE2 ("rxsB",(ulong)dds_ptr);

	ASSERT(WRK.recv_buf == NULL);
	ASSERT((WRK.recv_tcw_cnt % 2) == 0);

	/* malloc a large enough buffer, buffer will hold 2 packet per
	 * page
	 */
	bsize = WRK.recv_tcw_cnt * DMA_PSIZE / 2;
	WRK.recv_buf = xmalloc(bsize, DMA_L2PSIZE, kernel_heap);
	if (WRK.recv_buf == NULL)
	{
		return(ENOBUFS);
	}

	/* get xmem descriptor for receive buffer
	 */
	WRK.rbuf_xd.aspace_id = XMEM_INVAL;
	if (xmattach(WRK.recv_buf, bsize, &WRK.rbuf_xd, SYS_ADSPACE)
			!= XMEM_SUCC)
	{
		xmfree(WRK.recv_buf, kernel_heap);
		WRK.recv_buf = NULL;
		return(ENOBUFS);
	}

	/* pin the receive buffer, while we are at the process level
	 */
	if (pin(WRK.recv_buf, bsize))
	{
		xmdetach(&WRK.rbuf_xd);
		xmfree(WRK.recv_buf, kernel_heap);
		WRK.recv_buf = NULL;
		return(ENOBUFS);
	}

	/* initialize dma and buffer information
	 */
	for (i = 0; i < WRK.recv_tcw_cnt; i++)
	{
		WRK.recv_list[i].buf = WRK.recv_buf + (DMA_PSIZE/2*i);
		WRK.recv_list[i].rdma = (char *)(WRK.Recv_Tcw_Base +
							 (DMA_PSIZE/2*i));
	}

	/* Initialize the buffer descriptor pointers
	 */
	WRK.Recv_Index = 0;	/* first buffer descriptor              */

	TRACE1 ("rxsE");
	return(0);
}


/*****************************************************************************/
/*                                                                           */
/* NAME: ent_recv_start                                                      */
/*                                                                           */
/* FUNCTION: Sets up the adapter receive data structures and issues the      */
/*           receive start command to the adapter.                           */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the off level interrupt handler thread. */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*                                                                           */
/*    Output: TCWs for receive set up, receive list variables initialized.   */
/*                                                                           */
/*    Called From: entexecdque                                               */
/*                                                                           */
/*    Calls To: d_master, PIO_PUTSR,					     */
/*                                                                           */
/* RETURN:  0 - Successful completion                                        */
/*                                                                           */
/*****************************************************************************/
int ent_recv_start (
	register dds_t  *dds_ptr) /* DDS pointer - tells which adapter */
{
	int       i;         /* Loop Counter                             */
	int     ioa;         /* Attachment to the I/O Bus                */
	int     bus;         /* Attachment to Adapter ram                */
	int    mbox;         /* Receive mailbox address                  */
	int    ipri;
	ushort next_buf;
	recv_des_t *rd;

	TRACE2("rxbB",(ulong)dds_ptr); /* ent_recv_start begin */

	
	ipri = i_disable(INTCLASS2);
	/* Get access to the adapter shared memory and I/O interface
	 */
	bus = (int)BUSMEM_ATT( (ulong)DDI.cc.bus_id, DDI.ds.bus_mem_addr );
	ioa = (int)BUSIO_ATT(  (ulong)DDI.cc.bus_id, DDI.ds.io_port );

	/* Calculate host addresses to mailbox and receive lists
	 */
	mbox = bus + WRK.recv_mail_box;

	/* setup mailbox for receive
	 */
	PIO_PUTSR( mbox + offsetof( RECVMBOX, status ), 0 );
	PIO_PUTSR( mbox + offsetof( RECVMBOX, recv_list ), WRK.recv_list_off );

	/* Set up recv buffer descriptor pointers for all the buffer
	 * descriptors
	 */
        next_buf = WRK.recv_list_off;
	for (i = 0; i < WRK.recv_tcw_cnt; i++)
	{
      		WRK.recv_list[i].rbufd = next_buf;
		next_buf = PIO_GETSR(bus + next_buf + offsetof(BUFDESC, next));
	}
	ASSERT(next_buf == WRK.recv_list_off);

	/* Set up all the buffer descriptors with a DMA addresses and buffers
	 */
	for (i = 0; i < WRK.recv_tcw_cnt; i++)
	{
		rd = &WRK.recv_list[i];
		/* Map the buffer to the DMA address
	 	 */
      		d_master(WRK.dma_channel, DMA_READ|DMA_NOHIDE, rd->buf,
			 (size_t)(DMA_PSIZE/2), (struct xmem *)&(WRK.rbuf_xd),
			  rd->rdma);

		/* Fill in the buffer descriptor
		 */
		PIO_PUTLRX((long *)(bus + rd->rbufd + offsetof(BUFDESC,buflo)),
					(long)rd->rdma);
      		PIO_PUTC(bus + rd->rbufd + offsetof(BUFDESC,control), 0);
		PIO_PUTC(bus + rd->rbufd + offsetof(BUFDESC, status),  0);

	}

	/* Set EL in last buffer descriptor
	 */
	WRK.Recv_El_off = WRK.recv_list[WRK.recv_tcw_cnt - 1].rbufd;
	PIO_PUTC(bus + WRK.Recv_El_off +
			offsetof(BUFDESC, control), EL);

	/* Set intial value of last_netid for enhance performance on receive
	 */
	if (CIO.num_netids > 0)
	{
		WRK.last_netid = CIO.netid_table_ptr[0].netid;
		WRK.last_netid_length = CIO.netid_table_ptr[0].length;
		WRK.last_netid_index  = 0;
	}

	/* Issue receive command
	 */
	
	PIO_PUTC( ioa + COMMAND_REG, RX_START );

	/* Check to see if the command completed without errors */
	WRK.mbox_status = recv;
	mail_check_status(dds_ptr);

	/* Restore segment register to no longer access I/O
	 */
	BUSIO_DET(ioa);
	BUSMEM_DET(bus);

	i_enable(ipri);

	TRACE1 ("rxbE");

	return(0);
}

/*****************************************************************************/
/*                                                                           */
/* NAME: ent_recv_undo                                                       */
/*                                                                           */
/* FUNCTION: Undoes the effects of ENT_RECV_SETUP and ENT_RECV_START.        */
/*           Frees the mbufs allocated & frees the region.                   */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs under both the process thread and the off level    */
/*      interrupt handler.                                                   */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*                                                                           */
/*    Output: TCWs for receive set up, receive list variables initialized.   */
/*                                                                           */
/*    Called From: xxx_inact, entexecdque, ent_start_error                   */
/*                                                                           */
/*    Calls To: d_complete, m_free, M_XMEMD                                  */
/*                                                                           */
/* RETURN:  0 - Successful completion                                        */
/*                                                                           */
/*****************************************************************************/
int ent_recv_undo (
	dds_t *dds_ptr)		/* DDS pointer - tells which adapter. */
{
	int i;
	recv_des_t *rd;
	int rc;

	TRACE2("rxuB",(ulong)dds_ptr);

	/* Free up the DMA buffer resources
	 */
	for (i = 0; i < WRK.recv_tcw_cnt; i++)
	{
		/* Test to verify that the resource was allocated
		 */
		rd = &WRK.recv_list[i];
		if (rd->rbufd != 0)
		{
			/* Finish the DMA processing of the buffer
			 */
			d_complete (WRK.dma_channel, DMA_READ|DMA_NOHIDE,
				rd->buf, DMA_PSIZE/2,
				(struct xmem *)&WRK.rbuf_xd, rd->rdma);

         		/* Initialize these so this routine doesn't crash
			 * on close
			 */
			rd->rbufd = 0;
		}
	}

	/* free the receive buffer area if it was allocated
	 */
	if (WRK.recv_buf != NULL)
	{
		rc = unpin(WRK.recv_buf, WRK.recv_tcw_cnt * DMA_PSIZE / 2);
		assert(rc == 0);
 		rc = xmdetach(&WRK.rbuf_xd);
 		assert(rc == 0);
  		rc = xmfree(WRK.recv_buf, kernel_heap);
		assert(rc == 0);
 		WRK.recv_buf = NULL;
	}

	/* Intialize local indices
	 */
	WRK.Recv_Index = 0;
	WRK.Recv_El_off = 0;

	TRACE1 ("rxuE");

	return(0);

} /* end ent_recv_undo                                                       */











