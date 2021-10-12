static char sccsid[] = "@(#)07	1.4  src/bos/usr/ccs/lib/libc/msem_init.c, libcmsem, bos411, 9428A410j 5/23/94 11:39:44";
/*
 * COMPONENT_NAME: (LIBCMSEM) Standard C Library Msemaphore Functions
 *
 * FUNCTIONS: msem_init
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
#include <sys/mman.h>
#include <sys/atomic_op.h>
#include <errno.h>

msemaphore *
msem_init(msemaphore *msem, int initial_value)
{
	volatile msemaphore *sem=msem;

	switch (initial_value) {
	case	MSEM_LOCKED:
		_clear_lock((atomic_p) &sem->msem_state, (int) sem);
		break;
	case	MSEM_UNLOCKED:
		_clear_lock((atomic_p) &sem->msem_state,0);
		break;
	default:
		errno = EINVAL;
		return(NULL);
	}
	sem->msem_wanted = 0;

	return(sem);
}
