static char sccsid[] = "@(#)14	1.66  src/bos/usr/ccs/lib/libpthreads/pthread.c, libpth, bos41J, 9523B_all 6/6/95 09:55:26";
/*
 * COMPONENT_NAME: libpth
 * 
 * FUNCTIONS:
 *	pthread_alloc
 *	pthread_dealloc
 *	pthread_free
 *	pthread_fork_prepare
 *	pthread_fork_parent
 *	pthread_fork_child
 *	pthread_init
 *	pthread_create
 *	pthread_yield
 *	pthread_cleanup_unwind
 *	pthread_cleanup
 *	_pthread_body
 *	pthread_exit
 *	pthread_detach
 *	pthread_unjoin
 *	pthread_join
 *	pthread_once
 *	_pthread_deactivate
 *	_pthread_activate
 *	_pthread_event_wait
 *	_pthread_event_notify
 *	pthread_equal
 *	_pthread_internal_error
 *	_last_internal_error
 *	forkall
 *	dump_pthread_queue
 *	dump_pthread
 *	logPrintf
 *	dbgPrintf
 *	__funcblock_np
 *	pthread_setconcurrency_np
 *	pthread_getconcurrency_np
 *	pthread_getunique_np
 *	pthread_cleanup_push
 *	pthread_cleanup_pop
 *	pthread_test_exit_np
 *	pthread_clear_exit_np
 *	pthread_join_np
 *	pthread_cleanup_push_np
 *	pthread_cleanup_pop_np
 *	pthread_self
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
 * File: pthread.c
 *
 * This file contains all the functions to manipulate the pthreads themselves.
 * This include creation, deletion, caching, cancellation and fork.
 *
 * Pthreads map one to one with the underlying kernel threads (vp's). The
 * pthreads and the vp are bound for the life of the pthread (until the
 * thread returns or calls pthread_exit.
 */

#include "internal.h"
#include <signal.h>
#include <sys/priv.h>


/*
 * Local Variables
 */
private	spinlock_t	pthread_lock;
private	unsigned int	n_freepthreads;
private	pthread_queue	free_pthreads;
private	unsigned int	n_activepthreads;
private	unsigned int	n_runningpthreads;
private	pthread_d	all_pthreads;
pthread_queue   active_pthreads;

/*
 * Global Variables
 */
pthread_key_t	__pthread_cleanup_handlerqueue;
#ifdef DEBUG_PTH
#ifdef TRACE
int	pthread_trace = TRUE;
#else
int	pthread_trace = FALSE;
#endif
#endif
int th_ident = 0;
int attr_id;
spinlock_t		attr_id_lock;
extern	spinlock_t	cv_id_lock;
extern	spinlock_t	mtx_id_lock;
extern	pthread_queue	__dbx_known_pthreads;
extern pthread_queue	__dbx_known_attributes;
extern spinlock_t	dbx__attributes;
extern pthread_queue	__dbx_known_conditions;
extern spinlock_t	dbx__conditions;
extern	pthread_queue	__dbx_known_mutexes;
extern	spinlock_t	dbx__mutexes;
extern pthread_mutexattr_t		pthread_mutexattr_fast;
int			rootcheck;
struct unjoin_args
{
	pthread_d thread;
	pthread_d self;
};
extern int _cond_wait_join(pthread_cond_t *, spinlock_t *);
extern void _pthread_body(vp_t);


/*
 * Function:
 *	pthread_alloc
 *
 * Return value:
 *	NO_PTHREAD	if a pthread structure can't be allocated (errno is set)
 *	pointer to a pthread structure otherwise
 *
 * Description:
 *	Try to allocate a pthread structure. First look to see if a cached
 *	structure is available, if it is, reinitialise it and return. If not
 *	The allocate a new structure, with its mutex and condition variables,
 *	and then initialise that. The pthread struct returned is on the
 *	all_pthreads list.
 */
private pthread_d
pthread_alloc(void)
{
	static struct pthread	null_thread = { 0 };
	pthread_d		thread;

	thread = NO_PTHREAD;

	/* Only look for a cached thread is there is a chance that there might
	 * be one. This is a minor optimization for the case that there are
	 * no cached structures.
	 */
	if (n_freepthreads != 0) {

		/* n_freepthreads was non zero at one point but because we
		 * didn't have the lock we now have to check under the lock.
		 */
		_spin_lock(&pthread_lock);

		if (n_freepthreads != 0) {

			/* There is still at least one cached thread structure
			 * remove it from the free queue and drop the lock
			 * for the next allocation.
			 */
			thread = (pthread_d)queue_head(&free_pthreads);
			queue_remove(&thread->link);
			n_freepthreads--;
			_spin_unlock(&pthread_lock);

			/* We now have our used structure with its lock and
			 * condition variable. Re-initialise everything to look
			 * like a new structure.
			 */
			_spinlock_create(&thread->lock);
			_initialize_condition(&thread->done,
					      pthread_condattr_default);

			thread->state = 0;
			thread->flags = 0;
			thread->join_count = 0;

		} else {

			/* There are no free threads when we looked a second
			 * time so we free the lock and carry on to allocate
			 * a structure from scratch.
			 */
			_spin_unlock(&pthread_lock);
		}
	}

	/* Check we haven't got a thread structure yet. If not we have to
	 * make one from scratch.
	 */
	if (thread == NO_PTHREAD) {

		/* Allocate and initialise the structure itself. If this
		 * fails then the caller is informed.
		 */
		thread = (pthread_d)_pmalloc(sizeof(struct pthread));
		if (thread == NO_PTHREAD) {
			return (NO_PTHREAD);
		}
		*thread = null_thread;

		/* create lock to protect updates to the thread structure.
		 */
		_spinlock_create(&thread->lock);

		/* Now allocate the condition variable used for join.
		 * If this fails, the thread structure is freed, the 
		 * mutex deleted and the caller is informed.
		 */
		if (pthread_cond_init(&thread->done,
				      &pthread_condattr_default) != 0) {
			_spinlock_delete(&thread->lock);
			_pfree(thread->attr);
			_pfree(thread);
			return (NO_PTHREAD);
		}

		/* Put thread on the all_pthread list.
		 */
		_spin_lock(&pthread_lock);
		thread->all_thread_link = all_pthreads;
		all_pthreads = thread;
		_spin_unlock(&pthread_lock);
	}

	/* Success, the allocated structure is returned.
	 * The structure is on the all_pthreads list.
	 */
	return (thread);
}


/*
 * Function:
 *	pthread_dealloc
 *
 * Description:
 *	Performs the inverse function to pthread_alloc. The structure is not
 *	cached and the mutex and condition variable are deleted. This function
 *	assumes that the thread in not on any queue.
 */
private void
pthread_dealloc(pthread_d thread)
{
	_spinlock_delete(&thread->lock);
	_spinlock_delete(&thread->done.lock);
	pthread_cond_destroy(&thread->done);
	_pfree(thread->attr);
	_pfree(thread);
}


/*
 * Function:
 *	pthread_free
 *
 * Parameters:
 *	thread	- the thread structure to free
 *
 * Description:
 *	Free a thread previously allocated using pthread_alloc. Unlike
 *	pthread_dealloc, the thread structure is cached and it is
 *	assumed that the structure is on the active list.
 */
private void
pthread_free(pthread_d thread)
{
pthread_cond_t	*cond;

	/* Lock the active and free queues.
	 */
	_spin_lock(&pthread_lock);

	/* Free the attribute of the thread and the attribute of the condition
	 * of the thread.
	 */
	_pfree(thread->attr);
	cond = &thread->done;
	_pfree(cond->attr);

	/* Take off the active list
	 */
	n_activepthreads--;
	queue_remove(&thread->link);
	queue_remove(&thread->DBXlink);

	/* Put on the free queue.
	 */
	queue_append(&free_pthreads, &thread->link);
	n_freepthreads++;

	/* Drop the queue lock.
	 */
	_spin_unlock(&pthread_lock);
}


/*
 * Function:
 *	pthread_fork_prepare
 *
 * Description:
 *	Quiesce the threads package prior to a fork. This makes it easier
 *	to clean up in the child after the fork has completed.
 *	Should be done first.
 */
private void
pthread_fork_prepare(void)
{
	pthread_d	self;

	self = pthread_id_lookup(pthread_self());
	_spin_lock(&self->lock);

	_spin_lock(&pthread_lock);
}


/*
 * Function:
 *	pthread_fork_parent
 *
 * Description:
 *	This is called in the parent after a fork. All components are set
 *	free so the parent can continue to run as before. Components are
 *	released in the reverse order that they were frozen to avoid deadlock.
 *	Should be done last.
 */
private void
pthread_fork_parent(void)
{
	pthread_d	self;

	_spin_unlock(&pthread_lock);

	self = pthread_id_lookup(pthread_self());
	_spin_unlock(&self->lock);
}


/*
 * Function:
 *	pthread_fork_child
 *
 * Description:
 *	This is called in the child process after a fork. Clean up all the
 *	global data in all the subsystems and make it look like the world
 *	just after pthread_init returns. There is only one thread (and one
 *	vp therefore) running.
 *	Should be done last.
 */
private void
pthread_fork_child(void)
{
	pthread_d	self;
	pthread_d	thread;
        pthread_cleanup_handler_t       *handler;
	int		parent_errno;

	_spin_unlock(&pthread_lock);

	/* self will still give the same as the parent as the stack is at the
	 * same address.
	 */
	self = pthread_id_lookup(pthread_self());
	parent_errno = errno;
	{
#ifdef  errno
#undef  errno
#endif  /* errno */
	extern int      errno;

	errno = parent_errno;
	errnop = self->thread_errno = &errno;
	self->flags |= PTHREAD_INITIAL_THREAD;
	}
	
	_spin_unlock(&self->lock);

	/* Go through the list of all threads and free up all the resources
	 * they are using.
	 */

	_spinlock_delete(&dbx__conditions);
	_spinlock_delete(&dbx__mutexes);
	_spinlock_delete(&dbx__attributes);
	_spinlock_delete(&attr_id_lock);
	_spinlock_delete(&cv_id_lock);
	_spinlock_delete(&mtx_id_lock);

	while (all_pthreads != NULL) {
		thread = all_pthreads;
		all_pthreads = thread->all_thread_link;

		/* Dequeue the thread regardless of where is is linked to,
		 * the active queue, free queue or a condition waiters queue
		 * The important one is the condition queues. This is because
		 * we don't want non-existent pthread handles attached to a
		 * application created condition variable after the fork.
		 */

		if (thread != self)
			pthread_dealloc(thread);
	}
	self->all_thread_link = NULL;
	all_pthreads = self;

	/* Remove all the dead vps and their stacks.
	 * Can't do this earlier because we cleaned up all queues
	 * which may be on those stacks.
	 */
	_vp_prune();

	attr_id = 0;
	queue_init(&__dbx_known_attributes);
	queue_init(&__dbx_known_conditions);
	queue_init(&__dbx_known_mutexes);

	/* Initialise queues.
	 */
	n_freepthreads = 0;
	queue_init(&free_pthreads);
	queue_init(&active_pthreads);
	queue_init(&__dbx_known_pthreads);
	queue_append(&active_pthreads, &(self)->link);
	queue_append(&__dbx_known_pthreads, &(self)->DBXlink);
	n_activepthreads = 1;
	n_runningpthreads = 1;
	pid = getpid();
        /* free the  memory of handler */
        while ((handler = self->cleanup_queue) != NULL) {

                /* Remove the handler from the stack...
                 */
                self->cleanup_queue = handler->__next_handler;
                _pfree(handler);
        } /* while */
        while ((handler = self->handler_pool) != NULL) {

                /* Remove the handler from the stack...
                 */
                self->handler_pool = handler->__next_handler;

                _pfree(handler);
        }

}


/*
 * Function:
 *	pthread_init
 *
 * Description:
 *	Initialization function for the whole pthread package. Set up
 *	all the thread global data and locks and then call the initialization
 * 	functions for mutexes, condition variables, thread attributes, specific
 *	data, signal handling and vp and stack management.
 *
 *	Next the initial thread is created. This function is called from crt0
 *	so this flow of execution is the initial thread by the time main
 *	gets called. To create the initial thread, much of the code in thread
 *	create is duplicated.
 *
 *	Note:	Function assumes that the first call is made when single
 *		threaded so checking pthreads_started does not require locking.
 */
void
pthread_init(void)
{
	static volatile int	pthreads_started = FALSE;
	pthread_d	self;
	vp_t		vp;


	/* Ensure that this is only executed once regardless of how many times
	 * it is called.
	 */
	if (pthreads_started)
		return;
	attr_id = 0;		/* For DBX */
	__page_size = sysconf(_SC_PAGE_SIZE);
	__page_size_K = __page_size / 1024;
	__page_sizeX24 = __page_size * 24;
	__page_sizeX16 = __page_size * 16;
	__page_sizeM1 = __page_size - 1;

	rootcheck = privcheck(SET_PROC_RAC);

	NBCPU	= (int)sysconf(_SC_NPROCESSORS_CONF);
	if (NBCPU > 1) {			/* for mutex and spin locks */
		YIELDLOOPTIME = YIELDTIME;
		TICKLOOPTIME = 600;
	} else {
		YIELDLOOPTIME = 0;
		TICKLOOPTIME = 1;
	}

	_spinlock_create(&pthread_lock);
	_spinlock_create(&attr_id_lock);

	_spinlock_create(&dbx__attributes);
	_spinlock_create(&dbx__conditions);
	_spinlock_create(&dbx__mutexes);

	pthreads_started = TRUE;
	queue_init(&__dbx_known_attributes);
	queue_init(&__dbx_known_conditions);
	queue_init(&__dbx_known_mutexes);

	/* Initialise the active and free queues and the queue of all threads
	 * in existence.
	 */
	n_freepthreads = 0;
	queue_init(&free_pthreads);
	n_activepthreads = 0;
	queue_init(&active_pthreads);
	queue_init(&__dbx_known_pthreads);
	n_runningpthreads = 0;
	all_pthreads = NULL;

	/* Initialise all the other threads components.
	 *
	 * Note:
	 *	A startup call will set up any fork handlers for
	 *	the component. The order in which these are executed
	 *	is important to avoid deadlocks.
	 *
	 * These routines will register the fork handlers.
	 * The pre-fork handlers called in LIFO so lower level startup
	 * routines are called first.
	 * The post-fork handlers are taken in FILO order.
	 */
	_pthread_fork_startup();
	_pthread_malloc_startup();
	_pthread_mutex_startup();
	_pthread_cond_startup();

	vp = _pthread_vp_startup();
	_pthread_stack_startup(vp);

	_pthread_attr_startup();		/* after stack_startup() */
	_pthread_specific_startup();


	if (pthread_atfork(pthread_fork_prepare,
			      pthread_fork_parent,
			      pthread_fork_child))
		INTERNAL_ERROR("pthread_init");

	_pthread_sigwait_startup();

	/* Try to create the initial thread. Allocate the thread structure
	 * but do not create a vp as we are already executing in one,
	 * vp_init has returned a vp id to describe the initial vp instead.
	 */
	self = pthread_alloc();
	if (self == NO_PTHREAD)
		INTERNAL_ERROR("pthread_init");

	/* Note that we are the initial thread.
	 */
	if (pthread_attr_init(&self->attr))
		INTERNAL_ERROR("pthread_init");
	self->flags |= PTHREAD_INITIAL_THREAD;
	self->func = NILFUNC(void *);
	self->arg = 0;
	self->intr.mode = PTHREAD_CANCEL_DEFERRED;
	self->intr.state = PTHREAD_CANCEL_ENABLE;
	self->intr.pending = FALSE;
	self->th_id = ++th_ident;		/* for dbx */
	self->ti_hold = 0;

	/* The scope of the initial thread is PTHREAD_SCOPE_SYSTEM
	*/
	self->attr->contentionscope = PTHREAD_SCOPE_SYSTEM;

        /* The initial thread is created UNDETACHED
	*/
	self->attr->detachstate = PTHREAD_CREATE_UNDETACHED;

	/* Put the initial thread on the active queue.
	 */
	queue_append(&active_pthreads, &(self)->link);
	queue_append(&__dbx_known_pthreads, &(self)->DBXlink);
	n_activepthreads++;
	n_runningpthreads++;

	_vp_bind(vp, self);

	if (thread_setsched(self->vp->id, 
			self->attr->schedule.sched_priority, 
			self->attr->schedule.sched_policy, (int)vp)) {
		     INTERNAL_ERROR("pthread_init (thread_setsched)");
	}

	_specific_data_setup_initial(self);

	__key_create_internal(&__pthread_cleanup_handlerqueue, NILFUNC(void));
	_pthread_setspecific(self, __pthread_cleanup_handlerqueue,
			    (void *)&self->cleanup_queue);

	self->cleanup_queue = NULL;
        self->handler_pool = NULL;
	_pthread_libs_init(self);
	pid = getpid();

}


/*
 * Create the function pointer to pthread_init so that crt0 can find us.
 */
pthread_func_t _pthread_init_routine = (pthread_func_t)pthread_init;

/*
 * Function:
 *	pthread_create
 *
 * Parameters:
 *	thread	- pointer to the place to store the new pthread id
 *	attr	- pointer to the attributes of the newly created 
 *		  thread
 *	start_routine - Function that the new thread is to execute
 *	arg	- parameter to be passed to the start routine
 *
 * Return value:
 *	0	Success
 *	EINVAL	if the thread is an invalid pointer
 *		if the attribute pointer is invalid
 *	EAGAIN	If no thread structure could be allocated
 *		if no vp could be created for the thread to execute on
 *	EPERM	The caller does not have the appropriate permission to set
 *		the scheduling policy.
 *
 * Description:
 *	Create a new thread of execution. Create a thread structure and a
 *	vp and bind them together. The stack is associated with the vp rather
 *	than the pthread, which is probably a mistake, which is why the
 *	specific data must be set up here. The completely created pthread
 *	is then put on the active list before it is allowed to execute.
 */
int
pthread_create(pthread_t *th_id, const pthread_attr_t *attrp,
		  pthread_func_t start_routine, void *arg)
{
	vp_t		vp;
	pthread_d	thread;
	pthread_attr_t	attr;

	if (attrp == NULL)
            attr = pthread_attr_default;
	else
	    attr = *attrp;

	PT_LOG(("pthread_create\n", NULL));

	if ((attr == NO_ATTRIBUTE) || !(attr->flags & ATTRIBUTE_VALID) ||
	    (th_id == NULL)) {
		return (EINVAL);
	}
	if ((attr->schedule.sched_policy == SCHED_FIFO) ||
		(attr->schedule.sched_policy == SCHED_RR)) {
		if (rootcheck == EPERM)
			return (EPERM);
	}

	/* Allocate a thread structure and an attribute.
	 */
	thread = pthread_alloc();
	if (thread == NO_PTHREAD)
		return (EAGAIN);
	thread->ti_hold = 0;

	thread->attr = (pthread_attr_t)_pmalloc(sizeof(struct pthread_attr));
	if (thread->attr == NULL)
		return (EAGAIN);
	thread->attr->detachstate = attr->detachstate;
	thread->attr->inherit = attr->inherit;
	thread->attr->contentionscope = attr->contentionscope;
	thread->attr->schedule.sched_policy = attr->schedule.sched_policy;
	thread->attr->schedule.sched_priority = attr->schedule.sched_priority;

	/* Set up the defaults for cancellation.
	 */
	thread->intr.mode = PTHREAD_CANCEL_DEFERRED;
	thread->intr.state = PTHREAD_CANCEL_ENABLE;
	thread->intr.pending = FALSE;

	/* Create a vp for the thread to execute on.
	 */
	if (attrp == NULL)
        	vp = _vp_create(&pthread_attr_default);
	else
        	vp = _vp_create(attrp);
	if (vp == NO_VP) {
		pthread_dealloc(thread);
		return (EAGAIN);
	}

	/* Save the threads function information used by vp_resume.
	 */
	thread->func = start_routine;
	thread->arg = arg;

	PT_LOG(("pthread_create: new thread = %x\n", thread));

	_spin_lock(&pthread_lock);

	thread->th_id = ++th_ident;		/* for dbx */

	if (!(thread->state & PTHREAD_DETACHED))
            if (thread->attr->detachstate == PTHREAD_CREATE_DETACHED)
                thread->state |= PTHREAD_DETACHED;

	/* Put the new thread on the active queue.
	 */
	queue_append(&active_pthreads, &thread->link);
	queue_append(&__dbx_known_pthreads, &thread->DBXlink);
	n_activepthreads++;
	n_runningpthreads++;

	thread->cleanup_queue = NULL;
        thread->handler_pool = NULL;

	_vp_bind(vp, thread);

	/*
 	 * create the specific data area for a newly created thread. Find the
	 * specific data area beyond the cancel stack and zero it. 
	 * The PTHREAD_DATA page is used for thread specific data.
	 * The thread specific data is located at the top of the mmap area.
	 * see new_stack in stack.c
	 * Set the pointer in the pthread structure to point at this area.
	 */
	thread->specific_data =
			(specific_data_t *) thread->vp->specific_data_address;
	memset((void *)thread->specific_data, 0, PTHREAD_DATA);

	_spin_unlock(&pthread_lock);

	*th_id = pthread_id_create(thread);

	/* Set the signal mask of the calling thread in the vp 
	 * done in _vp_setup (called by _vp_resume). 
	 */

	/* Start the new thread executing.
	 */
	_vp_resume(vp);
	return (0);
}


/*
 * Function:
 *	pthread_yield
 *
 * Description:
 *	yield the cpu to another deserving thread. As there is a 1-1 mapping
 *	of pthreads to vps, we let the vp layer do the work.
 */
void
pthread_yield(void)
{
	PT_LOG(("pthread_yield\n", NULL));
	yield();
}


/*
 * Function:
 *	pthread_cleanup_unwind
 *
 * Parameters:
 *	self	- The pthread id of the calling thread
 *
 * Description:
 *	All the cleanup handlers that this thread has pushed into the
 *	cleanup stack are popped off and executed.
 */
private void
pthread_cleanup_unwind(pthread_d self)
{
	pthread_cleanup_handler_t	*handler;


	if ( self->cleanup_queue) {

	/*
	 *	This code reacquires the mutex, for the
	 *	cleanup handlers which was released by
	 *		pthread_cond_wait
	 *		pthread_cond_timedwait
	 *	which is required by POSIX
	 *
	 *	The self->lock does not have to be acquired
	 *		the only functions using
	 *			self->cond_cancel
	 *			self->mutex_cancel
	 *		are
	 *			pthread_cond_wait
	 *			pthread_cond_timedwait
	 *			pthread_cleanup_unwind
	 *		which can only be executed by the running
	 *		thread, so the thread serializes access to the fields
	 *
	 *	This is a DEPENDENCY
	 */
	if (self->cond_cancel) {
	    _spin_lock(&self->lock);
	    _spin_lock(&self->cond_cancel->lock);

	    /* reaquire mutex of pthread_cond_wait or pthread_condtimed_wait */
	    if (self->join_cancel) {
		_spin_lock((spinlock_t *)self->mutex_cancel);
	    } else
		pthread_mutex_lock(self->mutex_cancel);
	    _pthread_activate(self);

	    _spin_unlock(&self->cond_cancel->lock);
	    /*
	     *  The one-time condition has been satisfied
	     *  and MUST be reset to prevent accidental
	     *  reacquiring the mutex and cond, if
	     *  recursively called through
	     *  pthread_exit() in a cleanup handler
	     */
	    self->cond_cancel = NULL;
	    self->mutex_cancel = NULL;
	    _spin_unlock(&self->lock);
	}


	while ((handler = self->cleanup_queue) != NULL) {

		/* Remove the handler from the stack...
		 */
		self->cleanup_queue = handler->__next_handler;
                /* put it the free list of the thread */
                handler->__next_handler = self->handler_pool;
                self->handler_pool = handler;
		/* ...and call it.
		 */
		(*handler->__handler_function)(handler->__handler_arg);
        } /* while */
        } /* if */
        while ((handler = self->handler_pool) != NULL) {

                /* Remove the handler from the stack...
                 */
                self->handler_pool = handler->__next_handler;

                _pfree(handler);
        }
}


/*
 * Function:
 *	pthread_cleanup
 *
 * Parameters:
 *	self	- The pthread id of the calling thread
 *
 * Description:
 *	Cleanup a thread after it has exited. If this is the only thread
 *	left then exit the process. If not and there are other threads
 *	joining with this one then wake them up. If the thread has been
 *	detached then all the resources can be freed.
 */
private void
pthread_cleanup(pthread_d thread)
{
	vp_t	vp;

	_spin_lock(&thread->lock);

	/* Disable cancellation.
	 * Set that the pthread_exit is running
	*/
	thread->state |= PTHREAD_EXITED;

	if (thread->intr.pending == TRUE) {
		thread->intr.pending = FALSE;
		if (thread->intr.mode == PTHREAD_CANCEL_DEFERRED) {
 				/* case DEFERRED without cancellation point */
				/* thread_tsleep frees the lock */
			thread_tsleep(1, (atomic_p)&thread->lock, NULL);
				/* NOTREACHED if case DEFERRED without 
		         	* cancellation point
				*/
		} else {
			_spin_unlock(&thread->lock);
		}
	} else {
		_spin_unlock(&thread->lock);
	}

	/* Call any cleanup handlers.
	 */
	pthread_cleanup_unwind(thread);

	/* Call any specific data destructors for thread data.
	 */
	_specific_data_cleanup(thread);

	/* Exit if we are the last thread left.
	 */
	_spin_lock(&pthread_lock);
	if (--n_runningpthreads == 0) {
		_spin_unlock(&pthread_lock);
		exit(thread->returned);
	}

	_spin_unlock(&pthread_lock);

	vp = thread->vp;

	/* Lock the thread structure.
	 */
	_spin_lock(&thread->lock);

	thread->vp = NO_VP;
	if ((thread->state & PTHREAD_DETACHED) && (thread->join_count == 0)) {

		/* The thread is detached and there are no other threads
		 * waiting for this thread so we can simply free up our
		 * resources.
		 */
		_spin_unlock(&thread->lock);
		pthread_free(thread);

	} else {

		/* This thread is either not detached or is detached with
		 * other threads waiting. Note that the thread status is
		 * valid and then signal the waiting threads.
		 */
		thread->state |= PTHREAD_RETURNED;
		if (thread->join_count > 0)
			pthread_cond_broadcast(&thread->done);
		_spin_unlock(&thread->lock);
	}
	/* The thread function is complete suspend the vp for reuse by another
	 * thread.
	*/
	_vp_suspend(vp);
	if (thread->flags & PTHREAD_INITIAL_THREAD) {

		/* There is no jmpbuf set as this is the main() thread.
		 * Suspend ourselves. If we get resumed we call _pthread_body
		 * and pretend that this is a normal thread create.
		*/
		_pthread_body(vp);
	} else
		_longjmp(vp->exit_jmp, 1);

}


/*
 * Function:
 *	_pthread_body
 *
 * Parameters:
 *	vp	- the vp id of the calling thread
 *
 * Description:
 *	This is the function that every thread is created executing. It
 *	calls the start_routine specified to pthread create. As the vp
 *	being used can be cached and reused then this loop may be
 *	executed for a number of different pthreads.
 */
void
_pthread_body(vp_t vp)
{
	pthread_d	thread;

	for (;;) {
		PT_LOG(("_pthread_body: vp = %x\n", vp));

		/* Set the signal mask of the calling thread
		 */
		sigthreadmask(SIG_SETMASK, &vp->mask, NULL);

		/* Find out which thread we are this time round the loop.
		 */
		thread = vp->pthread;

		/* Set up the thread specific cleanup queue for cancellation.
		 */
		_pthread_setspecific(thread, __pthread_cleanup_handlerqueue,
					(void *)&thread->cleanup_queue);
		thread->cleanup_queue = NULL;
		thread->handler_pool = NULL;
		/* We have to be able to be to cope with both a pthread_exit
		 * or a return from this function. Pthread_exit will longjmp
		 * back here.
		 */
		if (_setjmp(vp->exit_jmp) == 0) {
			thread->returned = (*(thread->func))(thread->arg);

			/* The thread function simply returned.
			 * Clean the thread up since pthread_exit was not
			 * called.
			 */
			pthread_cleanup(thread);
		}

		PT_LOG(("_pthread_body: returned = %x\n", thread->returned));

		/* pthread_cleanup() function done _vp_suspend
		 */

		/* We are a new pthread at this point so we go round the loop
		 * again. Note that the vp and the stack are the same but
		 * the vp->pthread and therefore thread->func are different.
		 */
	}
}


void
_pthread_cleanup_push(pthread_push_t func, void * arg, pthread_d self)
{
        __pthread_cleanup_handler_t     *handler;

        /* if there is a handler in the free list : get it */
        if (self->handler_pool) {
           handler = self->handler_pool;
           self->handler_pool = self->handler_pool->__next_handler;
        } else {
           handler = (__pthread_cleanup_handler_t *) _pmalloc
                     (sizeof(struct __pthread_cleanup_handler_t));
           if (handler == 0){
              LAST_INTERNAL_ERROR("pthread_cleanup_push_np: memory saturation");
           }
        }
        handler->__handler_function = (void (*)())func;
        handler->__handler_arg = arg;
        handler->__next_handler = self->cleanup_queue;
        self->cleanup_queue = handler;
}

void
_pthread_cleanup_pop(int execute, pthread_d self)
{
        __pthread_cleanup_handler_t     *handler;

        handler = self->cleanup_queue;
        if (handler) {
           self->cleanup_queue = self->cleanup_queue->__next_handler;
           /* put it the free list of the thread */
           handler->__next_handler = self->handler_pool;
           self->handler_pool = handler;
           if (execute)
              (*handler->__handler_function)(handler->__handler_arg);
        }
}

/*
 * Function:
 *	pthread_exit
 *
 * Parameters:
 *	status - exit status of the thread
 *
 * Description:
 *	Save the exit status of the thread so that other threads joining
 *	with this thread can find it. If this is the initial thread (ie
 *	using the starting threads stack) then we can't longjmp as the
 *	thread was not called via _pthread_body so we fix it and cleanup
 *	as if it was. Otherwise just jump back as if the thread function
 *	returned (see _pthread_body).
 */
void
pthread_exit(void * status)
{
	pthread_d	thread;

	PT_LOG(("pthread_exit: status = %d\n", status));

	thread = pthread_id_lookup(pthread_self());

	thread->returned = status;

	/* There should be no more references to this pthread beyond
	 * this point until it is reallocated.
	 */
	pthread_cleanup(thread);
}


/*
 * Function:
 *      pthread_detach
 *
 * Parameters:
 *      thread - the thread to be detached
 *
 * Return value:
 *      0       Success
 *      EINVAL  if the thread id is invalid
 *      ESRCH   if the thread is already detached
 *
 * Description:
 *      Detaching a running thread simply consists of marking it as such.
 *      If the thread has returned then the resources are also freed.
 */
int
pthread_detach(pthread_t thread)
{
        pthread_d       pthread;

        if (thread == BAD_PTHREAD_ID) {
                return (EINVAL);
        }

        pthread = pthread_id_lookup(thread);

        PT_LOG(("pthread_detach: thread = %x\n", pthread));

        /* Lock the thread we are detaching.
         */
        _spin_lock(&pthread->lock);

        /* Check we are not detaching a detached thread.
         */
        if (pthread->state & PTHREAD_DETACHED) {
                _spin_unlock(&pthread->lock);
                return (ESRCH);
        }

        /* Invalidate the callers handle.
         */
        thread = BAD_PTHREAD_ID;

        /* Mark the thread detached.
         */
        pthread->state |= PTHREAD_DETACHED;

        if (pthread->state & PTHREAD_RETURNED && pthread->join_count == 0) {

                /* The thread is no longer executing and there is
                 * no-one joining with it so we can free the resources.
                 */
                _spin_unlock(&pthread->lock);
                pthread_free(pthread);
        } else {
                _spin_unlock(&pthread->lock);
	}

        return (0);
}


/*
 * Function:
 *	pthread_unjoin
 *
 * Parameters:
 *	thread - the thread that has been joined
 *
 * Description:
 *	Negate the effect of a join. This is used when a thread is cancelled
 *	when joining another thread. The thread would have been locked for
 *	us again so the join count is decremented. If this thread was the
 *	only joiner and the thread has detached then it is freed.
 */
private void
pthread_unjoin(struct unjoin_args *unjoin_str)
{
pthread_d	self = unjoin_str->self;
pthread_d	thread = unjoin_str->thread;

/* The thread has not to wait more than the other wakeup it */
        if (self->state & PTHREAD_WAITING) {
		self->state &= ~PTHREAD_WAITING;
		self->state &= ~PTHREAD_EVENT;
	}
	if ((--thread->join_count == 0) && (thread->state & PTHREAD_DETACHED)) {
		_spin_unlock(&thread->lock);
		pthread_free(thread);
	} else {
		_spin_unlock(&thread->lock);
	}
}


/*
 * Function:
 *	pthread_join
 *
 * Parameters:
 *	thread - The id of the thread to be waited for
 *	status - pointer to a place to store the target threads exit status
 *
 * Return value:
 *	0	Success
 *	EINVAL	if thread is an invalid pointer
 *	EDEADLK	if thread is the calling thread
 *	ESRCH	if the target thread is detached
 *
 * Description:
 *	Wait for a thread to exit. If the status parameter is non-NULL then
 *	that threads exit status is stored in it.
 */
int
pthread_join(pthread_t th_id, void **status)
{
	pthread_d	thread;
	pthread_d       self = pthread_id_lookup(pthread_self());
	struct unjoin_args unjoin_str;

	thread = pthread_id_lookup(th_id);

	PT_LOG(("pthread_join: thread = %x\n", thread));

	/* Check the target thread is specified.
	 */
	if (thread == NO_PTHREAD) {
		return (EINVAL);
	}

	/* We cannot wait for ourselves.
	 */
	if (thread == self) {
		return (EDEADLK);
	}

	_spin_lock(&thread->lock);

	/* We cannot wait for a detached thread.
	 */
	if (thread->state & PTHREAD_DETACHED) {
		_spin_unlock(&thread->lock);
		return (ESRCH);
	}

	/* Note that we are joining with this thread.
	 */
	thread->join_count++;

	/* Prepare for cancellation as pthread_cond_wait is a cancellation
	 * point.
	 */
	unjoin_str.thread = thread;
	unjoin_str.self = self;
	_pthread_cleanup_push((pthread_push_t)pthread_unjoin, 
						(void *)&unjoin_str, self);

	/* Wait for the thread to exit.
	 */
	if (!(thread->state & PTHREAD_RETURNED))
		while (_cond_wait_join(&thread->done, &thread->lock) == EINTR){}

	/* Pop the cleanup handler.
	 */
	_pthread_cleanup_pop(FALSE, self);

	/* Save the exit status if it is wanted.
	 */
	if (status != NULL)
		*status = thread->returned;

	/* Note that we have joined. If this means that no-one else is
	 * waiting to join and no-one else can join as the thread is
	 * detached then we can free the threads resources.
	 */
	thread->join_count--;

        /* Mark the thread detached.
         */
        thread->state |= PTHREAD_DETACHED;

        if (thread->join_count == 0) {

                /* The thread is no longer executing and there is
                 * no-one joining with it so we can free the resources.
                 */
		_spin_unlock(&thread->lock);
		pthread_free(thread);
        } else {
                _spin_unlock(&thread->lock);
	}

	return (0);
}


/*
 * Function:
 *	pthread_once
 *
 * Parameters:
 *	once_block - determines if whether the init_routine has been called
 *	init_routine - The initialization routine to be called
 *
 * Return value:
 *	0		Success
 *	EINVAL		if once_block is an invalid pointer
 *			if init_routine is an invalid pointer
 *	INTERNAL_ERROR	if a mutex cannot be allocated for the once_block
 *			if a condition variable cannot be allocated for the 
 *			once_block
 *
 * Description:
 *	This function ensures that the init_routine is called only once and
 *	that no thread returns from this function until the function has
 *	has returned.
 *
 *	The once block is a skeleton initially and the first caller fills
 *	the rest of the block in with the mutex and condition variable. this
 *	is done as mutexes and condition variables cannot be statically
 *	initialised.
 */
int
pthread_once(pthread_once_t *once_block, void (*init_routine)(void))
{
pthread_d       self;

	PT_LOG(("pthread_once: once_block = %x\n", once_block));

	/* Check the pointers were have been given are OK.
	 */
	if ((once_block == NULL) || (init_routine == NILFUNC(void))) {
		return (EINVAL);
	}

	/* Check if the once block has been initialised.
	 */
	if (once_block->initialized == FALSE) {
	
		/* We think the block needs initializing, check again
		 * under the lock to make sure.
		 */
		_spin_lock(&once_block->lock);
		if (once_block->initialized == FALSE) {

			/* This is the first time through. We need to create
			 * a mutex and a condition variable for the once block.
			 */
			if (!(once_block->mutex.flags & MUTEX_VALID)) {
				if (pthread_mutex_init(&once_block->mutex,
					     &pthread_mutexattr_fast)) {
					_spin_unlock(&once_block->lock);
					INTERNAL_ERROR("pthread_once: mutex");
				}
			}

			/* Got the mutex, now get the condition variable.
			 */
			if (!(once_block->executed.flags & COND_VALID)) {
				if (pthread_cond_init( &once_block->executed,
					      &pthread_condattr_default)) {
					_spin_unlock(&once_block->lock);
					INTERNAL_ERROR("pthread_once: cond");
				}
			}

			/* The once block is now complete.
			 */
			once_block->initialized = TRUE;
		}
		_spin_unlock(&once_block->lock);
	}

	if (once_block->completed == FALSE) {
		pthread_mutex_lock(&once_block->mutex);

		/* Check to see what state the initalization routine is in.
		 */
		if (once_block->completed != FALSE)
			pthread_mutex_unlock(&once_block->mutex);
		else {
			if (once_block->executing == TRUE) {

				self = pthread_id_lookup(pthread_self());
				/* The initalization routine is executing so
				 * we wait on the condition variable until
				 * it is done.
				 */
				_pthread_cleanup_push((pthread_push_t)
						pthread_mutex_unlock,
						(void *)&once_block->mutex,
						self);
				do {
					pthread_cond_wait(&once_block->executed,
							&once_block->mutex);
				} while (!once_block->completed);
				_pthread_cleanup_pop(TRUE, self);
			} else {

				/* The initalization routine has not been
				 * called yet. Mark the once block as
				 * executing and call the function.
				 */
				once_block->executing = TRUE;
				pthread_mutex_unlock(&once_block->mutex);

				(*init_routine)();

				/* We can now update the once block to show
				 * the initialization is complete and signal
				 * anyone who was waiting.
				 */
				pthread_mutex_lock(&once_block->mutex);
				once_block->executing = FALSE;
				once_block->completed = TRUE;
				pthread_mutex_unlock(&once_block->mutex);
				pthread_cond_broadcast(&once_block->executed);
			}
		}
	}
	return (0);
}


/*
 * Function:
 *	_pthread_deactivate
 *
 * Parameters:
 *	thread		- the thread to deactivate
 *	new_queue	- queue to place thread on
 *
 * Description:
 *	The action of deactivation removes the thread from the active
 *	queue and moves it to the wait queue supplied by the caller.
 *	We also mark that we are about to call _pthread_event_wait by
 *	setting PTHREAD_ABOUT_TO_WAIT.
 */
void
_pthread_deactivate(pthread_t th_id, pthread_queue *new_queue)
{
	pthread_d	thread;
	pthread_queue *next;
	pthread_queue *current;
	pthread_d cur_p;
	pthread_d next_p;
#define AttrPolicy(a)      ((a)->attr->schedule.sched_policy)
#define AttrPriority(a)    ((a)->attr->schedule.sched_priority)
#define IsFR(a)  (AttrPriority(a) == SCHED_FIFO || AttrPriority(a) == SCHED_RR)

	thread = pthread_id_lookup(th_id);

	/* The lock protecting the thread and the new queue must be
	 * held by the caller.
	 */
	_spin_lock(&pthread_lock);
	n_activepthreads--;
	queue_remove(&thread->link);

	/* if not su mode queue not sorted */
        if (rootcheck == EPERM) {
		queue_append(new_queue, &thread->link);
	}
	else {
 	/* if su mode the queue of waiters has to be sorted */
        /* SCHED_FIFO (sorted accordind to priority) ,*/
        /* SCHED_RR (sorted accordind to priority) ,*/
        /* SCHED_OTHER  sorted FIFO */

		do {
		/* case SCHED_OTHER or empty queue */
		if(AttrPolicy(thread) == SCHED_OTHER || queue_empty(new_queue)){
	                queue_append(new_queue, &thread->link);
			break;
		}
		/* case FIFO or RR  and not empty queue */
		current =  queue_head(new_queue);
                cur_p = (pthread_d) current;
		/* insert at the head */
	        if ((AttrPolicy(cur_p) == SCHED_OTHER) ||
		   (AttrPriority(thread) < AttrPriority(cur_p))) {
			queue_insert(new_queue,&thread->link);
			break;
		}
		/* case policy = FIFO or RR */
		while (current !=  queue_end(new_queue)) {
			next = queue_next(current);
			next_p = (pthread_d) next;
			if (next ==  queue_end(new_queue)) {
                           queue_append(new_queue,&thread->link);
			   break;
			}
			/* test if thread between   current and next  */
                        if ((AttrPolicy(next_p) == SCHED_OTHER) ||
                            (AttrPriority(thread) < AttrPriority(next_p))) {
			   queue_insert(current,&thread->link);
			   break;
			}
                        current = next;
	                cur_p = (pthread_d) current;
		} /* while */
		} while (0); /* do */
	}
        _spin_unlock(&pthread_lock);

	/* The following optimisation allows events arriving
	 * before the pthread waits to be "delivered" immediately.
	 */
	thread->state |= PTHREAD_ABOUT_TO_WAIT|PTHREAD_INACTIVE;
}


/*
 * Function:
 *	_pthread_activate
 *
 * Parameters:
 *	thread		- the thread to activate
 *
 * Description:
 *	The action of activation removes the thread from its current
 *	queue and moves it to the active queue.
 */
void
_pthread_activate(pthread_t th_id)
{
	pthread_d	thread;

	thread = pthread_id_lookup(th_id);

	/* The thread lock and the lock for the old queue
	 * must be held by the caller.
	 */

	_spin_lock(&pthread_lock);
	queue_remove(&thread->link);
	thread->state &= ~PTHREAD_INACTIVE;
	n_activepthreads++;
	queue_append(&active_pthreads, &thread->link);
	_spin_unlock(&pthread_lock);
}


/*
 * Function:
 *	_pthread_event_wait
 *
 * Parameters:
 *	th_id		- pthread
 *	event_ptr	- event to return
 *	abs_timeout	- boolean abolute/ relative timeout
 *	timeout		- timeout value
 *
 * Description:
 *	Wait for an event.
 *	If an event has arrived since the thread prepared to wait
 *	then just wake up otherwise change state to waiting and
 *	wait for an event.
 *	If a timeout occurs but an event has been sent wait for it.
 *	(EVT_TIMEOUTs are not "sent" they just happen)
 */
void
_pthread_event_wait(pthread_t th_id, int *event_ptr,
		   int abs_timeout, const struct timespec *timeout)
{
	pthread_d	thread;

	thread = pthread_id_lookup(th_id);

	/* The thread lock must be held by the caller.
	 */
	if (thread->state & PTHREAD_AWOKEN) {

		/* An event was delivered directly so don't wait.
		 */
		*event_ptr = thread->event;
		thread->state &= ~(PTHREAD_ABOUT_TO_WAIT|PTHREAD_AWOKEN);
		thread->state &= ~PTHREAD_EVENT;
		_spin_unlock(&thread->lock);

	} else {

		/* Wait for an event to arrive.
		 */
		thread->state &= ~PTHREAD_ABOUT_TO_WAIT;
		thread->state |= PTHREAD_WAITING;

		/* the unlock is done by thread_tsleep (&thread->lock)
		 */
		if (abs_timeout)
			_event_waitabs(&thread->lock, event_ptr, timeout);
		else
			_event_wait(&thread->lock, event_ptr, timeout);
		_spin_lock(&thread->lock);

		/* Check for timeout/ event race.
		 */
		if (*event_ptr == EVT_TIMEOUT && thread->state & PTHREAD_EVENT)
			*event_ptr = EVT_SIGNAL;
		thread->state &= ~PTHREAD_WAITING;
		thread->state &= ~PTHREAD_EVENT;
		_spin_unlock(&thread->lock);
	}
}


/*
 * Function:
 *	_pthread_event_notify
 *
 * Parameters:
 *	th_id	- pthread
 *	event	- event to send
 *
 * Description:
 *	Send an event to a pthread which is waiting.
 *	If the recipient has not yet waited then mark it as woken
 *	otherwise send an event to wake it up.
 *	Avoid races sending events by recording the wake up.
 */
int
_pthread_event_notify(pthread_t th_id, int event)
{
	pthread_d	thread;

	thread = pthread_id_lookup(th_id);

        /* The thread lock must be held by the caller.
         */

	if (thread->state & PTHREAD_EVENT)	/* thread already has event */
		return (FALSE);

	if (thread->state & PTHREAD_ABOUT_TO_WAIT) {

		/* The thread is about to wait. We deliver the
		 * event directly.
		 */
		thread->state |= PTHREAD_AWOKEN;
		thread->event = event;

	} else if (thread->state & PTHREAD_WAITING) {

		/* The thread is waiting, we deliver the event
		 * the slow way.
		 */
		if (event != EVT_SIGNAL)
			if (thread_twakeup(thread->vp->id, event)) {
					/* errno = ESRCH or EINVAL */
				INTERNAL_ERROR("_pthread_event_notify");
			}

	} else
		/* Thread was not waiting.
		 */
		return (FALSE);

	thread->state |= PTHREAD_EVENT;
	return (TRUE);

}


/*
 * Function:
 *	pthread_equal
 *
 * Parameters:
 *	t1	- one thread id
 *	t2	- the second id for comparison
 *
 * Return value:
 *	0 if the two thread ids do not refer to the same thread. Otherwise
 *	non-zero.
 *
 * Description:
 *	This function will normally be used as the macro in pthread.h
 */
#undef pthread_equal
int
pthread_equal(pthread_t t1, pthread_t t2)
{
	return (t1 == t2);
}

/*
 * Function:
 *	_pthread_internal_error
 *
 * Parameters:
 *	file	- __FILE__
 *	func	- name of function responsible
 *	line	- __LINE__
 *
 * Description:
 *	Fatal internal error. Print a message and die.
 *	Avoid taking locks.
 */
void
_pthread_internal_error(char *file, char *func, int line)
{
	char	msg[256];
	int	len;

	len = sprintf(msg, "Pthread internal error: %s, %s @ %d\n",
		      file, func, line);
	(void)write(2, msg, len);
#ifdef DEBUG_PTH
	dump_pthread();
	dump_vp();
#endif	/* DEBUG_PTH */
	exit(1);
}

/*
 * Function:
 *	_last_internal_error
 *
 * Parameters:
 *	file	- __FILE__
 *	func	- name of function responsible
 *	line	- __LINE__
 *
 * Description:
 *	Fatal internal error. Print a message and die.
 *	Avoid taking locks.
 */
void
_last_internal_error(char *file, char *func, int line)
{
	char	msg[256];
	int	len;

	len = sprintf(msg, "Pthread internal error: %s, %s @ %d\n",
		      file, func, line);
	(void)write(2, msg, len);
#ifdef DEBUG_PTH
	dump_pthread();
	dump_vp();
#endif	/* DEBUG_PTH */
	_exit(1);
}

/*
 * Function:
 *      forkall
 *
 * Parameters:
 *      pidp
 *
 * Return value:
 *      ENOSYS  This function is not supported
 *      EAGAIN
 *      ENOMEM
 *
 * Description:
 *      This function duplicates all the threads in the calling process in the
 *      child process.
 *      This function is not supported.
 */
int
forkall(pid_t *pidp)
{
        return (ENOSYS);
}


#ifdef DEBUG_PTH

/*
 * Function:
 *	dump_pthread_queue
 *
 * Parameters:
 *	queue - a queue of pthread structures to print
 *
 * Description:
 *	Print out information about all the threads in a specified queue of
 *	threads.
 */
void
dump_pthread_queue(pthread_queue *queue)
{
	pthread_d	thread;

	dbgPrintf("NAME     ADDRESS  FLAGS    STATE    VP\n");

	/* For every thread in the queue ...
	 */
	for (thread = (pthread_d)queue_head(queue);
	     thread != (pthread_d)queue_end(queue);
	     thread = (pthread_d)queue_next(&thread->link)) {

		/* First the thread information,
		 */
		dbgPrintf("%-8x %-8x %-8x %-8x ",
			thread, thread->flags, thread->state, thread->vp);

		/* then information about the thread's mutex,
		 */
		dump_mutex(&thread->lock);

		/* then information about the thread's condition variable.
		 */
		dump_cond(&thread->done);
		dbgPrintf("\n");
	}
}


/*
 * Function:
 *	dump_pthread
 *
 * Description:
 *	Print all the information available about all the thread global
 *	structures. These are the thread lock, the active and the free
 *	queues.
 */
void
dump_pthread(void)
{
	int	must_unlock;

	if (_spin_trylock(&pthread_lock)) {
		dbgPrintf("Thread lock already set\n");
		must_unlock = FALSE;
	} else
		must_unlock = TRUE;

	/* Even if we can't get the lock, we are in debug mode so we
	 * do our best.
	 */
	dbgPrintf("Active Queue: %d entries\n", n_activepthreads);
	if (n_activepthreads > 0)
		dump_pthread_queue(&active_pthreads);

	dbgPrintf("Free Queue: %d entries\n", n_freepthreads);
	if (n_freepthreads > 0)
		dump_pthread_queue(&free_pthreads);

	if (must_unlock) {
		_spin_unlock(&pthread_lock);
	}
}


#include	<stdarg.h>

void
logPrintf(const char *format, ...)
{
	va_list		ap;
	int		len;
	char		buf[1024];
	pthread_d	self;

	self = pthread_id_lookup(pthread_self());

	len = sprintf(buf, "[%x] ", self);
	va_start(ap, format);
	len += vsprintf(buf + len, format, ap);
	write(2, buf, len);
	va_end(ap);
}


void
dbgPrintf(const char *format, ...)
{
	va_list		ap;
	int		len;
	char		buf[1024];

	va_start(ap, format);
	len = vsprintf(buf, format, ap);
	write(2, buf, len);
	va_end(ap);
}
#endif	/* DEBUG_PTH */

/*
 * Function:
 *	__funcblock_np
 *
 * Description:
 *	To give a pc for a thread which is held by DBX
 */
void
__funcblock_np()
{
	for (;;);
}

/*
 * Function:
 *      pthread_setconcurrency_np
 *
 * Description:
 *      compliant with M:N (for FVT)
 */
int
pthread_setconcurrency_np(int level)
{
#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
        return (ENOTSUP);
#else
	return (ENOSYS);
#endif
}

/*
 * Function:
 *      pthread_getconcurrency_np
 *
 * Description:
 *      compliant with M:N (for FVT)
 */
int
pthread_getconcurrency_np(int *level)
{
#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
        return (ENOTSUP);
#else
	return (ENOSYS);
#endif
}

/*
 * Function:
 *      pthread_getunique_np
 *
 * Parameters:
 *	thread	 - pointer 
 *	sequence - pointer to the sequence number of thread
 *
 * Return value:
 *	0	success
 *	EINVAL	The value specified by thread is invalid
 *		The value specified by sequence is invalid
 *	ESRCH	The value specified by thread does not refer to an existing 
 *		thread.
 *
 * Description:	This function returns the sequence number of thread.
 *		This function is not portable.
 */

int
pthread_getunique_np(pthread_t *thread, int *sequence)
{
pthread_d	pthread;

	if ((thread == NULL) || (sequence == NULL))
		return (EINVAL);
        pthread = pthread_id_lookup(*thread);
	if ((pthread->vp == NULL) || (!pthread->vp->id))
		return (ESRCH);
	*sequence = pthread->th_id;
	return (0);
}

void
pthread_cleanup_push(pthread_push_t func, void * arg)
{
        pthread_d       self = pthread_id_lookup(pthread_self());

        _pthread_cleanup_push(func, arg, self);
}

void
pthread_cleanup_pop(int execute)
{
        pthread_d       self = pthread_id_lookup(pthread_self());

        _pthread_cleanup_pop(execute, self);
}

/*
 * Function:
 *      pthread_test_exit_np
 *
 * Return value:
 *	1	The calling thread is exited
 *	0	The calling thread is not in exit
 *
 *	status: the original status of pthread_exit() if exited.
 *
 * Description:	This function tests if the calling thread in exit
 *		This function is not portable.
 */

int
pthread_test_exit_np(int * status)
{
	pthread_d       self = pthread_id_lookup(pthread_self());

	_spin_lock(&self->lock);
	if (self->state & PTHREAD_EXITED) {
		*status = (int) self->returned;
		_spin_unlock(&self->lock);
		return(1);
	}
	_spin_unlock(&self->lock);
	return (0);
}

/*
 * Function:
 *      pthread_clear_exit_np
 *
 * Parameters:
 *      selfp   - pthread_self
 *
 * Return value:
 *      none
 *
 * Description:
 *      This non-posix function change the state of the thread :
 *      PTHREAD_EXITED
 *		This function is not portable.
 */
void
pthread_clear_exit_np(pthread_t selfp)
{
	pthread_d       self = pthread_id_lookup(selfp);

	_spin_lock(&self->lock);
	self->state &= ~PTHREAD_EXITED;
	self->returned = NULL;
	_spin_unlock(&self->lock);
}


/*
 * Function:
 *	pthread_join_np
 *
 * Parameters:
 *	thread - The id of the thread to be waited for
 *	status - pointer to a place to store the target threads exit status
 *
 * Return value:
 *	0	Success
 *	EINVAL	if thread is an invalid pointer
 *	EDEADLK	if thread is the calling thread
 *	ESRCH	if the target thread is detached
 *
 * Description:
 *	Wait for a thread to exit. If the status parameter is non-NULL then
 *	that threads exit status is stored in it.
 *	It is the same function as pthread_join but the thread is not detached
 *	This function is not portable.
 */
int
pthread_join_np(pthread_t th_id, void **status)
{
	pthread_d	thread;
	pthread_d       self = pthread_id_lookup(pthread_self());
	struct unjoin_args unjoin_str;

	thread = pthread_id_lookup(th_id);

	PT_LOG(("pthread_join: thread = %x\n", thread));

	/* Check the target thread is specified.
	 */
	if (thread == NO_PTHREAD) {
		return (EINVAL);
	}

	/* We cannot wait for ourselves.
	 */
	if (thread == self) {
		return (EDEADLK);
	}

	_spin_lock(&thread->lock);

	/* We cannot wait for a detached thread.
	 */
	if (thread->state & PTHREAD_DETACHED) {
		_spin_unlock(&thread->lock);
		return (ESRCH);
	}

	/* Note that we are joining with this thread.
	 */
	thread->join_count++;

	/* Prepare for cancellation as pthread_cond_wait is a cancellation
	 * point.
	 */
	unjoin_str.thread = thread;
	unjoin_str.self = self;
	_pthread_cleanup_push((pthread_push_t)pthread_unjoin, 
						(void *)&unjoin_str, self);

	/* Wait for the thread to exit.
	 */
	if (!(thread->state & PTHREAD_RETURNED))
		while (_cond_wait_join(&thread->done, &thread->lock) == EINTR){}

	/* Pop the cleanup handler.
	 */
	_pthread_cleanup_pop(FALSE, self);

	/* Save the exit status if it is wanted.
	 */
	if (status != NULL)
		*status = thread->returned;

	thread->join_count--;

	_spin_unlock(&thread->lock);

	return (0);
}

/*
 * Function:
 *      pthread_cleanup_push_np
 *
 * Parameters:
 *      func    - routine to push
 *      arg     - argument of the routine
 *      selfp   - address of pthread_self (output parameter)
 *
 * Return value:
 *      none
 *
 * Description:
 *
 *      This non-posix function is a performance enhanced version of
 *      pthread_cleanup_push().  It takes extra parameters to enable
 *      the push and pop routines to be used more efficiently together.
 *
 *      It pushes the specific routine onto the cleanup stack
 *
 */
void
pthread_cleanup_push_np(pthread_push_t func, void * arg, pthread_t *selfp)
{
        pthread_d       self;
        __pthread_cleanup_handler_t     *handler;

        *selfp = pthread_self();
        self = pthread_id_lookup(*selfp);

        /* -------------- pthread_cleanup_push() --------------*/

	/* if there is a handler in the free list : get it */
	if (self->handler_pool) {
           handler = self->handler_pool;
	   self->handler_pool = self->handler_pool->__next_handler;
    	} else {
           handler = (__pthread_cleanup_handler_t *) _pmalloc
                     (sizeof(struct __pthread_cleanup_handler_t));
	   if (handler == 0){
       	      LAST_INTERNAL_ERROR("pthread_cleanup_push_np: memory saturation");
           }
	}
        handler->__handler_function = (void (*)())func;
        handler->__handler_arg = arg;
        handler->__next_handler = self->cleanup_queue;
        self->cleanup_queue = handler;

}

/*
 * Function:
 *      pthread_cleanup_pop_np
 *
 * Parameters:
 *      execute - execute or not the routine
 *      selfp   - pthread_self
 *
 * Return value:
 *      none
 *
 * Description:
 *      This non-posix function combines 2 subroutines of the libpthreads :
 *      pthread_cleanup_pop() and pthread_clear_exit_np().  It takes an
 *      extra parameter, selfp, to enable the push and pop routines to be
 *      used more efficiently together.
 *
 *      remove the routine at the top of cleanup stack and optionally
 *      invoke it.
 *
 *      change the state of the thread : PTHREAD_EXITED
 *
 */
void
pthread_cleanup_pop_np(int execute, pthread_t selfp )
{
        pthread_d       self;
        __pthread_cleanup_handler_t     *handler;

        self = pthread_id_lookup(selfp);

        /* -------------- pthread_cleanup_pop() -------------- */

        handler = self->cleanup_queue;
        if (handler) {
           self->cleanup_queue = handler->__next_handler;
	   /* put it the free list of the thread */
           handler->__next_handler = self->handler_pool;
	   self->handler_pool = handler;
           if (execute)
              (*handler->__handler_function)(handler->__handler_arg);
        }
}

/*
 * Function:
 *      pthread_self
 *
 * Return value:
 *      The thread id of the calling thread
 *
 * Description:
 *      The thread id is found by calling thread_userdata()
 *      this system call gives back the vp address of the thread set
 *      by thread_setstate.
 */
#undef pthread_self
pthread_t
pthread_self(void)
{

        return(pthread_id_find(((vp_t)thread_userdata())->pthread));

}

