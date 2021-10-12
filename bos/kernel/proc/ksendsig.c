static char sccsid[] = "@(#)52	1.32  src/bos/kernel/proc/ksendsig.c, sysproc, bos41J, 9517A_all 4/25/95 17:57:16";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: sendsig
 *		sigcleanup
 *
 *   ORIGINS: 27, 83
 *
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#include <sys/user.h>
#include <sys/proc.h>
#include <sys/signal.h>
#include <sys/mstsave.h>
#include <sys/reg.h>
#include <sys/context.h>
#include <sys/intr.h>
#include <sys/pseg.h>
#include <sys/trchkid.h>
#include <sys/adspace.h>
#include <sys/syspest.h>
#include <sys/atomic_op.h>
#include "sig.h"

extern void kgetsig();		/* label of glue code that will call "func" */

/*
 * NAME: sendsig()
 *
 * FUNCTION: save sigcontext; setup call to signal handler through kgetsig()
 *
 * EXECUTION ENVIRONMENT: optional; 
 *      May page fault on user data
 *      May be called from interrupt handler: YES, called from dispatcher
 *      Called with interrupts disabled (?)
 *      Pinned: YES
 *
 * NOTES: 
 *      sendsig() is called from psig.
 *      sendsig() also changes the machine state save area so that
 *      kgetsig() will be run when the thread resumes in normal 
 *      thread execution mode. 
 *      kgetsig() is responsible for calling the signal handler with
 *              the parameters setup by sendsig, and calling sigreturn()
 *              to restore the saved signal context.
 *
 * INPUT:
 *      u_sigonstack determines whether to switch stacks for the handler
 *      u_sigstack determines the special signal stack, & whether on it already
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUES: 
 *      NONE
 *
 * EXTERNAL PROCEDURES CALLED: 
 */

static	ulong	fp_sigf = '\0';		/* the func descriptor for sigreturn */

/* 
 * The following structure is the list of parameters passed to kgetsig(). 
 * The structure will be an automatic and passed on the stack to kgetsig()
 */
struct	sig_save {
	int	signo;			/* the signal number */
	int	code;			/* exception code */
	struct	sigcontext *scp;	/* ptr to save context */
	ulong_t	msr;			/* user msr value */
	char	*sh_fp;			/* func descriptor for signal handler */
	char	*stkp;			/* stack ptr */
	char	*sh_ret;		/* function descriptor for sigreturn */
	vmhandle_t sreg;		/* segment register val for scp */
};
	
void	
sendsig(char *func, struct sigcontext *scp, int signo)
{
        struct proc *p;			/* current process */
	struct thread *t;		/* current thread */
	struct uthread *ut;		/* current uthread */
	struct sig_save parms; 		/* parameters for kgetsig() */

	/*
	 * This is system initialization code that is placed here for
	 * convenience but should really be done elsewhere.  sigreturn is 
	 * reached by branching through kernel glue code in kgetsig, after 
	 * returning from the signal handler.  At this point the code is
	 * executing in user mode.  We need to do the linking because it
	 * is kernel code. 
	 */
	if (fp_sigf == '\0')
		fp_sigf = (ulong)(ld_svcaddress("sigreturn"));
	
	t = curthread;
	ut = t->t_uthreadp;
	p = t->t_procp;

	/* reset sigcontext pointer */
	t->t_scp = 0;

	parms.signo = signo;
	if (signo == SIGILL || signo == SIGFPE) {
		parms.code = ut->ut_code;
		ut->ut_code = 0;
	}
	else
		parms.code = 0;
	parms.scp = scp;
	parms.msr = ut->ut_msr; 
	parms.sh_fp = func;  
	parms.sh_ret = (char *)(fp_sigf);
	parms.stkp = (char *)ut->ut_scsave[1];
	parms.sreg = ut->ut_scsave[0]; 

	/*
	 * The mask is substituted if and only if a signal handler 
	 * is called.  It is not substititued at the occurrence of
	 * a signal (sig_slih), because a handler is not necessarily 
	 * called.  Particularly in the case of ptrace.  
	 */
	if (ut->ut_flags & UTSIGSUSP) {
		if (copyout(&ut->ut_oldmask, &scp->sc_mask, sizeof(sigset_t)))
			/* target curthread to help debug */
			kthread_kill(-1, SIGKILL); 
		ut->ut_flags &= ~UTSIGSUSP;
	}
	
	/* kgetsig does not return */
	kgetsig(&parms, t);
}

/*
 * NAME: sigcleanup
 *
 * FUNCTION: changes the thread's signal mask and signal stack state
 *
 * NOTES:
 *      sigcleanup() is called by longjmp().  Unlike sigreturn(), it is
 *	always called from user mode. longjmp() uses the signal mask and
 *	onstack indicator in the sigcontext structure it is passed.
 *
 * RETURN VALUES: 
 *      0 - always successful 
 */
int 
sigcleanup(sigset_t *sigmask)
{
	struct	proc	*p;	/* pointer to current process block */
	struct thread	*t;	/* pointer to current thread block */
	register int	oldpri;		/* interrupt priority */

	TRCHKT_SYSC(SIGCLEANUP);

	oldpri = disable_lock(INTMAX, &proc_base_lock);

	t = curthread;
	p = t->t_procp;

        /* 
	 * If not using the SVID sigset interface restore saved signal mask; 
	 * check for unblockable signals in case signal handler changed 
         * sigcontext 
	 */
	if(!(p->p_flag & SSIGSET)) {
		unlock_enable(oldpri, &proc_base_lock);
		if (copyin(sigmask, &t->t_sigmask, sizeof(sigset_t))) {
			kthread_kill(-1, SIGKILL);
			return;
		}
		oldpri = disable_lock(INTMAX, &proc_base_lock);
	}
	SIGDELSET(t->t_sigmask,SIGKILL);
	SIGDELSET(t->t_sigmask,SIGSTOP);
	if (SIG_AVAILABLE(t,p))
		t->t_flags |= TSIGAVAIL;

	unlock_enable(oldpri, &proc_base_lock);

	return(0);
}
