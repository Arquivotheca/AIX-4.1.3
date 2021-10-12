static char sccsid[] = "@(#)82	1.12  src/bos/kernel/proc/sigsvcs_pn.c, sysproc, bos41J, 9512A_all 3/20/95 19:09:50";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: kthread_kill
 *		sigaction
 *		sigexec
 *		siglocalmask
 *		sigprocmask
 *		sigsuspend
 *
 *   ORIGINS: 27, 3, 83
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/proc.h>
#include <sys/audit.h>
#include <sys/systm.h>
#include <sys/sleep.h>
#include <sys/trchkid.h>
#include <sys/syspest.h>
#include <sys/atomic_op.h>
#include "sig.h"

extern int copyin();			/* copies from user space to kernel  */
extern int copyout();			/* copyies form kernel space to user */
extern int sigpriv();			/* check privilege to signal proc    */

/* 
 * NAME: _sigaction
 *
 * FUNCTION: change action to be taken for a specified signal
 *	     
 *	     This routine exists to provide libpthreads with an intercept
 *	     point for the call to sigaction.  Control flows immediately
 *	     to sigaction, with no additional logic.
 *
 */
_sigaction(int signo, const struct sigaction *nactp, struct sigaction *oactp)
{
	return sigaction(signo, nactp, oactp);
}

/* 
 * NAME: sigaction
 *
 * FUNCTION: change action to be taken for a specified signal
 *
 * NOTES: 
 *	3 major actions are possible: ignore (SIG_IGN), default (SIG_DFL)
 *		or catch signal (i.e. run signal handling function).
 *      if the signal is to be caught, a set of signals to be blocked
 *		while the handler is running may be specified.
 *	minor variations of the major action may be specified through
 *		through signal action flags
 *
 * DATA STRUCTURES:
 *	signal information is stored in the process block an u-block for
 *		each process; these are modified to reflect the new action
 *
 * RETURN VALUES: 
 *	 0 => successful
 *	-1 => unsuccessful, errno reflects the cause of failure
 *		EFAULT = a parameter was outside the process's address space
 *		EINVAL = an invalid signal number was specified
 */
sigaction(int signo, const struct sigaction *nactp, struct sigaction *oactp)
/* int signo;			signal whose action is being changed */
/* struct sigaction *nactp;	new signal action */
/* struct sigaction *oactp;	buffer for returning old signal action */
{
	struct sigaction kact;	/* kernel buffer for "safe" copies of action */
	register struct sigaction *kactp;	/* pointer to kact */
	struct proc *p;		/* pointer to process block of receiving proc */
	struct proc *q;		/* pointer to child process block */
	struct thread *th;
	int oldpri;		/* old interrupt priority */
	static int svcnum = 0;
	char *errorp = &curthread->t_uthreadp->ut_error;

	if(audit_flag && audit_svcstart("PROC_SetSignal", &svcnum, 0)){
		audit_svcfinis();
	}

	TRCHKGT_SYSC(SIGACTION, signo, nactp, oactp, NULL, NULL);

	kactp = &kact;

	/* check for invalid signal numbers */
	if (signo <= 0 || signo > SIGMAX)
	{
		*errorp = EINVAL;
		return(-1);
	}

	/* return old signal action if desired and possible */
	if (oactp != NULL)
	{	
		kactp->sa_handler = U.U_signal[signo];
		kactp->sa_mask = U.U_sigmask[signo];
		kactp->sa_flags = (int)U.U_sigflags[signo];
		if (copyout((caddr_t)kactp, (caddr_t)oactp, sizeof (kact)))
		{
			*errorp = EFAULT;
			return(-1);
		}
	}
	/* set new signal action if desired and possible */
	if (nactp != NULL) 
	{
		if (copyin((caddr_t)nactp, (caddr_t)kactp, sizeof (kact)))
		{
			*errorp = EFAULT;
			return(-1);
		}

		/* return an error if an attempt is made to catch or ignore 
		 * certain signals; i.e. SIGKILL, SIGSTOP 
		 */
		if ((kactp->sa_handler != SIG_DFL) &&
		    (signo == SIGKILL || signo == SIGSTOP))
		{
			*errorp = EINVAL;
			return(-1);
		}

		p = U.U_procp;

		/* don't let SIGKILL, or SIGSTOP be masked */
		SIGDELSET(kactp->sa_mask,SIGKILL);
		SIGDELSET(kactp->sa_mask,SIGSTOP);

		/* 
		 * Disable interrupts to serialize use of process
		 * block signal state information with pidsig(); and
		 * to make changes to u_block and process block atomically 
		 */
		oldpri = disable_lock(INTMAX, &proc_base_lock);

		/* set signal handler action and signal handler mask */
		U.U_signal[signo] = kactp->sa_handler;
		U.U_sigmask[signo] = kactp->sa_mask;

		/* set signal action flags */
		U.U_sigflags[signo] = (char)kactp->sa_flags;

		/* Process is using SVID sigset semantics*/
		if(U.U_sigflags[signo] & SA_SIGSETSTYLE)
			p->p_flag |= SSIGSET;

		if (signo == SIGCHLD) {
		    if (U.U_sigflags[signo] & SA_NOCLDSTOP) {
			/* do not send SIGCHLD when a child proc stops */
			p->p_flag |= SPPNOCLDSTOP;
		    } else {
			/* send this proc a SIGCHLD when a child proc stops */
			p->p_flag &= ~SPPNOCLDSTOP;
		    }
		}

		/* if major action is IGNORE */
		if (kactp->sa_handler == SIG_IGN) 
		{
			/* remove signal from pending signal set */
			SIGDELSET(p->p_sig,signo);
			th = p->p_threadlist;
			do {
				SIGDELSET(th->t_sig,signo);
				th = th->t_nextthread;
			} while (th != p->p_threadlist);

			/* add signal to ignored signal set */
			SIGADDSET(p->p_sigignore,signo);

			/* remove signal from caught signal set */
			SIGDELSET(p->p_sigcatch,signo); 

			/* process doesn't want signals on death of children */
			if (signo == SIGCHLD)
				p->p_flag |= SSIGNOCHLD;
		} 
		/* if major action is CATCH */
		else if (kactp->sa_handler != SIG_DFL)
		{
			/* add signal to caught signal set */
			SIGADDSET(p->p_sigcatch,signo);

			/* remove signal from ignored signal set */
			SIGDELSET(p->p_sigignore,signo);

			/* process DOES want signal on death of children */
			if (signo == SIGCHLD)
				p->p_flag &= ~SSIGNOCHLD;

			/* leave signal pending, if it is */
		}
		/* if major action is DEFAULT */
		else	
		{
			/* remove signal from caught signal set */
			SIGDELSET(p->p_sigcatch,signo);

			/* 
			 * if default for this signal is to ignore it
			 * add the signal to the ignored signal set,
			 * otherwise remove it from the ignored set 
			 */
			switch (signo)
			{
				case SIGURG:
				case SIGIO:
				case SIGWINCH:
				case SIGPWR:
					SIGADDSET(p->p_sigignore,signo);
					SIGDELSET(p->p_sig,signo);
					th = p->p_threadlist;
					do {
						SIGDELSET(th->t_sig,signo);
						th = th->t_nextthread;
					} while (th != p->p_threadlist);
					break;

				case SIGCHLD:
					/*
					 * process DOES want signal on 
					 * death of children 
					 */
					p->p_flag &= ~SSIGNOCHLD;
					/* fall thru */
				case SIGCONT:
					SIGDELSET(p->p_sig,signo);
					th = p->p_threadlist;
					do {
						SIGDELSET(th->t_sig,signo);
						th = th->t_nextthread;
					} while (th != p->p_threadlist);
					/* fall thru */
				default:
					SIGDELSET(p->p_sigignore,signo);
			}

			/*
			 * leave the signal pending, if the default
			 * action is something other than ignore
			 */
		}

#ifdef _POWER_MP
        	if (p->p_active != 1)
			simple_lock(&proc_int_lock);
#endif
		/* 
		 * Send SIGCHLD to this process if it has zombie children, 
		 * is expecting oldstyle SIGCHLD on death of children, and 
		 * is attempting to change the action of SIGCHLD.
		 */
		if ( (kactp->sa_flags & SA_OLDSTYLE) &&  (signo == SIGCHLD) 
		      && !(p->p_flag & SSIGNOCHLD) )
		{
			/* old style SIGCHLD handling causes SIGCHLD to be
			   sent to parent if any ZOMBIE children exist */
			for (q = p->p_child; q; q = q->p_siblings)
				if (q->p_stat == SZOMB)
				{	
					pidsig(p->p_pid, SIGCHLD);
					break;
				}	
		}
		th = p->p_threadlist;
		do {
			if (SIG_AVAILABLE(th,p))
				th->t_flags |= TSIGAVAIL;
			th = th->t_nextthread;
		} while (th != p->p_threadlist);

#ifdef _POWER_MP
        	if (p->p_active != 1)
			simple_unlock(&proc_int_lock);
#endif
		unlock_enable(oldpri, &proc_base_lock);
	}

	return(0);
}

/*
 * NAME: sigprocmask
 *
 * FUNCTION: allows process/thread to change its signal mask; add, delete, set
 *
 * DATA STRUCTURES:
 *	the process/thread signal mask in the process/thread block is altered
 *
 * RETURN VALUES: 
 *	0 => successful
 *     -1 => unsuccessful, errno set to cause of failure
 *	    EFAULT = user's mask is not in process address space
 *	    EINVAL = invalid "how" value
 *	    EPERM = user did not have privilege to change signal's mask
 */
sigprocmask(int how, const sigset_t *nmaskp, sigset_t *omaskp)
/* int	how;		indicates how mask is to be changes */
/* sigset_t  *nmaskp;	new signal set to use in changing mask */
/* sigset_t  *omaskp;	buffer for return of current signal mask set */ 
{
	register struct thread *t;
	register struct proc *p;
	sigset_t kmask;		/* kernel copy of signal mask */
	sigset_t *kmaskp = &kmask;
	int	 oldpri;
	char	*errorp;

	t = curthread;
	p = t->t_procp;
	errorp = &t->t_uthreadp->ut_error;

	/* return current process signal mask in user buffer, if desired */
	if (omaskp != NULL)
	{
		if (copyout((caddr_t)&t->t_sigmask,
					     (caddr_t)omaskp, sizeof(sigset_t)))
		{
			*errorp = EFAULT;
			return (-1);
		}
	}
	/* change process signal mask as requested */
	if (nmaskp != NULL) 
	{
		if (copyin((caddr_t)nmaskp, (caddr_t)kmaskp, sizeof(sigset_t)))
		{
			*errorp = EFAULT;
			return (-1);
		}

		/* do not allow SIGKILL, or SIGSTOP to be masked */
		SIGDELSET(kmask,SIGKILL);
		SIGDELSET(kmask,SIGSTOP);

		oldpri = disable_lock(INTMAX, &proc_base_lock);

		switch (how)
		{
			case SIG_BLOCK:
				SIGORSET(t->t_sigmask,kmask);
				break;
			case SIG_SETMASK:
				t->t_sigmask = kmask;
				break;
			case SIG_UNBLOCK:
				SIGMASKSET(t->t_sigmask,kmask);
				break;
			default:
				*errorp = EINVAL;
				unlock_enable(oldpri, &proc_base_lock);
				return (-1);
		}
	}
	else
		oldpri = disable_lock(INTMAX, &proc_base_lock);

	if (SIG_AVAILABLE(t,p))
		t->t_flags |= TSIGAVAIL;
	unlock_enable(oldpri, &proc_base_lock);

	return(0);
}

/*
 * NAME: siglocalmask
 *
 * FUNCTION: allows process to change the signal mask of all its local threads;
 *		add, delete, set
 *
 * DATA STRUCTURES:
 *	the thread signal masks in the thread blocks are altered
 *
 * RETURN VALUES: 
 *	0 => successful
 *     -1 => unsuccessful, errno set to cause of failure
 *	    EFAULT = user's mask is not in process address space
 *	    EINVAL = invalid "how" value or NULL "nmaskp" value
 *	    EPERM = user did not have privilege to change signal's mask
 */
siglocalmask(int how, const sigset_t *nmaskp)
/* int	how;		indicates how mask is to be changes */
/* sigset_t  *nmaskp;	new signal set to use in changing mask */
{
	register struct thread *t;
	register struct proc *p;
	register struct thread *th;
	sigset_t kmask;
	sigset_t *kmaskp = &kmask;
	int	 oldpri;
	char	 *errorp;

	t = curthread;
	p = t->t_procp;
	errorp = &t->t_uthreadp->ut_error;

	if (nmaskp == NULL) {
		*errorp = EINVAL;
		return(-1);
	}

	/* change thread signal masks as requested */
	if (copyin((caddr_t)nmaskp, (caddr_t)kmaskp, sizeof(sigset_t)))
	{
		*errorp = EFAULT;
		return(-1);
	}

	/* do not allow SIGKILL, or SIGSTOP to be masked */
	SIGDELSET(kmask,SIGKILL);
	SIGDELSET(kmask,SIGSTOP);

	oldpri = disable_lock(INTMAX, &proc_base_lock);

	switch (how) {
	case SIG_BLOCK:
		th = p->p_threadlist;
		do {
			if (th->t_flags & TLOCAL)
			{
				SIGORSET(th->t_sigmask,kmask);
				if (SIG_AVAILABLE(th,p))
					th->t_flags |= TSIGAVAIL;
			}
			th = th->t_nextthread;
		} while (th != p->p_threadlist);
		break;
	case SIG_SETMASK:
		th = p->p_threadlist;
		do {
			if (th->t_flags & TLOCAL)
			{
				th->t_sigmask = kmask;
				if (SIG_AVAILABLE(th,p))
					th->t_flags |= TSIGAVAIL;
			}
			th = th->t_nextthread;
		} while (th != p->p_threadlist);
		break;
	case SIG_UNBLOCK:
		th = p->p_threadlist;
		do {
			if (th->t_flags & TLOCAL)
			{
				SIGMASKSET(th->t_sigmask,kmask);
				if (SIG_AVAILABLE(th,p))
					th->t_flags |= TSIGAVAIL;
			}
			th = th->t_nextthread;
		} while (th != p->p_threadlist);
		break;
	default:
		*errorp = EINVAL;
		unlock_enable(oldpri, &proc_base_lock);
		return(-1);
	}

	unlock_enable(oldpri, &proc_base_lock);

	return(0);
}

/*
 * NAME: _sigsuspend
 *
 * FUNCTION: changes process's signal mask, and waits for a signal
 *
 *       This routine exists to provide libpthreads with an intercept
 *       point for the call to sigsuspend.  Control flows immediately
 *       to sigsuspend, with no additional logic.
 *
 */
_sigsuspend(const sigset_t *sigmask)
{
    return sigsuspend(sigmask);
}

/*
 * NAME: sigsuspend
 *
 * FUNCTION: changes process's signal mask, and waits for a signal
 *
 * ENVIRONMENT:
 *	The p_suspend count WILL NOT equal 0 upon entering this routine.
 *		if the process is a user process, crossing the svc boundary
 *			enforces incrementing the p_suspend count.
 *		if the process is a kernel process, p_suspend is intialized
 *			to 1 when the process is created. 
 *			this is the ONLY safe means for kprocs to handle
 *			signals.
 *
 * DATA STRUCTURES:
 *	the process signal mask in the process block is altered
 *	the process flags (SOMASK)
 *	the saved mask in the u_block (u.u_oldmask)
 *
 * RETURN VALUES: 
 *	0 => successful
 *     -1 => unsuccessful, errno set to cause of failure
 *	    EFAULT = user's mask is not in process address space
 *	    EPERM = user did not have privilege to change signal's mask
 */
sigsuspend(const sigset_t *sigmask)
/* sigset_t *sigmask;	new set of signals to block while waiting */
{
	struct thread *t = curthread;	/* thread to suspend */
	struct uthread *ut;		/* uthread structure */
	sigset_t kmask;			/* kernel copy of signals to block */
	sigset_t *kmaskp = &kmask;	/* pointer to "kmask" */
	int	sleeprtn;
	int	oldpri;
	char	*errorp;

	ut = t->t_uthreadp;

	errorp = &ut->ut_error;

	/* change process signal mask and wait for an unmasked signal */
	if (copyin((caddr_t)sigmask, (caddr_t)kmaskp,
		sizeof(sigset_t)))
	{
		*errorp = EFAULT;
		return(-1);
	}

	/*
	 * When returning from sigsuspend, we want the old mask to be
	 * restored after the signal that interrupted the sigsuspend
         * has been delivered.  Therefore, save the old mask and set
         * a flag so it will be restored on return from a signal handler.
	 */
	ut->ut_oldmask = t->t_sigmask;
	ut->ut_flags |= UTSIGSUSP;

	/* do not allow SIGKILL, or SIGSTOP to be masked */
	SIGDELSET(kmask,SIGKILL);
	SIGDELSET(kmask,SIGSTOP);

	oldpri = disable_lock(INTMAX, &proc_base_lock);

	t->t_sigmask = kmask;

	/*
	 * et_wait will just put us to sleep, and since nobody will do a 
	 * et_post to this process, the signal will cause us to be made
	 * ready
	 */
	do {
		sleeprtn = et_wait(EVENT_SHARED, EVENT_SHARED, EVENT_SIGRET);
	} while (sleeprtn != EVENT_SIG);
	
	unlock_enable(oldpri, &proc_base_lock);

	/* signal interrupted sleep, set errno accordingly */
	*errorp = EINTR;
	return (-1);
}

/*
 * NAME: kthread_kill()
 *
 * FUNCTION: send a signal to a thread
 *
 * EXECUTION ENVIRONMENT: callable at interrupt level; cannot page fault
 *
 * RETURN VALUES:
 *	none.
 */
void
kthread_kill(tid_t tid,		/* thread to signal		*/
	     int sig)		/* signal number to send	*/
{
	register struct thread *t;

	if (tid == -1)
		tid = curthread->t_tid;
	else {
		if (csa->prev == NULL) {
			t = VALIDATE_TID(tid);
			if (!t || (t->t_state == TSNONE))
				return;
			if (t->t_procp != U.U_procp) {
				ASSERT(FALSE);
				return;
			}
		}
	}

	/* tidsig() will check tid and sig for proper values  */

	tidsig(tid, sig);
}

/*
 * NAME: sigexec
 *
 * FUNCTION: perform exec changes to signal state
 * 
 * NOTE: called from exec
 */

void
sigexec()
{
        register int    i;
        register int    oldpri;         /* old interrupt priority */
        struct thread   *t;             /* current running thread */
        struct proc     *p;

        t = curthread;
        p = t->t_procp;

        /* almost ALL processes calling execve() catch some signals */
        /* serialize use of p_sigcatch field */
        oldpri = disable_lock(INTMAX, &proc_base_lock);

        /* reset caught signals; set behavior to "reliable". */
        while (i = fsig(p->p_sigcatch)) {
                SIGDELSET(p->p_sigcatch, i);	/* signal not caught now */
                U.U_signal[i] = SIG_DFL;	/* remove handler	 */
		/*
		 * For those signals where the default action is 'ignore',
		 * remove any pending signal.  This signal is pending only
		 * because there used to be a handler for it.
		 *
		 * POSIX specifies that the pending signal set is preserved
		 * on exec (3.1.2.2), but "Setting a signal action to SIG_DFL
		 * for a signal that is pending, and whose default action is
		 * to ignore the signal (for example, SIGCHLD), shall cause
		 * the pending signal to be discarded, whether or not it is
		 * blocked" (3.3.1.3)
		 *
		 */
		switch (i) {
		case SIGIO:
		case SIGURG:
		case SIGPWR:
		case SIGCHLD:
		case SIGDANGER:
		case SIGWINCH:
		case SIGWAITING:
			SIGDELSET(t->t_sig,i);
			SIGDELSET(p->p_sig,i);
			break;
		}
        }

        p->p_flag &= ~(SSIGNOCHLD | SSIGSET);

        t->t_uthreadp->ut_sigsp = 0;    /* reset stack state to user stack */
        unlock_enable(oldpri, &proc_base_lock);
}

