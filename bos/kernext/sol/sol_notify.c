static char sccsid[] = "@(#)45	1.2  src/bos/kernext/sol/sol_notify.c, sysxsol, bos411, 9428A410j 5/14/91 14:34:44";
/*
 * COMPONENT_NAME: (SYSXSOL) Serial Optical Link Device Handler
 *
 * FUNCTIONS:  sol_start_done
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
 * NAME: sol_start_done
 *
 * FUNCTION: Handles completion of CIO_START.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from the interrupt environment, and it can
 *	not page fault.
 *
 * (NOTES:)
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	None.
 */
void
sol_start_done(
int	status)		/* status of CIO_START operation	*/
{
	ushort			qid, i;
	cio_stat_blk_t		stat_blk;
	struct sol_open_struct	*open_ptr;
	struct super_header	*super_header;

	/*
	 *  This routine can get called multiple times at startup time, so
	 *  we should only do something if we are in the STARTING state.
	 */
	if (sol_ddi.start_state != SOL_STARTING) {
		return;
	}
	ASSERT(status == CIO_OK);

	/*
	 *  Set up receive buffers and headers.
	 */
	while (sol_ddi.num_mbuf < SOL_MAX_MMBUFS) {
		super_header = sol_buildchain((short)FALSE);
		if (super_header == NULL) {
			break;
		} else {
			sol_ddi.num_mbuf++;
			super_header->imcs_header.imcs_chain_word =
			    (struct imcs_header *) sol_ddi.hdrmlist;
			sol_ddi.hdrmlist = super_header;
		}
	}
	while (sol_ddi.num_clus < SOL_MAX_CMBUFS) {
		super_header = sol_buildchain((short)TRUE);
		if (super_header == NULL) {
			break;
		} else {
			sol_ddi.num_clus++;
			super_header->imcs_header.imcs_chain_word =
			    (struct imcs_header *) sol_ddi.hdrclist;
			sol_ddi.hdrclist = super_header;
		}
	}
	/*
	 *  Allocate range of subchannels.
	 */
	qid = SOL_IMCS_QID;
	imcs_declare(&qid, (caddr_t(*)())sol_get_header, DCL_LIBERAL_Q, 0);
	imcs_ctl(SOL_IMCS_QID, RCV);
	/*
	 *  Send START_DONE notifications to all users waiting
	 */
	for (i = 0 ; i < SOL_TOTAL_NETIDS ; i += 2) {
		open_ptr = sol_ddi.netid_table[i>>1];
		if (open_ptr != NULL) {
			stat_blk.code = CIO_START_DONE;
			stat_blk.option[0] = CIO_OK;
			stat_blk.option[1] = i * 2; /* netid */
			sol_report_status(open_ptr, &stat_blk);
		}
	}
	/*
	 *  Set the start state.
	 */
	sol_ddi.start_state = SOL_STARTED;
	return;
}
