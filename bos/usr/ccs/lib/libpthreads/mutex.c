static char sccsid[] = "@(#)13	1.20  src/bos/usr/ccs/lib/libpthreads/mutex.c, libpth, bos41J, 9520B_all 5/18/95 08:25:00";
/*
 * COMPONENT_NAME: libpth
 * 
 * FUNCTIONS:
 *	_pthread_mutexattr_startup
 *	pthread_mutexattr_init
 *	pthread_mutexattr_destroy
 *	_pthread_mutex_startup
 *	_initialize_mutex
 *	pthread_mutex_init
 *	pthread_mutex_destroy
 *	pthread_mutex_trylock
 *	pthread_mutex_lock
 *	pthread_mutex_unlock
 *	pthread_mutexattr_setprotocol
 *	pthread_mutexattr_getprotocol
 *	pthread_mutexattr_setprioceiling
 *	pthread_mutexattr_getprioceiling
 *	pthread_mutex_setprioceiling
 *	pthread_mutex_getprioceiling
 *	pthread_mutexattr_getpshared
 *	pthread_mutexattr_setpshared
 *	pthread_mutexattr_getkind_np
 *	pthread_mutexattr_setkind_np
 *	pthread_set_mutexattr_default_np
 *	pthread_lock_global_np
 *	pthread_unlock_global_np
 *	dump_mutex
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
 * File: mutex.c
 *
 * Support for mutexes and their attributes. There are no attributes defined
 * for mutexes currently. Mutexes are implemented using spin locks. The lock
 * is spun on for max_spin_limit attempts and then the thread yields between
 * each subsequent attempt at trying to get the lock.
 */

#include "internal.h"


/*
 * Global Variables
 */
pthread_mutexattr_t	pthread_mutexattr_default;
pthread_mutexattr_t	pthread_mutexattr_fast;
pthread_mutexattr_t	mutexattr_global_np;
#define ATTR_MUTEX	1
int			mtx_id = 0;
spinlock_t		mtx_id_lock;

pthread_queue		__dbx_known_pthreads; /* here workaround compiler bug 
					       * with -O
					      */
extern pthread_queue	__dbx_known_attributes;
extern spinlock_t	dbx__attributes;
__dbx_mutex		__dbx_known_mutexes;

spinlock_t		dbx__mutexes;
extern	int		attr_id;
extern	spinlock_t	attr_id_lock;
int			_mutex_kind_default = MUTEX_NONRECURSIVE_NP;


/*
 * Function:
 *	_pthread_mutexattr_startup
 *
 * Description:
 *	Initialize mutex attributes. This function creates the default
 *	attribute structure.
 */
private void
_pthread_mutexattr_startup(void)
{
	pthread_mutexattr_init(&pthread_mutexattr_default);
	pthread_mutexattr_init(&pthread_mutexattr_fast);
		/* attributes to initialize (pthread_mutexattr_setkind_np) */
	pthread_mutexattr_fast->mutex_kind = MUTEX_FAST_NP;

}


/*
 * Function:
 *	pthread_mutexattr_init
 *
 * Parameters:
 *	attr - pointer to the newly created attribute structure
 *
 * Return value:
 *	0	Success
 *	EINVAL	if the pointer passed is and invalid pointer
 *      ENOMEM  Insufficient memory exists to initialize the mutex attributes
 *              object.
 *
 * Description:
 *	The pthread_mutexattr_t is the attribute structure and this function
 *	marks it as being initialized. There is no real initialization to be
 *	done as there are no attributes.
 */
int
pthread_mutexattr_init(pthread_mutexattr_t *attr)
{

	if (attr == NULL) {
		return (EINVAL);
	}
	*attr = (pthread_mutexattr_t)_pmalloc(sizeof(struct pthread_attr));
	if (*attr == NULL)
		return (ENOMEM);
	memset(*attr, 0, sizeof(struct pthread_attr));

	_spin_lock(&dbx__attributes);
	queue_append(&__dbx_known_attributes, &(*attr)->link);
	_spin_unlock(&dbx__attributes);

	(*attr)->flags = MUTEXATTR_VALID;
	(*attr)->type = ATTR_MUTEX;
	(*attr)->mutex_kind = _mutex_kind_default;

	_spin_lock(&attr_id_lock);
	(*attr)->attr_id = ++attr_id;
	_spin_unlock(&attr_id_lock);

	return (0);
}


/*
 * Function:
 *	pthread_mutexattr_destroy
 *
 * Parameters:
 *	attr - pointer to the attribute structure to be deleted
 *
 * Return value:
 *	0	Success
 *	EINVAL	if the pointer passed is and invalid pointer
 *		if the structure had not be previously initialized
 *		the attribute was the default attribute
 *
 * Description:
 *	The attribute structure is simply marked as being no longer
 *	valid after the appropriate checks have been made.
 */
int
pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
	if ((attr == NULL) || (*attr == NO_MUTEX_ATTRIBUTE) ||
	    (*attr == pthread_mutexattr_default) ||
	    !((*attr)->flags & MUTEXATTR_VALID)) {
		return (EINVAL);
	}
	(*attr)->flags &= ~MUTEXATTR_VALID;

	_spin_lock(&dbx__attributes);
	queue_remove(&(*attr)->link);           /* link for dbx */
	_spin_unlock(&dbx__attributes);

	_pfree(*attr);
	*attr = NO_MUTEX_ATTRIBUTE;
	return (0);
}


/*
 * Function:
 *	_pthread_mutex_startup
 *
 * Description:
 *	Initialize everything for the mutex functions. This involves
 *	initializing mutex attributes and the limit on how many times
 *	to spin before yielding between attempts to get the mutex.
 */
void
_pthread_mutex_startup(void)
{

	_spinlock_create(&mtx_id_lock);
	_pthread_mutexattr_startup();

	pthread_mutexattr_init(&mutexattr_global_np);
		/* pthread_mutexattr_setkind_np */
	mutexattr_global_np->mutex_kind = MUTEX_RECURSIVE_NP;
	pthread_mutex_init (&_mutex_global_np, &mutexattr_global_np);
}


/*
 * Function:
 *	_initialize_mutex
 *
 * Description:
 *	Initialize a mutex structure. The mutex will be valid, unlocked and
 *	therefore unowned and un-named.
 */
int
_initialize_mutex(pthread_mutex_t *mutex, pthread_mutexattr_t attr)
{
	mutex->owner = NO_PTHREAD;
	_spinlock_create(&mutex->lock);
	mutex->flags = MUTEX_VALID;

	mutex->attr = (pthread_mutexattr_t)_pmalloc(sizeof(struct pthread_attr));
	if (mutex->attr == NULL)
		return (ENOMEM);

	mutex->attr->protocol = attr->protocol;
	mutex->attr->prio_ceiling = attr->prio_ceiling;
	mutex->attr->mutex_kind = attr->mutex_kind;

	_spin_lock(&mtx_id_lock);
	mutex->mtx_id = ++mtx_id;
	_spin_unlock(&mtx_id_lock);
	mutex->mtx_kind = attr->mutex_kind;
	mutex->lock_cpt = 0; 
/*
	mutex->reserved[3] = 0x01010101;
*/
	return (0);
}


/*
 * Function:
 *	pthread_mutex_init
 *
 * Parameters:
 *	mutex - the mutex to be created
 *	attr  - pointer to the attributes to indicate how it should 
 *             be created
 *
 * Return value:
 *	0       Success
 *	EINVAL  The pointer to the mutex is invalid
 *		The value specifie by attr is invalid
 *	EBUSY	Attempt to reinitialize the object referenced by mutex.
 *
 *	ENOMEM	Insufficient memory exists to initialize the mutex.
 *
 * Description:
 *	The initialize_mutex function is used to do all the real
 *	work of creation once the parameters have been checked.
 */
int
pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr)
{
pthread_mutex_t *elemutex;
__dbx_mutex	*ptdbxmutex;

        if (attr == NULL)
            attr = &pthread_mutexattr_default;
        if ((mutex == NO_MUTEX) ||
	    (*attr == NO_MUTEX_ATTRIBUTE)
	    || !((*attr)->flags & MUTEXATTR_VALID)) {
		return (EINVAL);
	}

/*
	if ((mutex->flags & MUTEX_VALID) && (mutex->reserved[3] == 0x01010101))
		return (EBUSY);
*/

	_spin_lock(&dbx__mutexes);
	ptdbxmutex = _pmalloc(sizeof(struct __dbx_mutex));
	if (ptdbxmutex == NULL) {
		_spin_unlock(&dbx__mutexes);
		return (ENOMEM);
	}
	ptdbxmutex->pt_mutex = mutex;
	mutex->link.next = (pthread_queue*)ptdbxmutex;	/* need to remove */
/*
	for (elemutex = (pthread_mutex_t *)queue_next(&__dbx_known_mutexes);
	     elemutex != (pthread_mutex_t *)&__dbx_known_mutexes;
	     elemutex = (pthread_mutex_t *)queue_next(&elemutex->link)) {
		if (mutex == elemutex) {
			_spin_unlock(&dbx__mutexes);
			return (EBUSY);
		}
	}
*/
	queue_append(&__dbx_known_mutexes, ptdbxmutex);
	_spin_unlock(&dbx__mutexes);

	return(_initialize_mutex(mutex, *attr));
}


/*
 * Function:
 *	pthread_mutex_destroy
 *
 * Parameters:
 *	mutex - the mutex to be deleted
 *
 * Return value:
 *	0	Success
 *	EINVAL	The pointer to the mutex was invalid
 *		The mutex was invalid
 *	EBUSY	The mutex was locked
 *
 * Description:
 *	After doing some validation that this is a real mutex and ready
 *	to be freed, the flag is set that the mutex is invalid. The
 *	mutex will be locked by the caller before it is made invalid.
 */
int
pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	if ((mutex == NO_MUTEX) || !(mutex->flags & MUTEX_VALID)) {
		return (EINVAL);
	}


	/* If we can't lock the mutex then someone is using it, so we fail.
	 */
	if (_check_lock((atomic_p)&mutex->lock, UL_FREE, UL_BUSY)) {
		return (EBUSY);
	}

	_spin_lock(&dbx__mutexes);
	queue_remove(mutex->link.next);           /* link for dbx */
	_pfree(mutex->link.next);

	_spin_unlock(&dbx__mutexes);

	mutex->flags &= ~MUTEX_VALID;
/*
	mutex->reserved[3] = 0;
*/
	if ((mutex->attr != pthread_mutexattr_default) &&
	    (mutex->attr != pthread_mutexattr_fast) &&
	    (mutex->attr != mutexattr_global_np))
		_pfree(mutex->attr);
	return (0);
}


/*
 * Function:
 *	pthread_mutex_trylock
 *
 * Parameters:
 *	mutex - a pointer to the mutex to be locked
 *
 * Return value:
 *      0       the lock was successful
 *      EINVAL  The pointer to the mutex was invalid
 *              The mutex was invalid
 *      EBUSY 	the mutex was already locked by another thread
 *
 * Description:
 *	Try to lock the mutex after being sure that we have been passed
 *	a real mutex.
 */
int
pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	pthread_t self;
	__dbx_mutex	*ptdbxmutex;

	if (mutex == NO_MUTEX)
		return (EINVAL);

	if (mutex->mtx_kind == MUTEX_FAST_NP) {	/* supposedly init. and valid */
		return(_get_lock((atomic_p)&mutex->lock) ? 0 : EBUSY);
	}

        if (mutex->flags & MUTEX_INITSTATIC) {
		_spin_lock(&dbx__mutexes);
        	if (mutex->flags & MUTEX_INITSTATIC) {
                	/* init by macro PTHREAD_INITIALIZER
                   	we complete the initialization */
	
			ptdbxmutex = _pmalloc(sizeof(struct __dbx_mutex));
			if (ptdbxmutex == NULL) {
				_spin_unlock(&dbx__mutexes);
				return (ENOMEM);
			}
			ptdbxmutex->pt_mutex = mutex;
							/* need to remove */
			mutex->link.next = (pthread_queue*)ptdbxmutex; 	
			queue_append(&__dbx_known_mutexes, ptdbxmutex);

                	mutex->flags &= ~MUTEX_INITSTATIC;
                	mutex->flags |= MUTEX_VALID;
                	mutex->attr = pthread_mutexattr_default;
			_spin_unlock(&dbx__mutexes);

			_spin_lock(&mtx_id_lock);
                	mutex->mtx_id = ++mtx_id;
			_spin_unlock(&mtx_id_lock);
			mutex->mtx_kind = MUTEX_NONRECURSIVE_NP;
			mutex->lock_cpt = 0;
		} else {
			_spin_unlock(&dbx__mutexes);
		}
	}

        if (!(mutex->flags & MUTEX_VALID))
                return (EINVAL);

	self = pthread_self();

	if (!_get_lock((atomic_p)&mutex->lock)) {
		if ((mutex->mtx_kind == MUTEX_RECURSIVE_NP) && 
					(mutex->owner == self)) {
			mutex->lock_cpt++;
        		return (0);
		} else {
                	return (EBUSY);
		}
	}

	mutex->owner = self;
        return (0);
}


/*
 * Function:
 *	pthread_mutex_lock
 *
 * Parameters:
 *	mutex - a pointer to the mutex to be locked
 *
 * Return value:
 *	0	Success
 *	EINVAL	The pointer to the mutex was invalid
 *		The mutex was invalid
 *	EDEADLK	The mutex is already owned by the caller
 *
 * Description:
 *	Try once to get the lock, if unsuccessful the blocking lock call
 *	is made. When this returns we have the lock so make the caller the
 *	owner.
 */
int
pthread_mutex_lock(pthread_mutex_t *mutex)
{
	pthread_t self;
	__dbx_mutex	*ptdbxmutex;

	if (mutex == NO_MUTEX)
		return (EINVAL);

	if (mutex->mtx_kind == MUTEX_FAST_NP) { /* supposedly init. and valid */
		_spin_lock(&mutex->lock);
		return(0);
	}

        if (mutex->flags & MUTEX_INITSTATIC) {
		_spin_lock(&dbx__mutexes);
        	if (mutex->flags & MUTEX_INITSTATIC) {
                	/* init by macro PTHREAD_MUTEX_INITIALIZER
                   	we complete the initialization */

			ptdbxmutex = _pmalloc(sizeof(struct __dbx_mutex));
			if (ptdbxmutex == NULL) {
				_spin_unlock(&dbx__mutexes);
				return (ENOMEM);
			}
			ptdbxmutex->pt_mutex = mutex;
 							/* need to remove */
			mutex->link.next = (pthread_queue*)ptdbxmutex;
			queue_append(&__dbx_known_mutexes, ptdbxmutex);

                	mutex->flags &= ~MUTEX_INITSTATIC;
                	mutex->flags |= MUTEX_VALID;
                	mutex->attr = pthread_mutexattr_default;
			_spin_unlock(&dbx__mutexes);
	
			_spin_lock(&mtx_id_lock);
                	mutex->mtx_id = ++mtx_id;
			_spin_unlock(&mtx_id_lock);
			mutex->mtx_kind = MUTEX_NONRECURSIVE_NP;
			mutex->lock_cpt = 0;
        	} else { 
			_spin_unlock(&dbx__mutexes);
		}
	}

	if (!(mutex->flags & MUTEX_VALID))
		return (EINVAL);

	self = pthread_self();

	if (mutex->owner == self) {
		if (mutex->mtx_kind == MUTEX_RECURSIVE_NP) {
			mutex->lock_cpt++;
			return (0);
		} else
			return (EDEADLK);
	}

	_spin_lock(&mutex->lock);
	mutex->owner = self;
	return (0);
}


/*
 * Function:
 *	pthread_mutex_unlock
 *
 * Parameters:
 *	mutex - a pointer to the mutex to be unlocked
 *
 * Return value:
 *	0	Success
 *	EINVAL	The pointer to the mutex was invalid
 *		The mutex was invalid
 *	EPERM	The mutex is not owned by the caller
 *
 * Description:
 *	Once the mutex is verified as real and locked by the caller
 *	it is unlocked.
 */
int
pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	pthread_t self;

	if (mutex == NO_MUTEX)
		return (EINVAL);

	if (mutex->mtx_kind == MUTEX_FAST_NP) {	/* supposedly init. and valid */
		_spin_unlock(&mutex->lock);
		return(0);
	}

	if (!(mutex->flags & MUTEX_VALID))
		return (EINVAL);

	self = pthread_self();

	if (mutex->owner != self)
		return (EPERM);

	if ((mutex->mtx_kind == MUTEX_RECURSIVE_NP) && (mutex->lock_cpt)) {
		mutex->lock_cpt--;
		return (0);
	}

	mutex->owner = NO_PTHREAD;
	_spin_unlock(&mutex->lock);
	return (0);
}


/*
 * Function:
 *	pthread_mutexattr_setprotocol
 *
 * Parameters:
 *	attr		- attributes object to be set
 *	protocol	- protocol to assign
 *
 * Return value:
 *	ENOSYS	This function is not supported
 *
 * Description:
 *	This function is not supported.
 */
int
pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr,
			      int protocol)
{
	return (ENOSYS);
}


/*
 * Function:
 *	pthread_mutexattr_getprotocol
 *
 * Parameters:
 *	attr	- attributes object to retrieve protocol from
 * 	protocol-
 *
 * Return value:
 *	ENOSYS	This function is not supported
 *
 * Description:
 *	This function is not supported.
 */
int
pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr,
			      int *protocol)
{
	return (ENOSYS);
}


/*
 * Function:
 *	pthread_mutexattr_setprioceiling
 *
 * Parameters:
 *	attr		- attributes object ot set
 *	prioceiling	- ceiling value to assign
 *
 * Return value:
 *	ENOSYS	This function is not supported
 *
 * Description:
 *	This function is not supported.
 */
int
pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int prioceiling)
{
	return (ENOSYS);
}


/*
 * Function:
 *	pthread_mutexattr_getprioceiling
 *
 * Parameters:
 *	attr	- attributes object to retrieve ceiling from
 *
 * Return value:
 *	ENOSYS	This function is not supported
 *
 * Description:
 *	This function is not supported.
 */
int
pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *attr,
                                 int *ceiling)
{
	return (ENOSYS);
}


/*
 * Function:
 *	pthread_mutex_setprioceiling
 *
 * Parameters:
 *	mutex		- mutex to change
 *	prioceiling	- ceiling value to assign
 *
 * Return value:
 *	ENOSYS	This function is not supported
 *
 * Description:
 *	This function is not supported.
 */
int
pthread_mutex_setprioceiling(pthread_mutex_t *mutex, int prio_ceiling,
                             int *old_ceiling)
{
	return (ENOSYS);
}


/*
 * Function:
 *	pthread_mutex_getprioceiling
 *
 * Parameters:
 *	mutex	- mutex to retrieve ceiling from
 *
 * Return value:
 *	ENOSYS	This function is not supported (ENOSYS)
 *
 * Description:
 *	This function is not supported.
 */
int
pthread_mutex_getprioceiling(pthread_mutex_t *mutex, int *ceiling)
{
	return (ENOSYS);
}


/*
 * Function:
 *	pthread_mutexattr_getpshared
 *
 * Parameters:
 *	attr	- Pointer to the attribute  
 *	pshared - Pointer to the process_shared
 *
 * Return value:
 *	ENOSYS	This function is not supported
 *
 * Description:
 * 	This function returns the value of the process-shared attribute from
 * 	the attributes structure referenced by attr.
 *	This function is not supported.
 */
int
pthread_mutexattr_getpshared (const pthread_mutexattr_t *attr, int *pshared)
{
	return (ENOSYS);
}


/*
 * Function:
 *	pthread_mutexattr_setpshared
 *
 * Parameters:
 *	attr	- Pointer to the attribute 
 *	pshared	- Process_shared
 *
 * Return value:
 *	ENOSYS	This function is not supported
 *
 * Description:
 *      This function is used to set the process-shared attribute in an 
 * 	initialized attributes structure referenced by attr.
 *	This function is not supported.
 */
int
pthread_mutexattr_setpshared (const pthread_mutexattr_t *attr, int pshared)
{
	return (ENOSYS);
}

/*
 * Function:
 *	pthread_mutexattr_getkind_np
 *
 * Parameters:
 *	attr	- Pointer to the attribute 
 *	kind	- Pointer to the  mutex type attribute 
 *
 * Return value:
 *	0	- Success
 *	EINVAL	- the attribute was invalid
 *		- the value specified by kind is invalid
 *
 * Description:	This function returns the mutex type attribute
 *		This function is not portable
 */

int
pthread_mutexattr_getkind_np (pthread_mutexattr_t *attr, int *kind)
{
	if (kind == NULL)
		return (EINVAL);
	if (attr == NULL) { 
		*kind = pthread_mutexattr_fast->mutex_kind;
		return(0);
	}
	if (!((*attr)->flags & MUTEXATTR_VALID))
		return (EINVAL);
	*kind = (*attr)->mutex_kind;
	return(0);
}

/*
 * Function:
 *	pthread_mutexattr_setkind_np
 *
 * Parameters:
 *	attr	- Pointer to the attribute 
 *	kind	- the mutex type attribute
 *
 * Return value:
 *	0	- Success
 *	EINVAL	- the value specified by attr is invalid
 *	ENOTSUP	- the value of the kind parameter is not supported.
 *
 * Description:	This function sets the mutex type attribute
 *		This function is not portable
 */

int
pthread_mutexattr_setkind_np (pthread_mutexattr_t *attr, int kind)
{
	if ((attr == NULL) || !((*attr)->flags & MUTEXATTR_VALID))
		return (EINVAL);
        if ((kind != MUTEX_FAST_NP) &&
            (kind != MUTEX_RECURSIVE_NP) && (kind != MUTEX_NONRECURSIVE_NP))
		return (ENOTSUP);
	(*attr)->mutex_kind = kind;
	return(0);
	
}

/*
 * Function:
 *	pthread_set_mutexattr_default_np
 *
 * Parameters:
 *	kind	- the mutex type attribute
 *
 * Return value:
 *	0	- Success
 *	ENOTSUP	- the value of the kind parameter is not supported.
 *
 * Description:	This function modifies the mutex kind default
 *		This function is not portable
 */

int
pthread_set_mutexattr_default_np (int kind)
{
        if ((kind != MUTEX_FAST_NP) &&
            (kind != MUTEX_RECURSIVE_NP) && (kind != MUTEX_NONRECURSIVE_NP))
		return (ENOTSUP);
	pthread_mutexattr_default->mutex_kind = kind;
	_mutex_kind_default = kind;
	return(0);
	
}


/*
 * Function:
 *      pthread_lock_global_np
 *
 * Description: locks the global_mutex. It is a recursive mutex.
 *
 */

void
pthread_lock_global_np()
{
        pthread_mutex_lock (&_mutex_global_np);
}


/*
 * Function:
 *      pthread_unlock_global_np
 *
 * Description: unlocks the global_mutex.
 *
 */

void
pthread_unlock_global_np()
{

        pthread_mutex_unlock (&_mutex_global_np);
}


#ifdef DEBUG_PTH
/*
 * Function:
 *	dump_mutex
 *
 * Parameters:
 *	mutex - the mutex to be dumped
 *
 * Description:
 *	This is a debugging function which prints out the state of a mutex and
 *	its owner.
 */
void
dump_mutex(pthread_mutex_t *mutex)
{
	if ((mutex == NO_MUTEX) || !(mutex->flags & MUTEX_VALID)) {
		dbgPrintf("NO MUTEX\n");
		return;
	}
	dbgPrintf("MTX: %x Own: ", mutex);

	if (mutex->owner == NO_PTHREAD)
		dbgPrintf("NONE ");
	else
		dbgPrintf("%x ", mutex->owner);
}
#endif	/* DEBUG_PTH */
