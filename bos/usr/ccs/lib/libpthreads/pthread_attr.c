static char sccsid[] = "@(#)16	1.9  src/bos/usr/ccs/lib/libpthreads/pthread_attr.c, libpth, bos41J, 9520B_all 5/18/95 08:25:15";
/*
 * COMPONENT_NAME: libpth
 * 
 * FUNCTIONS:
 *	_pthread_attr_startup
 *	pthread_attr_init
 *	pthread_attr_destroy
 *	pthread_attr_setstacksize
 *	pthread_attr_getstacksize
 *	pthread_attr_setstackaddr
 *	pthread_attr_getstackaddr
 *	pthread_attr_setschedpolicy
 *	pthread_attr_getschedpolicy
 *	pthread_attr_setschedparam
 *	pthread_attr_getschedparam
 *	pthread_attr_setinheritsched
 *	pthread_attr_getinheritsched
 *	pthread_attr_setscope
 *	pthread_attr_getscope
 *	pthread_attr_getdetachstate
 *	pthread_attr_setdetachstate
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
 * File: pthread_attr.c
 *
 * Support for thread attribute objects. Currently only the stack size
 * attribute is supported. The remaining attibutes are all scheduling.
 */

#include "internal.h"
#include <sys/pri.h>			/* for PRI_LOW */

/*
 * Global Data
 */
pthread_attr_t		pthread_attr_default;
#define ATTR_THREAD	0
pthread_queue		__dbx_known_attributes;
spinlock_t		dbx__attributes;
extern	int		attr_id;
extern	spinlock_t	attr_id_lock;

/*
 * Function:
 *	_pthread_attr_startup
 *
 * Description:
 *	This function is called from pthread_init().
 *	Note: _pthread_default_stack_size must be already set.
 */
void
_pthread_attr_startup(void)
{
	if (!(pthread_attr_default = (pthread_attr_t)
			       _pmalloc(sizeof(struct pthread_attr))))
		LAST_INTERNAL_ERROR("_pthread_attr_startup:memory saturation");

	if (_pthread_default_stack_size == 0)
		INTERNAL_ERROR("_pthread_attr_startup");

	pthread_attr_default->stacksize = _pthread_default_stack_size;
       pthread_attr_default->cancel_stacksize = _pthread_default_stack_size / 4;
	pthread_attr_default->flags = ATTRIBUTE_VALID;

	pthread_attr_default->type = ATTR_THREAD;
	pthread_attr_default->attr_id = ++attr_id;
	pthread_attr_default->detachstate = DEFAULT_DETACHSTATE;
	pthread_attr_default->process_shared = 0;
	pthread_attr_default->contentionscope = PTHREAD_SCOPE_SYSTEM;
	pthread_attr_default->schedule.sched_policy = DEFAULT_SCHED;
	pthread_attr_default->schedule.sched_priority = PRI_LOW;
				/* PRI_LOW = PIDLE-1 KERNEL == 1 POSIX */

	pthread_attr_default->inherit = PTHREAD_INHERIT_SCHED;
	pthread_attr_default->protocol = 0;
	pthread_attr_default->prio_ceiling = 0;
	pthread_attr_default->mutex_kind = MUTEX_NONRECURSIVE_NP;

	_spin_lock(&dbx__attributes);
	queue_append(&__dbx_known_attributes, &pthread_attr_default->link);
	_spin_unlock(&dbx__attributes);
}


/*
 * Function:
 *	pthread_attr_init
 *
 * Parameters:
 *	attr - a pointer to the attribute structure to be created
 *
 * Return value:
 *	0	Success
 *      EINVAL  the pointer to the attribute was invalid
 *      ENOMEM  Insufficient memory exits to create the thread attributes object
 *
 * Description:
 *	The structure is created by copying the default attribute structure
 *	into the new attribute.
 *
 */
int
pthread_attr_init(pthread_attr_t *attr)
{
	if (attr == NULL) {
		return (EINVAL);
	}
	if (!(*attr = (pthread_attr_t)_pmalloc(sizeof(struct pthread_attr))))
		return (ENOMEM);

	**attr = *pthread_attr_default;

	_spin_lock(&attr_id_lock);
	(*attr)->attr_id  = ++attr_id;
	_spin_unlock(&attr_id_lock);

	_spin_lock(&dbx__attributes);
	queue_append(&__dbx_known_attributes, &(*attr)->link);
	_spin_unlock(&dbx__attributes);

	return (0);
}


/*
 * Function:
 *	pthread_attr_destroy
 *
 * Parameters:
 *	attr - a pointer to the attribute structure to be deleted
 *
 * Return value:
 *	0	Success
 *	EINVAL	the pointer to the attribute was invalid (EINVAL)
 *		the attribute was invalid (EINVAL)
 *		the attribute was the default attribute (EINVAL)
 *
 * Description:
 *	attributes are deleted by marking them as invalid. No storage
 *	needs to be reclaimed.
 */
int
pthread_attr_destroy(pthread_attr_t *attr)
{
	if ((attr == NULL) || (*attr == NO_ATTRIBUTE) ||
	    (*attr == pthread_attr_default) ||
	    !((*attr)->flags & ATTRIBUTE_VALID)) {
		return (EINVAL);
	}
	(*attr)->flags &= ~ATTRIBUTE_VALID;;

	_spin_lock(&dbx__attributes);
	queue_remove(&(*attr)->link); 		/* link for dbx */
	_spin_unlock(&dbx__attributes);

	_pfree(*attr);
	*attr = NO_ATTRIBUTE;
	return (0);
}


/*
 * Function:
 *	pthread_attr_setstacksize
 *
 * Parameters:
 *	attr - a pointer to the attribute to be altered
 *	newsize - the size of the stack to be created when using this attribute
 *
 * Return value:
 *	0	Success
 *	EINVAL	the pointer to the attribute was invalid
 *		the attribute was invalid
 *		the attribute was the default attribute
 *		size was too big
 *
 * Description:
 *	The stacksize is rounded up to the nearest page so we don't
 *	get silly numbers. The value is a minimum value so when the
 *	stack is allocated the thread may get more than this.
 */
int
pthread_attr_setstacksize(pthread_attr_t *attr, size_t newsize)
{
unsigned int ksize;
unsigned int usersize;
unsigned int cancelsize;

	if ((attr == NULL) || (*attr == NO_ATTRIBUTE) ||
	    (*attr == pthread_attr_default) ||
	    !((*attr)->flags & ATTRIBUTE_VALID)) {
		return (EINVAL);
	}
	/*
	 * if we have gone 4096*64K then the requested size was too big
	 * and so we return an error. Also check that we have enough
	 * to add the size of the red zone and specific data without 
	 * overflowing.
	 */

	/* calculate the size in K */

	usersize = ((newsize + 4096 - 1) & ~(4096 - 1)) / 4096; 
	/* usersize contains a multiple of 4k */
	cancelsize = (usersize / 4) * 4;
	/* cancelsize contains the size of the cleanup stack in K */
	usersize = usersize * 4;
	/* usersize contains now the size of the user stack in K */

	ksize = usersize + cancelsize + K_RED_ZONE_SIZE;

	if (ksize > 4096*64)
		return (EINVAL);

	newsize = (ksize * 1024) - (cancelsize * 1024) - RED_ZONE_SIZE;
	if (newsize < PTHREAD_STACK_MIN) {
		newsize = PTHREAD_STACK_MIN;
		(*attr)->cancel_stacksize = newsize / 4;
	}
	else {
		(*attr)->cancel_stacksize = cancelsize * 1024;
	}
	(*attr)->stacksize = newsize;
	return (0);
}


/*
 * Function:
 *	pthread_attr_getstacksize
 *
 * Parameters:
 *	attr - a pointer to the attribute
 *
 * Return value:
 *	0	Success, stores the stacksize attribute value in stacksize
 *	EINVAL	the attribute was invalid
 *		the pointer to stacksize was invalid.
 *
 * Description:
 *	After checking that we have a real attribute we return whatever
 *	is in the stacksize element.
 */
int
pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize)
{
	if (stacksize == NULL)
		return (EINVAL);
	if (attr == NULL) {
		*stacksize = pthread_attr_default->stacksize;
		return (0);
	} 
	if ((*attr == NO_ATTRIBUTE) || !((*attr)->flags & ATTRIBUTE_VALID))
		return (EINVAL);
	*stacksize = (*attr)->stacksize;
	return (0);
}


/*
 * Function:
 *	pthread_attr_setstackaddr
 *
 * Parameters:
 *	attr 		- a pointer to the attribute to be altered
 *	stackaddr	- the thread creation stackaddr attribute
 *
 * Return value:
 *	ENOSYS	This function is not supported
 *
 * Description:
 *	This function is not supported.
 */
int
pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr)
{
	return (ENOSYS);
}


/*
 * Function:
 *	pthread_attr_getstackaddr
 *
 * Parameters:
 *	attr 		- a pointer to the attribute
 *	stackaddr	- the thread creation stackaddr attribute
 *
 * Return value:
 *	ENOSYS	This function is not supported
 *
 * Description:
 *	This function is not supported.
 */
int
pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr)
{
	return (ENOSYS);
}


/*
 * Function:
 *	pthread_attr_setschedpolicy
 *
 * Parameters:
 *	attr		a pointer to the attribute object
 *	policy		the policy attribute
 *
 * Return value:
 *	0	Success
 *	EINVAL	the pointer to the attribute was invalid 
 *	ENOTSUP policy not supported
*/
int
pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
	if ( (attr == NULL) || (*attr == NO_ATTRIBUTE) ||
	     (*attr == pthread_attr_default) ||
	   ! ((*attr)->flags & ATTRIBUTE_VALID) ){
		return (EINVAL);
	}

	if ((policy == SCHED_OTHER) ||
	    (policy == SCHED_FIFO) ||
	    (policy == SCHED_RR)) {
		(*attr)->schedule.sched_policy = policy;
   		return (0);
	}
	else
		return (ENOTSUP);
}


/*
 * Function:
 *	pthread_attr_getschedpolicy
 *
 * Parameters:
 *	attr		a pointer to the attribute object
 *	policy		a pointer where to return the policy attribute
 *
 * Return value:
 *	0	Success
 *	EINVAL	the pointer to the attribute was invalid 
*/
int
pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy)
{
	if (attr == NULL) { 
		*policy = pthread_attr_default->schedule.sched_policy;
		return (0);
	}
	if ((*attr == NO_ATTRIBUTE) || ! ((*attr)->flags & ATTRIBUTE_VALID) )
		return (EINVAL);

	*policy = (*attr)->schedule.sched_policy;
	return (0);
}

/*
 * Function:
 *	pthread_attr_setschedparam
 *
 * Parameters:
 *	attr		a pointer to the attribute object
 *	param		the param attribute
 *
 * Return value:
 *	0	Success
 *	EINVAL	the pointer to the attribute was invalid 
 *	ENOTSUP priority or policy not supported
*/
int
pthread_attr_setschedparam(pthread_attr_t *attr,
			   const struct sched_param *param)
{
	int prio, policy;


	if ( (attr == NULL) || (*attr == NO_ATTRIBUTE) ||
	     (*attr == pthread_attr_default) ||
	   ! ((*attr)->flags & ATTRIBUTE_VALID) ) {
		return (EINVAL);
	}

	prio = param->sched_priority;
	policy = param->sched_policy;

	if ( (prio > 0) && (prio <= PTHREAD_PRIO_MAX) &&
	     ((policy == SCHED_OTHER) || (policy == SCHED_FIFO) ||
	      (policy == SCHED_RR))) {
		if ( (policy == SCHED_OTHER) &&
	     		(prio != DEFAULT_PRIO) ) {	
 	   		return (ENOTSUP);
		}
   		(*attr)->schedule = *param;

		/* Kernel uses 0 as the most favored and 127 as the least
		* favored. The POSIX interfaces use the exact opposite
		*/
		(*attr)->schedule.sched_priority = PTHREAD_PRIO_MAX - prio;
   		return (0);
	}
	else
		return (ENOTSUP);
}


/*
 * Function:
 *	pthread_attr_getschedparam
 *
 * Parameters:
 *	attr		a pointer to the attribute object
 *	param		a pointer where to return the schedule attribute
 *
 * Return value:
 *	0	Success
 *	EINVAL	the pointer to the attribute was invalid 
*/
int
pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param)
{
	if (attr == NULL) {
		*param = pthread_attr_default->schedule;
		param->sched_priority = 
	    PTHREAD_PRIO_MAX - pthread_attr_default->schedule.sched_priority;
		return (0);
	} 
	if ((*attr == NO_ATTRIBUTE) || !((*attr)->flags & ATTRIBUTE_VALID) )
		return (EINVAL);

	*param = (*attr)->schedule;

	/* Kernel uses 0 as the most favored and 127 as the least
	* favored. The POSIX interfaces use the exact opposite
	*/
	param->sched_priority = PTHREAD_PRIO_MAX - (*attr)->schedule.sched_priority;
	return (0);
}


/*
 * Function:
 *	pthread_attr_setinheritsched
 *
 * Parameters:
 *	attr		a pointer to the attribute object
 *	inherit		the inherit attribute
 *			indicating inheritence of scheduling policy
 *
 * Return value:
 *	0	Success
 *	EINVAL	the pointer to the attribute was invalid 
 *	ENOTSUP the inherit is not supported
*/
int
pthread_attr_setinheritsched(pthread_attr_t *attr, int inherit)
{
	if ( (attr == NULL) || (*attr == NO_ATTRIBUTE) ||
	     (*attr == pthread_attr_default) ||
	   ! ((*attr)->flags & ATTRIBUTE_VALID) ) {
		return (EINVAL);
	}

	if ( (inherit == PTHREAD_INHERIT_SCHED) ||
	     (inherit == PTHREAD_EXPLICIT_SCHED) ) {
		(*attr)->inherit = inherit;
		return (0);
	}
	else
		return (ENOTSUP);
}


/*
 * Function:
 *	pthread_attr_getinheritsched
 *
 * Parameters:
 *	attr		a pointer to the attribute object
 *	inherit		a pointer where to return the inherit attribute
 *
 * Return value:
 *	0	Success
 *	EINVAL	the pointer to the attribute was invalid 
*/
int 
pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inheritsched)
{
	if (attr == NULL) {
		*inheritsched = pthread_attr_default->inherit;
		return (0);
	}
	if ((*attr == NO_ATTRIBUTE) || !((*attr)->flags & ATTRIBUTE_VALID))
		return (EINVAL);

	*inheritsched = (*attr)->inherit;
	return (0);
}


/*
 * Function:
 *	pthread_attr_setscope
 *
 * Parameters:
 *	attr 			- a pointer to the attribute structure to be set
 * 	contentionscope 	- the scope of attr
 *
 * Return value:
 *	0	Success
 *	EINVAL	the pointer to the attribute was invalid 
 *	ENOTSUP the scope is not supported
 *
 * Description:
 *	The structure is created by copying the default attribute structure
 *	into the new attribute.
 *
 */
int
pthread_attr_setscope(pthread_attr_t *attr, int contentionscope)
{
	if ( (attr == NULL) || (*attr == NO_ATTRIBUTE) ||
	     (*attr == pthread_attr_default) ||
	   ! ((*attr)->flags & ATTRIBUTE_VALID) ) {
		return (EINVAL);
	}

/* M:N
  	if ( (contentionscope == PTHREAD_SCOPE_SYSTEM) ||
	     (contentionscope == PTHREAD_SCOPE_PROCESS) ) {
*/
  	if (contentionscope == PTHREAD_SCOPE_SYSTEM) {
		(*attr)->contentionscope = contentionscope;
		return (0);
   	}
	else
		return (ENOTSUP);
}


/*
 * Function:
 *	pthread_attr_getscope
 *
 * Parameters:
 *	attr 	a pointer to the  attribute object 
 *	scope	a pointer to store the scope attribute
 *
 * Return value:
 *	0	Success
 *	EINVAL	the pointer to the attribute was invalid 
 */
int
pthread_attr_getscope(const pthread_attr_t *attr, int *scope)
{
	if (attr == NULL) {
   		*scope = pthread_attr_default->contentionscope;
   		return (0);
	}
	if ((*attr == NO_ATTRIBUTE) || !((*attr)->flags & ATTRIBUTE_VALID) )
		return (EINVAL);

   	*scope = (*attr)->contentionscope;
   	return (0);
}

/*
 * Function:
 *      pthread_attr_getdetachstate
 *
 * Parameters:
 *      attr    - Pointer to the attribute
 *      detachstate     - PTHREAD_CREATE_DETACHED (default value
 *                              or PTHREAD_CREATE_UNDETACHED
 *
 * Return value:
 *      0       Success
 *      EINVAL  the pointer to the attribute was invalid
 *
 * Description:
 *      This function gets the detachstate attribute from the attr object.
 */
int
pthread_attr_getdetachstate (const pthread_attr_t *attr, int *detachstate)
{
	if (attr == NULL) {
        	*detachstate = pthread_attr_default->detachstate;
        	return (0);
	}
	if ((*attr == NO_ATTRIBUTE) || !((*attr)->flags & ATTRIBUTE_VALID) )
		return (EINVAL);

        *detachstate = (*attr)->detachstate;
        return (0);
}


/*
 * Function:
 *      pthread_attr_setdetachstate
 *
 * Parameters:
 *      attr            - Pointer to the attribute
 *      detachstate     - PTHREAD_CREATE_DETACHED (default value )
 *                              or PTHREAD_CREATE_UNDETACHED
 *
 * Return value:
 *      0       Success
 *      EINVAL  the pointer to the attribute was invalid
 *              the value of detachstate was not valid.
 *
 * Description:
 *      This function sets the detachstate attribute in the attr object.
 */
int
pthread_attr_setdetachstate (pthread_attr_t *attr, int detachstate)
{
	if ( (attr == NULL) || (*attr == NO_ATTRIBUTE) ||
	     (*attr == pthread_attr_default) ||
	   ! ((*attr)->flags & ATTRIBUTE_VALID) ){
		return (EINVAL);
	}

        if ((detachstate != PTHREAD_CREATE_DETACHED) &&
            (detachstate != PTHREAD_CREATE_UNDETACHED))
           return(EINVAL);

        (*attr)->detachstate = detachstate;
        return (0);
}
