static char sccsid[] = "@(#)57  1.11  src/bos/kernext/cat/cat_select.c, sysxcat, bos411, 9428A410j 2/22/94 16:52:03";
/*
 * COMPONENT_NAME: (SYSXCAT) - Channel Attach device handler
 *
 * FUNCTIONS: catselect()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define FNUM 9
#include <sys/device.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/comio.h>
#include <sys/malloc.h>
#include <sys/devinfo.h>
#include <sys/uio.h>
#include <sys/pin.h>
#include <sys/intr.h>
#include <sys/lockl.h>
#include <sys/sleep.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/except.h>

#include "catdd.h"


/*****************************************************************************
**
** NAME:		catselect
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
** Returns:  	0 - Success
**
** Called By: 
**
** Calls To:
**
*****************************************************************************/
int
catselect(
	dev_t dev, 
	ulong events, 
	ushort *reventp, 
	int chan)
{
	struct ca *ca;
	open_t *openp;
	recv_elem_t *recvp;
	int spl;

	DDHKWD1(HKWD_DD_CATDD, DD_ENTRY_SELECT, 0, dev);

	if ((chan < 0) || (chan > CAT_MAX_OPENS)
		|| ((ca = catget(minor(dev))) == NULL)
		|| (((openp = &ca->open_lst[chan])->op_flags&OP_OPENED) == 0)) {
		DDHKWD1(HKWD_DD_CATDD,DD_EXIT_SELECT,ENXIO,dev);
		return ENXIO;
	}

	if (openp->op_mode & DKERNEL) {
		/*
		** illegal call from kernel process 
		*/
		DDHKWD1(HKWD_DD_CATDD,DD_EXIT_SELECT,EACCES,dev);
		return EACCES;
	}

	*reventp = 0;                   /* initialize return value */
	if ((events & ~POLLSYNC) == 0) { /* no events requested */
		DDHKWD1(HKWD_DD_CATDD,DD_EXIT_SELECT,0,dev);
		return 0;
	}

	/*
	** set return status for all requested events that are true 
	** Disable interrupts until we get the bits set if necessary.
	*/
	DISABLE_INTERRUPTS(spl);
	if (events & POLLOUT) {
		/*
		** POLLOUT is based on the ability to reserve PSCA
		** send buffers.  If send buffers are available then
		** xmit elements will be available because the number
		** of xmit elements is calculated to ensure this when
		** the driver is configured.
		*/
		if (!sfb_avail(ca)) {
			*reventp |= POLLOUT;
		}
	}
	if (events & POLLIN) {
		for( recvp = openp->recv_act; recvp != NULL; recvp = recvp->rc_next ) {
			if( recvp->rc_state == RC_COMPLETE ) {
				*reventp |= POLLIN;
				break;
			}
		}
	}
	if (events & POLLPRI) {
		if (openp->stat_act != NULL) {
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
			openp->op_select |= events;
		}
	}
	ENABLE_INTERRUPTS(spl);

	DDHKWD1(HKWD_DD_CATDD, DD_EXIT_SELECT, 0, dev);

	return 0;
} /* catselect() */
