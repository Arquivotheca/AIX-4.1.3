static char sccsid[] = "@(#)11	1.4  src/bos/usr/ccs/lib/libc/msem_unlock.c, libcmsem, bos411, 9428A410j 5/23/94 11:46:53";
/*
 * COMPONENT_NAME: (LIBCMSEM) Standard C Library Msemaphore Functions
 *
 * FUNCTIONS: msem_unlock
 *
 * ORIGINS: 65 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#include <sys/types.h>
#include <sys/atomic_op.h>
#include <errno.h>
#include <sys/mman.h>

int
msem_unlock(msemaphore *msem, int condition)
{
	volatile msemaphore *sem=msem;
	
	if ((sem->msem_wanted == -1) ||
	    (!((condition == MSEM_IF_WAITERS) || (condition ==0)))) {
		errno = EINVAL;
		return(-1);
	}

	if ((condition == MSEM_IF_WAITERS) && (sem->msem_wanted == 0)) {
		errno = EAGAIN;
		return(-1);
	}

	/* Clear the lock. */
	_clear_lock((atomic_p) &sem->msem_state, MSEM_UNLOCKED);
	
	if (sem->msem_wanted != 0) {	/* See if anyone is waiting. */
		return(mwakeup(sem));
	}
		
	return(0);
}
