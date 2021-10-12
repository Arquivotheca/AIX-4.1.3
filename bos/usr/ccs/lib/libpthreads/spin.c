static char sccsid[] = "@(#)20	1.9  src/bos/usr/ccs/lib/libpthreads/spin.c, libpth, bos41J, 9513A_all 3/23/95 02:00:21";
/*
 * COMPONENT_NAME: libpth
 * 
 * FUNCTIONS:
 *	_spin_lock
 *	_spinlock_create
 *	_spinlock_delete
 *	_spin_unlock
 *	_spin_trylock
 * 
 * ORIGINS:  71, 83
 * 
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.2
 */

/*
 * File: spin.c
 *
 * implementation of spin locks. We rely on two machine dependent functions,
 * _check_lock and _clear_lock. _spin_trylock() and _spin_unlock are #defined
 * to be these functions but we implement _spin_lock here. The aim is to be a 
 * little sociable here. We fast spin for a while (spin_limit times) and
 * then yield between each test of the lock after that to try and make sure
 * we don't hog the cpu completely.
 */

#include "internal.h"

/*
 * Function:
 *	_spin_lock
 *
 * Parameters:
 *	lock - a pointer to the lock word
 *
 * Description:
 *	This function is needed by lib_lock.c to pass to other libraries
 *	using threads spin locking.
 *	Fast spin for a while during the yield time and then yield spin 
 *	if the lock isn't set during one tick time and another sleep loop
 *	of one tick until we get the lock.  
 *	YIELDLOOPTIME UP =0 ; MP =40 upate in pthread_init()
 *	TICKLOOPTIME	600
 */
#undef _spin_lock
void
_spin_lock(spinlock_t *lock)
{
	register int	i;

	while (!_get_lock((atomic_p)lock)) {

		for (i = 1; i < YIELDLOOPTIME; i++) {
			TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_MISS,
				lock, *lock, LOCK_SWRITE_TRACE, CALLER, NULL);
			/* do nothing */
			if (_get_lock((atomic_p)lock))
				goto done;
		}

#ifdef _YIELD
		
		for (i = 1; i < TICKLOOPTIME; i++) {
			TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_MISS,
				lock, *lock, LOCK_SWRITE_TRACE, CALLER, NULL);
			yield();
			if (_get_lock((atomic_p)lock))
				goto done;
		}
#endif

		TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_MISS,
				lock, *lock, LOCK_SWRITE_TRACE, CALLER, NULL);
		switch(thread_waitlock((atomic_p)lock)) { /*kernel trace hook*/
		case -1:
			INTERNAL_ERROR("spin_lock");
			/* NOTREACHED */
		case 0:
			goto done;
		case 1:
			continue;
		}

	}

done:
	TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,
				lock, *lock, LOCK_SWRITE_TRACE, CALLER, NULL);
	return;
}


/*
 * Function:
 *	_spinlock_create
 *
 * Parameters:
 *	lock - pointer to the new lock
 *
 * Description:
 *	This function is needed by lib_lock.c to pass to other libraries
 *	using threads spin locking.
 */
#undef _spinlock_create
void
_spinlock_create(spinlock_t *lock)
{
	_clear_lock((atomic_p)lock, UL_FREE);
}


/*
 * Function:
 *	_spinlock_delete
 *
 * Parameters:
 *	lock - pointer to the lock to be deleted
 *
 * Description:
 *	This function is needed by lib_lock.c to pass to other libraries
 *	using threads spin locking.
 */
#undef _spinlock_delete
void
_spinlock_delete(spinlock_t *lock)
{
	_clear_lock((atomic_p)lock, UL_FREE);
}


/*
 * Function:
 *	_spin_unlock
 *
 * Parameters:
 *	lock - a pointer to the lock word
 *
 * Description:
 *	This function is normally a macro to reduce one extra function call
 *	overhead. It is needed as a function for lib_lock as the address has
 *	to be taken to put in the table of locking functions.
 */
#undef __spin_unlock
#define __spin_unlock(lock)						\
{									\
	int dummy;							\
									\
	if (_put_lock((atomic_p)lock))					\
		return;							\
									\
	switch(thread_unlock((atomic_p)lock)) { /*kernel trace hook*/	\
	case -1:							\
		INTERNAL_ERROR("spin_unlock");				\
		/* NOTREACHED */					\
	case 0:								\
	case 1:								\
		return;							\
	}								\
}

#undef _spin_unlock
void
_spin_unlock(spinlock_t *lock)
{
	TRCHKGT(HKWD_KERN_UNLOCK, lock, *lock, CALLER, NULL, 0);
	__spin_unlock(lock);
}

#ifdef _TRACE
void
_spin_unlock_trace(spinlock_t *lock)
{
	__spin_unlock(lock);
}
#endif


/*
 * Function:
 *	_spin_trylock
 *
 * Parameters:
 *	lock - a pointer to the lock word
 *
 * Return value:
 *	0	The lock was taken
 *	1	The lock was already locked
 *
 * Description:
 *	It is needed as a function for lib_lock as the address has
 *	to be taken to put in the table of locking functions.
 */
int
_spin_trylock(spinlock_t *lock)
{
	return(_get_lock((atomic_p)lock) ? 0 : 1);
}
