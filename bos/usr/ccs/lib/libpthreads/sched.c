static char sccsid[] = "@(#)17	1.7  src/bos/usr/ccs/lib/libpthreads/sched.c, libpth, bos412, 9448A 11/24/94 08:05:41";
/*
 * COMPONENT_NAME: libpth
 * 
 * FUNCTIONS:
 *	pthread_setschedparam
 *	pthread_getschedparam
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
 * File: sched.c
 *
 * This file contains all the support for the scheduling control of pthreads
 * which is currently none. As we use kernel threads to run on, we have to get
 * them to behave before we can export this functionality.
 */

#include "internal.h"
#include <sys/thread.h>

extern pthread_queue   __dbx_known_pthreads;
#define       existing_pthreads __dbx_known_pthreads

/*
 * Function:
 *	pthread_setschedparam
 *
 * Parameters:
 *	- thread
 *	- policy
 *	- param		the param attribute
 *
 * Return value:
 *	0	Success
 *	EINVAL	The value specified by policy or one of the scheduling 
 *		parameters associated with scheduling policy was invalid.
 *	ESRCH	The value specified by thread does not refer to a existing 
 *		thread.
 *	ENOTSUP An attempt was made to set policy or scheduling parameters
 *		to an unsupported value.
*/
int
pthread_setschedparam(pthread_t pthread, int policy,
		      const struct sched_param *param)
{
	pthread_d	thread;
	int		prio;
	int		opriority;
	int		opolicy;
	pthread_d	self;
	tid_t		tid;
        pthread_d       th;
	int             *ptelem;
	int             adjust;
	int 		trouv = 0;

	int off = sizeof(pthread_queue) / sizeof(int); 

	thread = pthread_id_lookup(pthread);
	prio = param->sched_priority;

	if (thread == NO_PTHREAD) {
		return (ESRCH);
	}
	if (thread->state & PTHREAD_RETURNED)
		return (ESRCH);


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
	    return (ESRCH);



	if ( ! ((policy == SCHED_OTHER) ||
		(policy == SCHED_FIFO) ||
		(policy == SCHED_RR)) ||
	     ! ((prio > 0) && (prio <128)) ) {
	   return (ENOTSUP);
	}

	if ( ((policy == SCHED_FIFO) || (policy == SCHED_RR)) &&
	     ! ((prio > 0) && (prio <128)) ) {
 	   return (EINVAL);
	}

	if ( (policy == SCHED_OTHER) &&
	     (prio != DEFAULT_PRIO) ) {	
 	   return (EINVAL);
	}

	opriority = thread->attr->schedule.sched_priority;
	opolicy	  = thread->attr->schedule.sched_policy;
   	thread->attr->schedule = *param;

	/* Kernel uses 0 as the most favored and 127 as the least
	* favored. The POSIX interfaces use the exact opposite
	*/
	thread->attr->schedule.sched_priority = PTHREAD_PRIO_MAX - prio;
   	thread->attr->schedule.sched_policy = policy;

	self = pthread_id_lookup(pthread_self());
	if (thread == self) 
	 	tid = -1;
	else
		tid = thread->vp->id;
	if (thread_setsched(tid, thread->attr->schedule.sched_priority,
				 thread->attr->schedule.sched_policy, NULL)) {
		thread->attr->schedule.sched_priority = opriority;
   		thread->attr->schedule.sched_policy = opolicy;
		return (EPERM);
	}
   	return (0); 
}

/*
 * Function:
 *	pthread_getschedparam
 *
 * Parameters:
 *	- thread
 *	- policy
 *	- param		the param attribute
 *
 * Return value:
 *	0	Success
 *	ESRCH	Not defined thread
*/
int
pthread_getschedparam(pthread_t pthread, int *policy, struct sched_param *param)
{
	pthread_d	thread;
        pthread_d       th;
	int             *ptelem;
	int             adjust;
	int 		trouv = 0;

	int off = sizeof(pthread_queue) / sizeof(int); 

	thread = pthread_id_lookup(pthread);

	if (thread == NO_PTHREAD) {
		return (ESRCH);
	}
	if (thread->state & PTHREAD_RETURNED)
		return (ESRCH);


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
	    return (ESRCH);


   	*param = thread->attr->schedule;

	/* Kernel uses 0 as the most favored and 127 as the least
	* favored. The POSIX interfaces use the exact opposite
	*/
	param->sched_priority = PTHREAD_PRIO_MAX - thread->attr->schedule.sched_priority;

   	*policy = thread->attr->schedule.sched_policy;
   	return (0); 
}
