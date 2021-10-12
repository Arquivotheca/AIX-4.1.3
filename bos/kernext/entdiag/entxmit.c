static char sccsid[] = "@(#)93  1.20  src/bos/kernext/entdiag/entxmit.c, diagddent, bos411, 9428A410j 10/28/93 15:57:08";
/*
 * COMPONENT_NAME: DIAGDDENT - Ethernet device handler
 *
 * FUNCTIONS: ent_tx_setup, ent_tx_undo, xxx_xmit, ent_tx_off_lvl,
 *            ent_tx_time_out
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
#include <sys/sleep.h>
#include <sys/timer.h>
#include <sys/trchkid.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/watchdog.h>
#include <sys/xmem.h>
#include <sys/syspest.h>
#include <stddef.h>

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

void ent_xmit_done(dds_t *dds_ptr, xmit_elem_t *xelm, int status);


/*****************************************************************************/
/*                                                                           */
/* NAME: ent_tx_setup                                                        */
/*                                                                           */
/* FUNCTION: Set up the TCWs for transmit.                                   */
/*           Initialize the transmit list variables.                         */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the process thread.                     */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*                                                                           */
/*    Output: transmit list variables initialized.                           */
/*                                                                           */
/*    Called From: xxx_act                                                   */
/*                                                                           */
/*                                                                           */
/* RETURN:  0 - Successful completion                                        */
/*                                                                           */
/*****************************************************************************/
int
ent_tx_setup (
	dds_t *dds_ptr)		/* DDS pointer - tells which adapter */
{
	int buf_size;

	TRACE2 ("txsB",(ulong)dds_ptr);

	ASSERT(WRK.xmit_buf == NULL);

	/* allocate space for transmit buffers
	 */
	buf_size = WRK.xmit_tcw_cnt * DMA_PSIZE / 2;
	WRK.xmit_buf = xmalloc(buf_size, DMA_L2PSIZE, pinned_heap);
	if (WRK.xmit_buf == NULL)
	{
		TRACE1("txsE");
		return(-1);
	}

	/* create an xmem descriptor for the tranmit descriptors
	 */
	WRK.xbuf_xd.aspace_id = XMEM_INVAL;
	if (xmattach(WRK.xmit_buf, buf_size, (struct xmem *)&WRK.xbuf_xd,
			SYS_ADSPACE) != XMEM_SUCC)
	{
		TRACE1("txsF");
		xmfree(WRK.xmit_buf, pinned_heap);
		return(-1);
	}
	

        /* Set up variable for Transmit list
	 */
	WRK.tx_list_next_buf  = 0;
	WRK.tx_list_next_out  = 0;
	WRK.tx_list_next_in  = 0;
	WRK.tx_tcw_use_count  = 0;
	WRK.xmits_queued = 0;
	WRK.xmit_el_off = (void *)-1;

	TRACE1 ("txsE");            /* Transmit set up (tx_setup) call end */

	return(0);

}

/*****************************************************************************/
/*
 * NAME: ent_tx_start
 *
 * FUNCTION: set up tranmit buffer descriptors
 *
 * EXECUTION ENVIRONEMT:
 *	Called from execdqueue under offlevel interrup
 *
 * RETURNS:
 *	None
 */
/*****************************************************************************/

void
ent_tx_start(
	dds_t *dds_ptr)		/* adapters device dependent area */
{
	int bus;
	ushort boffset;
	int i;

	TRACE2("TXS1", dds_ptr);

	bus = (int)BUSMEM_ATT(DDI.cc.bus_id, DDI.ds.bus_mem_addr);
	
	/* get the offsets for all the transmit buffer descriptors, and
	 * allocate an IO address for each buffer descriptor.  Setup the
	 * tranmit descriptor's io address
	 */
	boffset = WRK.xmit_list_off;
	for (i = 0; i < WRK.xmit_tcw_cnt; i++)
	{
		WRK.tx_buf_des[i].io_addr = WRK.tx_tcw_base + i * DMA_PSIZE/2;
		WRK.tx_buf_des[i].des_offset = boffset;
		WRK.tx_buf_des[i].sys_addr = WRK.xmit_buf + i * DMA_PSIZE/2;
		PIO_PUTLRX((long *)(bus + boffset + offsetof(BUFDESC, buflo)),
				WRK.tx_buf_des[i].io_addr);
		d_master(WRK.dma_channel, DMA_WRITE_ONLY,
			WRK.tx_buf_des[i].sys_addr, DMA_PSIZE/2,
			(struct xmem *)&WRK.xbuf_xd,
			(char *)WRK.tx_buf_des[i].io_addr);
		boffset = PIO_GETSR(bus + boffset + offsetof(BUFDESC, next));
	}

	/* setup the end of list descriptor pointer, and the index for first
	 * buffer descriptor to be used for a transmit
	 */
	WRK.xmit_el_off = &WRK.tx_buf_des[0];
	WRK.tx_des_next_in = 1;

	ASSERT(boffset == WRK.xmit_list_off);
	DASSERT(PIO_GETC(bus + WRK.xmit_el_off->des_offset +
		offsetof(BUFDESC, control)) & EL_BIT_MASK);

	BUSMEM_DET(bus);

	TRACE1("TXS1");
}

/*****************************************************************************/
/*
 * NAME: ent_free_chan_elem
 *
 * FUNCTION: Free a transmit elelment for a particular channel
 *
 * EXECUTION ENVIRONMENT:
 *	Called under process level
 *
 * RETURNS:
 *	1 if an element was found and removed
 *	0 if no element was found
 *
 */
/*****************************************************************************/

int
ent_free_chan_elem (
	dds_t *dds_ptr,
	chan_t chan)
{
	int ipri;
	uint dest;
	int source;

	/* serialize access to transmit lists
	 */
	ipri = i_disable(INTOFFL1);

	/* get first element on software queue
	 */
	dest = WRK.tx_list_next_in;

	/* loop while there are more elements on the software queue
	 */
	while (dest != WRK.tx_list_next_buf)
	{

		/* if the element is for this channel then remove it
		 */
		if (WRK.xmit_queue[dest].chan == chan)
		{
			/* inform caller that tranmit is done
			 */
			ent_xmit_done(dds_ptr, &WRK.xmit_queue[dest], CIO_OK);

			/* remove packet from the queue
			 */
			source = dest;
			XMITQ_INC(source);
			while (source != WRK.tx_list_next_buf)
			{
				WRK.xmit_queue[dest] = WRK.xmit_queue[source];
				XMITQ_INC(dest);
				XMITQ_INC(source);
			}
			XMITQ_DEC(WRK.tx_list_next_buf);
			i_enable(ipri);
			return(1);
		}
		XMITQ_INC(dest);
	}

	i_enable(ipri);
	return(0);
}

/*****************************************************************************/
/*                                                                           */
/* NAME: ent_tx_undo                                                         */
/*                                                                           */
/* FUNCTION: reset transmit pointers and flags.                              */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs under both the offlevel interrupt handler          */
/*      and the process thread.                                              */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*                                                                           */
/*    Output: Deleted transmit TCW list and flags reset.                     */
/*                                                                           */
/*    Called From: xxx_act - on failure                                      */
/*                 xxx_inact - normal shutdown                               */
/*                 ent_start_error - start failure                           */
/*                                                                           */
/*    Calls To:  d_complete, ent_xmit_done, M_XMEMD			     */
/*                                                                           */
/* RETURN:  0 - Successful completion                                        */
/*                                                                           */
/*****************************************************************************/

int
ent_tx_undo (
	dds_t *dds_ptr)		/* DDS pointer - tells which adapter */

{
	int rc;
	struct xmit_elem *xel;

	TRACE2 ("txuB",(ulong)dds_ptr);

	/* Turn off the Watch dog timer, and reset the reason it was set
	 */
	w_stop (&(WDT));
	WRK.wdt_setter = WDT_INACTIVE;

	/* Loop until caught up to tx list with all packets completed
	 */
	while ((WRK.xmit_queue[WRK.tx_list_next_out].in_use == TRUE) &&
			(WRK.tx_list_next_out != WRK.tx_list_next_in))
	{
		xel = &WRK.xmit_queue[WRK.tx_list_next_out];

		rc = d_complete(WRK.dma_channel, DMA_WRITE_ONLY,
			xel->tx_ds->sys_addr, (size_t)xel->bytes,
			(struct xmem *)&WRK.xbuf_xd,
			(char *)xel->tx_ds->io_addr);

		if (rc != DMA_SUCC)
			ent_logerr(dds_ptr, ERRID_ENT_ERR5, EFAULT, 0, 0);

		/* Return transmit element to cio code, then mark it as free
	         */
		ent_xmit_done(dds_ptr, xel, CIO_OK);
		xel->in_use = FALSE;
		WRK.tx_tcw_use_count--;

		/* bump the adapter out pointer
		 */
		XMITQ_INC(WRK.tx_list_next_out);
	}

	ASSERT(WRK.tx_tcw_use_count == 0);
	ASSERT(WRK.tx_list_next_in == WRK.tx_list_next_buf);

	/* free transmit data buffer
	 */
	if (WRK.xmit_buf != NULL)
	{
		rc = xmdetach((struct xmem *)&WRK.xbuf_xd);
		assert(rc == 0);
		rc = xmfree(WRK.xmit_buf, pinned_heap);
		assert(rc == 0);
		WRK.xmit_buf = NULL;
	}

	/* empty tramit queue
	 */
	WRK.tx_list_next_in = 0;
	WRK.tx_list_next_buf = 0;
	WRK.tx_list_next_out = 0;
	WRK.xmits_queued = 0;

	TRACE1 ("txuE");
	return(0);
}

/*****************************************************************************/
/*                                                                           */
/* NAME: ent_tx_time_out                                                     */
/*                                                                           */
/* FUNCTION: Report error on transmit.                                       */
/*           Clean up transmit resources.                                    */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the timer off level interrupt thread.   */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter.                              */
/*                                                                           */
/*    Output: Transmit done status reported and resources freed.             */
/*                                                                           */
/*    Called From: xxx_offl                                                  */
/*                                                                           */
/*    Calls To:  ent_xmit_done, w_stop, d_complete, M_XMEMD,                 */
/*                                                                           */
/* RETURN:  None                                                             */
/*                                                                           */
/*****************************************************************************/
void
ent_tx_time_out (
	dds_t *dds_ptr)		/* DDS pointer - tells which adapter. */
{
	xmit_elem_t *xel;
	int rc;

	TRACE2 ("txtB",(ulong)dds_ptr);

	/* stop the watchdog timer and reset the reason why it was set
	 */
	w_stop (&(WDT));
	WRK.wdt_setter = WDT_INACTIVE;

	xel = &WRK.xmit_queue[WRK.tx_list_next_out];

	/* finish DMA processing
	 */
	rc = d_complete(WRK.dma_channel, DMA_WRITE_ONLY,
			xel->tx_ds->sys_addr, (size_t)xel->bytes,
			(struct xmem *)&WRK.xbuf_xd,
			(char *)xel->tx_ds->io_addr);

	if (rc != DMA_SUCC)
		ent_logerr(dds_ptr, ERRID_ENT_ERR5, EFAULT, 0, 0);

	/* notify cio code that transmit is done
	 */
	ent_xmit_done(dds_ptr, xel, CIO_TIMEOUT);

	/* free resources associated with transmit
	 */
	xel->in_use = FALSE;
	WRK.tx_tcw_use_count--;

	XMITQ_INC(WRK.tx_list_next_out);

	if (WRK.tx_tcw_use_count > 0)
	{
		WRK.wdt_setter = WDT_XMIT;
		w_start(&(WDT));
	}

	TRACE1("txtE");
}
