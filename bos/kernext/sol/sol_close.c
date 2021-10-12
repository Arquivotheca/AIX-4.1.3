static char sccsid[] = "@(#)40	1.4  src/bos/kernext/sol/sol_close.c, sysxsol, bos411, 9428A410j 6/17/91 15:09:35";
/*
 * COMPONENT_NAME: (SYSXSOL) Serial Optical Link Device Handler
 *
 * FUNCTIONS:  sol_close
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
 * NAME: sol_close
 *
 * FUNCTION: Closes the serial optical link subsystem or single port
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called at close time from the process environment, 
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
 *	ENODEV	- devno is invalid
 */

int
sol_close(
dev_t			devno,	/* major/minor device number		*/
chan_t			chan)	/* channel allocated by sol_mpx		*/
{
	int			i;
	struct sol_open_struct	*open_ptr, *tmp_open_ptr;
	struct mbuf		**head_ptr, *next_ptr, *m;
	struct imcs_header	*hdr;
	struct pm		*pm;

	SYS_SOL_TRACE("PclB", devno, chan, 0);
	(void) lockl(&(sol_ddi.global_lock), LOCK_SHORT);
	if (minor(devno) > SOL_OPS_MINOR) {
		unlockl(&(sol_ddi.global_lock));
		SYS_SOL_TRACE("PclE", ENODEV, chan, 0);
		return ENODEV;
	}
	open_ptr = sol_ddi.open_ptr[chan];
	if (open_ptr->num_netids != 0) {
		/*
		 *  Loop through the table of started netids and remove
		 *  any of them that are for this open.
		 */
		for (i=0; i < SOL_TOTAL_NETIDS ; i += 2) {
			tmp_open_ptr = sol_ddi.netid_table[i>>1];
			if ((tmp_open_ptr != NULL) &&
			    (tmp_open_ptr->chan == chan)) {
				sol_ddi.netid_table[i>>1] = NULL;
				sol_ddi.num_netids--;
				tmp_open_ptr->num_netids--;
			}
		}
		if (sol_ddi.num_netids == 0) {
			sol_shutdown();
		}
	}

	if ((sol_ddi.chan_state[chan] == SOL_CH_NORM) ||
	    (sol_ddi.chan_state[chan] == SOL_CH_SNORM)) { /* norm open */
		sol_ddi.num_norm_opens--;
		/*
		 *  If we allocated a serialized subchannel for this open,
		 *  free it here.
		 */
		if (open_ptr->serialize) {
			sol_ddi.ser_sc[(open_ptr->subchannel - SOL_SER_SC)>>1]
			    = SOL_SC_AVAIL;
		}

		/*
		 *  Free all the mbufs on the receive queue
		 */

		if (!(open_ptr->devflag & DKERNEL)) {
			head_ptr = &open_ptr->recv_que.head;
			while (*head_ptr != NULL) {
				next_ptr = (*head_ptr)->m_nextpkt;
				m_freem(*head_ptr);
				*head_ptr = next_ptr;
			}
			open_ptr->recv_que.tail = NULL;
		}

		if (sol_ddi.num_norm_opens == 0) {
			m_dereg(&sol_ddi.mbreq);
		}
	} else { /* diag open */
		imcs_diag_stop(minor(devno));
		sol_ddi.num_diag_opens--;
		sol_ddi.port_state[minor(devno)] = SOL_NORM_MODE;
	}

	if ((sol_ddi.num_norm_opens == 0) && (sol_ddi.num_diag_opens == 0)) {
		imcs_terminate();
		sol_xmit_wait(open_ptr);	/* wait for xmits to complete */
		(void) dmp_del(sol_cdt_func);
		(void) xmfree((caddr_t) sol_ddi.cdt, (heapaddr_t) pinned_heap);
		(void *)unpincode(sol_com_write);
	}
	/*
	 *  Free the open structure
	 */
	sol_ddi.open_ptr[chan] = NULL;
	xmfree((caddr_t) open_ptr, pinned_heap);

	sol_ddi.chan_state[chan] = SOL_CH_CLOSED;
	unlockl(&(sol_ddi.global_lock));
	SYS_SOL_TRACE("PclE", 0, chan, 0);
	return 0;
}
