static char sccsid[] = "@(#)05	1.28  src/bos/usr/ccs/lib/libpthreads/cond.c, libpth, bos41J, 9521B_all 5/29/95 07:28:29";
/*
 * COMPONENT_NAME: libpth
 * 
 * FUNCTIONS:
 *	_pthread_condattr_startup	
 *	pthread_condattr_init	
 *	pthread_condattr_destroy	
 *	pthread_condattr_getpshared	
 *	pthread_condattr_setpshared	
 *	_pthread_cond_startup	
 *	_initialize_condition	
 *	pthread_cond_init	
 *	pthread_cond_destroy	
 *	_cond_wait_join
 *	pthread_cond_wait	
 *	pthread_cond_timedwait	
 *	pthread_cond_signal	
 *	pthread_cond_broadcast	
 *	pthread_delay_np
 *	pthread_get_expiration_np
 *	dump_cond	
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
 * File: cond.c
 *
 * Functions supporting condition variables and their attributes. In fact
 * there are no attributes currently defined for condition variables.
 * Condition variables block the vp that the thread is running on by waiting
 * for an event.
 */

#include "internal.h"

/*
 * Global Variables
 */
pthread_condattr_t	pthread_condattr_default;
#define ATTR_COND	2
int			cv_id = 0;
spinlock_t		cv_id_lock;

extern pthread_queue	__dbx_known_attributes;
extern spinlock_t	dbx__attributes;
__dbx_cond		__dbx_known_conditions;

spinlock_t		dbx__conditions;
extern	int		attr_id;
extern	spinlock_t	attr_id_lock;
extern pthread_mutexattr_t pthread_mutexattr_fast;

struct delay_args_np
{
	pthread_mutex_t *delay_mutex_np;
	pthread_cond_t  *delay_cond_np;
};


/*
 * Function:
 *	_pthread_condattr_startup
 *
 * Description:
 * 	Initialize condition attributes. This function creates the default
 *	attribute structure.
 */
private void
_pthread_condattr_startup()
{
	pthread_condattr_init(&pthread_condattr_default);
}


/*
 * Function:
 *	pthread_condattr_init
 *
 * Parameters:
 *	attr - pointer to the newly created attribute structure
 *
 * Return value:
 *	0	Success
 *	EINVAL	if the pointer passed is an invalid pointer
 *      ENOMEM  Insufficient memory exits to initialize the condition variable
 *              attributes object.
 *
 * Description:
 *	The pthread_condattr_t is the attribute structure and this function
 *	marks it as being initialized. There is no real initialization to be
 *	done as there are no attributes.
 */
int
pthread_condattr_init(pthread_condattr_t *attr)
{
	if (attr == NULL) {
		return (EINVAL);
	}
	*attr = (pthread_condattr_t)_pmalloc(sizeof(struct pthread_attr));
	if (*attr == NO_COND_ATTRIBUTE) {
		return (ENOMEM);
	}
	memset(*attr, 0, sizeof(struct pthread_attr));

	_spin_lock(&dbx__attributes);
	queue_append(&__dbx_known_attributes, &(*attr)->link);
	_spin_unlock(&dbx__attributes);

	(*attr)->flags = CONDATTR_VALID;
	(*attr)->type = ATTR_COND;

	_spin_lock(&attr_id_lock);
	(*attr)->attr_id  = ++attr_id;
	_spin_unlock(&attr_id_lock);

	return (0);
}


/*
 * Function:
 *	pthread_condattr_destroy
 *
 * Parameters:
 *	attr - pointer to the attribute structure to be deleted
 *
 * Return value:
 *	0	Success
 *	EINVAL	if the pointer passed is an invalid pointer
 *		if the structure had not be previously initialized
 *		the attribute was the default attribute
 *
 * Description:
 *	The attribute structure is simply marked as being no longer
 *	valid after the appropriate checks have been made.
 */
int
pthread_condattr_destroy(pthread_condattr_t *attr)
{
	if ((attr == NULL) || (*attr == NO_COND_ATTRIBUTE) ||
	    (*attr == pthread_condattr_default) ||
	    !((*attr)->flags & CONDATTR_VALID)) {
		return (EINVAL);
	}
	(*attr)->flags &= ~CONDATTR_VALID;

	_spin_lock(&dbx__attributes);
	queue_remove(&(*attr)->link);           /* link for dbx */
	_spin_unlock(&dbx__attributes);

	_pfree(*attr);
	*attr = NO_COND_ATTRIBUTE;
	return (0);
}


/*
 * Function:
 *	pthread_condattr_getpshared
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
pthread_condattr_getpshared (const pthread_condattr_t *attr, int *pshared)
{
	return (ENOSYS);
}


/*
 * Function:
 *	pthread_condattr_setpshared
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
pthread_condattr_setpshared (const pthread_condattr_t *attr, int pshared)
{
	return (ENOSYS);
}


/*
 * Function:
 *	_pthread_cond_startup
 *
 * Description:
 *	Intialize all of the condition variable functions. In fact there
 *	is no initialization to do other than for condition attributes.
 */
void
_pthread_cond_startup()
{
	_spinlock_create(&cv_id_lock);
	_pthread_condattr_startup();
}


/*
 * Function:
 *	_initialize_condition
 *
 * Parameters:
 *	cond - the condition variable to be initialized
 *	attr - attributes to indicate how it should be initialized
 *
 * Description:
 *	Initialize a condition variable. Make the the queue of waiting
 *	pthreads empty, unlock the spin lock protecting the structure
 *	and make the condition valid.
 */
int
_initialize_condition(pthread_cond_t *cond, pthread_condattr_t attr)
{
	queue_init(&cond->waiters);
	_spinlock_create(&cond->lock);
	cond->flags = COND_VALID;

	cond->attr = (pthread_condattr_t)_pmalloc(sizeof(struct pthread_attr));
	if (cond->attr == NULL)
		return (ENOMEM);

	cond->mutex = 0;
	cond->cptwait = 0;

	_spin_lock(&cv_id_lock);
        cond->cv_id = ++cv_id;
	_spin_unlock(&cv_id_lock);
/*
	cond->reserved = 0x01010101;
*/
	return(0);
}


/*
 * Function:
 *	pthread_cond_init
 *
 * Parameters:
 *	cond - the condition variable to be created
 *	attr - pointer to the attributes to indicate how it should 
 *             be created
 *
 * Return value:
 *	0	Success
 *	EINVAL	The pointer to the condition variable was invalid
 *		The condition attribute was invalid
 *	EBUSY	Attempt to reinitialize the object referenced by condition.
 *
 *	ENOMEM	Insufficient memory exists to initialize the condition variable.
 *
 * Description:
 *	The initialize_condition function is used to do all the real
 *	work of creation once the parameters have been checked.
 */
int
pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *attr)
{
pthread_cond_t *elemcond;
__dbx_cond     *ptdbxcond;

	PT_LOG(("pthread_cond_init\n", NULL));

        if (attr == NULL)
            attr = &pthread_condattr_default;
        if ((cond == NO_COND) ||
	    (*attr == NO_COND_ATTRIBUTE) ||
	    !((*attr)->flags & CONDATTR_VALID)) {
		return (EINVAL);
	}

/*
	if ((cond->flags & COND_VALID) && (cond->reserved == 0x01010101))
		return (EBUSY);
*/
	_spin_lock(&dbx__conditions);
	ptdbxcond = _pmalloc(sizeof(struct __dbx_cond));
	if (ptdbxcond == NULL) {
		_spin_unlock(&dbx__conditions);
		return (ENOMEM);
	}
	ptdbxcond->pt_cond = cond;
	cond->link.next = (pthread_queue*)ptdbxcond;  /* need to remove */
/*
	for (elemcond = (pthread_cond_t *)queue_next(&__dbx_known_conditions);
	     elemcond != (pthread_cond_t *)&__dbx_known_conditions;
	     elemcond = (pthread_cond_t *)queue_next(&elemcond->link)) {
		if (cond == elemcond) {
			_spin_unlock(&dbx__conditions);
			return (EBUSY);
		}
	}
*/
	queue_append(&__dbx_known_conditions, ptdbxcond);
	_spin_unlock(&dbx__conditions);

	return (_initialize_condition(cond, *attr));
}


/*
 * Function:
 *	pthread_cond_destroy
 *
 * Parameters:
 *	cond - the condition variable to be deleted
 *
 * Return value:
 *	0	Success
 *	EINVAL	The pointer to the condition variable was invalid
 *		The condition variable was not valid
 *	EBUSY	At least one thread is waiting on the condition variable
 *
 * Description:
 *	Destroy a condition variable if there are no waiters. It is
 *	destroyed by marking it invalid.
 */
int
pthread_cond_destroy(pthread_cond_t *cond)
{
	PT_LOG(("pthread_cond_destroy: cond = %x\n", cond));

	if ((cond == NO_COND) || !(cond->flags & COND_VALID)) {
		return (EINVAL);
	}

	/* Lock the condition structure before we touch it.
	 */
	_spin_lock(&cond->lock);

	/* Check to see if anyone is one the queue waiting to be signalled.
	 * If so we return an error.
	 */
	if (!queue_empty(&cond->waiters)) {
		_spin_unlock(&cond->lock);
		return (EBUSY);
	}

	/* No one is waiting. Make the condition structure invalid
	 * so future calls with this handle fail and then unlock it.
	 */

	_spin_lock(&dbx__conditions);
	queue_remove(cond->link.next);           /* link for dbx */
	_pfree(cond->link.next);

	_spin_unlock(&dbx__conditions);

	cond->mutex = 0;
	cond->cptwait = 0;
	cond->flags &= ~COND_VALID;
/*
	cond->reserved = 0;
*/
	_spin_unlock(&cond->lock);
	if (cond->attr != pthread_condattr_default) 
		_pfree(cond->attr);
	return (0);
}


/*
 * Function:
 *	_cond_wait_join
 *
 * Parameters:
 *	cond - the condition variable being waited on
 *	lock - the lock associated with this condition.
 *
 * Return value:
 *	0	Success
 *	EINTR	unix signal interrupts the thread_tsleep system call.
 *
 * Description:
 *	Once the checks have been completed, locks are taken on both
 *	the condition variable structure and the thread. The thread is
 *	taken off the thread queues (_pthread_deactivate) and queued onto
 *	condition variable with the state changed to indicate the wait.
 *	The thread is then blocked waiting for an event. This will be either
 *	a signal event, meaning that the condition has either been signalled
 *	or broadcast, or a cancellation event. If it was signalled the thread
 *	has already been reactivated and so the lock is reaquired and return.
 *	If the event was a cancel we must remove the thread from the waiting
 *	queue, put the thread back on the active thread queues
 *	(_pthread_activate) and the cancellation point is created with
 *	pthread_testcancel().
 */
int
_cond_wait_join(pthread_cond_t *cond, spinlock_t *lock)
{
	pthread_d	thread;
	pthread_d	self;
	int		event;
	int		async_cancel;

	PT_LOG(("_cond_wait_join: cond = %x\n", cond));

	self = pthread_self();
	thread = pthread_id_lookup(self);

	/* Lock the condition variable and the thread.
	 */
	_spin_lock(&cond->lock);

	_spin_lock(&thread->lock);

	/* See if the thread has a cancel pending.
	 * If so, unlock the condition variable and thread
	 * (in reverse order) before we check if we can
	 * act on the request.  We leave the lock locked
	 * so it will be held when the cleanup routines
	 * are called.  If we return, we can't act on the
	 * cancellation, so relock the condition variable and
	 * thread and proceed.
	 */

	if ((thread->intr.pending == TRUE) &&
	    (thread->intr.state == PTHREAD_CANCEL_ENABLE)) {
		thread->cond_cancel = NULL; 
		_spin_unlock(&thread->lock);
		_spin_unlock(&cond->lock);

		/* the 3 following lines replace the pthread_testcancel */
		_spin_lock(&self->lock);
		_pthread_intr_detect(self);
		_spin_unlock(&self->lock);

		/* Does not reach here */
	}

	thread->cond_cancel = cond; 
	thread->mutex_cancel = (pthread_mutex_t*)lock; 
	thread->join_cancel = 1;	/* for _pthread_unwind() the lock
					 * is a spinlock
					*/ 

	_pthread_deactivate(thread, &cond->waiters);

	/* Unlock everything (in reverse order) before we
	 * wait for the event.
	 * thread->lock will be free by the thread_tsleep.
	 */
	_spin_unlock(&cond->lock);
	_spin_unlock(lock);

	_pthread_event_wait(thread, &event, FALSE, NO_TIMEOUT);
	_spin_lock(lock);

	/* An event has arrived. Lock the condition and the thread
	 * before we look to see what it was.
	 */
	_spin_lock(&cond->lock);
	_spin_lock(&thread->lock);

	thread->cond_cancel = NULL; 
	thread->mutex_cancel = NULL; 

	if (event != EVT_SIGNAL) {	/* EVT_CANCEL, EVT_SIGPOST */
		if (event == EVT_SIGPOST) {
				/* for _pthread_event_notify/sigwait */
			thread->event |= EVT_SIGPOST;
			_pthread_activate(thread);
			_spin_unlock(&thread->lock);
			_spin_unlock(&cond->lock);
			return (EINTR);
		}
		_pthread_activate(thread);
	}

	async_cancel = thread->intr.pending
		       && thread->intr.state == PTHREAD_CANCEL_ENABLE
		       && thread->intr.mode == PTHREAD_CANCEL_ASYNCHRONOUS;

	if (event == EVT_CANCEL || async_cancel) {
		_spin_unlock(&thread->lock);
		_spin_unlock(&cond->lock);

		if (async_cancel) {
			_spin_unlock(lock);
		}

		pthread_testcancel();
		INTERNAL_ERROR("pthread_cond_wait");
		/* NOTREACHED */
	}
#ifdef DEBUG_PTH
	else if (event != EVT_SIGNAL)
		INTERNAL_ERROR("pthread_cond_wait");
#endif	/* DEBUG_PTH */

	/* The thread was signalled. The thread is already back on
	 * the active list so the thread and condition are unlocked
	 * and the lock is re locked for the caller. The only way this
	 * could fail is if the lock had been deleted while the thread
	 * was waiting.
	 */
	_spin_unlock(&thread->lock);
	_spin_unlock(&cond->lock);
	return(0);
}


/*
 * Function:
 *	pthread_cond_wait
 *
 * Parameters:
 *	cond - the condition variable being waited on
 *	mutex - the locked mutex associated with this condition.
 *
 * Return value:
 *	0	Success
 *	EINVAL	The pointer to the condition variable is invalid
 *		The condition variable is invalid
 *		The pointer to the mutex is invalid
 *		The mutex is invalid
 *		Different mutexes were supplied for concurrent pthread_cond_wait
 * 		or pthread_cond_timedwait operations on the same condition
 *		variable
 *	EDEADLK	The mutex is not locked by the caller
 *
 * Description:
 *	Once the checks have been completed, locks are taken on both
 *	the condition variable structure and the thread. The thread is
 *	taken off the thread queues (_pthread_deactivate) and queued onto
 *	condition variable with the state changed to indicate the wait.
 *	The thread is then blocked waiting for an event. This will be either
 *	a signal event, meaning that the condition has either been signalled
 *	or broadcast, or a cancellation event. If it was signalled the thread
 *	has already been reactivated and so the mutex is reaquired and return.
 *	If the event was a cancel we must remove the thread from the waiting
 *	queue, put the thread back on the active thread queues
 *	(_pthread_activate) and the cancellation point is created with
 *	pthread_testcancel().
 */
int
pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	pthread_d	thread;
	pthread_d	self;
	int		event;
	int		async_cancel;
	__dbx_cond     *ptdbxcond;

	PT_LOG(("pthread_cond_wait: cond = %x\n", cond));

        if (cond == NO_COND)
                return (EINVAL);

        if (cond->flags & COND_INITSTATIC) {
		_spin_lock(&dbx__conditions);
        	if (cond->flags & COND_INITSTATIC) {
                	/* init by macro PTHREAD_COND_INITIALIZER
                   	we complete the initialization */

			ptdbxcond = _pmalloc(sizeof(struct __dbx_cond));
			if (ptdbxcond == NULL) {
				_spin_unlock(&dbx__conditions);
				return (ENOMEM);
			}
			ptdbxcond->pt_cond = cond;
							/* need to remove */
			cond->link.next = (pthread_queue*)ptdbxcond;  
                	queue_append(&__dbx_known_conditions, ptdbxcond);

                	queue_init(&cond->waiters);
                	cond->flags &= ~COND_INITSTATIC;
                	cond->flags |= COND_VALID;
                	cond->attr = pthread_condattr_default;
			_spin_unlock(&dbx__conditions);

			_spin_lock(&cv_id_lock);
                	cond->cv_id = ++cv_id;
			_spin_unlock(&cv_id_lock);
        	} else {
			_spin_unlock(&dbx__conditions);
		}
	}

        if (!(cond->flags & COND_VALID) ||
	    (mutex == NO_MUTEX) || !(mutex->flags & MUTEX_VALID)) {
		return (EINVAL);
	}

	if (mutex->mtx_kind == MUTEX_RECURSIVE_NP)
		return (EINVAL);

	self = pthread_self();
	thread = pthread_id_lookup(self);

	if (mutex->mtx_kind != MUTEX_FAST_NP)
		if (pthread_mutex_getowner_np(mutex) != thread)
			return (EDEADLK);

	/* Different mutexes were supplied for concurrent pthread_cond_wait() or
	   pthread_cond_timedwait() operations on the same condition variable
	*/
	if (cond->mutex)
 	   if (cond->mutex != mutex)
		return (EINVAL);
	
	/* Lock the condition variable and the thread.
	 */
	_spin_lock(&cond->lock);

	_spin_lock(&thread->lock);

	/* See if the thread has a cancel pending.
	 * If so, unlock the condition variable and thread
	 * (in reverse order) before we check if we can
	 * act on the request.  We leave the mutex locked
	 * so it will be held when the cleanup routines
	 * are called.  If we return, we can't act on the
	 * cancellation, so relock the condition variable and
	 * thread and proceed.
	 */

	if ((thread->intr.pending == TRUE) &&
	    (thread->intr.state == PTHREAD_CANCEL_ENABLE)) {
		thread->cond_cancel = NULL; 
		_spin_unlock(&thread->lock);
		_spin_unlock(&cond->lock);

		/* the 3 following lines replace the pthread_testcancel */
		_spin_lock(&self->lock);
		_pthread_intr_detect(self);
		_spin_unlock(&self->lock);

		/* Does not reach here */
	}

	cond->mutex = mutex;
	cond->cptwait++;

	thread->cond_cancel = cond; 
	thread->mutex_cancel = mutex; 
	thread->join_cancel = 0;	/* for _pthread_unwind() the lock
					 * is a mutex
					*/ 

	_pthread_deactivate(thread, &cond->waiters);

	/* Unlock everything (in reverse order) before we
	 * wait for the event.
	 * thread->lock will be free by the thread_tsleep.
	 */
	_spin_unlock(&cond->lock);

	pthread_mutex_unlock(mutex);
	_pthread_event_wait(thread, &event, FALSE, NO_TIMEOUT);
	pthread_mutex_lock(mutex);

	/* An event has arrived. Lock the condition and the thread
	 * before we look to see what it was.
	 */
	_spin_lock(&cond->lock);
	_spin_lock(&thread->lock);

	thread->cond_cancel = NULL; 
	thread->mutex_cancel = NULL; 

	if (event != EVT_SIGNAL) {	/* EVT_CANCEL, EVT_SIGPOST */
		if (event == EVT_SIGPOST) {
				/* for _pthread_event_notify/sigwait */
			thread->event |= EVT_SIGPOST;
			_pthread_activate(thread);
			if (cond->cptwait > 0) {
                        	cond->cptwait--;
				if (!cond->cptwait)
                            	cond->mutex = 0;
			}
			_spin_unlock(&thread->lock);
			_spin_unlock(&cond->lock);
			return (EINTR);
		}
		_pthread_activate(thread);
		if (cond->cptwait > 0) {
                        cond->cptwait--;
			if (!cond->cptwait)
                            cond->mutex = 0;
		}
	}

	async_cancel = thread->intr.pending
		       && thread->intr.state == PTHREAD_CANCEL_ENABLE
		       && thread->intr.mode == PTHREAD_CANCEL_ASYNCHRONOUS;

	if (event == EVT_CANCEL || async_cancel) {
		_spin_unlock(&thread->lock);
		_spin_unlock(&cond->lock);

		if (async_cancel)
			pthread_mutex_unlock(mutex);

		pthread_testcancel();
		INTERNAL_ERROR("pthread_cond_wait");
		/* NOTREACHED */
	}
#ifdef DEBUG_PTH
	else if (event != EVT_SIGNAL)
		INTERNAL_ERROR("pthread_cond_wait");
#endif	/* DEBUG_PTH */

	/* The thread was signalled. The thread is already back on
	 * the active list so the thread and condition are unlocked
	 * and the mutex is re locked for the caller. The only way this
	 * could fail is if the mutex had been deleted while the thread
	 * was waiting.
	 */
	_spin_unlock(&thread->lock);
	_spin_unlock(&cond->lock);
	return (0);
}


/*
 * Function:
 *	pthread_cond_timedwait
 *
 * Parameters:
 *	cond - the condition variable being waited on
 *	mutex - the locked mutex associated with this condition.
 *	timeout - the maximum time to wait for the condition to be signalled
 *
 * Return value:
 *	0	Success
 *	EINVAL	The pointer to the condition variable is invalid
 *		The condition variable is invalid
 *		The pointer to the mutex is invalid
 *		The mutex is invalid
 *		timeout is invalid
 *		Different mutexes were supplied for concurrent pthread_cond_wait
 * 		or pthread_cond_timedwait operations on the same condition
 *		variable
 *	EDEADLK	The mutex is not locked by the caller
 *	ETIMEDOUT The timeout occurred
 *
 * Description:
 *	This function works in the same way as pthread_cond_wait with
 *	the exception of the added complexity of the timeout. The timeout
 *	is delivered as a timeout event. If the timeout happened then the
 *	thread is put back on the active queue (_pthread_activate) and the
 *	error is returned (although a timeout is not really an error).
 */
int
pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
		       const struct timespec *timeout)
{
	pthread_d	thread;
	pthread_d	self;
	int		event;
	int		async_cancel;
	__dbx_cond     *ptdbxcond;

	PT_LOG(("pthread_cond_timedwait: cond = %x\n", cond));

        if (cond == NO_COND)
                return (EINVAL);

        if (cond->flags & COND_INITSTATIC) {
		_spin_lock(&dbx__conditions);
        	if (cond->flags & COND_INITSTATIC) {
                	/* init by macro PTHREAD_COND_INITIALIZER
                   	we complete the initialization */

			ptdbxcond = _pmalloc(sizeof(struct __dbx_cond));
			if (ptdbxcond == NULL) {
				_spin_unlock(&dbx__conditions);
				return (ENOMEM);
			}
			ptdbxcond->pt_cond = cond;
							/* need to remove */
			cond->link.next = (pthread_queue*)ptdbxcond;  
                	queue_append(&__dbx_known_conditions, ptdbxcond);

                	queue_init(&cond->waiters);
                	cond->flags &= ~COND_INITSTATIC;
                	cond->flags |= COND_VALID;
                	cond->attr = pthread_condattr_default;
			_spin_unlock(&dbx__conditions);

			_spin_lock(&cv_id_lock);
                	cond->cv_id = ++cv_id;
			_spin_unlock(&cv_id_lock);
        	} else {
			_spin_unlock(&dbx__conditions);
		}
	}

        if (!(cond->flags & COND_VALID) ||
	    (mutex == NO_MUTEX) || !(mutex->flags & MUTEX_VALID) ||
	    (timeout == NO_TIMEOUT) || timeout->tv_nsec < 0) {
		return (EINVAL);
	}

	if (mutex->mtx_kind == MUTEX_RECURSIVE_NP)
		return (EINVAL);

	self = pthread_self();
	thread = pthread_id_lookup(self);

	if (mutex->mtx_kind != MUTEX_FAST_NP)
		if (pthread_mutex_getowner_np(mutex) != thread)
			return (EDEADLK);

	/* Different mutexes were supplied for concurrent pthread_cond_wait() or
	   pthread_cond_timedwait() operations on the same condition variable
	*/
	if (cond->mutex)
 	   if (cond->mutex != mutex)
		return (EINVAL);

	/* Lock the condition variable and the thread.
	 */
	_spin_lock(&cond->lock);

	_spin_lock(&thread->lock);

	/* See if the thread has a cancel pending.
	 * If so, unlock the condition variable and thread
	 * (in reverse order) before we check if we can
	 * act on the request.  We leave the mutex locked
	 * so it will be held when the cleanup routines
	 * are called.  If we return, we can't act on the
	 * cancellation, so relock the condition variable and
	 * thread and proceed.
	 */

	if ((thread->intr.pending == TRUE) &&
	    (thread->intr.state == PTHREAD_CANCEL_ENABLE)) {
		thread->cond_cancel = NULL; 
		_spin_unlock(&thread->lock);
		_spin_unlock(&cond->lock);

		/* the 3 following lines replace the pthread_testcancel */
		_spin_lock(&self->lock);
		_pthread_intr_detect(self);
		_spin_unlock(&self->lock);

		/* Does not reach here */
	}

	cond->mutex = mutex;
	cond->cptwait++;

	thread->cond_cancel = cond; 
	thread->mutex_cancel = mutex; 
	thread->join_cancel = 0;	/* for _pthread_unwind() the lock
					 * is a mutex
					*/ 

	_pthread_deactivate(thread, &cond->waiters);

	/* Unlock everything (in reverse order) before we
	 * wait for the event.
	 * thread->lock will be free by the thread_tsleep.
	 */
	_spin_unlock(&cond->lock);

	pthread_mutex_unlock(mutex);
	_pthread_event_wait(thread, &event, TRUE, timeout);
	pthread_mutex_lock(mutex);

	/* An event has arrived. Lock the condition and the thread
	 * before we look to see what it was.
	 */
	_spin_lock(&cond->lock);
	_spin_lock(&thread->lock);

	thread->cond_cancel = NULL; 
	thread->mutex_cancel = NULL; 

	if (event != EVT_SIGNAL) {	/* EVT_CANCEL,EVT_TIMEOUT,EVT_SIGPOST */
		if (event == EVT_SIGPOST) {
				/* for _pthread_event_notify/sigwait */
			thread->event |= EVT_SIGPOST;
			_pthread_activate(thread);
			if (cond->cptwait > 0) {
                        	cond->cptwait--;
				if (!cond->cptwait)
                            	cond->mutex = 0;
			}
			_spin_unlock(&thread->lock);
			_spin_unlock(&cond->lock);
			return (EINTR);
		}
		_pthread_activate(thread);
		if (cond->cptwait > 0) {
                        cond->cptwait--;
			if (!cond->cptwait)
                            cond->mutex = 0;
		}
	}

	async_cancel = thread->intr.pending
		       && thread->intr.state == PTHREAD_CANCEL_ENABLE
		       && thread->intr.mode == PTHREAD_CANCEL_ASYNCHRONOUS;

	if (event == EVT_CANCEL || async_cancel) {

		_spin_unlock(&thread->lock);
		_spin_unlock(&cond->lock);

		if (async_cancel)
			pthread_mutex_unlock(mutex);

		pthread_testcancel();
		INTERNAL_ERROR("pthread_cond_timedwait");
		/* NOTREACHED */
	} else if (event == EVT_TIMEOUT) {

		_spin_unlock(&thread->lock);
		_spin_unlock(&cond->lock);

		/* Return after a timeout. The mutex must be locked
		 * and we return ETIMEDOUT.
		 */
		return (ETIMEDOUT);
	}
#ifdef DEBUG_PTH
	else if (event != EVT_SIGNAL)
		INTERNAL_ERROR("pthread_cond_timedwait");
#endif	/* DEBUG_PTH */
	
	/* The thread was signalled. The thread is already back on
	 * the active list so the thread and condition are unlocked
	 * and the mutex is re locked for the caller. The only way this
	 * could fail is if the mutex had been deleted while the thread
	 * was waiting.
	 */
	_spin_unlock(&thread->lock);
	_spin_unlock(&cond->lock);
	return (0);
}


/*
 * Function:
 *	pthread_cond_signal
 *
 * Parameters:
 *	cond - the condition variable that is to be signalled.
 *
 * Return value:
 *	0	Success
 *	EINVAL	The pointer to the condition variable is invalid
 *		The condition variable is invalid
 *
 * Description:
 *	Scan through the waiter list of threads looking for one that
 *	has not been cancelled. The first one found is removed from the
 *	list, put on the active thread list and sent the signal event.
 *
 */
int
pthread_cond_signal(pthread_cond_t *cond)
{
	pthread_d	thread;
	pthread_d	favored_thread;

	PT_LOG(("pthread_cond_signal: cond = %x\n", cond));

	if ((cond == NO_COND) || !(cond->flags & COND_VALID)) {
		return (EINVAL);
	}

	/* Lock the condition as we are about to change the waiting queue.
	 */
	_spin_lock(&cond->lock);

	/* Step through every thread in the queue.
	 */
	  for (thread = (pthread_d)queue_head(&cond->waiters);
	     thread != (pthread_d)queue_end(&cond->waiters);
	     thread = (pthread_d)queue_next(&thread->link)) {

		/* Lock the thread before we look at it.
		 */
		_spin_lock(&thread->lock);

		/* If an event is sent then this is either a cancel or
		 * a cond signal. In either case we don't want to do it again
		 * and we continue looking.
		 */
		if (_pthread_event_notify(thread, EVT_SIGNAL)) {
			int do_twakeup = (thread->state & PTHREAD_WAITING);
			_pthread_activate(thread);
			_spin_unlock(&thread->lock);
			if (cond->cptwait > 0) {
                        	cond->cptwait--;
				if (!cond->cptwait)
                            	cond->mutex = 0;
			}
			_spin_unlock(&cond->lock);
			if (do_twakeup) 
				if (thread_twakeup(thread->vp->id, EVT_SIGNAL)){
						/* errno = ESRCH or EINVAL */
					INTERNAL_ERROR("pthread_cond_signal");
				}
			return(0);
		}
		_spin_unlock(&thread->lock);
	  }
	
	_spin_unlock(&cond->lock);
	return (0);
}


/*
 * Function:
 *	pthread_cond_broadcast
 *
 * Parameters:
 *	cond - the condition variable that is to be broadcast.
 *
 * Return value:
 *	0	Success
 *	EINVAL	The pointer to the condition variable is invalid
 *		The condition variable is invalid
 *
 * Description:
 *	Step through the list of waiting threads and send a signal
 *	event to every thread that has not already been cancelled.
 */
int
pthread_cond_broadcast(pthread_cond_t *cond)
{
	pthread_d	thread;
	pthread_d	next_thread;

	PT_LOG(("pthread_cond_broadcast: cond = %x\n", cond));

	if ((cond == NO_COND) || !(cond->flags & COND_VALID)) {
		return (EINVAL);
	}

	/* Lock the condition as we are about to change the waiting queue.
	 */
	_spin_lock(&cond->lock);

	cond->cptwait = 0;
	cond->mutex = 0;

	/* Step through every thread in the queue.
	 */
	for (thread = (pthread_d)queue_head(&cond->waiters);
	     thread != (pthread_d)queue_end(&cond->waiters);
	     thread = next_thread) {

		/* Lock the thread and take a note of its next pointer
		 * in case we remove it from the list.
		 */
		_spin_lock(&thread->lock);
		next_thread = (pthread_d)queue_next(&thread->link);

		/* If an event is sent then this is either a cancel or
		 * a cond signal. In either case we don't want to do it again
		 * and we continue looking.
		 */
		if (_pthread_event_notify(thread, EVT_SIGNAL)) {
			int do_twakeup = (thread->state & PTHREAD_WAITING);
			_pthread_activate(thread);
			_spin_unlock(&thread->lock);
			if (do_twakeup) 
				if (thread_twakeup(thread->vp->id, EVT_SIGNAL)){						/* errno = ESRCH or EINVAL */
					INTERNAL_ERROR("pthread_cond_signal");
				}
		} else {
			_spin_unlock(&thread->lock);
		}
	}
	_spin_unlock(&cond->lock);
	return (0);
}


static void
_release_delay_np(struct delay_args_np *args_np)
{
        pthread_mutex_unlock (args_np->delay_mutex_np);
	pthread_mutex_destroy(args_np->delay_mutex_np);
	pthread_cond_destroy(args_np->delay_cond_np);
}


/*
 * Function:
 *      pthread_delay_np
 *
 * Parameters:
 *	interval - pointer to the number of seconds that the calling thread 
 *		   waits before continuing execution.
 *
 * Return value:
 *	0	Success
 *	EINVAL  The value specified by interval is invalid.
 *
 * Description: Causes a thread to wait for a specified period
 */

int
pthread_delay_np(struct timespec *interval)
{
pthread_mutex_t cond_mutex;
pthread_cond_t  cond_var;
struct timespec abs;
struct delay_args_np args_np;
int ret;
pthread_d       self = pthread_id_lookup(pthread_self());

	if (interval == NULL)
		return(EINVAL);
	if ((interval->tv_sec < 0) || (interval->tv_nsec < 0))
		return(EINVAL);
	if (pthread_get_expiration_np(interval, &abs))
		return(EINVAL);
	pthread_mutex_init (&cond_mutex, &pthread_mutexattr_fast);
	pthread_cond_init (&cond_var, &pthread_condattr_default);
	args_np.delay_mutex_np = &cond_mutex;
	args_np.delay_cond_np = &cond_var;
        pthread_mutex_lock (&cond_mutex);
	_pthread_cleanup_push((pthread_push_t)_release_delay_np, &args_np,
						self);
	while (1) {
		ret = pthread_cond_timedwait(&cond_var, &cond_mutex, &abs);
		if (ret == ETIMEDOUT) {
			ret = 0;
			break;
		}
		if (ret == EINVAL) break;
	}
	_pthread_cleanup_pop(1, self);
	return(ret);
}


/*
 * Function:
 *      pthread_get_expiration_np
 *
 * Parameters:
 *	delta   - pointer to the number of seconds to add to the current system
 *		  time
 *
 *	abstime - Value representing the expiration time
 *
 * Return value:
 *	0	Success
 *	EINVAL  The value specified by delta is invalid.
 *
 * Description: Adds a specified interval to the current absolute system time
 *		and returns a new absolute time.
 */

int
pthread_get_expiration_np(struct timespec *delta, struct timespec *abstime)
{
struct timespec vnow;
struct timespec * now;

	if ((delta == NULL) || (abstime == NULL)) {
		return(EINVAL);
	}
	if ((delta->tv_sec < 0) || (delta->tv_nsec < 0))
		return(EINVAL);
	now = &vnow;
	getclock(TIMEOFDAY, now);
	abstime->tv_sec = now->tv_sec + delta->tv_sec;
	abstime->tv_nsec = now->tv_nsec + delta->tv_nsec;
	if (abstime->tv_nsec > (1000000000 - 1)) {
		abstime->tv_nsec -= 1000000000;
		abstime->tv_sec++;
	}
	return(0);
}


#ifdef DEBUG_PTH
/*
 * Function:
 *	dump_cond
 *
 * Parameters:
 *	cond - thre condition variable to be dumped
 *
 * Description:
 *	This is a debugging function which prints out the state of a condition
 *	variable and the threads waiting on it.
 */
void
dump_cond(cond)
pthread_cond_t	*cond;
{
	pthread_queue	*q;

	if ((cond == NO_COND) || !(cond->flags & COND_VALID)) {
		dbgPrintf("NO CONDITION\n");
		return;
	}
	dbgPrintf("CND: %x Wait:", cond);

	for (q = queue_head(&cond->waiters);
	     q != queue_end(&cond->waiters);
	     q = queue_next(q))
		dbgPrintf(" %x", q);
}
#endif	/* DEBUG_PTH */
