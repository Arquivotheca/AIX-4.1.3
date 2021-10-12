static char sccsid[] = "@(#)15	1.8  src/bos/kernel/proc/setpinit.c, sysproc, bos41J, 9512A_all 3/20/95 18:56:53";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: setpinit
 *              setpswap
 *
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/types.h>
#include <sys/errno.h>
#include <sys/lockl.h>
#include <sys/proc.h>
#include <sys/systm.h>

#define	SWAP_PID	0		
#define	INIT_PID	1	


/*
 * NAME: setpinit()
 *
 * FUNCTION: sets the process parent id to init
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a kproc
 *                                                                    
 * RETURN VALUE DESCRIPTION:
 *	 0	= successful completion
 *	 EINVAL	= failed, not a kernel process
 */

int
setpinit()
{
	struct proc *p = curproc;
	struct proc *initproc;
	struct proc *parent;
	struct proc *q, *r;
	int ipri;

	if (!(p->p_flag & SKPROC))
	{
		return(EINVAL);
	}

	initproc = PROCPTR(INIT_PID);
	parent = PROCPTR(p->p_ppid);

	ipri = disable_lock(INTMAX, &proc_base_lock);

	/* take this process off it's parent's list */

	/* if the parent points to this process */
	if (parent->p_child == p)
	{
		parent->p_child = p->p_siblings;
	}
	else
	{
		for (q = parent->p_child, r = parent; 
			q != NULL; r = q, q = q->p_siblings)
		{
			if (q->p_pid == p->p_pid)
			{
				r->p_siblings = q->p_siblings;
				break;
			}
		}
	}

	/* put process on list of init's children */
	p->p_ppid = INIT_PID;
	p->p_siblings = initproc->p_child;
	initproc->p_child = p;

	unlock_enable(ipri, &proc_base_lock);

	return(0);
}

/*
 * NAME: setpswap()
 *
 * FUNCTION: sets the process parent id to init
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a kproc
 *                                                                    
 * RETURN VALUE DESCRIPTION:
 *	 0	= successful completion
 *	 EINVAL	= failed, not a kernel process
 */

int
setpswap()
{
	struct proc *p = curproc;
	struct proc *swapproc;
	struct proc *parent;
	struct proc *q, *r;
	int ipri;

	if (!(p->p_flag & SKPROC))
	{
		return(EINVAL);
	}

	swapproc = PROCPTR(SWAP_PID);
	parent = PROCPTR(p->p_ppid);

	ipri = disable_lock(INTMAX, &proc_base_lock);

	/* take this process off it's parent's list */

	/* if the parent points to this process */
	if (parent->p_child == p)
	{
		parent->p_child = p->p_siblings;
	}
	else
	{
		for (q = parent->p_child, r = parent; 
			q != NULL; r = q, q = q->p_siblings)
		{
			if (q->p_pid == p->p_pid)
			{
				r->p_siblings = q->p_siblings;
				break;
			}
		}
	}

	/* put process on list of init's children */
	p->p_ppid = SWAP_PID;
	p->p_siblings = swapproc->p_child;
	swapproc->p_child = p;

	unlock_enable(ipri, &proc_base_lock);

	return(0);
}
