static char sccsid[] = "@(#)82	1.23  src/bos/kernel/proc/uexcept.c, sysproc, bos41J, 9512A_all 3/20/95 19:36:55";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: except_add
 *              prochadd
 *              prochdel
 *              uexadd
 *              uexblock
 *              uexclear
 *              uexdel
 *              uexpid
 *
 *   ORIGINS: 27,83
 *
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


/*
 *      uexadd() and uexdel() are routines used to add and delete system wide
 *              exception handlers for processes in user mode only.
 *      prochadd() and prochdel() are resource termination/intialization
 *              routines that will be called to allocate/free resources
 *      default_uexcp() is the default exception handler for user mode processes
 *
 * Note: This code resides in pinned memory and executes with
 *       interrupts disabled.
 */

#include <sys/types.h>
#include <sys/proc.h>
#include <sys/lockl.h>
#include <sys/intr.h>
#include <sys/mstsave.h>
#include <sys/user.h>
#include <sys/except.h>
#include <sys/signal.h>
#include <sys/sleep.h>
#include <sys/errno.h>
#include <sys/low.h>
#include <sys/reg.h>
#include <sys/syspest.h>
#include <sys/systm.h>
#include "swapper_data.h"


struct proch	*proch_anchor = NULL;
struct uexcepth	*uexcept_anchor = NULL;

/* proch_gen is used in case a process deletes a resource handler from
 * the list.
 */
int		proch_gen = 0;


void
uexadd(struct uexcepth *exp)
{
	int	ipri;
	struct	uexcepth	*current, *prev;

	/* arg check */
	if (!exp)
		return;

	ipri = disable_lock(INTMAX, &proc_base_lock);

	/* 
	 * check if handler already in list.
	 * if so remove it.
	 */
	current = uexcept_anchor;
	for(prev=NULL; current != NULL ;prev=current,current=current->next)
	{
	 	if (current == exp) 
		{
			if(prev == NULL)
			{
				/* first one in list */
				uexcept_anchor = current->next;
			}
			else
				prev->next = current->next;
			break;
		}
	}

	/* 
	 * now add handler to list 
	 */
	exp->next = uexcept_anchor;
	uexcept_anchor = exp;
	unlock_enable(ipri, &proc_base_lock);
}

void
uexdel(struct uexcepth *exp)
{
	int	ipri;
	struct	uexcepth	*junk, *prev;

	if (!uexcept_anchor)
		return;
	ipri = disable_lock(INTMAX, &proc_base_lock);
	
	junk = uexcept_anchor;
	for(prev=NULL; junk != NULL; prev=junk, junk=junk->next)
	{
		if (junk == exp)
		{
			if(prev == NULL)
				/* first one in list */
				uexcept_anchor = junk->next;
			else
				prev->next = junk->next;
			break;
		}
	}
	unlock_enable(ipri, &proc_base_lock);
}

pid_t
uexpid(void)
{
	return(curthread->t_procp->p_pid);
}

void
uexblock(tid_t tid)
{
	register struct ppda *myppda = PPDA;	/* current ppda 	  */
	struct thread *t = THREADPTR(tid);	/* current thread pointer */
	int ipri;

	/* caller must be blocking the current thread */
	assert(tid == myppda->_curthread->t_tid);

	ipri = disable_lock(INTMAX, &proc_base_lock);

	/*
	 *  Put this thread to sleep
	 *  dispatch() will choose another ready thread.
	 *  The current thread is suspended in the act of returning.
	 */
	ASSERT(t->t_state == TSRUN);	/* dispatch will put TSSLEEP here */
	t->t_wtype = TWUEXCEPT;
	t->t_wchan = 0;

	if (myppda->_csa->prev == NULL) {
#ifdef _POWER_MP
		simple_unlock(&proc_base_lock);
#endif
		swtch();		/* call dispatch process path */
		i_enable(ipri);
		return;
	}

	myppda->_ficd = 1;		/* call dispatch interrupt path */	

	unlock_enable(ipri, &proc_base_lock);
}

void
uexclear(tid_t tid)
{
	struct thread *t;
	int ipri;

	ipri = disable_lock(INTMAX, &proc_base_lock);
#ifdef _POWER_MP
	simple_lock(&proc_int_lock);
#endif

	if (!(t = VALIDATE_TID(tid)) || t->t_state == TSZOMB ||
		t->t_state == TSNONE)
		assert(0);

	/* If the thread has been stopped while waiting
	 * we do not ready the thread, rather, we set t_wtype to TNOWAIT
	 * so that when the thread receives the SIGCONT, it will
	 * know that the event has occurred.
	 */
	if (t->t_state == TSSTOP)
		t->t_wtype = TNOWAIT;
	else if (t->t_state == TSRUN) {
		/*
		 * This state is introduced to prevent the thread from being
		 * put on the runqueue twice.  The dispatcher is run after the
		 * exception and queues the current thread on the runqueue.
		 * This code assumes that the dispatcher will change the
		 * state of the thread to TSSLEEP if an intervening uexclear
		 * does not occur.
		 */
		ASSERT(t->t_wtype == TWUEXCEPT);
		t->t_wtype = TNOWAIT;
	} else {
		t->t_wchan = NULL;
		setrq(t, E_WKX_PREEMPT, RQTAIL);
	}

#ifdef _POWER_MP
	simple_unlock(&proc_int_lock);
#endif
	unlock_enable(ipri, &proc_base_lock);
}

void
prochadd(struct proch *proch)
{
	struct	proch	*current, *prev;
	int     waslocked;

	/* arg check */
	if (!proch)
		return;

	waslocked = lockl(&kernel_lock,LOCK_SHORT);

	/* 
	 * check if routine already in list.
	 * if so remove it.
	 */
	current = proch_anchor;
	for(prev=NULL; current != NULL ;prev=current,current=current->next)
	{
	 	if (current == proch) 
		{
			if(prev == NULL)
			{
				/* first one in list */
				proch_anchor = current->next;
			}
			else
				prev->next = current->next;
			break;
		}
	}

	/* 
	 * now add routine to list 
	 */
	proch->next = proch_anchor;
	proch_anchor = proch;
	proch_gen++;

	if (waslocked != LOCK_NEST)
	    unlockl(&kernel_lock);
}

void
prochdel(struct proch *proch)
{
	struct	proch	*junk, *prev;
	int     waslocked;

	waslocked = lockl(&kernel_lock,LOCK_SHORT);

	if (!proch_anchor)
	{
		if (waslocked != LOCK_NEST)
	 	   unlockl(&kernel_lock);
		return;
	}
	
	junk = proch_anchor;
	for(prev = NULL; junk != NULL; prev = junk, junk = junk->next)
	{
		if (junk == proch)
		{
			if(prev == NULL)
				/* first one in list */
				proch_anchor = junk->next;
			else
				prev->next = junk->next;
			break;
		}
	}
	proch_gen++;

	if (waslocked != LOCK_NEST)
	    unlockl(&kernel_lock);

}

extern int default_uexcp();
struct	uexcepth	def_excp;
void
except_add()
{
	def_excp.handler = *default_uexcp;
	uexadd(&def_excp);
}	
