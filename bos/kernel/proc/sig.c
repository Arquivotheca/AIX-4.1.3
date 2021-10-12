static char sccsid[] = "@(#)58	1.117  src/bos/kernel/proc/sig.c, sysproc, bos41J, 9516A_all 4/13/95 15:09:26";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: cont
 *		issig
 *		pgsignal
 *		pidsig
 *		psig
 *		sig_chk
 *		stop
 *		stop_thread
 *		tidsig
 *		
 *   ORIGINS: 27, 3, 26, 83
 *
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


#include <sys/param.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/sleep.h>
#include <sys/param.h>
#include <sys/pri.h>
#include <sys/limits.h>
#include <sys/syspest.h>
#include <sys/intr.h>
#include <sys/trchkid.h>
#include <sys/lockl.h>
#include <sys/low.h>
#include <sys/acct.h>
#include <sys/systm.h>
#include <sys/atomic_op.h>
#include "swapper_data.h"
#include "sig.h"

extern int proc_event;

extern int fsig();		/* get lowest signal number from set 	 */
extern int procxmt();		/* perform requested trace command   	 */
extern int core();		/* create a core file of current process */
extern void swtch();		/* switch to another thread 		 */
extern void pidsig();		/* sends a signal to process  		 */
extern void tidsig();		/* sends a signal to thread		 */
extern void cont();		/* continue a process			 */
extern void stop();		/* stop a process			 */
extern void stop_thread();	/* stop the current thread		 */
extern void setrq();		/* make thread runnable			 */
extern void sendsig();		/* change context to call signal handler */
extern void kexit();		/* terminate process 			 */
extern void schedsig();		/* swappin process that has been signaled*/


/*
 * NAME: pgsignal
 *
 * FUNCTION: sends a signal to all procs in the specified proc's terminal group
 *
 * NOTES:
 *	This routine replace the gsignal() and signal() routines that
 *	existed in AIX V1 and AIX V2.
 *	This routine is called by tty.c for quits and interrupts.
 *
 * RETURN VALUES: 
 *	NONE
 */

void 
pgsignal(pid_t pid, int signo)
{
    	register struct proc	*p;		/* pointer to process group   */
    	register int		ipri;		/* saved interrupt priority   */
	register int		hadbaselock;	/* caller held lock	      */
    
    	/* send signal to all members of this processes tty group */
    	if (pid != 0) 
	{
		ipri = i_disable(INTMAX);

#ifdef _POWER_MP
		if (!(hadbaselock = lock_mine(&proc_base_lock)))
			simple_lock(&proc_base_lock);
#endif

		/* Use pid to select a process group to post the signal. */
		if ((p = VALIDATE_PID(pid)) && p->p_pgrp) 
		{
			p = PROCPTR(p->p_pgrp)->p_ganchor;
			for ( ; p ; p = p->p_pgrpl)
		    		pidsig(p->p_pid, signo);
		}

#ifdef _POWER_MP
		if (!hadbaselock)
			simple_unlock(&proc_base_lock);
#endif

		i_enable(ipri);
    	}
}

/*
 * NAME: pidsig
 *
 * FUNCTION: sends a signal to a process
 *
 * NOTES:
 *	This routine took a process block pointer (vs. pid) in AIX V2 
 *
 * RETURN VALUES: 
 *	NONE
 */

void 
pidsig(pid_t pid, int signo)
/* pid_t pid;	        identifier of process to receive the signal */
/* int	signo;		signal to send */
{
	void (*action)();	    /* indicates action to be taken	   */
	struct	proc	*p;	    /* pointer to process block of "pid"   */
	int	oldpri; 	    /* old interrupt priority	           */
	int 	discard_cont;	    /* indicate if SIGCONT remains pending */
	int	hadintlock=TRUE;    /* caller held lock		           */
	int	hadbaselock;	    /* caller held lock		           */
	int	sigmasked;	    /* whether the signal is masked or not */
	struct	thread *th;	    /* to traverse the process' thread list */
	struct thread *interruptible = NULL;	/* interruptible thread     */	
	struct thread *pagewait = NULL;		/* nfs page waiter	    */	
	register int link_register; 		/* Return address of caller */

	GET_RETURN_ADDRESS(link_register);
	TRCHKL3T(HKWD_KERN_PIDSIG, pid, signo, link_register);

	if (signo <= 0 || signo > SIGMAX)
		return;

	discard_cont = FALSE;

	/* serialize signalling operations and use of signal fields */
	oldpri = i_disable(INTMAX);

#ifdef _POWER_MP
	if (!(hadbaselock = lock_mine(&proc_base_lock)))
		simple_lock(&proc_base_lock);
#endif

	p = VALIDATE_PID(pid);
	/* do not post an exiting process or exited process */
	if (!p || p->p_flag & SEXIT || p->p_stat == SNONE)
		goto out;

	/* don't bother with ignored signals */
	if ( SIGISMEMBER(p->p_sigignore,signo) 			&& 
	     !(p->p_flag & STRC) 				&& 
	     !(signo == SIGCONT || signo == SIGTTOU || 
	       signo == SIGTSTP || signo == SIGTTIN) )
		goto out;	

	/* check whether the signal is masked or not */
	th = p->p_threadlist;
	do {
		if (!(sigmasked = SIGISMEMBER(th->t_sigmask,signo)))
			break;
		th = th->t_nextthread;
	} while (th != p->p_threadlist);

	/* Determine the correct action for the signo */
	if (sigmasked)
		action = SIG_HOLD;
	else if ( SIGISMEMBER(p->p_sigcatch,signo) )
		action = SIG_CATCH;
	else { 
		action = SIG_DFL;
		/* 
	 	 * take special default "IGNORE" action: i.e. 
		 * remove the signal from the pending set 
	 	 */
	     	if ( signo == SIGIO     || signo == SIGURG  || 
		     signo == SIGPWR    || signo == SIGCHLD || 
		     signo == SIGDANGER || signo == SIGWINCH||
		     signo == SIGWAITING )
			goto out;
	}

	/* set signal to pending for process */
	SIGADDSET(p->p_sig,signo);

#ifdef _POWER_MP
	if (!(hadintlock = lock_mine(&proc_int_lock)))
		simple_lock(&proc_int_lock);
#endif

	/* take further action for specific signals */
	switch (signo) {
	case SIGTERM:
	    	/* 
	     	 * Change nice value of proc if signal is a terminating
	       	 * signal; i.e. SIGKILL or default SIGTERM 
		 */
		if ((p->p_flag & STRC) || action != SIG_DFL)
			break;
		/* keep going ... */
	case SIGKILL:
		/* NOTE: SIGKILL action is always SIG_DFL */
		if (p->p_nice > P_NICE_DEFAULT) 
		{
			SET_NICE(p,NZERO);
		}	
		break;
	case SIGCONT:
		/*
		 * POSIX 3.3.1.2: 
		 * 		  On SIGCONT, "all pending stop signals
		 *  		  for that process shall be discarded." 
		 */
		SIGDELSET(p->p_sig,SIGSTOP);
		SIGDELSET(p->p_sig,SIGTSTP);
		SIGDELSET(p->p_sig,SIGTTIN);
		SIGDELSET(p->p_sig,SIGTTOU);

		/*
		 * POSIX 3.3.1.2: "... If SIGCONT is blocked and not ignored,
		 *		  it shall remain pending until it is either
		 *		  unblocked or a stop signal is generated for
		 *		  the process."
		 *
		 *		  Also, if there is a signal catcher for
		 *		  SIGCONT, then the signal remains pending.
		 */
		if ((action == SIG_HOLD && !SIGISMEMBER(p->p_sigignore,signo))||
		    (action == SIG_CATCH))
		    /*  discard_cont = FALSE */ ; /* initialized above */
		else
		    discard_cont = TRUE; /* pidsig() completes delivery */

                /*
                 * POSIX 3.3.1.2: "...for a process that is stopped,
                 *                the process shall be continued, even
                 *                if the SIGCONT signal is blocked or
                 *                ignored."
		 *
		 * Note it also needs to be continued if caught. 
                 */
		if ((p->p_int & SSUSP) || (p->p_stat == SSTOP)) {
			if (!(p->p_flag & STRC))
			{
				if (discard_cont)
					SIGDELSET(p->p_sig,signo);
				cont(p);
			}
		}
		break;
	case SIGSTOP:
	case SIGTSTP:
	case SIGTTIN:
	case SIGTTOU:
		/*
		 * POSIX 3.3.1.2: "When any stop signal ... is generated for
		 *                 a process, any pending SIGCONT signals
		 *                 for that process shall be discarded."
		 */
		SIGDELSET(p->p_sig,SIGCONT);
		if (SIGISMEMBER(p->p_sigignore,signo)) {
			SIGDELSET(p->p_sig,signo);
			goto out;
		}
		break;
	default:
		break;
	}

	/*
	 * Defer further processing for signal which is held.
	 */
	if (action == SIG_HOLD)
		goto out;
	
	switch(signo) {

	case SIGSTOP:
	case SIGTSTP:
	case SIGTTIN:
	case SIGTTOU:
	case SIGCONT:
		/* 
		 * Don't set TSIGAVAIL for job control, because an action
		 * is taken at generation time.  SSUSP is set/cleared. 
		 */
		if ((action != SIG_CATCH) && !(p->p_flag & STRC))
			break;
		/* Fall through */
	default :
		/*
	 	 * Continue processing for signals that are caught or 
		 * default action.
	 	 *
	 	 * Note: We mark all threads not masking the signal with 
		 * TSIGAVAIL.  The one which will actually get the signal 
		 * will reset this flag for its fellow threads.
	 	 */
		th = p->p_threadlist;
		do {
			if (!SIGISMEMBER(th->t_sigmask,signo))
				th->t_flags |= TSIGAVAIL;
			th = th->t_nextthread;
		} while (th != p->p_threadlist);
	}

	/*
	 * Take action based on process state of receiving process
	 */

	/* process is idling */

	if (p->p_stat == SIDL)
		goto out;

	/* process is stopping or already stopped */

	if ((p->p_int & SSUSP) || (p->p_stat == SSTOP)) {
		/*
		 * If traced process is already stopped,
		 * then no further action is necessary.
		 */
		if (p->p_flag & STRC)
			goto out;

		switch (signo) {
		case SIGKILL:
			/* 
			 * ptrace employs SIGKILL after detaching 
			 * from the traced process, when the debugger
			 * exits.  Therefore overwrite any signal that 
			 * might have been there.
			 */
                	th = p->p_threadlist;
                	do {
                                th->t_cursig = SIGKILL;
                        	th = th->t_nextthread;
                	} while (th != p->p_threadlist);
			cont(p);
			break;
		case SIGSTOP:
		case SIGTSTP:
		case SIGTTIN:
		case SIGTTOU:
			/*
			 * Already stopped, don't need to stop again, so
			 * remove this signal from pending signals.
			 * (Shell could get confused if we stopped again.)
			 */
			SIGDELSET(p->p_sig, signo);
			goto out;
		case SIGCONT:
                        /*
                         * If caught, then we might have to wakeup 
			 * a sleeping thread.
                         */
			break;
		default:
			/*
			 * The signal will be seen once the process is resumed
			 * with a SIGCONT.
			 */
			goto out;
		}
	}

	if (action == SIG_HOLD)
		goto out;

	/* process is swapping out or already swapped out */

	if ((p->p_int & SGETOUT) || (p->p_stat == SSWAP))
		schedsig(p);

	/* process is active */

	ASSERT(p->p_stat == SACTIVE);

        /*
         * Don't take default signal action for the job control
         * signals, if the signal is being caught or if it is a
         * traced process.  For traced processes, we should
         * schedule a thread and let the process notify the
         * debugger in issig.
         */
	if ((action != SIG_CATCH) && !(p->p_flag & STRC))
	{
		switch(signo) {
        	case SIGCONT:
			/* Already continued process above */
               		goto out;
        	case SIGSTOP:
        	case SIGTSTP:
        	case SIGTTIN:
        	case SIGTTOU:
			/*
			 * Don't stop system processes (ie. init).
			 */
			if (p->p_ppid == 0)
				break;

               		/*
                 	 * POSIX 3.3.1.3: discards SIGTSTP, SIGTTIN, SIGTTOU 
			 * to a process in an orphaned pgrp if the signal 
			 * would have stopped the process.
                 	 */
			SIGDELSET(p->p_sig,signo);
               		if ((signo == SIGSTOP) || !(p->p_flag & SORPHANPGRP)) 
                      		stop(p, signo);

               		/* Signal delivery is complete */
             		goto out;
		}
	}

        /*
         * Choose a thread to run.  If one is currently running, then we
         * don't need to wake anybody up.  It will eventually discover the
         * signal.  Otherwise wake up a thread in a long sleep. However,
         * if there is a thread waiting for signals, it will be targetted.
         */
        th = p->p_threadlist;
        do {
                if (!SIGISMEMBER(th->t_sigmask,signo)) {
                        switch(th->t_wtype) {
                        case TNOWAIT :
				if (th->t_state == TSRUN)
					goto out;
				break;
                        case TWCPU :
                                goto out;
                        case TWPGIN :
                                pagewait = th;
                                break;
                        default :
                                if (th->t_flags & TWAKEONSIG) {
                                        interruptible = th;
                                        if (th->t_flags & TSIGWAIT)
                                                goto run;
				}
                                break;
                        }
                }
                th = th->t_nextthread;
        } while (th != p->p_threadlist);

run:
        /*
         * Check if thread is sleeping w/o SWAKEONSIG.
         * If so, call vcs_interrupt() if the thread is
         * in page wait.  this routine will check if the
         * page wait is interruptable; if so, it will
         * ready the thread.
         */
        /* NOTE: even SIGKILL can't wake it up */
        if (interruptible) {
		if (interruptible->t_state == TSSLEEP)
                        setrq(interruptible, E_WKX_PREEMPT, RQTAIL);
		else
			interruptible->t_wtype = TNOWAIT;
        }
        else {
                if (pagewait)
                        vcs_interrupt(pagewait);
        }

out:
	/* Don't discard continue if traced process. */
	if (discard_cont && !(p->p_flag & STRC))
		SIGDELSET(p->p_sig,signo);

#ifdef _POWER_MP
	if (!hadintlock)
		simple_unlock(&proc_int_lock);
	if (!hadbaselock)
		simple_unlock(&proc_base_lock);
#endif

	i_enable(oldpri);
}

/*
 * NAME: tidsig
 *
 * FUNCTION: sends a signal to a thread
 *
 * RETURN VALUES: 
 *	NONE
 */

void 
tidsig(tid_t tid, int signo)
/* tid_t tid;	        identifier of thread to receive the signal */
/* int	signo;		signal to send */
{
	void (*action)();	/* indicates action to be taken		*/
	struct	thread	*t;	/* pointer to thread block of "tid"	*/
	struct	proc	*p;	/* pointer to process block of owner	*/
	int	oldpri; 	/* old interrupt priority		*/
	int 	discard_cont;	/* indicate if SIGCONT remains pending  */
	int	hadintlock=TRUE;/* caller held lock			*/
	int	hadbaselock;	/* caller held lock			*/
	int	sigmasked;	/* whether the signal is masked or not	*/
	struct thread *th;	/* to traverse the process' thread list	*/
	register int link_register; /* Return address of caller */

	if ( signo <= 0 || signo > SIGMAX)
		return;

	discard_cont = FALSE;

	/* serialize signalling operations and use of signal fields */
	oldpri = i_disable(INTMAX);

#ifdef _POWER_MP
	if (!(hadbaselock = lock_mine(&proc_base_lock)))
		simple_lock(&proc_base_lock);
#endif

	t = VALIDATE_TID(tid);
	/* do not post a terminating thread or terminated thread */
	if (!t || (t->t_flags & TTERM) || (t->t_state == TSNONE))
		goto out;
	p = t->t_procp;

	GET_RETURN_ADDRESS(link_register);
	TRCHKL4T(HKWD_KERN_TIDSIG, p->p_pid, tid, signo, link_register);

	/* do not post an exiting process */
	if (p->p_flag & SEXIT)
		goto out;

	/* don't bother with ignored signals */
	if ( SIGISMEMBER(p->p_sigignore,signo) 			&& 
	     !(p->p_flag & STRC) 				&& 
	     !(signo == SIGCONT || signo == SIGTTOU || 
	       signo == SIGTSTP || signo == SIGTTIN) )
		goto out;	

	/* check whether the signal is masked or not */
	sigmasked = SIGISMEMBER(t->t_sigmask,signo);

	/* Determine the correct action for the signo */
	if ( sigmasked )
		action = SIG_HOLD;
	else if ( SIGISMEMBER(p->p_sigcatch,signo) )
		action = SIG_CATCH;
	else { 
		action = SIG_DFL;
		/* 
	 	 * take special default "IGNORE" action: i.e. 
		 * remove the signal from the pending set 
	 	 */
	     	if ( signo == SIGIO     || signo == SIGURG  || 
		     signo == SIGPWR    || signo == SIGCHLD || 
		     signo == SIGDANGER || signo == SIGWINCH||
		     signo == SIGWAITING )
			goto out;
	}

	/* set signal to pending for thread */
	SIGADDSET(t->t_sig,signo);

#ifdef _POWER_MP
	if (!(hadintlock = lock_mine(&proc_int_lock)))
		simple_lock(&proc_int_lock);
#endif

	/* take further action for specific signals */
	switch (signo) {
	case SIGTERM:
	    	/* 
	     	 * Change nice value of proc if signal is a terminating
	       	 * signal; i.e. SIGKILL or default SIGTERM 
		 */
		if ((p->p_flag & STRC) || action != SIG_DFL)
			break;
		/* keep going ... */
	case SIGKILL:
		/* NOTE: SIGKILL action is always SIG_DFL */
		if (p->p_nice > P_NICE_DEFAULT) 
		{
			SET_NICE(p,NZERO);
		}	
		break;
	case SIGCONT:
		/*
		 * POSIX 3.3.1.2: 
		 * 		  On SIGCONT, "all pending stop signals
		 *  		  for that process or thread shall be 
		 *		  discarded." 
		 */
		SIGDELSET(p->p_sig,SIGSTOP);
		SIGDELSET(p->p_sig,SIGTSTP);
		SIGDELSET(p->p_sig,SIGTTIN);
		SIGDELSET(p->p_sig,SIGTTOU);
		th = t;
		do {
			SIGDELSET(th->t_sig,SIGSTOP);
			SIGDELSET(th->t_sig,SIGTSTP);
			SIGDELSET(th->t_sig,SIGTTIN);
			SIGDELSET(th->t_sig,SIGTTOU);
			th = th->t_nextthread;
		} while (th != t);

		/*
		 * POSIX 3.3.1.2: "... If SIGCONT is blocked and not ignored,
		 *		  it shall remain pending until it is either
		 *		  unblocked or a stop signal is generated for
		 *		  the process."
		 *
		 *		  Also, if there is a signal catcher for
		 *		  SIGCONT, then the signal remains pending.
		 */
		if (((action == SIG_HOLD)&&!SIGISMEMBER(p->p_sigignore,signo))||
		    (action == SIG_CATCH))
		    /*  discard_cont = FALSE */ ; /* initialized above */
		else
		    discard_cont = TRUE; /* tidsig() completes delivery */

                /*
                 * POSIX 3.3.1.2: "...for a process that is stopped,
                 *                the process shall be continued, even
                 *                if the SIGCONT signal is blocked or
                 *                ignored."
                 *
                 * Note it also needs to be continued if caught. 
                 */
                if ((p->p_int & SSUSP) || (p->p_stat == SSTOP)) {
                        if (!(p->p_flag & STRC))
			{
				if (discard_cont)
					SIGDELSET(t->t_sig,signo);
                                cont(p);
			}
                }
		break;
	case SIGSTOP:
	case SIGTSTP:
	case SIGTTIN:
	case SIGTTOU:
		/*
		 * POSIX 3.3.1.2: "When any stop signal ... is generated for
		 *                 a process, any pending SIGCONT signals
		 *                 for that process or thread shall be 
		 *		   discarded."
		 */
		SIGDELSET(p->p_sig,SIGCONT);
		th = t;
		do {
			SIGDELSET(th->t_sig,SIGCONT);
			th = th->t_nextthread;
		} while (th != t);

		if ( SIGISMEMBER(p->p_sigignore,signo) ) 
		{
			SIGDELSET(p->p_sig,signo);
			goto out;
		}
		break;
	default:
		break;
	}

	/*
	 * Defer further processing for signal which is held.
	 */
	if (action == SIG_HOLD)
		goto out;
	
        switch(signo) {

        case SIGSTOP:
        case SIGTSTP:
        case SIGTTIN:
        case SIGTTOU:
        case SIGCONT:
                /*
                 * Don't set TSIGAVAIL for job control, because an action
                 * is taken at generation time.  SSUSP is set/cleared.
                 */
                if (action != SIG_CATCH)
                        break;
		/* Fall through */
        default :
		/*
	 	 * continue processing for signals that are caught 
		 * or default action
	 	 */
		t->t_flags |= TSIGAVAIL;
        }

	/*
	 * take action based on process state of receiving process
	 */

	/* process is idling */

	if (p->p_stat == SIDL)
		goto out;

	/* process is stopping or already stopped */

	if ((p->p_int & SSUSP) || (p->p_stat == SSTOP)) {
		/*
		 * If traced process is already stopped,
		 * then no further action is necessary.
		 */
		if (p->p_flag & STRC)
			goto out;

		switch (signo) {
		case SIGKILL:
			/* continue stopped process receiving SIGKILL */
			cont(p);
			break;
		case SIGSTOP:
		case SIGTSTP:
		case SIGTTIN:
		case SIGTTOU:
			/*
			 * Already stopped, don't need to stop again, so
			 * remove this signal from pending signals.
			 * (Shell could get confused if we stopped again.)
			 */
			SIGDELSET(p->p_sig,signo);
			goto out;
                case SIGCONT:
                        /*
                         * If caught, then we might have to wakeup
                         * a sleeping thread.
                         */
                        break;
		default:
			/*
			 * The signal will be seen once the process is resumed
			 * with a SIGCONT.
			 */
			goto out;
		}
	}

        if (action == SIG_HOLD)
                goto out;

	/* process is swapping out or already swapped out */

	if ((p->p_int & SGETOUT) || (p->p_stat == SSWAP))
		schedsig(p);

	/* process is active */

	ASSERT(p->p_stat == SACTIVE);

        /*
         * Don't take default signal action for the job control
         * signals, if the signal is being caught or if it is a
         * traced process.  For traced processes, we should
         * schedule a thread and let the process notify the
         * debugger in issig.
         */
        if ( (action != SIG_CATCH) && !(p->p_flag & STRC) )
        {
                switch(signo) {
                case SIGCONT:
                        /* Already continued process above. */
                        goto out;
                case SIGSTOP:
                case SIGTSTP:
                case SIGTTIN:
                case SIGTTOU:
                        /*
                         * Don't stop system processes (ie. init).
                         */
                        if (p->p_ppid == 0)
                                break;

                        /*
                         * POSIX 3.3.1.3: discards SIGTSTP, SIGTTIN, SIGTTOU
                         * to a process in an orphaned pgrp if the signal
                         * would have stopped the process.
                         */
			SIGDELSET(p->p_sig,signo);
                        if ((signo == SIGSTOP) || !(p->p_flag & SORPHANPGRP))
                                stop(p, signo);

                        /* Signal delivery is complete */
                        goto out;
                }
        }

	switch(t->t_wtype) {
	case TNOWAIT :	
	case TWCPU :	
		break;
	case TWPGIN :
		vcs_interrupt(t);
		break;
	default :	
		if (t->t_flags & TWAKEONSIG) {
 			if (t->t_state == TSSLEEP)
	                	setrq(t, E_WKX_PREEMPT, RQTAIL);
			else
			  	t->t_wtype = TNOWAIT;
		}
	}

out:
	if (discard_cont && !(p->p_flag & STRC))
		SIGDELSET(t->t_sig,signo);
#ifdef _POWER_MP
	if (!hadintlock)
		simple_unlock(&proc_int_lock);
	if (!hadbaselock)
		simple_unlock(&proc_base_lock);
#endif
	i_enable(oldpri);
}

/*
 * NAME: issig
 *
 * FUNCTION: check for signal to deliver, put it in t_cursig
 *
 * DATA STRUCTURES:
 *	may change signal information in process and thread blocks
 *
 * RETURN VALUES: 
 *	0 => no signal available for delivery
 *	non-zero => there is a signal to deliver (signal number is in t_cursig)
 */

issig(void)
{
	register struct thread *t;
	register struct proc *p;
	register int signo;
	sigset_t sigbits;
	int oldpri; 				/* saved interrupt priority */
	int procsig, pending;

	ASSERT(csa->intpri == INTBASE);

	t = curthread;
	p = t->t_procp;

	ASSERT(!(t->t_flags & TKTHREAD));

        TRCHKL3T(HKWD_KERN_ISSIG, p->p_pid, t->t_tid, t->t_cursig);

	oldpri = disable_lock(INTMAX, &proc_base_lock);

issig_top:

	/* Pending terminate request? */ 
	if ((p->p_int & STERM) || (t->t_flags & TTERM)) 
	{
		unlock_enable(oldpri, &proc_base_lock);
		thread_terminate();
		/* Does not return */
	}

	/* 
	 * Check for signal ready to be delivered.  t_cursig may have 
	 * been set by an interrupted sleep (see e_sleep) or by thread_kill().
	 */
	if ((t->t_cursig) && !(p->p_flag & STRC))
	{
		unlock_enable(oldpri, &proc_base_lock);
		return (t->t_cursig);
	}
	else
	{
		if(p->p_flag & STRC)
		{
			signo = t->t_cursig;
			if (signo != 0) goto traced;
		}
	}
	for (;;) 
	{
		/* get set of pending signals that are not masked */
		sigbits = p->p_sig;
		SIGORSET(sigbits,t->t_sig);
		if (!(p->p_flag & STRC))
		{
			SIGMASKSET(sigbits,p->p_sigignore);
		}
		SIGMASKSET(sigbits,t->t_sigmask);
		if (SIGSETEMPTY(sigbits)) 
		{
			break;
		}

		/* get rightmost unmasked pending signal to deliver */
		signo = fsig(sigbits);

		/* remove the signal from those that are pending */
		DEL_PENDING_SIG(t,p,signo,procsig);

		/* set t_cursig to the signal chosen for delivery */
		t->t_cursig = signo;

		/* 
		 * Give the debugger a chance to take the signal and change
		 * the signal that the traced process sees 
		 */
traced:		if ((p->p_flag & STRC) && (signo != SIGKILL)) 
		{
#ifdef _POWER_MP
			simple_lock(&proc_int_lock);
#endif

			/* Notify debugger for each signal received. */ 
			pidsig(p->p_ipc->ip_dbp->p_pid,SIGCHLD);

			/*
			 * If traced, always stop, and stay
			 * stopped until released by the parent.
			 */
			do {
				(void)i_disable(INTMAX);
#ifdef _POWER_MP
				/* Held first time through */
				if (!lock_mine(&proc_base_lock))
					simple_lock(&proc_base_lock);
				if (!lock_mine(&proc_int_lock))
					simple_lock(&proc_int_lock);
#endif

				/* Identify this thread for ptrace */
				t->t_flags |= TTRCSIG;

				/* Stop the process */
				stop(p, signo);

				/* 
				 * stop the current thread and give the 
				 * debugger a chance to run.
				 */
				stop_thread(t);

				/* Reset identification */
				t->t_flags &= ~TTRCSIG;
#ifdef _POWER_MP
				simple_unlock(&proc_int_lock);
#endif
				if (!(p->p_flag & STRC)    ||
				     (p->p_int & (SSUSPUM|STERM)))
					goto dbg_detached;
#ifdef _POWER_MP
				simple_unlock(&proc_base_lock);
#endif
				i_enable(INTBASE);

			} while (!procxmt());

			oldpri = disable_lock(INTMAX, &proc_base_lock);
dbg_detached:
			/*
			 * If the traced bit got turned off (PT_DETACH),
			 * go back up to the top to rescan signals.
			 * This ensures that p_sig* and u_signal are consistent.
			 *
			 * If parent wants us to take the signal,
			 * then it will leave it in t->t_cursig;
			 * otherwise we just look for signals again.
			 */
			signo = t->t_cursig;
			if ((signo == 0) || (p->p_flag & STRC) == 0) 
			{
				if (signo)
					ADD_PENDING_SIG(t,p,signo,procsig);
				continue;
			}
			/*
			 * If signal is being masked put it back
			 * into t/p_sig and look for other signals.
			 */
			if (SIGISMEMBER(t->t_sigmask,signo))
			{
				ADD_PENDING_SIG(t,p,signo,procsig);
				continue;
			}
		}
		/* found signal to deliver */
		ASSERT((t->t_cursig == signo) && (signo != 0));
		if (SIG_DFL == U.U_signal[signo])
		{
			switch (signo) {
			case SIGTSTP:
			case SIGTTIN:
			case SIGTTOU:
			       	/*
				 * POSIX 3.3.1.3: discard SIGTSTP, SIGTTIN,
			         *                SIGTTOU if signal would have
				 *                stopped the process.
			         */
				if (p->p_flag & SORPHANPGRP) {
				    continue;
				}
				/* keep going ... */
			case SIGSTOP:
				if (p->p_flag & STRC)
					continue;
				/* 
				 * a SIGCONT may have been posted during
				 * signal delivery and recheck t/p_sig	
				 */
				IS_PENDING_SIG(p,SIGCONT,pending);
				if (!pending)
				    stop_thread(t);
				goto issig_top;
				/* NOTE: signal delivery is complete */
			case SIGCONT:
			case SIGURG:
			case SIGWINCH:
			case SIGIO:
			case SIGCHLD:
			case SIGDANGER:
			case SIGPWR:
			case SIGWAITING:
				/*
				 * These signals are normally not
				 * sent if the action is the default.
				 */
				continue;		/* == ignore */
			default:
				goto send;
			}
		}
		else if (SIG_IGN == U.U_signal[signo])
		{
			/*
			 * Masking above should prevent us
			 * ever trying to take action on a held
			 * or ignored signal, unless process is traced.
			 */
			ASSERT(p->p_flag & STRC);
			continue;
		}
		else
		{
			/*
			 * This signal has an action, let
			 * psig process it.
			 */
			goto send;
		}
	}
	/*
	 * Didn't find a signal to send.
	 * Reset t_cursig; it could be left set to some signal not delivered
	 * due to ptrace (procxmt).
	 */
	t->t_cursig = 0;
	t->t_scp = 0;
	t->t_flags &= ~TSIGAVAIL;
	unlock_enable(oldpri, &proc_base_lock);
	return (0);
send:
	/* 
	 * Return non-zero; psig() should be called to process caught 
	 * signals and signals requiring termination of the process.
	 */
	ASSERT(signo != 0);
	t->t_flags |= TSIGAVAIL;
	unlock_enable(oldpri, &proc_base_lock);
	return (signo);
}

/*
 * NAME: stop
 *
 * FUNCTION: requests suspension of a process
 *
 * NOTES:    
 * 	This function also suspends threads in the process, if 
 *		1) they are not running and they are in user mode 
		2) they are sleeping interruptibly.
 *
 * DATA STRUCTURES:
 *	changes the process state field of the process block
 *
 * RETURN VALUES: 
 *	NONE
 */
void
stop(struct proc *p, int signo)
{
	register struct thread *t;	/* the current thread           */
	register struct ptipc *ipc;	/* IPC area pointer             */ 
	register int change;		/* suspend thread or not	*/
	register int hadintlock;	/* held proc_int_lock on input  */

	ASSERT(csa->intpri == INTMAX);
	ASSERT(p->p_stat == SACTIVE || p->p_stat == SSWAP);
#ifdef _POWER_MP
	ASSERT(lock_mine(&proc_base_lock));
#endif

	/* Don't suspend exiting/execing/core dumping processes */
	if (p->p_int & (SSUSPUM|STERM))
		return;

#ifdef _POWER_MP
	if (!(hadintlock = lock_mine(&proc_int_lock)))
		simple_lock(&proc_int_lock);
#endif

	p->p_int |= SSUSP;

	/* The signal that stopped the process is part of the wait status */
	p->xp_stat = signo;

	/* The process may be partially swapped out */
	schedsig(p);

	t = p->p_threadlist;
	do {
		change = FALSE;
		
		switch(t->t_wtype) {
		case TNOWAIT :
			/* Thread is either running, stopped or terminated. */
			break;
		case TWCPU :
			/* If thread is on runq and in user mode */
			if (t->t_suspend == 0) {
				change = TRUE;
				remrq(t);
			}
			break;
		default :
			/* If thread is in a long sleep. */
			if ((t->t_flags & TWAKEONSIG) && (t->t_state != TSSTOP))
				change = TRUE;
			break;
		}

		/* Complete state transition */
		if (change) {
			p->p_suspended++;
        		TRCHKL3T(HKWD_KERN_KSUSPEND, t->t_tid, 
				 	p->p_suspended, p->p_active);
			t->t_state = TSSTOP;
		}
				
		t = t->t_nextthread;
	} while (t != p->p_threadlist);

	/* if everybody is suspended */
	if (p->p_suspended == p->p_active) {
                struct  proc    *pp;    	/* ptr p's parent process */

		/* The thread that has not been stopped yet. */
		ASSERT(!(p->p_flag & STRC));

                /*
                 * POSIX 3.3.4: Unless parent process set SA_NOCLDSTOP,
                 * send SIGCHLD to parent process.
                 */
                pp = PROCPTR(p->p_ppid);
                if (!(pp->p_flag & SPPNOCLDSTOP))
                {
                       	pidsig(p->p_ppid, SIGCHLD);
                       	ep_post(EVENT_CHILD,p->p_ppid);
                }
#ifdef _POWER_MP
		fetch_and_and((int *)&p->p_atomic, ~SWTED);
#else
		p->p_atomic &= ~SWTED;
#endif
		p->p_stat = SSTOP;
                e_wakeup((int *)&p->p_synch);
	}

#ifdef _POWER_MP
	if (!hadintlock)
		simple_unlock(&proc_int_lock);
#endif
}

/*
 * NAME: stop_thread
 *
 * FUNCTION: puts the thread in STOP state, signals and/or wakes up parent
 *
 * DATA STRUCTURES:
 *	changes the thread state field in the thread structure
 *
 * RETURN VALUES: 
 *	NONE
 */
void
stop_thread(struct thread *t)
{
	register struct proc *p; 	/* the current process          */
	register struct ptipc *ipc;	/* IPC area pointer             */ 
	register int hadintlock ;	/* held proc_int_lock on input  */ 

	p = t->t_procp;

	ASSERT(csa->prev == NULL);
	ASSERT(csa->intpri == INTMAX);
	ASSERT(curthread == t);
	ASSERT(p->p_stat == SACTIVE || p->p_stat == SSWAP);
#ifdef _POWER_MP
	ASSERT(lock_mine(&proc_base_lock));
#endif

	/* Don't suspend exiting/execing/core dumping processes */
	if (p->p_int & (SSUSPUM|STERM))
		return;

#ifdef _POWER_MP
	if (!(hadintlock = lock_mine(&proc_int_lock)))
		simple_lock(&proc_int_lock);
#endif

	/* Consider oneself suspended */
	p->p_suspended++;		

       	TRCHKL3T(HKWD_KERN_KSUSPEND, t->t_tid, p->p_suspended, p->p_active);

	/* if everybody is suspended */
	if (p->p_suspended == p->p_active) {

        	if (p->p_flag & STRC) {
                	ipc = p->p_ipc;
                	e_wakeup (&ipc->ip_event);
                	ep_post (EVENT_CHILD, ipc->ip_dbp->p_pid);
        	}
        	else
        	{
                	struct  proc    *pp;    /* ptr p's parent process */

                	/*
                  	 * POSIX 3.3.4: Unless parent process set SA_NOCLDSTOP,
                 	 * send SIGCHLD to parent process.
                 	 */
                	pp = PROCPTR(p->p_ppid);
                	if (!(pp->p_flag & SPPNOCLDSTOP))
                	{
                        	pidsig(p->p_ppid, SIGCHLD);
                        	ep_post(EVENT_CHILD,p->p_ppid);
                	}
        	}
#ifdef _POWER_MP
		fetch_and_and((int *)&p->p_atomic, ~SWTED);
#else
		p->p_atomic &= ~SWTED;
#endif
		p->p_stat = SSTOP;
                e_wakeup((int *)&p->p_synch);
	}

	t->t_state = TSSTOP;	

#ifdef _POWER_MP
	simple_unlock(&proc_int_lock);
	locked_swtch();			/* choose another thread to run */
	simple_lock(&proc_base_lock);
	if (hadintlock)
		simple_lock(&proc_int_lock);
#else
	swtch();			/* choose another thread to run */
#endif
}

/*
 * NAME: cont
 *
 * FUNCTION: continues a process formerly stopped or stopping
 *
 * NOTES:    This function must be called with interrupts disabled (and
 *		proc_int_lock held if _POWER_MP)
 *
 * RETURN VALUES:
 *	NONE
 */
void
cont(struct proc *p)
{
	register struct thread *th;	/* to traverse list */
	register int hadintlock; 	/* held proc_int_lock on input */

	ASSERT(csa->intpri == INTMAX);
#ifdef _POWER_MP
	ASSERT(lock_mine(&proc_base_lock));
	if (!(hadintlock = lock_mine(&proc_int_lock)))
		simple_lock(&proc_int_lock);
#endif

	p->p_int &= ~(SSUSP);

	th = p->p_threadlist;
	do {
		if ((th->t_state == TSSTOP) && 
		    (!(th->t_flags & TSETSTATE) || th->t_suspend))
		{
			if (th->t_wtype == TNOWAIT)
				setrq(th, E_WKX_PREEMPT, RQTAIL);
			else if (th->t_flags & TWAKEONSIG) { 
			    	if (th->t_flags & (TINTR|TSUSP))
					setrq(th, E_WKX_PREEMPT, RQTAIL);
			    	else if (SIG_AVAILABLE(th, p)) {
					th->t_flags |= TSIGAVAIL;
					setrq(th, E_WKX_PREEMPT, RQTAIL);
				}
				else {
					th->t_flags &= ~TSIGAVAIL;
					th->t_state = TSSLEEP;
				}
			}	
			else 
				th->t_state = TSSLEEP;
			p->p_suspended--;
			switch(th->t_cursig) {
			case SIGSTOP :
			case SIGTSTP :
			case SIGTTIN :
			case SIGTTOU :
				th->t_cursig = 0;
			}
		}
		th = th->t_nextthread;
	} while (th != p->p_threadlist);

	p->p_stat = SACTIVE;

#ifdef _POWER_MP
	if (!hadintlock)
		simple_unlock(&proc_int_lock);
#endif
}

/*
 * NAME: psig
 *
 * FUNCTION: take signal action (calls kexit or sendsig) for t_cursig
 *
 * NOTE:
 *	normally called after issig() if issig() returns non-zero
 *	In AIX V1 and AIX V2 psig did not take parameter; this changed
 *	because psig() may now be called at dispatcher time; and because
 *	the svc handler does not save the register contents in the same
 *	way or place as the dispatcher.
 *
 * RETURN VALUES: 
 *	NONE
 *	if action is to terminate process, never returns
 */
psig(struct sigcontext *sc)
/* struct sigcontext	*sc;		pointer to saved context */
{
	struct thread	*t;	/* current thread block pointer */
	struct uthread	*ut;	/* current uthread block pointer */
	struct proc	*p;	/* current process block pointer */
	int		signo;	/* signal whose action is being taken */
	void		(*action)();
	int		oldpri;	/* saved interrupt priority */

	t = curthread;
	p = t->t_procp;
	signo = t->t_cursig;

	ASSERT(signo != 0);

	if (p->p_pid > 0)		/* if we're not booting */
		fetch_and_add(&(U.U_ru.ru_nsignals), 1);

	t->t_cursig = 0;		/* reset the signal */

	/* serialize use of proc signal fields */
	oldpri = disable_lock(INTMAX, &proc_base_lock);

	action = U.U_signal[signo];
	if (action != SIG_DFL) 
	{
		ut = t->t_uthreadp;

		ASSERT(action != SIG_IGN && !SIGISMEMBER(t->t_sigmask,signo));

		/* 
		 * If oldstyle delivery requested, reset action to SIG_DFL
		 * and do not mask this signal during signal delivery.
		 */
		if (U.U_sigflags[signo] & SA_OLDSTYLE )
		{	if (signo != SIGILL && signo != SIGTRAP)
			{
				SIGDELSET(p->p_sigcatch,signo);
				U.U_signal[signo] = SIG_DFL;
			}
			/* 
			 * Change current signal mask to signals currently 
			 * masked ORed with signal handler mask (but NOT 
			 * current signal).
			 */
			SIGORSET(t->t_sigmask,U.U_sigmask[signo]);
		}
		else
		/* 
		 * If newstyle delivery, change current signal mask to 
		 * signals currently masked ORed with signal handler mask
		 * ORed with current signal.
		 */
		{
			SIGADDSET(t->t_sigmask,signo);
			SIGORSET(t->t_sigmask,U.U_sigmask[signo]);
		}

		SIGDELSET(t->t_sigmask,SIGKILL);
		SIGDELSET(t->t_sigmask,SIGSTOP);
		
		if (!(SIG_AVAILABLE(t,p)))
			t->t_flags &= ~TSIGAVAIL;

		unlock_enable(oldpri, &proc_base_lock);

		/* 
		 * Call sendsig to branch to the signal handler.
		 */
		sendsig(action, sc, signo);
	}
	/* action is termination type default */
	else
	{	
		unlock_enable(oldpri, &proc_base_lock);

		switch (signo) {
		/* 
		 * Create core file for those signals whose default action
		 * include this function.
		 */
		case SIGILL:
		case SIGIOT:
		case SIGBUS:
		case SIGQUIT:
		case SIGTRAP:
		case SIGFPE:
		case SIGSEGV:
		case SIGSYS:
		case SIGPRE:
		case SIGEMT:
			if (core(signo, sc) == 1)
				/* Indicate successful core dump */
				signo += 0x80;
			break;
		default:
			break;
		}
		/* Set accounting flag - killed by signal */
		U.U_acflag |= AXSIG;

		/* 
		 * Exit with correct value indicating signal number and
		 * whether core failed.
		 */
		kexit(signo);
	}
}


/*
 * NAME: sig_chk
 *
 * FUNCTION:    Identifies a pending signal.  If called by a kernel only 
 *		thread, then the signal is removed from the pending mask 
 *		and t_cursig is cleared.  If called from a kernel thread 
 *		then the signal is lefting pending.  This feature allows
 *		kernel extension to take signal dependent actions for
 *		threads in user processes without clearing the signal.
 *
 * RETURNS: 0 - no signal pending
 *	    > 0 the signal number
 *
 * ENVIRONMENT: callable by any process
 *
 */

sig_chk()
{
	struct thread *t;	/* the current thread */
	struct proc *p;		/* the current proc */
	int signo;		/* return the signal number */
	int ipri;		/* the callers interrupt priority */

	t = curthread;

	ipri = disable_lock(INTMAX, &proc_base_lock);

	/*
	 * check_sig returns the number of the signal that should
	 * be delivered next.  Stop signals are an exception, since
	 * they are delivered by stopping the curent thread. The signal
	 * to be delivered is removed from the pending mask and 
	 * promoted to t_cursig, which quarantees that it and not another
	 * signal will be delivered next.  However, since kprocs don't 
	 * run in user mode, this function is used instead for signal 
	 * delivery.  Therefore, clear the signal, if this is a 
	 * kthread, since it has been delivered.
	 */ 
	signo = check_sig(t, 0);	

	if (t->t_flags & TKTHREAD) 
		t->t_cursig = 0; 		/* Clear the signal */

	unlock_enable(ipri, &proc_base_lock);

	return(signo);
}
