static char sccsid[] = "@(#)48	1.2  src/bos/kernext/sol/sol_read.c, sysxsol, bos411, 9428A410j 5/14/91 14:35:09";
/*
 * COMPONENT_NAME: (SYSXSOL) Serial Optical Link Device Handler
 *
 * FUNCTIONS:  sol_read
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
 * NAME: sol_read
 *
 * FUNCTION: Provides means for receiving data for user-mode caller.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from the process environment, and it can
 *	page fault.                   
 *
 * NOTES:
 *	This routine can be called only by user-mode callers.
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0		- successful completion                        
 *	EACCES		- attempt to call read from kernel-mode
 *	ENODEV		- devno is invalid
 *	EINTR		- system cll was interrupted
 *	EMSGSIZE	- to much data to fit in user's buffer
 *	EFAULT		- invalid address specified
 *	ENOCONNECT	- the device has not been started
 */

int
sol_read(
dev_t		devno,		/* major/minor device number		*/
struct uio	*uiop,		/* pointer to uio struct 		*/
int		chan,		/* mpx channel number			*/
cio_read_ext_t	*arg)		/* NULL, or pointer to read_extension	*/
{

	struct sol_open_struct	*open_ptr;
	struct mbuf		*tempmbufp, *mptr;
	cio_read_ext_t		read_ext;
	int			total_bytes, bytes_to_move, rc, oldpri;

	SYS_SOL_TRACE("PrdB", uiop, chan, arg);
	if (minor(devno) != SOL_OPS_MINOR) {
		SYS_SOL_TRACE("PrdE", ENODEV, chan, 0);
		return ENODEV;
	}
	open_ptr = sol_ddi.open_ptr[chan];
	if (open_ptr == NULL) {
		SYS_SOL_TRACE("PrdE", ENODEV, chan, 0);
		return ENODEV;
	}
	if (open_ptr->devflag & DKERNEL) {
		SYS_SOL_TRACE("PrdE", EACCES, chan, 0);
		return EACCES;
	}
	if (open_ptr->recv_que.head == NULL) {
		if (open_ptr->devflag & DNDELAY) {
			SYS_SOL_TRACE("PrdE", 0, chan, 0);
			return 0;
		}
	}

	/*
	 * sol_get_rcv will try to get a rcv element. If there is not one
	 * available, the routine will sleep.  Once there is one available,
	 * it will be taken off the receive queue, and the mbuf pointer
	 * will be returned.
	 */

	rc = sol_get_rcv(open_ptr, &open_ptr->recv_que, &mptr);

	if (rc == EINTR) {
		SYS_SOL_TRACE("PrdE", EINTR, chan, 0);
		return EINTR;
	}

	/*
	 * At this point we just got an mbuf chain off the receive queue.
	 * Now calculate the length of the packet.
	 */

	tempmbufp = mptr;
	total_bytes = 0;
	while (tempmbufp != NULL) {
		total_bytes += tempmbufp->m_len;
		tempmbufp = tempmbufp->m_next;
	}
	SYS_SOL_TRACE("Prd1", mptr, total_bytes, 0);
	rc = 0;
	/* 
	 *  If the user buffer is too small:
	 *  	1) if no ext provided, no data copied, EMSGSIZE returned
	 *	2) if ext provided, truncate data, return CIO_BUF_OVFLW in ext
	 */
	if (total_bytes > uiop->uio_resid) {
		if (arg == NULL) {
			rc = EMSGSIZE;
		} else { /* ext provided */
			read_ext.status = CIO_BUF_OVFLW;
			read_ext.netid = (ushort) *((caddr_t)
			    ((int)MTOD(mptr, uchar *) + SOL_NETID_OFFSET));
			total_bytes = uiop->uio_resid;
		}
	} else if (arg != NULL) {
		read_ext.status = CIO_OK;
		read_ext.netid = (ushort) *((caddr_t)
		    ((int)MTOD(mptr, uchar *) + SOL_NETID_OFFSET));
	}
	/*
	 * If ext was provided, or user buffer is big enough, move the data
	 */
	if (rc == 0) {
		for (tempmbufp = mptr ; total_bytes > 0 ;
		    total_bytes -= bytes_to_move,
		    tempmbufp = tempmbufp->m_next) {
			bytes_to_move = tempmbufp->m_len;
			if (bytes_to_move > total_bytes) {
				bytes_to_move = total_bytes;
			}
			if (uiomove(MTOD(tempmbufp,uchar *), bytes_to_move,
			    UIO_READ, uiop)) {
				rc = EFAULT;
			}
		}
	}

	/*
	 *  Free the mbuf chain.
	 */

	m_freem(mptr);

	if ((arg != NULL) && (rc == 0)) {
		rc = MOVEOUT(open_ptr->devflag, &read_ext, arg,
		    sizeof(cio_read_ext_t));
	}
	/*
	 *  Before returning, clear out the uio_offset field to prevent
	 *  over-running the counter.
	 */
	uiop->uio_offset = 0;
	SYS_SOL_TRACE("PrdE", rc, chan, 0);
	return rc;
}
