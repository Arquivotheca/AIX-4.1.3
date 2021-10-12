static char sccsid[] = "@(#)53	1.15  src/bos/kernext/cat/cat_mpx.c, sysxcat, bos411, 9428A410j 2/22/94 16:51:25";
/*
 * COMPONENT_NAME: (SYSXCAT) - Channel Attach device handler
 *
 * FUNCTIONS: catmpx()
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
#include <sys/dma.h>
#include <sys/errno.h>
#include <sys/syspest.h>
#include <sys/except.h>
#include <sys/id.h>

#include "catdd.h"

extern int	caglobal_lock;			/* Global Lock Variable */


/*****************************************************************************
** NAME:	catmpx
**
** FUNCTION:	Either allocates a channel with the adapter in exclusive
**		(diagnostic) mode, allocates the next free channel, or
**		deallocates a given channel.
**		
**		Get the channel structure for this adapter.
**		Check for a dead adapter.
**		Acquire the adapter lock.
**		Deallocate a multiplexed channel:
**			free the memory if the channel exists.
**		or ... Allocate the diagnostic mode channel:
**			Verify access priviledge (superuser).
**			Verify exlusive access (no other open()s).
**			Allocate memory for an open structure.
**			Set adapter mode to DIAGNOSTIC
**			Return the channel ID.
**		or ... Allocate the next free channel:
**			Set adapter mode to NORMAL.
**			Find next free channel ID.
**			Allocate memory for open structure.
**			Return the channel ID.
**
** EXECUTION ENVIRONMENT: process thread only.
**
** NOTES:
**    Input:
**		device major/minor numbers
**		mode: delete a channel, open next free channel, or open in
**			diagnostic mode
**    Output:
**		channel number or error code
**    Called From:
**		kernel system call handler
**    Calls:
**		lockl() unlockl() catget() xmfree() xmalloc()
**
** RETURNS:	0 - Success (and allocated channel number)
**		EINTR - received signal while trying to acquire lock
**		ENXIO - device doesn't exist
**		EBUSY - already open... exclusive open (diagnostic mode) failed 
**		EBUSY - maximum open()s exceeded
**		ENODEV - couldn't deallocate a non-existent mpx-channel
**		EFAULT - KFREE() (xmemfree()) couldn't free allocated memory
**		EINVAL - invalid device extension (NOT 'D')
**		ESHUTDOWN - the adapter has been shutdown
**
*****************************************************************************/
int
catmpx(
	dev_t dev, 		/* major and minor number */
	int *chanp, 	/* address for returning allocated channel number */
	char *channame)	/* pointer to special file name suffix */
{
	struct ca *ca;
	open_t *openp;
	int i;
	int spl;

	DDHKWD1(HKWD_DD_CATDD, DD_ENTRY_MPX, 0, dev);

	/*
	** Get the ca structure for this adapter. 
	*/
	if ((ca = catget(minor(dev))) == NULL) {
		DDHKWD4(HKWD_DD_CATDD,DD_EXIT_MPX, ENXIO, dev,
				((channame != NULL) ? *channame : 0), 0, 0);
		return ENXIO;
	}

	/*
	** Either Deallocate or Allocate a channel.
	*/
	if (channame == NULL) {
		/*
		** Deallocate a channel
		*/
		if( ((openp = &ca->open_lst[*chanp])->op_flags & OP_OPENED) == 0) {
			DDHKWD4(HKWD_DD_CATDD,DD_EXIT_MPX,ENODEV,dev,0,0,0);
			return ENODEV;
		}
		openp->op_flags &= ~OP_OPENED;
		ca->flags &= ~CATDIAG;

		/*
		** Free the channel.
		*/
		free_open_struct(ca, openp);
	} else {
		/*
		** Acquire Adapter Lock, waits for lock or returns early on signal
		*/
		if (lockl(&ca->adap_lock, LOCK_SIGRET) != LOCK_SUCC) {
			DDHKWD4(HKWD_DD_CATDD, DD_EXIT_MPX, EINTR, dev,
					((channame!=NULL)?*channame:0), 0, 0);
			return EINTR;
		}
			
		/*
		** Allocate a channel in either diagnostic or normal mode.
		*/
		if (*channame == 'D') {
			/*
			** Check the callers access priviledge
			** (Must be superuser)
			*/
			if (getuidx(ID_EFFECTIVE)) {
				unlockl(&ca->adap_lock);
				DDHKWD4(HKWD_DD_CATDD, DD_EXIT_MPX, EACCES,
					dev, *channame, 0, 0);
				return EACCES;
			}
			/*
			** Open the adapter in diagnostic mode
			*/
			if (ca->num_opens) {
				unlockl(&ca->adap_lock);
				DDHKWD4(HKWD_DD_CATDD, DD_EXIT_MPX, EBUSY,
					dev, *channame, 0, 0);
				return EBUSY;
			}
			ca->flags |= CATDIAG;
			bzero((openp = &ca->open_lst[0]), sizeof(open_t));
			openp->op_flags |= OP_OPENED;
			*chanp = 0;
		} else if (*channame != 'C' && *channame != 0) {
			/*
			** Invalid extension --- only 'D' and 'C' are allowed...
			*/
			unlockl(&ca->adap_lock);
			DDHKWD4(HKWD_DD_CATDD, DD_EXIT_MPX, EINVAL,
				dev, *channame, 0, 0);
			return EINVAL;
		} else {
			/*
			** Allocate the next available channel in normal mode.
			*/
			if (ca->flags & CATDIAG) {
				/*
				** Fail if already open in diagnostic mode.
				*/
				unlockl(&ca->adap_lock);
				DDHKWD4(HKWD_DD_CATDD, DD_EXIT_MPX, EBUSY,
					dev, *channame, 0, 0);
				return EBUSY;
			}

			if (ca->num_opens >= CAT_MAX_OPENS) {
				/*
				** Fail if already have maximum number of opens.
				*/
				unlockl(&ca->adap_lock);
				DDHKWD4(HKWD_DD_CATDD, DD_EXIT_MPX, EBUSY,
					dev, *channame, 0, 0);
				return EBUSY;
			}

			/*
			** Fail if already in exclusive-use/
			** Customer Engineer mode.
			*/
			if (ca->flags & CAT_CE_OPEN) {
				unlockl(&ca->adap_lock);
				DDHKWD4(HKWD_DD_CATDD, DD_EXIT_MPX, EBUSY,
					dev, *channame, 0, 0);
				return EBUSY;
			}

			if (*channame == 'C') {
				/*
				** Check the callers access priviledge
				** (Must be superuser)
				*/
				if (getuidx(ID_EFFECTIVE)) {
					unlockl(&ca->adap_lock);
					DDHKWD4(HKWD_DD_CATDD, DD_EXIT_MPX,
						EACCES, dev, *channame, 0, 0);
					return EACCES;
				}
				ca->flags |= CAT_CE_OPEN;
			}

			/*
			** Find the next free channel
			*/
			for (i=0; i < CAT_MAX_OPENS && (ca->open_lst[i].op_flags&OP_OPENED); i++)
				;
			if (i < CAT_MAX_OPENS) {
				bzero((openp = &ca->open_lst[i]), sizeof(open_t));
				openp->op_flags |= OP_OPENED;
				if (*channame == 'C')
					openp->op_flags |= OP_CE_OPEN;
				openp->op_pid = getpid();
			} else {
				unlockl(&ca->adap_lock);
				DDHKWD4(HKWD_DD_CATDD, DD_EXIT_MPX, EBUSY,
					dev, *channame, 0, 0);
				return EBUSY;
			}
				
			*chanp = i;
		}

		/*
		** If this is the first open for this adapter
		** allocate and initialize the necessary data
		** structures and make sure the driver is pinned.
		*/
		if (ca->num_opens == 0) {
			if (cat_init_dev(ca) || cat_check_status(ca)) {
				openp->op_flags &= ~(OP_OPENED | OP_CE_OPEN);
				unlockl(&ca->adap_lock);
				DDHKWD4(HKWD_DD_CATDD, DD_EXIT_MPX, EIO, dev,
					*channame, 0, 0);
				return EIO;
			}
		}
		ca->num_opens++;

		unlockl(&ca->adap_lock);
	}
	DDHKWD4(HKWD_DD_CATDD, DD_EXIT_MPX, 0, dev, *channame, *chanp, 0);
	return 0;
} /* cat_mpx() */
