static char sccsid[] = "@(#)39	1.33  src/bos/kernext/tokdiag/tokxmit.c, diagddtok, bos411, 9428A410j 10/26/93 16:26:25";
/*
 * COMPONENT_NAME: (SYSXTOK) - Token-Ring device handler
 *
 * FUNCTIONS:  write_get_user(), 
 *             tx_setup(), tx_undo(), clean_tx(), tx_limbo_startup(),
 *             move_tx_list()
 *
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/cblock.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/dump.h>
#include <sys/errno.h>
#include "tok_comio_errids.h"
#include <sys/err_rec.h>
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

#include "tokdshi.h"
#include "tokdds.h"
#include "tokdslo.h"
#include "tokproto.h"
#include "tokprim.h"

extern tracetable_t    tracetable;
extern cdt_t   ciocdt;
extern dd_ctrl_t   dd_ctrl;

/* #define TOKDEBUG_TX  (1) */


/*--------------------------------------------------------------------*/
/************           Transmit List Setup             ***************/
/*--------------------------------------------------------------------*/

/*
*
*  Set up the Transmit List Chain so the adapter can have access to it. It
*  will first setup the Transmit TCW management table then for EACH Transmit
*  List element do the following:
*      - Put on a IOCC cache boundary (64 bytes)
*      - Set TX CSTAT to 0
*      - calculate the Bus address of the memory and store it in
*        the previous List element's forward pointer
*
*  RETURN CODES:
*      0 - Successful completion
*      ENOBUFS - No Bus Address space available
*/

int
tx_setup(register dds_t *p_dds)
{
	int i, j;
	t_tx_list *p_tmp;
	t_tx_list *p_d_tmp;
	int buf_size;


	p_dds->wrk.p_tx_head = (t_tx_list *)( (int)p_dds->wrk.p_mem_block +
                                     ACA_TX_CHAIN_BASE);

	p_dds->wrk.p_d_tx_head = (t_tx_list *)( (int)p_dds->wrk.p_d_mem_block
                                               + ACA_TX_CHAIN_BASE);
	p_dds->wrk.tx_tcw_base = p_dds->ddi.dma_base_addr + TX_AREA_OFFSET;

	if (DDI.ring_speed == 0)
		WRK.tx_buf_size = 4480;	/* multiple of 128 which contains */
					/* the maximum frame size (done for */
					/* cache alignment) */
	else
		WRK.tx_buf_size = 5 * CLBYTES;

	buf_size = WRK.tx_buf_size * MAX_TX_TCW;

	if ( (WRK.tx_tcw_base + buf_size) >
	     (DDI.dma_base_addr + DDI.dma_bus_length) )
		return(ENOBUFS);
		
	WRK.xmit_buf = xmalloc(buf_size, PGSHIFT, pinned_heap);
	if (WRK.xmit_buf == NULL)
		return(ENOBUFS);
	WRK.xbuf_xd.aspace_id = XMEM_INVAL;
	if (xmattach(WRK.xmit_buf, buf_size, (struct xmem *)&WRK.xbuf_xd,
		SYS_ADSPACE) != XMEM_SUCC)
	{
		xmfree(WRK.xmit_buf, pinned_heap);
		return(ENOBUFS);
	}
	
        /* Set up variable for Transmit list
	 */
	WRK.tx_list_next_buf  = 0;
	WRK.tx_list_next_out  = 0;
	WRK.tx_list_next_in  = 0;
	WRK.tx_tcw_use_count  = 0;
	WRK.xmits_queued = 0;

	p_d_tmp = p_dds->wrk.p_d_tx_head;
	p_tmp = p_dds->wrk.p_tx_head;

	for (i = 0; i < MAX_TX_TCW; i++) {
		WRK.tx_buf_des[i].io_addr = WRK.tx_tcw_base +
						(i * WRK.tx_buf_size);
		WRK.tx_buf_des[i].sys_addr = WRK.xmit_buf +
						(i * WRK.tx_buf_size);
		d_master(WRK.dma_chnl_id, DMA_WRITE_ONLY,
			WRK.tx_buf_des[i].sys_addr, WRK.tx_buf_size,
			(struct xmem *)&WRK.xbuf_xd,
			(char *)WRK.tx_buf_des[i].io_addr);
	}

	WRK.tx_des_next_in = 1;

	/*
	*   Setup the Tx Chain List Elements
	*/
	for (i=0 ; i < MAX_TX_TCW; i++ ) {
		p_tmp[i].tx_cstat = 0;
		p_tmp[i].p_tx_elem = NULL;
		p_dds->wrk.p_tx_elem[i] = NULL;

		for (j=0; j<=2; ++j) {
			p_tmp[i].gb[j].cnt = 0;
			p_tmp[i].gb[j].addr_hi = NULL;
			p_tmp[i].gb[j].addr_lo = NULL;
			p_tmp[i].p_d_addr[j] = NULL;
			p_dds->wrk.p_d_tx_addrs[i][j] = 0;
		}

		p_tmp[i].p_d_fwdptr = &p_d_tmp[i+1];
		p_dds->wrk.p_d_tx_fwds[i] = &p_d_tmp[i+1];
	}

	/* Set up last TX Chain element */
	p_tmp[MAX_TX_TCW-1].p_d_fwdptr = p_dds->wrk.p_d_tx_head;
	p_dds->wrk.p_d_tx_fwds[MAX_TX_TCW-1] = p_dds->wrk.p_d_tx_head;

	p_dds->wrk.p_tx_tail = &p_tmp[MAX_TX_TCW-1];
	p_dds->wrk.p_d_tx_tail = &p_d_tmp[MAX_TX_TCW-1];

	p_dds->wrk.p_tx_next_avail = p_dds->wrk.p_tx_head;
	p_dds->wrk.p_d_tx_next_avail = p_dds->wrk.p_d_tx_head;
	p_dds->wrk.p_tx_1st_update = p_dds->wrk.p_tx_head;
	p_dds->wrk.p_d_tx_1st_update = p_dds->wrk.p_d_tx_head;
	p_dds->wrk.tx_chain_full = FALSE;
	p_dds->wrk.tx_chain_empty = TRUE;
	p_dds->wrk.issue_tx_cmd = TRUE;

	/* Flush the System Cache */
	vm_cflush(p_dds->wrk.p_mem_block, PAGESIZE);

	return(0);
}  /* end function tx_setup */

/*--------------------------------------------------------------------*/
/***************        Transmit Undo                   ***************/
/*--------------------------------------------------------------------*/
int
tx_undo(register dds_t *p_dds)
{
	int i;

	p_dds->wrk.p_tx_tcw_list = NULL;
	p_dds->wrk.p_tx_head = NULL;
	p_dds->wrk.p_d_tx_head = NULL;
	p_dds->wrk.p_tx_tail = NULL;
	p_dds->wrk.p_d_tx_tail= NULL;
	p_dds->wrk.tx_tcw_base = NULL;
	p_dds->wrk.tx_chain_full = FALSE;
	p_dds->wrk.tx_chain_empty = TRUE;
	p_dds->wrk.issue_tx_cmd = TRUE;

	for (i = 0; i < MAX_TX_TCW; i++) 
		d_complete(WRK.dma_chnl_id, DMA_WRITE_ONLY,
			WRK.tx_buf_des[i].sys_addr, WRK.tx_buf_size,
			(struct xmem *)&WRK.xbuf_xd,
			(char *)WRK.tx_buf_des[i].io_addr);

	xmdetach(&(WRK.xbuf_xd));

	xmfree(WRK.xmit_buf, pinned_heap);

	return(0);
}

/*
 * NAME: clean_tx
 *
 * FUNCTION:
 *     Cleans out the TX chain and TX queue of all completed and/or
 *     pending TX requests in preparation for going into limbo mode.
 *     Packets which have not been transmitting and
 *     TX done acknowledgement has been requested are acknowledge with
 *     a TOK_TX_ERROR status.
 *
 *     The TX blocking variable(s) are set so as to prevent any writes
 *     from being accepted while in limbo mode.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes on the OFLV.
 */
int clean_tx( dds_t *p_dds)
{
	int rc;
	xmt_elem_t  *p_tx_elem;
	int	x;

	x = i_disable(INTCLASS2);
	/* Turn off the Watch dog timer, and reset the reason it was set
	 */
	w_stop (&(WDT));
	WRK.wdt_setter = WDT_INACTIVE;

	tx_chain_undo(p_dds);

	/* Loop until caught up to tx list with all packets completed
	 */
	while (WRK.tx_list_next_out != WRK.tx_list_next_in)
	{
		if (WRK.xmit_queue[WRK.tx_list_next_out].in_use == FALSE) {
			XMITQ_INC(WRK.tx_list_next_out);
			continue;
		}
		p_tx_elem = &WRK.xmit_queue[WRK.tx_list_next_out];

		rc = d_complete(WRK.dma_chnl_id, DMA_WRITE_ONLY,
			p_tx_elem->tx_ds->sys_addr, (size_t)p_tx_elem->bytes,
			(struct xmem *)&WRK.xbuf_xd,
			(char *)p_tx_elem->tx_ds->io_addr);

		/* Return transmit element to cio code, then mark it as free
	         */
		tok_xmit_done(p_dds, p_tx_elem, TOK_TX_ERROR, NULL);
		p_tx_elem->in_use = FALSE;
		WRK.tx_tcw_use_count--;

		/* bump the adapter out pointer
		 */
		XMITQ_INC(WRK.tx_list_next_out);
	}

	p_dds->wrk.p_tx_next_avail = p_dds->wrk.p_tx_head;
	p_dds->wrk.p_d_tx_next_avail = p_dds->wrk.p_d_tx_head;
	p_dds->wrk.p_tx_1st_update = p_dds->wrk.p_tx_head;
	p_dds->wrk.p_d_tx_1st_update = p_dds->wrk.p_d_tx_head;
	p_dds->wrk.tx_chain_full = FALSE;
	p_dds->wrk.tx_chain_empty = TRUE;
	p_dds->wrk.issue_tx_cmd = TRUE;

	/* empty tramit queue
	 */
	WRK.tx_tcw_use_count = 0;
	WRK.tx_list_next_in = 0;
	WRK.tx_list_next_buf = 0;
	WRK.tx_list_next_out = 0;
	WRK.xmits_queued = 0;

	i_enable(x);
	return(0);
}  /* end function clean_tx() */

int
tx_chain_undo(dds_t *p_dds)
{
	t_tx_list   *p_tx_tmp;
	t_tx_list   *p_d_tx_tmp;
	t_tx_list   tmp_tx_list;
	unsigned int    done=FALSE;

	p_tx_tmp = p_dds->wrk.p_tx_1st_update;
	p_d_tx_tmp = p_dds->wrk.p_d_tx_1st_update;

	while ( (p_tx_tmp != p_dds->wrk.p_tx_next_avail) && !done ) {
		move_tx_list( p_dds, &tmp_tx_list, 
			p_tx_tmp, p_d_tx_tmp, DMA_READ);

		if (tmp_tx_list.tx_cstat== 0) {
			done = TRUE;
		}
		p_dds->wrk.tx_chain_full = FALSE;

		/*
		* Zero out the CSTAT, data count, and address fields.
		* Put the change back to the Adapter Control Area
		*/
		tmp_tx_list.tx_cstat = 0;
		tmp_tx_list.frame_size = 0;
		tmp_tx_list.p_tx_elem = NULL;
		bzero(&tmp_tx_list.gb, sizeof(tmp_tx_list.gb));

		move_tx_list( p_dds, &tmp_tx_list, 
			   p_tx_tmp, p_d_tx_tmp, DMA_WRITE_ONLY);

		NEXT_TX_AVAIL(p_tx_tmp, p_dds);
		NEXT_D_TX_AVAIL(p_d_tx_tmp, p_dds);

		if (p_tx_tmp == p_dds->wrk.p_tx_next_avail)
			p_dds->wrk.tx_chain_empty = TRUE;
	}
	return(0);
}

/*
 * NAME: tx_limbo_startup
 *
 * FUNCTION:
 *     This function prepares the device driver for transmitting of data
 *     after successfully cycling through limbo mode.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes on the OFLV.
 *
 * (NOTES:) More detailed description of the function, down to
 *      what bits / data structures, etc it manipulates.
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *      software error recovery.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */

int tx_limbo_startup( dds_t *p_dds)
{
   int sil, rc, i;
   t_tx_list	*p_d_tmp;	/* Temp. TX List Bus address */
   t_tx_list	*p_tmp;		/* Temp. TX List virtual address */
   t_tx_list	tmp_tx_list;	/* Temp. Tx list element */

	bzero(&tmp_tx_list, sizeof(t_tx_list) );

	p_d_tmp = p_dds->wrk.p_d_tx_head;	/* start at the head */
	p_tmp = p_dds->wrk.p_tx_head;		/* start at the head */

	/*
	 * Re-initialize the Transmit Chain in the ACA
	 */
	for ( i=0; i < TX_CHAIN_SIZE; ++i)
	{
		tmp_tx_list.p_d_fwdptr = p_dds->wrk.p_d_tx_fwds[i];
           	rc = d_kmove(&tmp_tx_list, p_d_tmp,
                        (sizeof(t_tx_list)),
                        p_dds->wrk.dma_chnl_id,
                        p_dds->ddi.bus_id, DMA_WRITE_ONLY);

		if (rc == EINVAL) 	/* IOCC is NOT buffered */
           	   bcopy(&tmp_tx_list, p_tmp,
                        (sizeof(t_tx_list)) );

		++p_d_tmp; 	/* move to next Tx list element in chain */
		++p_tmp; 	/* move to next Tx list element in chain */

	} /* end for loop */


   sil = i_disable(INTCLASS2);
   p_dds->wrk.issue_tx_cmd = TRUE;
   i_enable(sil);

   return(0);
}  /* end function tx_limbo_startup() */



/*
 * NAME: move_tx_list
 *
 * FUNCTION:
 *
 *	Moves a Transmit List element either to or from the Transmit Chain.
 *	The TX chain resides in the ACA.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes on the OFLV or on the process thread.
 *
 * NOTES:
 *
 *	DMA_READ:	Specifying this flag causes this routine to move
 *			the TX list element from the TX Chain.  The TX element
 *			and the Bus addresses for this list are placed into
 *			the TX list element after the ACA's copy is d_kmoved.
 *
 *	DMA_WRITE_ONLY:	Specifying this flag causes this routine to move
 *			the TX list to the TX chain.  The TX element and the
 *			Bus addresses for this list are saved in the work
 *			section of the DDS before the list is moved to the
 *			ACA via the d_kmove() kernel service.
 *      
 *
 * RECOVERY OPERATION:
 *	None
 *
 * DATA STRUCTURES:
 *
 * RETURNS: 
 *	Returns the value of d_kmove() kernel service.
 */

int
move_tx_list( register dds_t 		*p_dds,		/* DDS pointer */
		register t_tx_list	*tmp_tx_list,	/* entry being moved */
		register t_tx_list	*p_tx_list,	/* TX list buff ptr */
		register t_tx_list	*p_d_tx_list, 	/* TX list BUS addr  */
		register int		flag)		/* direction flag */
{
   int rc=0, i;

	switch ( flag )
	{
		case DMA_READ:
		{
			/*
			 * get the TX list element
			 * from the ACA.
			 */
			rc = d_kmove( tmp_tx_list, p_d_tx_list,
					(unsigned int)TX_LIST_SIZE, 
					p_dds->wrk.dma_chnl_id,
                			p_dds->ddi.bus_id, DMA_READ);

			if (rc == EINVAL)
			{	/* IOCC is NOT buffered */
				bcopy( p_tx_list, tmp_tx_list,
				   (unsigned int)TX_LIST_SIZE);
				rc = 0;
			}

			/*
			 * get the index for the master
			 * copy array of the transmit elements
			 */
			i = GET_TX_INDEX(p_d_tx_list, p_dds->wrk.p_d_tx_head);

			/* 
			 * return the originals 
			 */
			tmp_tx_list->p_tx_elem =  p_dds->wrk.p_tx_elem[i];
			tmp_tx_list->p_d_addr[0] = 
				p_dds->wrk.p_d_tx_addrs[i][0];
			tmp_tx_list->p_d_addr[1] = 
				p_dds->wrk.p_d_tx_addrs[i][1];
			tmp_tx_list->p_d_addr[2] = 
				p_dds->wrk.p_d_tx_addrs[i][2];

			break;
		} /* end case DMA_READ */
		case DMA_WRITE_ONLY:
		{
			/*
			 * get the index for the master
			 * copy array of the transmit elements
			 */
		
			i = GET_TX_INDEX(p_d_tx_list, p_dds->wrk.p_d_tx_head);

			/* 
			 * save the originals in the safe place
			 */
			p_dds->wrk.p_tx_elem[i] = tmp_tx_list->p_tx_elem;
			p_dds->wrk.p_d_tx_addrs[i][0] = 
				tmp_tx_list->p_d_addr[0];
			p_dds->wrk.p_d_tx_addrs[i][1] = 
				tmp_tx_list->p_d_addr[1];
			p_dds->wrk.p_d_tx_addrs[i][2] = 
				tmp_tx_list->p_d_addr[2];

			/*
			 * put in the forward pointer  
			 */
			tmp_tx_list->p_d_fwdptr = p_dds->wrk.p_d_tx_fwds[i];

			/*
			 * move the TX list element
			 * into the ACA.
			 */
			rc = d_kmove( tmp_tx_list, p_d_tx_list,
					(unsigned int)TX_LIST_SIZE, 
					p_dds->wrk.dma_chnl_id,
                			p_dds->ddi.bus_id, DMA_WRITE_ONLY);

			if (rc == EINVAL)
			{	/* IOCC is NOT buffered */
				bcopy( tmp_tx_list, p_tx_list,
				   (unsigned int)TX_LIST_SIZE);
				rc = 0;
			}
			break;
		} /* end case DMA_WRITE_ONLY */
		default:
			TRACE3("FooT", TX_MOVE_LIST_0, flag);
	} /* end switch */
return(rc);
} /* end move_tx_list() */

/*****************************************************************************/
/*                                                                           */
/* NAME: tok_tx_time_out                                                     */
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
/*    Called From: offl                                                      */
/*                                                                           */
/*    Calls To:  tok_xmit_done, w_stop, d_complete, M_XMEMD,                 */
/*                                                                           */
/* RETURN:  None                                                             */
/*                                                                           */
/*****************************************************************************/
void
tok_tx_time_out (
	dds_t *p_dds)		/* DDS pointer - tells which adapter. */
{
	t_tx_list   *p_tx_tmp, *p_d_tx_tmp;
	t_tx_list   tmp_tx_list;		/* temp Transmit List Element */
	xmt_elem_t *p_tx_elem;
	int rc;

	TRACE2 ("txtB",(ulong)p_dds);

	/* stop the watchdog timer and reset the reason why it was set
	 */
	w_stop (&(WDT));
	WRK.wdt_setter = WDT_INACTIVE;

	p_tx_tmp = p_dds->wrk.p_tx_1st_update;
	p_d_tx_tmp = p_dds->wrk.p_d_tx_1st_update;
	rc = move_tx_list( p_dds, &tmp_tx_list, p_tx_tmp, p_d_tx_tmp,
			DMA_READ);

	p_tx_elem = tmp_tx_list.p_tx_elem;
	if (p_tx_elem->in_use == FALSE)
		return;

	/* finish DMA processing
	 */
	rc = d_complete(WRK.dma_chnl_id, DMA_WRITE_ONLY,
			p_tx_elem->tx_ds->sys_addr, (size_t)p_tx_elem->bytes,
			(struct xmem *)&WRK.xbuf_xd,
			(char *)p_tx_elem->tx_ds->io_addr);

	/* notify cio code that transmit is done
	 */
	tok_xmit_done(p_dds, p_tx_elem, CIO_TIMEOUT, NULL);

	/* free resources associated with transmit
	 */
	p_tx_elem->in_use = FALSE;
	WRK.tx_tcw_use_count--;
	XMITQ_INC(WRK.tx_list_next_out);

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
			   p_tx_tmp, p_d_tx_tmp, DMA_WRITE_ONLY);

	NEXT_TX_AVAIL(p_tx_tmp, p_dds);
	NEXT_D_TX_AVAIL(p_d_tx_tmp, p_dds);

	if (p_tx_tmp == p_dds->wrk.p_tx_next_avail)
		p_dds->wrk.tx_chain_empty = TRUE;

	/* update the 1st TX Chain element to update pointer */
	p_dds->wrk.p_tx_1st_update = p_tx_tmp;
	p_dds->wrk.p_d_tx_1st_update = p_d_tx_tmp;

	if (WRK.tx_tcw_use_count > 0)
	{
		WRK.wdt_setter = WDT_XMIT;
		w_start(&(WDT));
	}

	TRACE1("txtE");
}

/*****************************************************************************/
/*
 * NAME: tok_free_chan_elem
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
tok_free_chan_elem (
	dds_t *p_dds,
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
	while (dest != WRK.tx_list_next_buf) {
		/* if the element is for this channel then remove it
		 */
		if (WRK.xmit_queue[dest].chan == chan) {
			tok_xmit_done(p_dds, &WRK.xmit_queue[dest], CIO_OK, NULL);

			/* remove packet from the queue
			 */
			source = dest;
			XMITQ_INC(source);
			while (source != WRK.tx_list_next_buf) {
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
