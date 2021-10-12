/* @(#)26	1.1  src/bos/usr/ccs/lib/libc_r/rec_mutex.h, libcthrd, bos411, 9428A410j 10/20/93 14:47:31 */
/*
 *   COMPONENT_NAME: LIBCTHRD
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */

/*
 * rec_mutex.h
 *
 * Mutex locks which allow recursive calls by the same thread.
 */

#ifndef _REC_MUTEX_H
#define _REC_MUTEX_H 1

/*
 * Recursive mutex definition.
 */

#include <lib_lock.h>

struct rec_mutex {
	lib_threadid_t	thread_id;	/* id of thread holding the lock */
	int		count;		/* Number of outstanding locks */
	lib_mutex_t	mutex;		/* The mutex itself */
};

typedef  struct rec_mutex	*rec_mutex_t;

/*
 * Recursive mutex operations.
 */

extern int		_rec_mutex_alloc(rec_mutex_t *);
extern int		_rec_mutex_init(rec_mutex_t);
extern void		_rec_mutex_free(rec_mutex_t);
extern void		_rec_mutex_lock(rec_mutex_t);
extern void		_rec_mutex_unlock(rec_mutex_t);
extern int		_rec_mutex_trylock(rec_mutex_t);

#endif /* _REC_MUTEX_H */
