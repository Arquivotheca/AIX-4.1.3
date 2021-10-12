static char sccsid[] = "@(#)77        1.1  src/bos/kernext/dmpa/dmpa_select.c, sysxdmpa, bos411, 9428A410j 4/30/93 12:49:22";
/*
 *   COMPONENT_NAME: (SYSXDMPA) MP/A DIAGNOSTICS DEVICE DRIVER
 *
 *   FUNCTIONS: mpaselect
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

#include <sys/device.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/malloc.h>
#include <sys/devinfo.h>
#include <sys/uio.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/except.h>
#include <sys/dmpauser.h>
#include <sys/dmpadd.h>


/*****************************************************************************
**
** NAME:                mpaselect
**
** FUNCTION:
**
** EXECUTION
** ENVIRONMENT: Can be called from the process environment only.
**
** NOTES:
**
** Input:
**
** Output:
**
** Returns:     0 - Success
**
** Called By:
**
** Calls To:
**
*****************************************************************************/
int
mpaselect(
	dev_t dev,
	ulong events,
	ushort *reventp,
	int chan)
{
	struct acb      *acb;         /* pointer to adapter control block */
	recv_elem_t *recvp;
	int spl;

	/* log a trace hook */
	DDHKWD2 (HKWD_DD_MPADD, DD_ENTRY_SELECT, 0, dev, events);


	if ( ((acb = get_acb(minor(dev))) == NULL) ||
			!(OPENP.op_flags&OP_OPENED) ) {

		DDHKWD3(HKWD_DD_MPADD, DD_EXIT_IOCTL, ENXIO, dev,0,0x02);
		return ENXIO;
	}

	if (OPENP.op_mode & DKERNEL) {
		/*
		** illegal call from kernel process
		*/
		DDHKWD3(HKWD_DD_MPADD,DD_EXIT_SELECT,EACCES,dev,0,0x03);
		return EACCES;
	}


	*reventp = 0;                   /* initialize return value */
	if ((events & ~POLLSYNC) == 0) { /* no events requested */
		DDHKWD3(HKWD_DD_MPADD,DD_EXIT_SELECT,0,dev,0,0x04);
		return 0;
	}

	/*
	** set return status for all requested events that are true
	** Disable interrupts until we get the bits set if necessary.
	*/
	DISABLE_INTERRUPTS(spl);
	if (events & POLLOUT) {
		/*
		** POLLOUT is based on the xmit q having at least one
		** free element left.
		*/
		if (acb->xmit_free != NULL) {
			*reventp |= POLLOUT;
		}
	}
	if (events & POLLIN) {
		for( recvp = acb->act_recv_head; recvp != NULL; recvp = recvp->rc_next ) {
			if( recvp->rc_state == RC_COMPLETE ) {
				*reventp |= POLLIN;
				break;
			}
		}
	}
	if (events & POLLPRI) {
		if (acb->act_stat_head != NULL) {
			*reventp |= POLLPRI;
		}
	}

	/*
	** If no requested event was found, then see if async notification wanted
	*/
	if (*reventp == 0) {
		if (!(events & POLLSYNC)) {
			/*
			** This is an asynchronous request. Set flags so notification
			** with selnotify() will be done later.
			*/
			OPENP.op_select |= events;

		}
	}
	ENABLE_INTERRUPTS(spl);

	DDHKWD3(HKWD_DD_MPADD, DD_EXIT_SELECT, 0, dev, *reventp , 0x05);

	return 0;
} /* catselect() */
