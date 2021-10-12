static char sccsid[] = "@(#)46	1.4  src/bos/kernext/sol/sol_open.c, sysxsol, bos411, 9428A410j 6/17/91 15:11:42";
/*
 * COMPONENT_NAME: (SYSXSOL) Serial Optical Link Device Handler
 *
 * FUNCTIONS:  sol_open
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dump.h>
#include <sys/dma.h>
#include <sys/errno.h>
#include <sys/err_rec.h>
#include <sys/errids.h>
#include <sys/intr.h>
#include <sys/ioctl.h>
#include <sys/limits.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/pri.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/timer.h>
#include <sys/trchkid.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/watchdog.h>
#include <sys/comio.h>  /* this and following files must be in this sequence */
#include <sys/soluser.h>
#include "soldd.h"

extern struct sol_ddi	sol_ddi;

/*
 * NAME: sol_open
 *
 * FUNCTION: Opens the serial optical link subsystem or single port
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called at open time from the process environment,
 *	and it can page fault.                   
 *
 * (NOTES:) 
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0	- successful completion                        
 *	ENODEV	- invalid minor number specified
 *	EINVAL	- invalid parameter specified
 *	ENOMEM	- couldn't malloc memory
 *	EBUSY	- attempt to open a port already open in diag mode
 */

int
sol_open(
dev_t			devno,	/* major/minor device number		*/
ulong			mode,	/* open mode (kernel or user process)	*/
chan_t			chan,	/* channel allocated by sol_mpx		*/
struct kopen_ext	*ext)	/* open extension 			*/
{
	int			errnoval,i;
	uint			cdt_size;
	struct sol_open_struct	*open_ptr;
	struct sol_sta_que_elem	*stat_ptr;

	SYS_SOL_TRACE("PopB", mode, chan, ext);
	(void) lockl(&(sol_ddi.global_lock), LOCK_SHORT);

	/*
	 *  We should fail here if invalid devno, this channel has not
	 *  been allocated, the channel is closed, or the channel has
	 *  already been opened.
	 */

	if ((minor(devno) > SOL_OPS_MINOR) || (sol_ddi.chan_state[chan] == 
	    SOL_CH_AVAIL) || (sol_ddi.chan_state[chan] == SOL_CH_CLOSED) ||
	    (sol_ddi.open_ptr[chan] != NULL)) {
		sol_ddi.chan_state[chan] = SOL_CH_CLOSED;
		unlockl(&(sol_ddi.global_lock));
		SYS_SOL_TRACE("PopE", ENODEV, chan, 0);
		return ENODEV;
	}
	if ((sol_ddi.num_norm_opens == 0) && (sol_ddi.num_diag_opens == 0)) {
		/*
		 *  For first open, pin the code, register the dump table,
		 *  initialize imcs, and clear all stat counters.
		 */
		if (errnoval = pincode(sol_com_write)) {
			sol_ddi.chan_state[chan] = SOL_CH_CLOSED;
			unlockl(&(sol_ddi.global_lock));
			SYS_SOL_TRACE("PopE", errnoval, chan, 0);
			return errnoval;
		}

		/*
		 *  Allocate, initialize, and register for component dump.
		 */
		cdt_size = (uint) (sizeof (struct cdt_head) +
		    (SOL_TOTAL_OPENS + 7) * sizeof (struct cdt_entry));
		sol_ddi.cdt = (struct cdt *) xmalloc(cdt_size, 2,
		    (heapaddr_t) pinned_heap);
		if (sol_ddi.cdt == NULL) {
			(void *) unpincode(sol_com_write);
			sol_ddi.chan_state[chan] = SOL_CH_CLOSED;
			unlockl(&(sol_ddi.global_lock));
			SYS_SOL_TRACE("PopE", errnoval, chan, 0);
			return ENOMEM;
		}
		bzero((caddr_t) sol_ddi.cdt, cdt_size);
		sol_ddi.cdt->cdt_magic = DMP_MAGIC;
		bcopy("soldd", sol_ddi.cdt->cdt_name, 6);
		sol_ddi.cdt->cdt_len = cdt_size;
		(void) dmp_add(sol_cdt_func);

		if (imcs_init() == -1) { /* couldn't malloc imcs_headers */
			(void) dmp_del(sol_cdt_func);
			(void) xmfree((caddr_t) sol_ddi.cdt,
			    (heapaddr_t) pinned_heap);
			(void *) unpincode(sol_com_write);
			sol_ddi.chan_state[chan] = SOL_CH_CLOSED;
			unlockl(&(sol_ddi.global_lock));
			SYS_SOL_TRACE("PopE", errnoval, chan, 0);
			return ENOMEM;
		}
		/*
		 *  Clear all stat counters
		 */
		bzero((caddr_t)&sol_ddi.stats, sizeof(sol_query_stats_t));
	}
	if ((sol_ddi.chan_state[chan] == SOL_CH_NORM) ||
	    (sol_ddi.chan_state[chan] == SOL_CH_SNORM)) { /* normal open */
		if (mode & DKERNEL) {	/* kernel-mode open */
			open_ptr = (struct sol_open_struct *)
			    xmalloc(sizeof(struct sol_open_struct), 2,
			    pinned_heap);
			if (open_ptr == NULL) {
				/*
				 *  Clean up if malloc failed.
				 */
				if ((sol_ddi.num_norm_opens == 0) &&
				    (sol_ddi.num_diag_opens == 0)){/*1st open*/
					imcs_terminate();
					(void) dmp_del(sol_cdt_func);
					(void) xmfree((caddr_t) sol_ddi.cdt,
					    (heapaddr_t) pinned_heap);
					(void *)unpincode(sol_com_write);
				}
				sol_ddi.chan_state[chan] = SOL_CH_CLOSED;
				unlockl(&(sol_ddi.global_lock));
				SYS_SOL_TRACE("PopE", ENOMEM, chan, 0);
				return ENOMEM;
			}
			bzero((caddr_t) open_ptr,
			    sizeof(struct sol_open_struct));
			open_ptr->xmit_fn = ext->tx_fn;
			open_ptr->recv_fn = ext->rx_fn;
			open_ptr->stat_fn = ext->stat_fn;
			open_ptr->open_id = ext->open_id;
		} else {		/* user-mode open */
			/*
			 *  malloc open structure and status queue elements
			 */
			open_ptr = (struct sol_open_struct *)
			    xmalloc(sizeof(struct sol_open_struct) + 
			    sol_ddi.ops_info.sta_que_size *
			    sizeof(struct sol_sta_que_elem), 2, pinned_heap);
			if (open_ptr == NULL) {
				/*
				 *  Clean up if malloc failed.
				 */
				if ((sol_ddi.num_norm_opens == 0) &&
				    (sol_ddi.num_diag_opens == 0)) {/*1st open*/
					imcs_terminate();
					(void) dmp_del(sol_cdt_func);
					(void) xmfree((caddr_t) sol_ddi.cdt,
					    (heapaddr_t) pinned_heap);
					(void *)unpincode(sol_com_write);
				}
				sol_ddi.chan_state[chan] = SOL_CH_CLOSED;
				unlockl(&(sol_ddi.global_lock));
				SYS_SOL_TRACE("PopE", ENOMEM, chan, 0);
				return ENOMEM;
			}
			bzero((caddr_t) open_ptr,
			    sizeof(struct sol_open_struct));
			/* 
			 *  Set up the rcv queue head and tail ptrs.
			 */
			open_ptr->recv_que.head = NULL;
			open_ptr->recv_que.tail = NULL;

			/* 
			 *  Set up the stat queue head, tail, and next ptrs.
			 *  This is a ring queue, where the head is NULL
			 *  when the queue is empty, head=tail when there
			 *  is one element, and tail->next = head when it
			 *  is full.
			 */
			open_ptr->stat_que.head = NULL;
			open_ptr->stat_que.tail = (struct sol_sta_que_elem *)
			    ((uint) open_ptr + sizeof(struct sol_open_struct));
			stat_ptr = open_ptr->stat_que.tail;
			while ((uint) stat_ptr < ((uint) open_ptr->
			    stat_que.tail + sol_ddi.ops_info.sta_que_size *
			    sizeof(struct sol_sta_que_elem))) {
				stat_ptr->next = (struct sol_sta_que_elem *)
				    ((uint) stat_ptr +
				    sizeof(struct sol_sta_que_elem));
				stat_ptr = stat_ptr->next;
			}
			((struct sol_sta_que_elem *) ((uint) stat_ptr -
			    sizeof(struct sol_sta_que_elem)))->next =
			    open_ptr->stat_que.tail;

			/*
			 *  Initialize the event word for read.
			 */
			open_ptr->recv_event = EVENT_NULL;
		}
		if (sol_ddi.chan_state[chan] == SOL_CH_SNORM) {
			/*
			 *  For a serialized user, we have to allocate a
			 *  unique subchannel.
			 */
			i = 0;
			while (sol_ddi.ser_sc[i] == SOL_SC_IN_USE)
				i++;
			open_ptr->subchannel = SOL_SER_SC + 2*i;
			sol_ddi.ser_sc[i] = SOL_SC_IN_USE;
			open_ptr->serialize = TRUE;
		} else {
			/*
			 *  All non-serialized users use the same subchannel.
			 */
			open_ptr->subchannel = SOL_MIN_SC;
			open_ptr->serialize = FALSE;
		}
		if (sol_ddi.num_norm_opens == 0) {
			/*
			 *  Register mbuf usage - to execute, we need
			 *  a minimum of one chain each of small mbufs
			 *  and clusters.  Initially, we will get
			 *  SOL_MAX_MMBUFS small mbuf chains, and
			 *  SOL_MAX_CMBUFS cluster chains.
			 */
			sol_ddi.mbreq.low_mbuf = SOL_MMBUF_LEN;
			sol_ddi.mbreq.low_clust = SOL_CMBUF_LEN;
			sol_ddi.mbreq.initial_mbuf = SOL_MMBUF_LEN *
			    SOL_MAX_MMBUFS;
			sol_ddi.mbreq.initial_clust = SOL_CMBUF_LEN *
			    SOL_MAX_CMBUFS;
			m_reg(&sol_ddi.mbreq);
		}
		sol_ddi.num_norm_opens++;
	} else { /* diag mode */
		open_ptr = (struct sol_open_struct *)
		    xmalloc(sizeof(struct sol_open_struct), 2, pinned_heap);
		if (open_ptr == NULL) {
			/*
			 *  Clean up if malloc fails.
			 */
			if ((sol_ddi.num_norm_opens == 0) &&
			    (sol_ddi.num_diag_opens == 0)) {/*1st open*/
				imcs_terminate();
				(void) dmp_del(sol_cdt_func);
				(void) xmfree((caddr_t) sol_ddi.cdt,
				    (heapaddr_t) pinned_heap);
				(void *)unpincode(sol_com_write);
			}
			sol_ddi.chan_state[chan] = SOL_CH_CLOSED;
			unlockl(&(sol_ddi.global_lock));
			SYS_SOL_TRACE("PopE", ENOMEM, chan, 0);
			return ENOMEM;
		}
		bzero((caddr_t) open_ptr, sizeof(struct sol_open_struct));
		sol_ddi.port_state[minor(devno)] = SOL_DIAG_MODE;
		imcs_diag_start(minor(devno));
		sol_ddi.num_diag_opens++;
	}
	open_ptr->chan = chan;
	open_ptr->num_netids = 0;
	open_ptr->devflag = mode;
	open_ptr->select_req = 0;
	open_ptr->close_event = EVENT_NULL;
	open_ptr->xmit_count = 0;
	sol_ddi.open_ptr[chan] = open_ptr;
	unlockl(&(sol_ddi.global_lock));
	SYS_SOL_TRACE("PopE", 0, chan, 0);
	return 0;
}
