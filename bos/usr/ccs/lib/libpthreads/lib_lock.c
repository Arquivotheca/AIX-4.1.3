static char sccsid[] = "@(#)10	1.7  src/bos/usr/ccs/lib/libpthreads/lib_lock.c, libpth, bos41J, 9515A_all 4/12/95 10:42:34";
/*
 * COMPONENT_NAME: libpth
 * 
 * FUNCTIONS:
 *	__pthread_libmutex_create
 *	__pthread_libmutex_delete
 *	__pthread_libmutex_lock
 *	__pthread_libmutex_unlock
 *	__pthread_libmutex_trylock
 *	__pthread_libdata_hdl
 *	__pthread_libdata_ref
 *	_pthread_libs_init
 *	_lib_declare_lock_functions
 *	_lib_declare_data_functions
 * 
 * ORIGINS:  71  83
 * 
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.2
 */

/*
 * File: lib_lock.c
 *
 * This file contains the locking and data functions for the libraries
 * and the loader.
 */

#include "internal.h"
#include <lib_lock.h>
#include <lib_data.h>
extern void *_errno_hdl;

/*
 * Local Variables
 */

private lib_lock_functions_t	lock_funcs;
private lib_data_functions_t	data_funcs;


/*
 * Function:
 *	__pthread_libmutex_create
 *
 * Parameters:
 *	mutex - a pointer to a pthread_mutex_t *
 *
 * Return value:
 *	0	success, the mutex id is stored in *mutex
 *	EINVAL	otherwise. set as in pthread_mutex_init
 *  	ENOMEM  Insufficient memory
 *
 * Description:
 *	Pthread mutexes are structures and not malloc'd. library mutexes
 *	are pointers as they have no idea of the size and cannot find out
 *	in a threads provider independent way. So we have to malloc the
 *	mutex and initialize it.
 */
private int
__pthread_libmutex_create(pthread_mutex_t **mutex)
{
	pthread_mutex_t	*m;
	int		ret;

	/* Allocate space for the structure.
	 */
	if (!(m = (pthread_mutex_t *)_pmalloc(sizeof(pthread_mutex_t))))
		return (ENOMEM);

	/* Initialize the mutex.
	 */
	if ((ret = pthread_mutex_init(m, &pthread_mutexattr_default)))
		_pfree(m);
	else
		*mutex = m;
	return (ret);
}


/*
 * Function:
 *	__pthread_libmutex_delete
 *
 * Parameters:
 *	mutex - a pointer to the mutex to delete
 *
 * Return value:
 *	0	success, and the mutex id is null'd
 *	EINVAL	The pointer is NULL or the mutex is NULL
 *		whatever pthread_mutex_destroy returns
 *
 * Description:
 *
 */
private int
__pthread_libmutex_delete(pthread_mutex_t **mutex)
{
	pthread_mutex_t	*m;
	int		ret;

	/* Check the pointer is ok to dereference.
	 */
	if (mutex == NULL) {
		return (EINVAL);
	}

	/* Find the pthread mutex and check that it is OK.
	 */
	m = *mutex;
	if (m == NO_MUTEX) {
		return (EINVAL);
	}

	/* Destroy the pthread mutex and free its memory.
	 */
	if ((ret = pthread_mutex_destroy(m)) == 0) {
		*mutex = NO_MUTEX;
		_pfree(m);
	}
	return (ret);
}


/*
 * Function:
 *	__pthread_libmutex_lock
 *
 * Parameters:
 *	mutex - pointer to the mutex to be locked
 *
 * Return value:
 *	Same as pthread_mutex_lock
 *
 * Description:
 *	This is needed to dereference the mutex passed for the pthread
 *	interface.
 */
private int
__pthread_libmutex_lock(pthread_mutex_t **mutex)
{
	return (pthread_mutex_lock(*mutex));
}


/*
 * Function:
 *	__pthread_libmutex_unlock
 *
 * Parameters:
 *	mutex - pointer to the mutex to be unlocked
 *
 * Return value:
 *	Same as pthread_mutex_unlock
 *
 * Description:
 *	This is needed to dereference the mutex passed for the pthread
 *	interface.
 */
private int
__pthread_libmutex_unlock(pthread_mutex_t **mutex)
{
	return (pthread_mutex_unlock(*mutex));
}


/*
 * Function:
 *	__pthread_libmutex_trylock
 *
 * Parameters:
 *	mutex - pointer to the mutex to be tested
 *
 * Return value:
 *	Same as pthread_mutex_trylock
 *
 * Description:
 *	This is needed to dereference the mutex passed for the pthread
 *	interface.
 */
private int
__pthread_libmutex_trylock(pthread_mutex_t **mutex)
{
	return (pthread_mutex_trylock(*mutex));
}


/*
 * Function:
 *	__pthread_libdata_hdl
 *
 * Parameters:
 *	hdl	- new handle
 *
 * Return value:
 *	0 Success
 *	-1 As per pthread_key_create()
 *
 * Description:
 *	Get a key for a thread data item.
 */
private int
__pthread_libdata_hdl(void **hdl)
{
	return (__key_create_internal((pthread_key_t *)hdl, NILFUNC(void)));
}


/*
 * Function:
 *	__pthread_libdata_ref
 *
 * Parameters:
 *	hdl	- handle to use
 *
 * Return value:
 *	NULL key is invalid
 *	not-NULL address of data item
 *
 * Description:
 *	Retrieve address of thread data associated with handle.
 */
private void *
__pthread_libdata_ref(void *hdl)
{
	void	*addr;

	if (_pthread_getspecific_addr((pthread_key_t)hdl, &addr))
		return (NULL);
	return (addr);
}


/*
 * Function:
 *	pthread_libs_init
 *
 * Description:
 *	Called by pthread_init, initialize the library locking interfaces.
 *	the structure is constructed with pointers to all the functions
 *	and the two functions (to the reentrant C library and the loader)
 *	are called to initialize them with the locking entry points.
 */
void
_pthread_libs_init(pthread_d main)
{
	void	_libc_declare_lock_functions();
	void	_libc_declare_data_functions();

	/* Initialize the mutex functions.
	 */
	lock_funcs.mutex_create = (lib_mutex_func_t)__pthread_libmutex_create;
	lock_funcs.mutex_delete = (lib_mutex_func_t)__pthread_libmutex_delete;
	lock_funcs.mutex_lock = (lib_mutex_func_t)__pthread_libmutex_lock;
	lock_funcs.mutex_unlock = (lib_mutex_func_t)__pthread_libmutex_unlock;
	lock_funcs.mutex_trylock = (lib_mutex_func_t)__pthread_libmutex_trylock;

	/* Initialize the spinlock functions.
	 */
#undef _spinlock_create
	lock_funcs.spinlock_create = (lib_spinlock_func_t)_spinlock_create;
#undef _spinlock_delete
	lock_funcs.spinlock_delete = (lib_spinlock_func_t)_spinlock_delete;
#undef _spin_lock
	lock_funcs.spinlock_lock = (lib_spinlock_func_t)_spin_lock;
#undef _spin_unlock
	lock_funcs.spinlock_unlock = (lib_spinlock_func_t)_spin_unlock;
	lock_funcs.spinlock_trylock = (lib_spinlock_func_t)_spin_trylock;

	/* Initialize the thread id function.
	 */
	lock_funcs.thread_id = (lib_threadid_func_t)pthread_self;

	/* Initialize the cleanup_push and cleanup_pop functions.
	 */
	lock_funcs.cleanup_push = (lib_cleanup_push_t)pthread_cleanup_push;
	lock_funcs.cleanup_pop = (lib_cleanup_pop_t)pthread_cleanup_pop;

	/* Initialise the data functions.
	 */
	data_funcs.data_hdl = __pthread_libdata_hdl;
	data_funcs.data_ref = __pthread_libdata_ref;

	/* Set up the locks and data for libc_r.
	 */
	_libc_declare_lock_functions(&lock_funcs);
	_libc_declare_data_functions(&data_funcs);

}


/*
 * Function:
 *	_lib_declare_lock_functions
 *
 * Parameters:
 *	funcs - addr of lock functions
 *
 * Description:
 *	Copy the pthread exported library locking interfaces to a
 *	lib_lock_functions_t provided by the caller.
 */
void
_lib_declare_lock_functions(lib_lock_functions_t *funcs)
{
	/* assign lock functions */
	*funcs = lock_funcs;
}


/*
 * Function:
 *	_lib_declare_data_functions
 *
 * Parameters:
 *	funcs - addr of data functions
 *
 * Description:
 *	Copy the pthread exported library data interfaces to a
 *	lib_data_functions_t provided by the caller.
 */
void
_lib_declare_data_functions(lib_data_functions_t *funcs)
{
	/* assign data functions */
	*funcs = data_funcs;
}


