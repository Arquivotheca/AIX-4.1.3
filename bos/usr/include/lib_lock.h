/* @(#)01	1.2  src/bos/usr/include/lib_lock.h, libcthrd, bos411, 9433B411a 8/8/94 17:42:02 */
/*
 *   COMPONENT_NAME: LIBCTHRD
 *
 *   FUNCTIONS: LIB_LOCK_FUNCTION
 *		LIB_LOCK_FUNCTION_TRYLOCK
 *		lib_mutex_create
 *		lib_mutex_delete
 *		lib_mutex_lock
 *		lib_mutex_trylock
 *		lib_mutex_unlock
 *		lib_spinlock_create
 *		lib_spinlock_delete
 *		lib_spinlock_lock
 *		lib_spinlock_trylock
 *		lib_spinlock_unlock
 *		lib_thread_id
 *
 *   ORIGINS: 27,71
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/* lib_lock.h,v $ $Revision: 1.4 $ (OSF) */

/*
 * Library locking functions provided by a threads package usable by a library
 */

#ifndef _LIB_LOCK_H_
#define _LIB_LOCK_H_

typedef	void	*lib_mutex_t;
typedef	void	*lib_spinlock_t;
typedef void	*lib_threadid_t;

typedef	int		(*lib_mutex_func_t)(lib_mutex_t *);
typedef	int		(*lib_spinlock_func_t)(lib_spinlock_t *);
typedef	lib_threadid_t	(*lib_threadid_func_t)(void);
typedef void		(*lib_cleanup_push_t)(void (*)(), void *);	
typedef void		(*lib_cleanup_pop_t)(int);	

typedef struct lib_lock_functions {
	lib_mutex_func_t	mutex_create;
	lib_mutex_func_t	mutex_delete;
	lib_mutex_func_t	mutex_lock;
	lib_mutex_func_t	mutex_unlock;
	lib_mutex_func_t	mutex_trylock;
	lib_spinlock_func_t	spinlock_create;
	lib_spinlock_func_t	spinlock_delete;
	lib_spinlock_func_t	spinlock_lock;
	lib_spinlock_func_t	spinlock_unlock;
	lib_spinlock_func_t	spinlock_trylock;
	lib_threadid_func_t	thread_id;
	lib_cleanup_push_t	cleanup_push;
	lib_cleanup_pop_t	cleanup_pop;
} lib_lock_functions_t;

#ifndef ESUCCESS
#define ESUCCESS	0
#endif

#ifndef NO_THREAD
#define NO_THREAD	(lib_threadid_t)0
#endif

#define	LIB_LOCK_FUNCTION(lockstruct, operation, arg) \
	((lockstruct).operation ? (*(lockstruct).operation)(arg) : ESUCCESS)

/**********
special case for trylock. if the operations does 
not exist, then return 1
**********/
#define	LIB_LOCK_FUNCTION_TRYLOCK(lockstruct, operation, arg) \
	((lockstruct).operation ? (*(lockstruct).operation)(arg) : 1)


#define lib_mutex_create(lockstruct, lock) \
		LIB_LOCK_FUNCTION(lockstruct, mutex_create, lock)

#define lib_mutex_delete(lockstruct, lock) \
		LIB_LOCK_FUNCTION(lockstruct, mutex_delete, lock)

#define lib_mutex_lock(lockstruct, lock) \
		LIB_LOCK_FUNCTION(lockstruct, mutex_lock, lock)

#define lib_mutex_unlock(lockstruct, lock) \
		LIB_LOCK_FUNCTION(lockstruct, mutex_unlock, lock)

#define lib_mutex_trylock(lockstruct, lock) \
		LIB_LOCK_FUNCTION_TRYLOCK(lockstruct, mutex_trylock, lock)

#define lib_spinlock_create(lockstruct, lock) \
		LIB_LOCK_FUNCTION(lockstruct, spinlock_create, lock)

#define lib_spinlock_delete(lockstruct, lock) \
		LIB_LOCK_FUNCTION(lockstruct, spinlock_delete, lock)

#define lib_spinlock_lock(lockstruct, lock) \
		LIB_LOCK_FUNCTION(lockstruct, spinlock_lock, lock)

#define lib_spinlock_unlock(lockstruct, lock) \
		LIB_LOCK_FUNCTION(lockstruct, spinlock_unlock, lock)

#define lib_spinlock_trylock(lockstruct, lock) \
		LIB_LOCK_FUNCTION(lockstruct, spinlock_trylock, lock)

#define lib_thread_id(lockstruct) \
	((lockstruct).thread_id ? (*(lockstruct).thread_id)() : 0)

#define lib_cleanup_push(lockstruct, _function, lock) \
	if ((lockstruct).cleanup_push) { (*(lockstruct).cleanup_push)(_function, lock); }

#define lib_cleanup_pop(lockstruct, _flag) \
	if ((lockstruct).cleanup_pop) { (*(lockstruct).cleanup_pop)(_flag); }

#endif /* _LIB_LOCK_H_ */
