static char sccsid[] = "@(#)92	1.1  src/bos/kernext/fddidiag/fddisel_t.c, diagddfddi, bos411, 9428A410j 11/1/93 11:01:02";
/*
 *   COMPONENT_NAME: DIAGDDFDDI
 *
 *   FUNCTIONS: fddi_select
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "fddiproto.h"
#include <sys/mbuf.h>
#include <sys/errno.h>
/* 
 * get access to the global device driver control block
 */
extern fddi_ctl_t	fddi_ctl;

/*
 * NAME: fddi_select
 *                                                                    
 * FUNCTION: determines if a specified event has occurred on the FDDI
 *                                                                    
 * EXECUTION ENVIRONMENT: USER mode process environment only
 *                                                                   
 * NOTES: 
 *
 *	Results of the condition checks are passed back to the caller
 *	by reference through the 'reventp' parameter. A bitwise OR
 *	of the following conditions are returned:
 *
 *		POLLIN	 Receive data is available
 *		POLLOUT	 Transmit available
 *		POLLPRI	 Status is available
 *		POLLSYNC Do not give asynchronous notification for
 *			 the requested events - synchronous only
 *
 *	When POLLSYNC is not requested and all the events requested
 *	are false then store the events requested for later notification.
 *
 * RECOVERY OPERATION: 
 *
 *	If we are in an unrecoverable state return 1's so the caller
 *	doesn't sleep forever on an event that will not happen.
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *
 *	0	if successful
 *	ENXIO	Indicates an invalid minor number or channel was specified.
 *	EACCES	Indicates an invalid call from a kernel process.
 *
 */  

int
fddi_select(
	dev_t	devno,		/* major/minor number */
	ushort	events,		/* specifies events to be checked */
	ushort	*p_revents,	/* return by reference the results */
	chan_t	chan)		/* channel number */
{
	fddi_acs_t	*p_acs;
	fddi_open_t	*p_open;
	int		adap;
	int		rc;

	FDDI_DBTRACE("selB", events, chan, 0);
	/* sanity check the minor number */
	if ( ((adap = minor(devno)) < 0) || (adap >= FDDI_MAX_MINOR) )
		return(ENODEV);

	/* grab global device lock */
	rc = lockl(&fddi_ctl.fddilock, LOCK_SIGRET);

	if( rc != LOCK_SUCC )
	{
		return(EINTR);
	}
	if ( (p_acs = fddi_ctl.p_acs[adap]) == NULL )
	{
		/* unlockl global driver lock */
		unlockl(&fddi_ctl.fddilock);
		return(ENODEV);
	}
	FDDI_DBTRACE("sel1",p_acs->dev.state, 0, 0);

	if ((p_open = fddi_ctl.p_open_tab [chan]) == NULL)
	{
		/* unlockl global driver lock */
		unlockl(&fddi_ctl.fddilock);
		return(EINVAL);
	}

	if (p_open->devflag & DKERNEL)
	{
		/* unlockl global driver lock */
		unlockl(&fddi_ctl.fddilock);
		return (EACCES);
	}
	/*
	 * Call non-pageable routine to check events
	 *	returns the values to be passed back to caller
	 *	by reference
	 */
	*p_revents = fddi_chk_events (p_acs, p_open, events);

	/* unlockl global driver lock */
	unlockl(&fddi_ctl.fddilock);

	FDDI_DBTRACE("selE", *p_revents, 0, 0);
	return (0);
}
