static char sccsid[] = "@(#)59	1.40.1.17  src/bos/kernel/proc/sigsvcs.c, sysproc, bos412, 9445C412b 11/6/94 17:22:24";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: kill
 *		pause
 *		sigpending
 *		sigpriv
 *		sigstack
 *		thread_kill
 *
 *   ORIGINS: 27, 3, 83
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1993
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
#include <sys/sleep.h>
#include <sys/trchkid.h>
#include <sys/syspest.h>
#include <sys/id.h>
#include <sys/atomic_op.h>
#include "sig.h"

extern int copyin();			/* copies from user space to kernel  */
extern int copyout();			/* copyies form kernel space to user */
extern uid_t getuidx(int);

/*
 * NAME: sigstack
 *
 * FUNCTION: changes signal stack and flag of whether running on signal stack
 *
 * DATA STRUCTURES:
 *	the process signal stack pointer in u_block
 *	the onstack indicator in the process block
 *
 * RETURN VALUES: 
 *	0 => successful
 *     -1 => unsuccessful, errno set to cause of failure
 *	    EFAULT = signal stack is not in process address space
 */
sigstack(struct sigstack *nssp, struct sigstack *ossp)
/* struct sigstack *nssp;	 new signal stack information */
/* struct sigstack *ossp;	 buffer to return current signal stack info */
{
	struct sigstack kss;	/* kernel copy of stack information */
	struct sigstack *kssp = &kss;
	struct uthread 	*ut;

	ut = curthread->t_uthreadp;

	/* put current signal stack info in user's buffer is desired */
	if (ossp != NULL) 
	{
		kssp->ss_sp = ut->ut_sigsp;
		if ( ut->ut_flags & UTSIGONSTACK )
			kssp->ss_onstack = 1;
		else
			kssp->ss_onstack = 0;
		if ( copyout((caddr_t)kssp, (caddr_t)ossp, 
		    		sizeof (struct sigstack)) )
		{	ut->ut_error = EFAULT;
			goto out;
		}
	}
	/* set process's signal stack info as requested */
	if (nssp != NULL) 
	{	
		if ( copyin((caddr_t)nssp, (caddr_t)kssp, 
				sizeof (struct sigstack)) )
		{	ut->ut_error = EFAULT;
			goto out;
		}
		/* save special signal stack */
		ut->ut_sigsp = kssp->ss_sp;
		/* set indicator of whether process is currently running on
		   the special signal stack */
		if (kssp->ss_onstack == 1)
			ut->ut_flags |= UTSIGONSTACK;
		else
			ut->ut_flags &= ~UTSIGONSTACK;
	}
out:
	if (ut->ut_error != 0)
		return(-1);
	else
		return(0);
}

/*
 * NAME: kill()
 *
 * FUNCTION: send a signal to a process or multiple processes
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a process
 *
 * RETURN VALUES: 
 *	0 = successful
 *	-1 = failed, errno indicates cause of failure
 */ 

kill(pid_t pid,			/* process(es) or process group to signal */
     int sig)			/* signal number to send		  */
{
    register struct proc *p;
    int	ipri;					/* saved interrupt priority  */
    uid_t   effuid;
    uid_t   realuid;
    int     privcheck_rc;

    TRCHKGT_SYSC(KILL, pid, sig, NULL, NULL, NULL);

    /* Check for valid signal */
    if (sig < 0 || sig > SIGMAX) {
	u.u_error = EINVAL;
	return(-1);
    }

    effuid = getuidx(ID_EFFECTIVE);
    realuid = getuidx(ID_REAL);
    privcheck_rc = privcheck(BYPASS_DAC_KILL);

    simple_lock(&proc_tbl_lock);

    switch (pid) {
    case -1:
	/* 
	 * Generate signal to all processes except proc0 and proc1, 
	 * e.g. swapper and init.  
	 */
	if ( sig != 0) {
	    for(p = (struct proc *)&proc[2]; p < max_proc; p++)
	    {
		if (p->p_stat == SNONE)
		    continue;

		if ( sigpriv(p, sig, privcheck_rc, effuid, realuid) )		/* check user privileges */
		    pidsig(p->p_pid, sig);

		/* Don't set EPERM when signalling multiple procs */
	    }
	}
	break;

    case 1:
	/* Prevent proc 1 (init) from receiving these signals */
	switch(sig)	{
	case SIGKILL:
	case SIGSTOP:
	case SIGTSTP:
	case SIGCONT:
	    u.u_error = EINVAL;
	    goto out;
	}
	/* fall through */

    default:
	if (pid > 0) {				/* send to a process */
	    p = VALIDATE_PID(pid);
	    if ( (p == 0) || (p->p_stat == SNONE) ) {
		u.u_error = ESRCH;
		goto out;
	    }

	    /* 
	     * Generate signal to single specified process
	     * Note: should not be able to signal proc0	
	     */
	    if  ( sigpriv(p, sig, privcheck_rc, effuid, realuid) ) {
		if ( sig != 0 ) 
		    pidsig(pid, sig);
	    } else {
		u.u_error = EPERM;
		goto out;
	    }
	} else {
	    if (pid == 0)		/* signal caller's process group */
		pid = u.u_procp->p_pgrp;
	    else			/* pid is < -1, send to pgrp -pid */
		pid = -pid;
	    p = VALIDATE_PID(pid);

	    /*
	     * Check that the pid corresponds to a process slot,
	     * the slot anchors a process group, and
	     * the process group is the requested process group.
	     */
	    if (!p || !(p = p->p_ganchor) || p->p_pgrp != pid) {
		u.u_error = ESRCH;	/* no process group */
		goto out;
	    }

	    for ( ; p ; p = p->p_pgrpl) {
		if ( !sigpriv(p, sig, privcheck_rc, effuid, realuid) )
		    u.u_error = EPERM;
		else if (sig)
		    pidsig(p->p_pid, sig);	
	    }
	}
	break;
    }

out:
    simple_unlock(&proc_tbl_lock);

    return (u.u_error ? -1 : 0);
}

/*
 * NAME: thread_kill()
 *
 * FUNCTION: send a signal to a thread
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a thread
 *
 * RETURN VALUES:
 *	0 = successful
 *	-1 = failed, u.u_error set to
 *		ESRCH,  if tid is invalid
 *		EINVAL, if sig is invalid
 *		EPERM,  if target thread not in current one's process
 */
int
thread_kill(tid_t tid,		/* thread to signal		*/
	    int sig)		/* signal number to send	*/
{
	register struct thread *t;

	/* Check for valid signal */
	if (sig < 0 || sig > SIGMAX) {
		u.u_error = EINVAL;
		return(-1);
	}

	simple_lock(&proc_tbl_lock);

	if (tid == -1)
		tid = curthread->t_tid;
	else {
		t = VALIDATE_TID(tid);

		if (!t || (t->t_state == TSNONE)) {
			simple_unlock(&proc_tbl_lock);
			u.u_error = ESRCH;
			return(-1);
		}
		if (t->t_procp != curproc) {
			simple_unlock(&proc_tbl_lock);
			u.u_error = EPERM;
			return(-1);
		}
	}

	if (sig)
		tidsig(tid, sig);

	simple_unlock(&proc_tbl_lock);

	return (0);
}


/*
 * NAME: pause()
 *
 * FUNCTION: This routine exists to allow the lipthreads code to have
 *           a point at which it can intercept the call to pause
 *
 *	     Control flows directly to pause, below
 *
 */
_pause(void)
{
	return pause();
}



/*
 * NAME: pause()
 *
 * FUNCTION: wait for a signal
 *
 * RETURN VALUES: 
 *	-1 = interrupted by signal, otherwise signal must have terminated proc
 */
pause(void)
{
	int	sleeprtn;

	/*
	 * et_wait will just put us to sleep, and since nobody will do a 
	 * et_post to this process, the signal will cause us to be made
	 * ready
	 */
	do {
		sleeprtn = et_wait(EVENT_SHARED, EVENT_SHARED, EVENT_SIGRET);
	} while (sleeprtn != EVENT_SIG);
	
	/* signal interrupted sleep, set errno accordingly */
	u.u_error = EINTR;
	return(-1);
}

/*
 * NAME: _sigpending()
 *
 * FUNCTION: This routine exists to allow the lipthreads code to have
 *           a point at which it can intercept the call to sigpending
 *
 *	     Control flows directly to sigpending, below
 *
 */
_sigpending( sigset_t *set )
{
	return sigpending( set );
}

/*
 * NAME: sigpending()
 *
 * FUNCTION: Report what signals are pending
 *
 * NOTE:
 *	No lock is used since the value is returned to user mode and is
 *	therefore inherently unstable.
 *
 * RETURN VALUES: 
 *	A return value of -1 is returned if "set" is invalid 
 *	otherwise a zero is returned.
 */
sigpending( sigset_t *set )
{
	sigset_t tempsig;
	struct thread *t;
	struct proc *p;

	t = curthread;
	p = t->t_procp;

	tempsig.losigs = (p->p_sig.losigs|t->t_sig.losigs)&t->t_sigmask.losigs;
	tempsig.hisigs = (p->p_sig.hisigs|t->t_sig.hisigs)&t->t_sigmask.hisigs;

	if (copyout( &tempsig, set, sizeof( sigset_t )))
	{
		u.u_error = EINVAL;
		return( -1 );
	}

	return( 0 );
}

/*
 * NAME: sigpriv()
 *
 * FUNCTION: check current process for privilege to signal another process
 *
 * RETURN VALUES:
 *      1 = has privilege
 *      0 = does not have privilege
 * NOTE: A SIGCONT can be sent to any process in the same session
 *       unless extended security controls impose further implementation-
 *       defined restrictions.
 */
static int
sigpriv(struct proc *p, int signo, int privcheck_rc, int effuid, int realuid)
/* struct proc  *p;             pointer to process block to check */
/* int          signo;          signal number */
{
        /* check to see if current process has permission to kill
           the process whose process block is specified: must
           be superuser or have real/effective uid match real/effective
           uid of process specified.  Or for SIGCONT, caller is in
           the same session as p, but see "NOTE" above.              */
        if ( (privcheck_rc == 0) ||
             effuid == p->p_uid           ||
             realuid == p->p_uid          ||
             effuid == p->p_suid          ||
             realuid == p->p_suid         ||
             (signo == SIGCONT && u.u_procp->p_sid == p->p_sid) )
                return(TRUE);           /* current proc has kill priv */
        else
                return(FALSE);          /* current proc does NOT have priv */
}
