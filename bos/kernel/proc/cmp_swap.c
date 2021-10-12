/*
 * COMPONENT_NAME: (SYSPROC) 
 *
 * FUNCTIONS:	cmp_swap
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/lockl.h>
#include <sys/signal.h>
#include <sys/syspest.h>

Simple_lock cs_lock;

/*
 * NAME: cmp_swap
 *
 * FUNCTION:
 *	This system call is functionally equivalent to the cs() 
 * system call.  It is called on Power PC machines when the cs()
 * address is not word aligned.  See cs() system call for more
 * information.
 *
 * EXECUTION ENVIRONMENT:
 *	system call
 *
 * NOTES:
 *	This should only run on Power PC machines, but since it
 * is a system call it could be called on any machine (accidentally),
 * thus it must be kept in common object
 *
 * RETURNS:
 *	0 - successful
 *	1 - failure
 */
int
cmp_swap(
	int *dest,		/* destination address */
	int comp,		/* compare value */
	int value)		/* new value to store */
{
	int current;		/* current value of word */
	int lock_state;		/* state of cs_lock */
	int mem_state;		/* nonzero if memory error occurred */

	lock_state = 0;

	/*
	 * touch the user address to reduce the number of page
	 * faults while holding the cs_lock
	 */
	mem_state = copyin(dest, &current, sizeof(current));
	if (mem_state != 0)
	{
		goto error_out;
	}

	/*
	 * get lock to serialize cs() operation.
	 */
	simple_lock(&cs_lock);
	lock_state = 1;

	/*
	 * read current value at destination address.  If reads
	 * DSIs or current value is not correct error out
	 */
	mem_state = copyin(dest, &current, sizeof(current));
	if (mem_state != 0 || current != comp)
	{
		goto error_out;
	}

	/*
	 * store new value.  If write DSIs error out
	 */
	mem_state = copyout(&value, dest, sizeof(value));
	if (mem_state != 0)
	{
		goto error_out;
	}

	simple_unlock(&cs_lock);
	return(0);

error_out:
	/*
	 * free lock if held
	 */
	if (lock_state)
	{
		simple_unlock(&cs_lock);
	}

	/*
	 * kill process/thread if an error occurred while accessing
	 * user memory
	 */
	if (mem_state != 0)
	{
		kthread_kill(thread_self(), SIGSEGV);
	}

	return(1);
}
