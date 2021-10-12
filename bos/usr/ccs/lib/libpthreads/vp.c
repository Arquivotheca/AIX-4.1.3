static char sccsid[] = "@(#)23	1.20  src/bos/usr/ccs/lib/libpthreads/vp.c, libpth, bos41J, 9517B_all 4/26/95 09:49:12";
/*
 * COMPONENT_NAME: libpth
 * 
 * FUNCTIONS:
 *	vp_alloc
 *	vp_fork_prepare
 *	vp_fork_parent
 *	vp_fork_child
 *	_pthread_vp_startup
 *	vp_dealloc
 *	vp_detach
 *	_vp_suspend
 *	_vp_resume
 *	_vp_create
 *	_vp_bind
 *	_vp_call
 *	_vp_prune
 *	dump_vp_stats
 *	dump_vp_queue
 *	dump_vp
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
 * File: vp.c
 *
 * This file contains all the functions that manipulate virtual processors or
 * vps. This include creation, deletion, caching, cancellation and fork.
 *
 * pthreads map 1:1 with vps during their execution but a vp may execute
 * several pthreads during the course of a processes life time. Free vps
 * are hashed onto VP_HASH_MAX hash queues based on stack size.
 */

#include <macros.h>		/* defined min */

#include "internal.h"
#include <sys/thread.h>
#include <procinfo.h>

extern void *_errno_hdl;

/*
 * Local Definitions
 */
#ifdef	STATISTICS
typedef struct {
	int	vp_creates;
	int	cache_misses;
	int	cache_hits;
	int	cache_adds;
	int	cache_frees;
} vp_stats_t;
#endif	/* STATISTICS */


/*
 * Local Macros
 */
#define	VP_HASH_NEXT(val)	(val == VP_HASH_MAX-1 ? 0 : val + 1)
#define	VP_HASH_PREV(val)	(val == 0 ? VP_HASH_MAX - 1 : val - 1)
#define	VP_HASH_INDEX(key)	(min((key / VP_HASH_RANGE), (VP_HASH_MAX - 1)))


/*
 * Local Variables
 */
private	spinlock_t	vp_lock;
private	unsigned int	n_freevps;
private	pthread_queue	active_vps;
private	pthread_queue	free_vps[VP_HASH_MAX];
private unsigned long	cache_high, cache_low;
#ifdef	STATISTICS
private vp_stats_t	vp_stats;
#endif	/* STATISTICS */
#define FOREVER	0

/*
 * Function:
 *	vp_alloc
 *
 * Return value:
 *	NO_VP	if a new vp could not be created
 *	pointer to a vp structure otherwise
 *
 * Description:
 *	Allocate a new vp data structure. The kernel thread
 *	is created elsewhere.
 */
private vp_t
vp_alloc(void)
{
	vp_t	vp;

	/* Get the memory for the structure.
	 * and Allocate a specific data area for the vp
	 * in the same malloc.
	 */
	vp = (vp_t)_pmalloc(sizeof(struct vp) + PTHREAD_DATA);
	if (vp == NO_VP) {
		return (NO_VP);
	}
	vp->specific_data_address = (unsigned long)vp + sizeof(struct vp);

	/* All done. Note that this is an ordinary vp and not attached to 
	 * a pthread.
	 */
	vp->flags = 0;
	vp->pthread = NO_PTHREAD;
	vp->async_func = (pthread_func_t)NULL;

	return (vp);
}


/*
 * Function:
 *	vp_fork_prepare
 *
 * Description:
 *	Quiesce the vp sub system prior to a fork. The lock protects all the
 *	global vp data likely to be changed.
 */
private void
vp_fork_prepare(void)
{
	_spin_lock(&vp_lock);
}


/*
 * Function:
 *	vp_fork_parent
 *
 * Description:
 *	Unlock global vp data.
 */
private void
vp_fork_parent(void)
{
	_spin_unlock(&vp_lock);
}


/*
 * Function:
 *	vp_fork_child
 *
 * Description:
 *	Unlock global vp data.
 *	Reset the thread id .
 */
private void
vp_fork_child(void)
{
	vp_t	self;

	_spin_unlock(&vp_lock);

	self = (vp_t)thread_userdata();

	/* Reset the vp id.
	 */
	self->id = (tid_t)thread_self();

}


/*
 * Function:
 *	_pthread_vp_startup
 *
 * Return value:
 *	A pointer to a vp structure for the initial vp
 *
 * Description:
 *	This function is called by pthread_init to initialize the vp global
 *	data and to create the vp for the initial thread.
 *	Register the fork handlers.
 *	This function is called from pthread_init().
 */
vp_t
_pthread_vp_startup(void)
{
	vp_t	vp;
	int	i;
	int	table();

	/* Initialize the active vp queue and the free queues to be empty
	 * Start off with no free vps and initialize the lock which protects
	 * all this data.
	 */
	queue_init(&active_vps);
	for (i = 0; i < VP_HASH_MAX; i++)
		queue_init(&free_vps[i]);
	n_freevps = 0;
	_spinlock_create(&vp_lock);

	/* Allocate a vp structure for the initial vp and chain it onto the
	 * active list.
	 */
	vp = vp_alloc();
	if (vp == NO_VP)
		INTERNAL_ERROR("_vp_startup: NO_VP");
	queue_append(&active_vps, &vp->link);

	/* Mark the vp as active and get its kernel thread id.
	 */
	vp->flags |= (VP_INITIAL_STACK | VP_STARTED);
	vp->id = (tid_t)thread_self();

	/* init cache high and low water marks from system maximum
	 * (currently these are fixed).
	 */
	cache_high = 348;
	cache_low = VP_CACHE_LOW(cache_high);
	cache_high = VP_CACHE_HIGH(cache_high);

	if (pthread_atfork(vp_fork_prepare,
			      vp_fork_parent,
			      vp_fork_child))
		 INTERNAL_ERROR("_pthread_vp_startup");

	return (vp);
}


/*
 * Function:
 *	vp_dealloc
 *
 * Parameters:
 *	vp	- the vp to be deallocated
 *
 * Description:
 *	Destroy a vp. This function frees all things associated with the vp.
 */
private void
vp_dealloc(vp_t vp)
{
	/* Remove the vp from any queue it may be on, get rid of the stack
	 * it is using and then free up the vp structure itself.
	 */
	queue_remove(&vp->link);
	_dealloc_stack(vp);
	_pfree(vp);
}


/*
 * Function:
 *	vp_detach
 *
 * Parameters:
 *	The vp to be detached
 *
 * Description:
 *	This function is similar to pthread_detach in that it marks the
 *	resources of a vp free to be reused when necessary. It does this
 *	by putting it on the free queue.
 */
private void
vp_detach(vp_t vp)
{
	unsigned int	free_queue;

	vp->async_func = (pthread_func_t)NULL;
	/* Find out which free queue this vp should be put on.
	 */
	free_queue = VP_HASH_INDEX(vp->stack.size);

	/* Lock the vp data and add the vp to the appropriate free vp
	 * list. Update the free vp count.
	 */
	queue_remove(&vp->link);
	queue_append(&free_vps[free_queue], &vp->link);
	n_freevps++;
}


/*
 * Function:
 *	_vp_suspend
 *
 * Parameters:
 *	vp	- the vp to suspend. This must be the callers vp.
 *
 * Description:
 *	The vp is detached so that it can be cached and reused.
 *	It checks whether the cache_high water mark has been exceeded
 *	and purges free vps if it has before suspending execution by
 *	waiting for an event to arrive on its port.
 *	Note:	It is assumed any other free vp is fair game for the purge.
 *		It is not assumed that the vp itself is free.
 */
void
_vp_suspend(vp_t self)
{
	int		event;
	int		i;
	vp_t		vp, vp_next, vp_last;
	register caddr_t addr;
	register size_t len;
	stk_t	       stack;
	sigset_t        set;

	/* Check cache high water mark to see if a purge is required.
	 */
	_spin_lock(&vp_lock);

	/* Cache the vp and then suspend execution.
	 */
	vp_detach(self);

#ifdef	STATISTICS
	vp_stats.cache_adds++;
#endif	/* STATISTICS */
	if (n_freevps > cache_high) {

		/* Work through vps from lowest stacksize freeing them until
		 * only cache_low remain.
		 */
		for (i = 0; n_freevps > cache_low && i < VP_HASH_MAX; i++) {
			vp_last = (vp_t)queue_end(&free_vps[i]);
			vp_next = (vp_t)queue_head(&free_vps[i]);
			while (vp_next != vp_last) {
				vp = vp_next;
				vp_next = (vp_t)queue_next(&vp->link);
				if (vp == self)
					continue;
				
				if (thread_terminate_ack(vp->id)) {
				     INTERNAL_ERROR("_vp_suspend: thread_terminate_ack");
				}
				vp_dealloc(vp);

#ifdef	STATISTICS
				vp_stats.cache_frees++;
#endif	/* STATISTICS */
				if (--n_freevps == cache_low)
					break;
			}
		}
	}

			/* thread_tsleep free the mutex &vp_lock) */
	SIGFILLSET(set);
	sigprocmask(SIG_SETMASK, &set, NULL);
	do {
	    event = thread_tsleep(FOREVER, (atomic_p)&vp_lock, NULL);

	/* We have been woken up. Check that this is the correct event type
	 * We do not expect anything else but a resume.
	 */

  	} while (event != EVT_RESUME);

	return;

}


/*
 * Function:
 *	_vp_resume
 *
 * Parameters:
 *	vp	- the vp to be started up
 *
 * Description:
 *	Start a vp executing. The vp may be in one of two states. either
 *	it is a new kernel thread in which case it needs to be resumed by
 *	the kernel or it is a cached vp and is woken by sending it an event.
 */
void
_vp_resume(vp)
vp_t	vp;
{
	pthread_d		thread;
	pthread_d		self;

	if (vp->flags & VP_STARTED) {

		/* This is a cached vp, send the resume event.
		 */
		thread = pthread_id_lookup(vp->pthread);
		if (thread->attr->inherit == PTHREAD_INHERIT_SCHED) {
			self = pthread_id_lookup(pthread_self());
			thread->attr->schedule.sched_policy =
				self->attr->schedule.sched_policy;
			thread->attr->schedule.sched_priority =
				self->attr->schedule.sched_priority;
		}

		if (thread_setsched(thread->vp->id, 
				thread->attr->schedule.sched_priority, 
				thread->attr->schedule.sched_policy, NULL)) {
		     INTERNAL_ERROR("_vp_resume (thread_setsched)");
		}
		/*
		 * set up the vp signal mask for later in _pthread_body
		*/
		if (sigprocmask(SIG_SETMASK, NULL, &vp->mask)) {
			 INTERNAL_ERROR("_vp_resume");
		}
		_vp_setup(vp);

	} else {
		/* This is a new kernel thread. Mark is as started and
		 * get the kernel to resume it (thread_setstate).
		 */
		vp->flags |= VP_STARTED;
		/* Set up the stack and execute pthread_body.
		 */
		_vp_setup(vp);
	}
}


/*
 * Function:
 *	vp_new
 *
 * Parameters:
 *	stack_size	
 *	cancel_stack_size	
 *
 * Description:
 *	create a new vp
 */
private vp_t
vp_new(size_t stack_size, size_t cancel_stack_size)
{
vp_t vp;

		/* There are no free vps so we have to create one for
		 * ourselves. This involves allocating a vp structure,
		 * a kernel thread, a stack and then initializing the
		 * lot. This is then put on the active queue and returned.
		 */
#ifdef	STATISTICS
		_spin_lock(&vp_lock);
		vp_stats.cache_misses++;
		_spin_unlock(&vp_lock);
#endif	/* STATISTICS */
		vp = vp_alloc();
		if (vp == NO_VP)
			return (NO_VP);

		/* Set the cancel stack size in the vp
		 */
		vp->cancel_stack_size = cancel_stack_size;

		/* Create a stack to use.
		 */
		if (!_alloc_stack(vp, stack_size)) {
			_pfree(vp);
			return (NO_VP);
		}
		/* Now create the kernel thread.
		 */
		if ((vp->id = thread_create()) == -1 ) {
			_dealloc_stack(vp);
			_pfree(vp);
			return (NO_VP);
		}
		/* Put the new thread on the active queue.
		 */
		_spin_lock(&vp_lock);
		queue_append(&active_vps, &vp->link);
		_spin_unlock(&vp_lock);
		return (vp);
}

/*
 * Function:
 *	_vp_create
 *
 * Parameters:
 *	attr	- the attributes to create the vp with
 *
 * Return value:
 *	NO_VP	if the vp creation failed
 *	a pointer to the new vp otherwise
 *
 * Description:
 *	Create a vp ready to be resumed. If there are free vps (cached) then
 *	try and find one with at least the stack size specified in the
 *	attributes passed. If the are vps cached but the stacks are too small
 *	then realloc the stack. This should be quicker that creating a new one
 *	from scratch. If there are no vps cached then create a new vp
 *	completely.
 */
vp_t
_vp_create(const pthread_attr_t *attr)
{
        vp_t            vp;
        int             start;
        int             end;
        size_t          stack_size;
        size_t          oldstacksize;
        size_t          oldcancelstacksize;
        int             status;


        status = pthread_attr_getstacksize(attr, &stack_size);

	/* Check to see if there are any used vps around for us to reuse.
	 */
	_spin_lock(&vp_lock);

#ifdef	STATISTICS
	vp_stats.vp_creates++;
#endif	/* STATISTICS */
	if (n_freevps == 0) {
		_spin_unlock(&vp_lock);
		return (vp_new(stack_size, (*attr)->cancel_stacksize));
	}

	/* There is a used vp for us somewhere. We make the assumption that
	 * it is cheaper to reuse a vp than to create a new one from scratch.
	 * The vp free queue is a hash queue based on stacksize. Search it
	 * to find one that has at least the stack size requested. If not take
	 * the first one that comes along.
	 */
	start = VP_HASH_INDEX(stack_size);
	end = start;
	vp = NO_VP;

	do {
		if (!queue_empty(&free_vps[start])) {

		    /* We have found a vp to use. Remove it from the
		     * free list and break out of the search loop.
		     * We still have to check that its stack is big
		     * enough.
		     */
		    vp = (vp_t)queue_head(&free_vps[start]);

				queue_remove(&vp->link);
				n_freevps--;
#ifdef	STATISTICS
				vp_stats.cache_hits++;
#endif	/* STATISTICS */
		    _spin_unlock(&vp_lock);
		    break;
		}

		/* No cached vp found yet, look in the next hash bucket.
		 */
		start = VP_HASH_NEXT(start);
	} while (start != end);

	if (vp == NO_VP)
		 INTERNAL_ERROR("_vp_create");

	/* Check to see that the stacksize of the vp we found was big
	 * enough. If not we have to replace the stack with a bigger
	 * one. This means we have to restart the vp.
	 */
	if (vp->stack.size < stack_size) {

		/* We need a new stack so all the user context on the
		 * last one will be lost. This means we have to restart
		 * the vp as if it was new.
		 */
		vp->flags &= ~VP_STARTED;
		_spin_lock(&vp_lock);
		oldstacksize = vp->stack.size;
		oldcancelstacksize = vp->cancel_stack_size;
		_dealloc_stack(vp);
		vp->cancel_stack_size = (*attr)->cancel_stacksize;
		if (!_alloc_stack(vp, stack_size)) {
						/* restore the old free_vps */
			vp->cancel_stack_size = oldcancelstacksize;
			if (!_alloc_stack(vp, oldstacksize)) {
			    queue_append(&free_vps[VP_HASH_INDEX(oldstacksize)],
 					&vp->link);
			    n_freevps++;
			    _spin_unlock(&vp_lock);
                            return (NO_VP);
			} else {
			    _spin_unlock(&vp_lock);
		 	    INTERNAL_ERROR("_vp_create: alloc_stack");
			}
		}
		_spin_unlock(&vp_lock);
	}

	/* All done, put the vp on the active queue and return.
	 */
	_spin_lock(&vp_lock);
	queue_append(&active_vps, &vp->link);
	_spin_unlock(&vp_lock);
	return (vp);
}


/*
 * Function:
 *	_vp_bind
 *
 * Parameters:
 *	vp	- the vp to bind to the pthread
 *	thread	- the pthread to binf to the vp
 *
 * Description:
 *	Mark both the thread and vp that they belong to each other. The thread
 *	may be NO_PTHREAD which means that the vp is being unbound from the
 *	thread probably in order to be put on the free list
 */
void
_vp_bind(vp_t vp, pthread_d thread)
{
	vp->pthread = thread;
	if (thread != NO_PTHREAD)
		thread->vp = vp;
}


/*
 * Function:
 *	_vp_call
 *
 * Parameters:
 *	vp	- the vp to make the asynchronous function call.
 *	func	- the function to call
 *	arg	- the argument to pass to the function
 *
 * Description:
 *	This is used for asynchronous pthread cancellation. The vp is
 *	stopped in its tracks, a fake call frame is built and the vp resumed
 *	executing the trampoline code which calls the function.
 */
void
_vp_call(vp_t vp, pthread_func_t func, void *arg)
{
	/* Remember the function to call and its argument.
	 */
	vp->async_func = func;
	vp->async_arg = arg;

	/* Build the fake stack frame and then resume the thread executing
	 * the delivery function with thread_setstate system call.
	 */
	_vp_call_setup(vp);
}


/*
 * Function:
 *	_vp_prune
 *
 * Description:
 *	Free all the vps except the caller.
 *	Used after a fork to clean up dead wood. Cannot be done in
 *	_vp_fork_child because (later) pthread cleanup requires access
 *	to the which are deallocated here.
 */
void
_vp_prune(void)
{
	int	i;
	vp_t	vp, all_vps, self;

	/* First clean out the cache.
	 */
	_spin_lock(&vp_lock);
	for (i = 0; i < VP_HASH_MAX; i++) {
		while (!queue_empty(&free_vps[i])) {
			vp = (vp_t)queue_head(&free_vps[i]);
			vp_dealloc(vp);
		}
		queue_init(&free_vps[i]);
	}
	n_freevps = 0;

	/* Clean out all previously active vps.
	 */
	self = (vp_t)thread_userdata();
	all_vps = (vp_t)queue_head(&active_vps);
	while (all_vps != (vp_t)queue_end(&active_vps)) {
		vp = all_vps;
		all_vps = (vp_t)queue_next(&vp->link);
		if (vp != self)
			vp_dealloc(vp);
	}
	_spin_unlock(&vp_lock);
}


#ifdef DEBUG_PTH

#ifdef	STATISTICS
dump_vp_stats(void)
{
	dbgPrintf("total creates %4d, ", vp_stats.vp_creates);
	dbgPrintf("full creates  %4d, ", vp_stats.cache_misses);
	dbgPrintf("cache creates %4d\n", vp_stats.cache_hits);
	dbgPrintf("caches        %4d, ", vp_stats.cache_adds);
	dbgPrintf("decaches      %4d, ", vp_stats.cache_frees);

	dbgPrintf("current cache %4d\n", n_freevps);
}
#endif	/* STATISTICS */


/*
 * Function:
 *	dump_vp_queue
 *
 * Parameters:
 *	queue	- the queue to print out
 *
 * Description:
 *	Debugging function to print all the vps on a given vp queue
 */
void
dump_vp_queue(pthread_queue *queue)
{
	vp_t	vp;

	if (queue_empty(queue)) {
		dbgPrintf("QUEUE EMPTY\n");
		return;
	}

	dbgPrintf("VP       FLAGS    ID       STACKBASE STACKSIZE"
		  " EVT PORT PTHREAD\n");
	for (vp = (vp_t)queue_head(queue);
	     vp != (vp_t)queue_end(queue);
	     vp = (vp_t)queue_next(&vp->link)) {
		dbgPrintf("%-8x %-8x %-8x %-8x  %-8x  %-8x\n",
			  vp, vp->flags,
			  vp->id, vp->stack.base, vp->stack.size,
			  vp->pthread);
	}
}


/*
 * Function:
 *	dump_vp
 *
 * Description:
 *	Debugging function to print out all the vo structures including all
 *	the free and active vp queues.
 */
void
dump_vp(void)
{
	int	i;
	int	must_unlock;

	if (!_spin_trylock(&vp_lock)) {
		dbgPrintf("VP lock already set\n");
		must_unlock = FALSE;
	} else
		must_unlock = TRUE;

	dbgPrintf("Active VPs:\n");
	dump_vp_queue(&active_vps);
	dbgPrintf("Free VPs: %d entries\n", n_freevps);
	for (i = 0; i < VP_HASH_MAX; i++) {
		dbgPrintf("Free Queue %d\n", i);
		dump_vp_queue(&free_vps[i]);
	}
#ifdef	STATISTICS
	dump_vp_stats();
#endif	/* STATISTICS */
	if (must_unlock)
		_spin_unlock(&vp_lock);
}
#endif	/* DEBUG_PTH */

