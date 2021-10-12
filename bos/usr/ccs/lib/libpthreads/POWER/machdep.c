static char sccsid[] = "@(#)11	1.18  src/bos/usr/ccs/lib/libpthreads/POWER/machdep.c, libpth, bos41J, 9519A_all 5/5/95 01:58:45";
/*
 * COMPONENT_NAME: libpth
 * 
 * FUNCTIONS:
 *	_vp_setup
 *	_vp_call_setup
 *	_vp_call_setup_intr
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
 * File: machdep.c
 */

#include "internal.h"
#include <sys/thread.h>
#include <sys/pseg.h>
#include <sys/machine.h>
extern void *_errno_hdl;
#ifdef errno
#undef errno
#endif /*errno*/
        extern int      errno;

/*
 * Function:
 *	_vp_setup
 *
 * Parameters:
 *	vp - the structure describing the thread being set up
 *
 * Description:
 *	Set up the initial state of a thread so that it will
 *	invoke _pthread_body(vp) by the thread_setstate system call.
 */
void
_vp_setup(register vp_t vp)
{
	register int		*top;
	struct tstate		nstate;
	extern int		_pthread_body();
	struct func_desc *f_desc;
	specific_data_t *data;
	pthread_d		thread;
	pthread_d		self;
	struct sigset_t		oset;

	memset((void *)&nstate, 0, sizeof(struct tstate));
	thread = pthread_id_lookup(vp->pthread);
	if (thread->attr->inherit == PTHREAD_INHERIT_SCHED) {
		self = pthread_id_lookup(pthread_self());
		nstate.policy = self->attr->schedule.sched_policy;
		nstate.priority = self->attr->schedule.sched_priority;
		thread->attr->schedule.sched_policy =
			self->attr->schedule.sched_policy;
		thread->attr->schedule.sched_priority =
			self->attr->schedule.sched_priority;
	}
	else
	{
		nstate.policy = thread->attr->schedule.sched_policy;
		nstate.priority = thread->attr->schedule.sched_priority;
	}
	if (thread->attr->contentionscope == PTHREAD_SCOPE_PROCESS) { ;
		nstate.flags = TSTATE_LOCAL;	
	}

	f_desc = (struct func_desc *)_pthread_body;
	top = (int *)vp->stack.limit;
	*top = NULL;		/* back link pointer */

	if (thread->flags & PTHREAD_INITIAL_THREAD) {
           vp->pthread->thread_errno = (int *)&errno;
	} else {
	   data = &vp->pthread->specific_data[(pthread_key_t)_errno_hdl];
	   vp->pthread->thread_errno = (int *)&data->value; 
	}
						/* pointer in segment 2 */
	nstate.errnop_addr = (int **)&vp->pthread->thread_errno;
	nstate.userdata = (int)vp; 

	/*
	 * Set up call frame and registers.
	 * Set pc to procedure entry, pass one arg in register,
	 * allocate the stack frame.
	 */

	nstate.mst.msr = DEFAULT_USER_MSR;
	nstate.mst.iar = (ulong_t)f_desc->entry_point;
	nstate.mst.gpr[3] = (int) vp;	/* argument to function */
	nstate.mst.gpr[2] = (ulong_t)f_desc->toc_ptr;
	nstate.mst.gpr[1] = ((int) top) - STKMIN;

	/* 
	 * Set up the signal mask HERE	
	 * Empty (for safety) the pending set
	*/
	sigprocmask(SIG_SETMASK, NULL, &oset);
	nstate.sigmask = vp->mask = oset;
	SIGINITSET(nstate.psig);

	if (thread_setstate(vp->id, &nstate, NULL))
		INTERNAL_ERROR("_vp_setup");
}


/*
 * Function:
 *	_vp_call_setup
 *
 * Parameters:
 *	vp - the structure describing the target vp to make the call
 *
 * Description:
 *	The thread must be in a suspended state when this function
 *	is called. 
 *	This function is use for cancellation, we do the same treatment that for
 * 	a new thread.
 */
void
_vp_call_setup(vp_t vp)
{
	register int		*top;
	struct tstate		nstate;
	struct func_desc *f_desc;
	specific_data_t *data;
	pthread_d		thread;


	f_desc = (struct func_desc *)(int)vp->async_func;

	/*   for preserve the local variables
	 *   used by pthread_body
	 *   To locate the address of cancel stack, we have to add
	 *   stackbase address + cancel stack size - 1
	*/
	top = (int *)(vp->stack.base + vp->cancel_stack_size - sizeof(void *));

	*top = NULL;		/* back link pointer */
	memset((void *)&nstate, 0, sizeof(struct tstate));
	thread = pthread_id_lookup(vp->pthread);
        if (vp->pthread->flags & PTHREAD_INITIAL_THREAD) {
           vp->pthread->thread_errno = (int *)&errno;
        } else {
           data = &vp->pthread->specific_data[(pthread_key_t)_errno_hdl];
           vp->pthread->thread_errno = (int *)&data->value;
        }

	nstate.errnop_addr = (int **)&vp->pthread->thread_errno;
	nstate.userdata = (int)vp; 
	nstate.policy = thread->attr->schedule.sched_policy;
	nstate.priority = thread->attr->schedule.sched_priority;
	nstate.sigmask = vp->mask;
	nstate.mst.msr = DEFAULT_USER_MSR;
	nstate.mst.iar = (ulong_t)f_desc->entry_point;
	nstate.mst.gpr[3] = (int)vp->async_arg; /* argument to function */
	nstate.mst.gpr[2] = (ulong_t)f_desc->toc_ptr;
	nstate.mst.gpr[1] = ((int) top) - STKMIN;

	if (thread_setstate(vp->id, &nstate, NULL))
		INTERNAL_ERROR("_vp_call_setup");
}


/*
 * Function:
 *	_vp_call_setup_intr
 *
 * Parameters:
 *	vp - the structure describing the target vp to make the call
 *
 * Description:
 *	This function is use for synchronous cancellation
 *	For DEFERRED cancel the cancellation point is in the kernel
 *	thread_setstate is needed with TSTATE_INTR flag.
 */
void
_vp_call_setup_intr(vp_t vp)
{
	register int		*top;
	struct tstate		nstate;
	struct func_desc *f_desc;
	specific_data_t *data;
	pthread_d		thread;


	f_desc = (struct func_desc *)(int)vp->async_func;

	/*   for preserve the local variables
	 *   used by pthread_body
	 *   stackbase address + cancel stack size - 1
	*/
	top = (int *)(vp->stack.base + vp->cancel_stack_size - sizeof(void *));

	*top = NULL;		/* back link pointer */
	memset((void *)&nstate, 0, sizeof(struct tstate));
	thread = pthread_id_lookup(vp->pthread);
        if (vp->pthread->flags & PTHREAD_INITIAL_THREAD) {
           vp->pthread->thread_errno = (int *)&errno;
        } else {
           data = &vp->pthread->specific_data[(pthread_key_t)_errno_hdl];
           vp->pthread->thread_errno = (int *)&data->value;
        }

	nstate.errnop_addr = (int **)&vp->pthread->thread_errno;
	nstate.userdata = (int)vp; 
	nstate.policy = thread->attr->schedule.sched_policy;
	nstate.priority = thread->attr->schedule.sched_priority;
	nstate.sigmask = vp->mask;
	nstate.mst.msr = DEFAULT_USER_MSR;
	nstate.mst.iar = (ulong_t)f_desc->entry_point;
	nstate.mst.gpr[3] = (int)vp->async_arg; /* argument to function */
	nstate.mst.gpr[2] = (ulong_t)f_desc->toc_ptr;
	nstate.mst.gpr[1] = ((int) top) - STKMIN;
	nstate.flags = TSTATE_INTR; 

	if (thread_setstate(vp->id, &nstate, NULL))
		INTERNAL_ERROR("_vp_call_setup_intr");
}
