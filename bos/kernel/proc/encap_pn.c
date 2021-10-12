static char sccsid[] = "@(#)58	1.10  src/bos/kernel/proc/encap_pn.c, sysproc, bos41J, 9512A_all 3/20/95 18:29:15";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: limit_sigs
 *		sigsetmask
 *
 *   ORIGINS: 27, 83
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


#include <sys/systm.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/encap.h>
#include "sig.h"

/*
 * NAME: limit_sigs
 *
 * FUNCTION:
 *
 *	The service limit_sigs() changes the signal mask for the current 
 *	thread such that only the signals specified in nmask will be
 *	delivered, unless they are currently being blocked or ignored.  
 *
 *	Caught signals are blocked in the following algorithm, which
 *	allows exec to operate in diskless environment without being
 *	interrupted by SIGCHLDs.  The sigexec function then removes
 *	pending signals, when it restores the default signal dispositions.  
 *	In other applications the restoration of the signal masks via 
 *	sigsetmask results in the delivery of pending	signals.  
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *
 * RETURNS:
 *	None.
 */
void
limit_sigs( sigset_t *nmask, sigset_t *omask )
{
        sigset_t fillmask;
	register struct thread *t;
	register struct proc *p;
	sigset_t sigmask;
	register ipri;

	t = curthread;
	p = t->t_procp;

	ipri = disable_lock(INTMAX, &proc_base_lock);

	sigmask = t->t_sigmask;
	if (omask != NULL)
		*omask = sigmask;
	if (nmask == NULL) {
		unlock_enable(ipri, &proc_base_lock);
		return;
	}

	SIGFILLSET(fillmask);
	
	if ( !SIGISMEMBER(sigmask, SIGURG) && 
	     !SIGISMEMBER(p->p_sigcatch, SIGURG) )
		SIGDELSET(fillmask, SIGURG);

	if ( !SIGISMEMBER(sigmask, SIGWINCH) && 
	     !SIGISMEMBER(p->p_sigcatch, SIGWINCH) )
		SIGDELSET(fillmask, SIGWINCH);

	if ( !SIGISMEMBER(sigmask, SIGIO) && 
	     !SIGISMEMBER(p->p_sigcatch, SIGIO) )
		SIGDELSET(fillmask, SIGIO);

	if ( !SIGISMEMBER(sigmask, SIGCHLD) && 
	     !SIGISMEMBER(p->p_sigcatch, SIGCHLD) )
		SIGDELSET(fillmask, SIGCHLD);

	if ( !SIGISMEMBER(sigmask, SIGDANGER) && 
	     !SIGISMEMBER(p->p_sigcatch, SIGDANGER) )
		SIGDELSET(fillmask, SIGDANGER);

	/*
	 * Algorithm: 
	 *
	 *   sigmask |= (fillmask & ~(sigmask | sigignore)) & ~newmask
	 *
	 */

	SIGMASKSET(fillmask, sigmask);
	SIGMASKSET(fillmask, p->p_sigignore);
        SIGMASKSET(fillmask, *nmask);
	SIGORSET(t->t_sigmask, fillmask);

        if (SIG_AVAILABLE(t,p))
           	t->t_flags |= TSIGAVAIL;
	else 
		t->t_flags &= ~TSIGAVAIL;

	unlock_enable(ipri, &proc_base_lock);
}

/*
 * NAME: sigsetmask
 *
 * FUNCTION:
 *
 *	The service sigsetmask() is used to restore
 *	the set of blocked signals for the calling
 *	thread.
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *
 * RETURNS:
 *	None.
 */
void
sigsetmask( sigset_t *sigmask )
{
	register struct thread *t;
	register ipri;

	t = curthread;

	SIGDELSET(*sigmask, SIGKILL);
	SIGDELSET(*sigmask, SIGSTOP);

	ipri = disable_lock(INTMAX, &proc_base_lock);

	t->t_sigmask = *sigmask;

        if (SIG_AVAILABLE(t, t->t_procp))
                t->t_flags |= TSIGAVAIL;

	unlock_enable(ipri, &proc_base_lock);
}

