/* @(#)03	1.2  src/bos/usr/ccs/lib/libc/ts_supp.h, libcthrd, bos411, 9428A410j 2/4/94 12:38:22 */
/*
 *   COMPONENT_NAME: LIBC
 *
 *   FUNCTIONS: TS_EINVAL
 *		TS_ERROR
 *		TS_FDECLARELOCK
 *		TS_FLOCK
 *		TS_FOUND
 *		TS_FTRYLOCK
 *		TS_FUNLOCK
 *		TS_LOCK
 *		TS_RETURN
 *		TS_TRYLOCK
 *		TS_UNLOCK
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

/* ts_supp.h,v $ $Revision: 1.2.4.2 $ (OSF) */

/*
 * Macros for thread safe work.
 *
 * The idea is to make the shared library code easier to read
 * and maintain by avoiding the loathsome #ifdef.
 * Sometimes this works.
 */

#ifndef _TS_SUPP_H_
#define	_TS_SUPP_H_

#if defined(_THREAD_SAFE) || defined(_REENTRANT)

#include	<errno.h>

/**********
  TS_LOCK	 no return value
  TS_UNLOCK	 no return value
  TS_TRYLOCK	 returns non-zero if the lock was taken,
	    	 0 if the lock failed
**********/
#define	TS_LOCK(lock)		_rec_mutex_lock(lock)
#define	TS_TRYLOCK(lock)	(!_rec_mutex_trylock(lock))
#define	TS_UNLOCK(lock)		_rec_mutex_unlock(lock)


/**********
  TS_FLOCK	no return
  TS_FUNLOCK	no return
  TS_FTRYLOCK	returns non-zero if lock was taken,
		0 if the lock failed.
**********/
#define	TS_FDECLARELOCK(lock)	filelock_t lock;
#define	TS_FLOCK(lock, iop)	(lock = _flockfile(iop))
#define	TS_FTRYLOCK(lock, iop)	(lock = _ftestfile(iop))
#define	TS_FUNLOCK(lock)	_funlockfile(lock)

#define	TS_EINVAL(arg)		if (arg) return (errno = EINVAL, -1)
#define	TS_ERROR(e)		errno = (e)
#define	TS_RETURN(ts, nts)	return (ts)

#define	TS_SUCCESS		0
#define	TS_FAILURE		-1
#define	TS_FOUND(ret)		(TS_SUCCESS)
#define	TS_NOTFOUND		(errno = ESRCH, TS_FAILURE)

#else

#define	TS_LOCK(lock)
#define	TS_TRYLOCK(lock)	1
#define	TS_UNLOCK(lock)

#define	TS_FDECLARELOCK(lock)
#define	TS_FLOCK(lock, iop)
#define	TS_FTRYLOCK(lock, iop)	1
#define	TS_FUNLOCK(lock)

#define	TS_EINVAL(arg)
#define	TS_ERROR(e)
#define	TS_RETURN(ts, nts)	return (nts)

#define	TS_SUCCESS		1
#define	TS_FAILURE		0
#define	TS_FOUND(ret)		(ret)
#define	TS_NOTFOUND		(TS_FAILURE)

#endif	/* _THREAD_SAFE || _REENTRANT */

#endif	/* _TS_SUPP_H_ */
