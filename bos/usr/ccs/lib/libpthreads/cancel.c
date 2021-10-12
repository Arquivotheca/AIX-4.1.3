static char sccsid[] = "@(#)04	1.21  src/bos/usr/ccs/lib/libpthreads/cancel.c, libpth, bos41J, 9523B_all 6/6/95 09:55:07";
/*
 * COMPONENT_NAME: libpth
 * 
 * FUNCTIONS:
 *	pthread_cancel_deliver
 *	_pthread_intr_detect
 *	pthread_testcancel
 *	pthread_setcancelstate
 *	pthread_setcanceltype
 *	pthread_cancel	
 *	sig_cncl_handler	
 *	sig_cncl_fork_prepare	
 *	sig_cncl_fork_parent	
 *	sig_cncl_fork_child	
 *	sig_cncl_init	
 *	pthread_signal_to_cancel_np	
 *	pthread_setcancelstate_np
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
 * File: cancel.c
 *
 * This file contains all the functions which manipulate cancellation
 * requests.
 */

#include "internal.h"
#include <signal.h>

extern pthread_queue   __dbx_known_pthreads;
#define       existing_pthreads __dbx_known_pthreads

#define FOREVER 0


/*
 * Local Variables
 */
static pthread_mutex_t	sig_cncl_mutex;		/* sig_cncl lock */
static pthread_cond_t	sig_cncl_cond;		/* sig_cncl cv */
static pthread_t	sig_cncl_target;	/* thread to cancel */
static sigset_t		sig_cncl_set;		/* signals waited for */
static pthread_t	sig_cncl_thread = NULL;	/* waiting thread */
static int		sig_cncl_init_ok;	/* startup sync */
static pthread_attr_t	attr_can_np;
extern pthread_mutexattr_t pthread_mutexattr_fast;


/*
 * Function:
 *	pthread_cancel_deliver
 *
 * Parameters:
 *	thread - the thread id of the calling thread
 *
 * Description:
 *	This function exits the thread.
 *	This implements the action of being cancelled.
 *	Assume pending flag has been cleared and
 *	general cancellation is off
 */
 void
pthread_cancel_deliver(pthread_d thread)
{
	_spin_lock(&thread->lock);
	thread->state |= PTHREAD_EXITED;
	thread->intr.pending = FALSE;
	thread->vp->async_func = NULL;
	_spin_unlock(&thread->lock);
	pthread_exit((void *)-1);
}


/*
 * Function:
 *	_pthread_intr_detect
 *
 * Description:
 *	Decide if there is an outstanding cancellation that may be delivered
 *	and if so deliver it. The target thread must be the calling thread
 *	and must be locked before entry.
 */
void
_pthread_intr_detect(pthread_d self)
{
	/* If general cancellability is on and a cancel is pending then
	 * cancel the thread.
	 */
	if ((self->intr.pending == TRUE) &&
	    (self->intr.state == PTHREAD_CANCEL_ENABLE)) {

		/* Clean up and exit.
		 */
		if (self->vp->async_func)

					/* we do a thread_tsleep to run the
					   routine set by thread_setstate
					   with the flag TINTR */
			thread_tsleep(FOREVER,(atomic_p)&self->lock, NULL); 
		else {
			self->state |= PTHREAD_EXITED;
			self->intr.pending = FALSE;
			_spin_unlock(&self->lock);
			pthread_exit((void *)-1); /* cancel calling threads 
						  */
		}
		/* NOTREACHED */
	}
}


/*
 * Function:
 *	pthread_testcancel
 *
 * Description:
 *	Open a cancellation point. The thread will be cancelled if general
 *	cancellability is on and a cancel is pending.
 */
void
pthread_testcancel(void)
{
	pthread_d	self;

	self = pthread_id_lookup(pthread_self());

	_spin_lock(&self->lock);
	_pthread_intr_detect(self);
	_spin_unlock(&self->lock);
}


/*
 * Function:
 *	pthread_setcancelstate
 *
 * Parameters:
 *	state   - what state to set cancelability.
 *	olstate - pointer to the previous cancelability state.
 *
 * Return value:
 *	0	Success and returns the previous cancelability state at the
 *		location referenced by oldstate.
 *	EINVAL	if state is not either PTHREAD_CANCEL_ENABLE or
 *		PTHREAD_CANCEL_DISABLE
 *
 * Description:
 *	Sets the calling thread's cancelability state to the indicated state
 *	and returns the previous cancelability state.
 */
int
pthread_setcancelstate(int state, int *oldstate)
{
	pthread_d	self;

	/* Check the new state is valid.
	 */
	if ((state != PTHREAD_CANCEL_ENABLE) && 
	    (state != PTHREAD_CANCEL_DISABLE)) {
	   return (EINVAL);
	}

	self = pthread_id_lookup(pthread_self());

	_spin_lock(&self->lock);

	/* The old state is to be returned so save it before the new state
	 * is assigned.
	 */
	if (oldstate != NULL)
		*oldstate = self->intr.state;
	self->intr.state = state;
	if (state == PTHREAD_CANCEL_ENABLE)
		_pthread_intr_detect(self);
	_spin_unlock(&self->lock);
	return (0);
}


/*
 * Function:
 *	pthread_setcanceltype
 *
 * Parameters:
 *	state - what state to set cancelability.
 *	oldtype - pointer to the previous cancelability type.
 *
 * Return value:
 *	0	Success and returns the previous canclability type at the
 *		location referenced by oldtype.
 *	EINVAL	if state is not either PTHREAD_CANCEL_DEFERRED or
 * 		PTHREAD_CANCEL_ASYNCHRONOUS
 *
 * Description:
 *	Sets the calling thread's cancelability type to the indicated type
 *	and returns the previous cancelability type.
 */
int
pthread_setcanceltype(int type, int *oldtype)
{
	pthread_d	self;

	/* Check the new type is valid.
	 */
	if ((type != PTHREAD_CANCEL_DEFERRED) &&
	    (type != PTHREAD_CANCEL_ASYNCHRONOUS)) {
	   return (EINVAL);
	}

	self = pthread_id_lookup(pthread_self());

	_spin_lock(&self->lock);

	/* The old type is to be returned so save it before the new type
	 * is assigned.
	 */
	if (oldtype != NULL)
		*oldtype = self->intr.mode;
	self->intr.mode = type;

	/* If async cancels are allowed open a cancellation point.
	 */
	if (type == PTHREAD_CANCEL_ASYNCHRONOUS)
		_pthread_intr_detect(self);
	_spin_unlock(&self->lock);
	return (0);
}

void *th_bidon(void *arg)
{
}

/*
 * Function:
 *	pthread_cancel
 *
 * Parameters:
 *	thread - the id of the target thread to cancel
 *
 * Return value:
 *	0	Success
 *	EINVAL	if thread is an invalid pointer
 *
 * Description:
 *	If the target threads general cancelability is off the cancellation
 *	is marked pending. If the general cancelability is on then if the
 *	async cancelability is on, the thread is forced to cancel immediately.
 *	Otherwise the cancellation is marked pending and if the thread is at
 *	a cancellation point it is aborted.
 */
int
pthread_cancel(pthread_t th_id)
{
	pthread_d	thread;
	pthread_t th;
	static volatile int	multi_threaded = FALSE;

	thread = pthread_id_lookup(th_id);

	if (thread == NO_PTHREAD) {
		return (EINVAL);
	}

	_spin_lock(&thread->lock);

	if (thread->state & PTHREAD_EXITED) {
		_spin_unlock(&thread->lock);
		return (0);
	}

	/* If general cancelability is off then mark the cancel pending
	 * and return.
	 */
	if (thread->intr.state == PTHREAD_CANCEL_DISABLE) {
		thread->intr.pending = TRUE;
		_spin_unlock(&thread->lock);
		return (0);
	}

	if (thread->intr.pending) {
                _spin_unlock(&thread->lock);
                return (0);
	}
	if (thread->state & PTHREAD_EVENT
	    || _pthread_event_notify(thread, EVT_CANCEL)
	    || thread->intr.mode != PTHREAD_CANCEL_ASYNCHRONOUS) {

		/* Either the thread was waiting and will wake up
		 * or it has async cancels turned off.
		 * Mark the cancel pending so the thread will see it
		 * at the next cancellation point (or wake up).
		 */
		if (thread->flags & PTHREAD_INITIAL_THREAD) {
		    if (!multi_threaded)
			if (thread == pthread_self()) {

			/* Case: initial thread is canceling itself
			 * if there is only the initial thread , we create
			 * a thread so the kernel will see the program as a
			 * multi-threaded program.
			 * if we don't this thread_setstate gives the error
			 * ESRCH.
		 	*/
			multi_threaded = TRUE;        /* to do this only once */
			pthread_create (&th, NULL, th_bidon, (void*)NULL);
			}
		}
		thread->vp->async_func = (pthread_func_t)pthread_cancel_deliver;
        	thread->vp->async_arg = thread;
		_vp_call_setup_intr(thread->vp); /* thread_setstate with							    TSTATE_INTR flags */

		thread->intr.pending = TRUE;
		_spin_unlock(&thread->lock);
	} else {

		/* The cancel is an async cancel and the thread
		 * was not waiting.
		 * Turn off cancellation, consume any pending cancels
		 * and force the thread to cancel.
		 */
		thread->intr.pending = FALSE;

		/* If we are canceling the calling thread then just call the
		 * the cancellation delivery function which never returns.
		 * If we are cancelling another thread then we have to muck
		 * with its PC to get it to call the cancellation delivery
		 * function itself.
		 */
		if (thread == pthread_self()) {
			thread->state |= PTHREAD_EXITED;
			_spin_unlock(&thread->lock);
			pthread_exit((void *)-1);
		} else {
			_spin_unlock(&thread->lock);
			_vp_call(thread->vp,
				 (pthread_func_t)pthread_cancel_deliver,
				 thread);
		}
	}
	return (0);
}


/*
 * Function:
 *	sig_cncl_handler
 *
 * Parameters:
 *	dummy	- unused
 *
 * Description:
 *	Turn async signals map to a "cancel".  A cancel unwind happens
 *	only at well defined points.
 *	Note that the signals must be blocked before calling sigwait() but
 *	cannot be unblocked.
 */
private void *
sig_cncl_handler(void *dummy)
{
int sig;
	/* Wait for and handle asynchronous signals.
	 */
	sigthreadmask(SIG_BLOCK, &sig_cncl_set, NULL);

	pthread_mutex_lock(&sig_cncl_mutex);
	pthread_cond_signal(&sig_cncl_cond);
	pthread_mutex_unlock(&sig_cncl_mutex);

	if (sigwait(&sig_cncl_set, &sig))
		INTERNAL_ERROR("pthread_signal_to_cancel_np:sig_cncl_handler");
	pthread_cancel(sig_cncl_target);
}


/*
 * Function:
 *	sig_cncl_fork_prepare
 *
 * Description:
 *	Quiesce signal to cancel data.
 */
private void
sig_cncl_fork_prepare(void)
{
	pthread_mutex_lock(&sig_cncl_mutex);
}


/*
 * Function:
 *	sig_cncl_fork_parent
 *
 * Description:
 *	Release signal to cancel data.
 */
private void
sig_cncl_fork_parent(void)
{
	pthread_mutex_unlock(&sig_cncl_mutex);
}


/*
 * Function:
 *	sig_cncl_fork_child
 *
 * Description:
 *	Release signal to cancel data.
 */
private void
sig_cncl_fork_child(void)
{
	sig_cncl_thread = NULL;
	pthread_mutex_unlock(&sig_cncl_mutex);
}


/*
 * Function:
 *	sig_cncl_init
 *
 * Description:
 *	Initialise the async signal to cancel data and set up fork
 *	handlers.
 */
private void
sig_cncl_init(void)
{
	if (pthread_mutex_init(&sig_cncl_mutex, &pthread_mutexattr_fast) == 0
	    && pthread_cond_init(&sig_cncl_cond, &pthread_condattr_default) == 0
	    && pthread_atfork(sig_cncl_fork_prepare,
			  	 sig_cncl_fork_parent,
			  	 sig_cncl_fork_child) == 0)
		sig_cncl_init_ok = TRUE;
	else
		sig_cncl_init_ok = FALSE;
    	pthread_attr_init(&attr_can_np);
    	pthread_attr_setdetachstate(&attr_can_np, PTHREAD_CREATE_UNDETACHED);
}


/*
 * Function:
 *	pthread_signal_to_cancel_np
 *
 * Parameters:
 *	sigset	- pointer to the set of signals to wait for
 *	target	- pointer to the cancel thread
 *
 * Return value:
 *	0	- Success
 *	EINVAL 	- invalid target or sigset invalid
 *	EAGAIN 	- could not create handler thread
 *
 * Description:
 *	Create a handler thread to sigwait() for a set of signals and
 *	cancel a target thread when sigwait() returns. Successive calls
 *	of this function override the previous.
 */
int
pthread_signal_to_cancel_np(sigset_t *sigset, pthread_t *target)
{
        pthread_d       thread;
        pthread_d       th;
	int             *ptelem;
	int             adjust;
	int 		trouv = 0;
	int off = sizeof(pthread_queue) / sizeof(int); 

	static pthread_once_t	init_once_block = PTHREAD_ONCE_INIT;
	pthread_d       self = pthread_id_lookup(pthread_self());

	thread = pthread_id_lookup(*target);

	if (target == NULL || sigset == NULL) {
		return (EINVAL);
	}

        for (
                ptelem = (int *)queue_next(&existing_pthreads),
		adjust = (int)(ptelem - off),
		th = (pthread_d)adjust;

                ptelem != (int *)&existing_pthreads;

                ptelem = (int *)queue_next(&th->DBXlink),
		adjust = (int)(ptelem - off),
		th = (pthread_d)adjust ) {

                if (thread == th) {
			trouv++;
			break;
		}
        }
	if (!trouv)
	    return (EINVAL);

	pthread_once(&init_once_block, sig_cncl_init);
	if (!sig_cncl_init_ok)
		return (EAGAIN);

	pthread_mutex_lock(&sig_cncl_mutex);
	_pthread_cleanup_push((pthread_push_t)pthread_mutex_unlock, 
				&sig_cncl_mutex, self);

	if (sig_cncl_thread != NULL) {
		pthread_cancel(sig_cncl_thread);
		pthread_join(sig_cncl_thread, NULL);
		sig_cncl_thread = NULL;
	}
	sig_cncl_set = *sigset;
	sig_cncl_target = *target;
	if (pthread_create(&sig_cncl_thread, &attr_can_np,
			   sig_cncl_handler, NULL)) {
		pthread_mutex_unlock(&sig_cncl_mutex);
		return (EAGAIN);
	}
	pthread_cond_wait(&sig_cncl_cond, &sig_cncl_mutex);

	_pthread_cleanup_pop(1, self);
	return (0);
}

/*
 * Function:
 *	pthread_setcancelstate_np
 *
 * Parameters:
 *	state   - what state to set cancelability.
 *	olstate - pointer to the previous cancelability state.
 *
 * Return value:
 *	0	Success and returns the previous cancelability state at the
 *		location referenced by oldstate.
 *	EINVAL	if state is not either PTHREAD_CANCEL_ENABLE or
 *		PTHREAD_CANCEL_DISABLE
 *
 * Description:
 *	Sets the calling thread's cancelability state to the indicated state
 *	and returns the previous cancelability state.
 *	pthread_setcancelstate_np() is not a cancel point even if state is 
 *	PTHREAD_CANCEL_ENABLE
 *	This function is not portable.
 */
int
pthread_setcancelstate_np(int state, int *oldstate)
{
	pthread_d	self;

	/* Check the new state is valid.
	 */
	if ((state != PTHREAD_CANCEL_ENABLE) && 
	    (state != PTHREAD_CANCEL_DISABLE)) {
	   return (EINVAL);
	}

	self = pthread_id_lookup(pthread_self());

	_spin_lock(&self->lock);

	/* The old state is to be returned so save it before the new state
	 * is assigned.
	 */
	if (oldstate != NULL)
		*oldstate = self->intr.state;
	self->intr.state = state;
	_spin_unlock(&self->lock);
	return (0);
}
