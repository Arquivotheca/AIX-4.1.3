static char sccsid[] = "@(#)08	1.4  src/bos/usr/ccs/lib/libc/msem_lock.c, libcmsem, bos411, 9428A410j 5/23/94 11:43:34";
/*
 * COMPONENT_NAME: (LIBCMSEM) Standard C Library Msemaphore Functions
 *
 * FUNCTIONS: msem_lock
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
msem_lock(msemaphore *msem, int condition)
{
	volatile msemaphore *sem=msem;
	/*
	 * Check to see if the semaphore has been removed or if the
	 * condition parameter is bad.
	 */
	if ((sem->msem_wanted == -1) ||
	    (!((condition == MSEM_IF_NOWAIT) || (condition == 0)))) {
		errno = EINVAL;
		return(-1);
	}

	/*
	 * Atomically test and set msem_state
	 * to locked state when in unlocked state
	 */
	while (_check_lock((atomic_p)&sem->msem_state,
			MSEM_UNLOCKED,MSEM_LOCKED)) {

		if (condition == MSEM_IF_NOWAIT) {
			errno = EAGAIN;
			return(-1);
		}

		if (msleep(sem) != 0) {
			return(-1);
		}

		/*
		 * we have to check to see if the semaphore is still valid
		 * as someone may have removed it while we were asleep
		 */
		if (sem->msem_wanted == -1) {
			errno = EINVAL;
			return(-1);
		}
		
	}
	
	return(0);
}
