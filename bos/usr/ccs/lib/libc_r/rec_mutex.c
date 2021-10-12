static char sccsid[] = "@(#)25	1.4  src/bos/usr/ccs/lib/libc_r/rec_mutex.c, libcthrd, bos411, 9428A410j 5/20/94 12:19:23";
/*
 *   COMPONENT_NAME: LIBCTHRD
 *
 *   FUNCTIONS: _libc_declare_lock_functions
 *		_rec_mutex_alloc
 *		_rec_mutex_free
 *		_rec_mutex_init
 *		_rec_mutex_lock
 *		_rec_mutex_trylock
 *		_rec_mutex_unlock
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

#include <lib_lock.h>
#include "rec_mutex.h"

#define	NULL	0

lib_lock_functions_t	_libc_lock_funcs;


void
_libc_declare_lock_functions(lib_lock_functions_t *f)
{
	_libc_lock_funcs = *f;
	_libc_locks_init();
	/*
	 * Import the errno routines here so that the libc_r ones
	 * will be used regardless.
	 */
	(void)_Seterrno(0);
}


int
_rec_mutex_alloc(rec_mutex_t *m)
{
	extern char *malloc();

	*m = (rec_mutex_t) malloc(sizeof(struct rec_mutex));
	if (*m == NULL)
		return(-1);
	if (_rec_mutex_init(*m)) {
		free((char *)*m);
		return(-1);
	}
	return(0);
}


int
_rec_mutex_init(rec_mutex_t m)
{
	m->thread_id = NO_THREAD;
	m->count = 0;
/*
	return (lib_spinlock_create(_libc_lock_funcs, &m->mutex));
*/
	/**********
	  the spinlock function is a void, so always succeed
	**********/
	lib_spinlock_create(_libc_lock_funcs, &m->mutex);
	return(0);
}


void
_rec_mutex_free(rec_mutex_t m)
{
	lib_spinlock_delete(_libc_lock_funcs, &m->mutex);
	free((char *) m);
}


/********
	returns:
	     0 on success
	    -1 on failure
**********/
int
_rec_mutex_trylock(register rec_mutex_t m)
{
	register lib_threadid_t self;

	self = lib_thread_id(_libc_lock_funcs);

	if (m->thread_id == self) {	/* If already holding lock */
		m->count += 1;
		return(0);
	}
	if (lib_spinlock_trylock(_libc_lock_funcs, &m->mutex) == 0) {
		/* We got the lock */
		m->count = 1;
		m->thread_id = self;
		return(0);
	}
	return(-1);
}


void
_rec_mutex_lock(register rec_mutex_t m)
{
	register lib_threadid_t self;

	self = lib_thread_id(_libc_lock_funcs);

	if (m->thread_id == self) {	/* If already holding lock */
		m->count += 1;
	} else {
		lib_spinlock_lock(_libc_lock_funcs, &m->mutex);
		m->count = 1;
		m->thread_id = self;
	}
}


void
_rec_mutex_unlock(register rec_mutex_t m)
{
	register lib_threadid_t self;

	self = lib_thread_id(_libc_lock_funcs);

	/* Must be holding lock to unlock! */
	if (m->thread_id == self) {
		if (--(m->count) == 0) {
			m->thread_id = NO_THREAD;
			lib_spinlock_unlock(_libc_lock_funcs, &m->mutex);
		}
	}
}
