/* @(#)27	1.2  src/bos/usr/ccs/lib/libc_r/stdio_lock.h, libcthrd, bos411, 9428A410j 2/4/94 12:44:58 */
/*
 *   COMPONENT_NAME: LIBCTHRD
 *
 *   FUNCTIONS: _flockfile
 *		_ftestfile
 *		_funlockfile
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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */

/*
 * Thread safe locking extentions for stdio.h
 */

#ifndef	_STDIO_LOCK_H
#define	_STDIO_LOCK_H

#include "rec_mutex.h"

typedef	rec_mutex_t	filelock_t;	/* pointer to rec_mutex struct */

/*********
  NOTE:
	These macros return the opposite of the lock, unlock and
	trylock routines.  BE CAREFULL
**********/

#define	_funlockfile(filelock) \
		(((filelock) == (rec_mutex_t) NULL) ? (void)0 : \
					_rec_mutex_unlock(filelock))

#define	_flockfile(iop)	\
		(filelock_t)(((iop)->_lock == (rec_mutex_t) NULL) ? NULL : \
			(_rec_mutex_lock((iop)->_lock), (iop)->_lock))

#define	_ftestfile(iop)	\
	(filelock_t)(((iop)->_lock == (rec_mutex_t) NULL) ? NULL : \
		((_rec_mutex_trylock((iop)->_lock) == 0) ? (iop)->_lock : \
		NULL))

#endif /* _STDIO_LOCK_H */
