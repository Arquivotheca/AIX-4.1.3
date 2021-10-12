static char sccsid[] = "@(#)93	1.10  src/bos/kernel/proc/forkstack.c, sysproc, bos41J, 9512A_all 3/20/95 18:46:31";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: acquire_forkstack
 *		release_forkstack
 * 
 *   ORIGINS: 27, 83
 *
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#include <sys/proc.h>
#include <sys/user.h>
#include <sys/sleep.h>
#include <sys/pseg.h>
#include <sys/syspest.h>
#include <sys/atomic_op.h>

/*
 * NAME: acquire_forkstack()
 *
 * FUNCTION:
 *	Acquire the fork stack.
 *
 * EXECUTION ENVIRONMENT:
 *      Preemptable
 *      May Page Fault (except within disabled critical section)
 *
 * RETURNS:
 *	address of forkstack, if successful
 *	DO NOT RETURN if the system call should be aborted, self termination
 */
char *
acquire_forkstack(struct proc *p)
{
	register int ipri;

	if (p->p_active == 1)
	{
	    /*
	     * Assuming STERM is always set synchronously by an active thread,
	     * it can't be set if we are the only active thread.
	     */
	    ASSERT(!(p->p_int & STERM));
	    p->p_flag |= SFORKSTACK;
	}
	else
	{
	    ipri = disable_lock(INTMAX, &proc_base_lock);
	    for (;;)
	    {
		if (p->p_int & (STERM|SSUSPUM))
		{
		    unlock_enable(ipri, &proc_base_lock);
		    thread_terminate();
		    /* does not return */
		}
                else if ((p->p_flag & STRC) && (p->p_int & SSUSP))
                {
                    /*
                     * Suspend the current thread, if it is being
                     * traced.  SSUSP is set only after having received
                     * an exception, which means that it will wake up
                     * and look for a debugger request.  Therefore, 
		     * traced threads should not proceed until the 
		     * debugger has had a chance to make a request.
                     * exit performs a cont to expedite process exit. 
                     */
                    stop_thread(curthread);
		    continue;
                }


		if (p->p_flag & SFORKSTACK)
		    e_sleep_thread((int *)&p->p_synch, 
				&proc_base_lock, LOCK_HANDLER);
		else
		    break;
	    }
	    p->p_flag |= SFORKSTACK;
	    unlock_enable(ipri, &proc_base_lock);
	}

	return((char *)&__ublock);
}

/*
 * NAME: release_forkstack()
 *
 * FUNCTION:
 *	Release the fork stack.
 *
 * EXECUTION ENVIRONMENT:
 *	Preemptable
 *	May Page Fault (except within disabled critical section)
 *
 * RETURNS:
 *	none
 */
void
release_forkstack(struct proc *p)
{
	register int ipri;

	if (p->p_active == 1)
	{
	    p->p_flag &= ~SFORKSTACK;
	}
	else
	{
	    ipri = disable_lock(INTMAX, &proc_base_lock);
	    p->p_flag &= ~SFORKSTACK;
	    e_wakeup((int *)&p->p_synch);
	    unlock_enable(ipri, &proc_base_lock);
	}
}
