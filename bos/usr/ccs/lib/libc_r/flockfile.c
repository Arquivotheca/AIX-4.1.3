static char sccsid[] = "@(#)19	1.2  src/bos/usr/ccs/lib/libc_r/flockfile.c, libcthrd, bos411, 9428A410j 2/4/94 12:42:25";
/*
 *   COMPONENT_NAME: LIBCTHRD
 *
 *   FUNCTIONS: flockfile
 *		ftrylockfile
 *		funlockfile
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */

/* flockfile.c,v $ $Revision: 1.6 */

/*
 * This file contains the POSIX (P1003.4a) functions for locking and unlocking
 * of stdio file descriptors.
 */

#include <stdio.h>
#include "stdio_lock.h"

#undef flockfile
#undef funlockfile
#undef ftrylockfile

/*
 * If lock is available, take it.  Otherwise, wait (blocked) until it becomes
 * available.
 */
void
flockfile(FILE *iop)
{
	if (iop->_lock != (void *) NULL) {
		_rec_mutex_lock(iop->_lock);
	}
}

void
funlockfile(FILE *iop)
{
	if (iop->_lock != (void *) NULL) {
		_rec_mutex_unlock(iop->_lock);
	}
}

/*
 * Calls:  _rec_mutex_trylock() which returns non-zero on success and 0 on 
 *	   failure.
 *	
 * Return:  0 		If we succeed in getting the lock.
 * 	    non-zero	If it is already locked or if iop->_lock is NULL.
 */
int
ftrylockfile(FILE *iop)
{
	if (iop->_lock != (void *) NULL) {
		return (_rec_mutex_trylock(iop->_lock));
	}
	return (-1);	/* iop->_lock is NULL, so fail to get the lock */
}
