static char sccsid[] = "@(#)09	1.86  src/bos/kernel/proc/POWER/sig_slih.c, sysproc, bos41J, 9519A_all 4/29/95 15:32:03";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: check_sig
 *		sig_deliver
 *		sig_process
 *		sig_setup
 *		sig_slih
 *		sigreturn
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


#include <sys/param.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/pseg.h>
#include <sys/reg.h>
#include <sys/machine.h>
#include <sys/vmker.h>
#include <sys/adspace.h>
#include <sys/intr.h>
#include <sys/errno.h>
#include <sys/trchkid.h>
#include <sys/uio.h>
#include <sys/systm.h>
#include <sys/syspest.h>
#include <sys/mstsave.h>
#include <sys/systemcfg.h>
#include <sys/atomic_op.h>
#include "sig.h"

extern	copyin();
extern  int fsig();		/* get lowest signal number from set */
extern  void tsig();
extern  void sig_deliver();
extern  void sigreturn();

static struct func_desc *deliver_ptr = (struct func_desc *)sig_deliver;


/*
 *  Name:       sig_slih() -- signal "second level interrupt handler"
 *
 *  Function:
 *	
 *	Check why sig_slih has been entered :
 *
 *	o STERM/TTERM : termination
 *	  -- Modify thread-level mstsave area so that:
 *		IAR points to entry point sig_deliver (-> thread_terminate)
 *	  -- Return(0) to resume that mst
 *
 *	o SSUSP/TSUSP : suspension
 *	  -- Call ksuspend
 *	  -- Return(1) to invoke the dispatcher
 *
 *	o TSIGAVAIL : signal
 *	  -- Call sig_process
 *	  -- Return(0) to resume that mst
 *
 *	o SGETOUT : swapout
 *	  -- Call sched_swapout
 *	  -- Return(1) to invoke the dispatcher
 *
 * Notes:
 *	  -- Entered from call_sigslih() on a new-bought mst,
 *		INTMAX prio set in our mstsave area.
 *	  -- if POWER_MP, we return with the proc_base_lock held
 *		if we are to invoke the dispatcher (code 1) to
 *		ensure atomicity of state change. When returning
 *		for a simple resumption of the mst (code 0), the
 *		proc_base_lock is not held. That logic is in
 *		ksuspend() and sched_swapout().
 */

int
sig_slih()
{
	struct thread	  *t;			/* current thread */
	struct uthread	  *ut;			/* current uthread */
	struct proc	  *p;			/* current proc */
	struct ppda	  *myppda = PPDA;	/* current ppda */
	struct mstsave	  *mycsa;		/* current csa */
	int		  sregval;		/* segment register value */
	int		  **atterrnopaddr;	/* attached address of errnop */
	int		  *errnoaddr;		/* real address of errno */
	int		  *atterrnoaddr;	/* attached address of errno */

	mycsa = myppda->_csa;

	ASSERT(mycsa->intpri == INTMAX);
      	
	t = myppda->_curthread;
	p = t->t_procp;

	/*
	 * We may be here due to a termination request (STERM/TTERM). This
	 * request is never reset, therefore we can safely check it without
	 * any lock.
	 */
	if (p->p_int & STERM || t->t_flags & TTERM) {
		sig_setup(0, NULL);
		return(0);
	}

	ut = t->t_uthreadp;

	/* 
	 * If coming from SVC, errno has not been updated yet.  This
	 * path may have been taken for reasons other than signal
	 * delivery.  ie. swapping or job control.  Therefore, we 
	 * cannot depend on the signal delivery path to straighten
	 * out errno.
	 */
	if (ut->ut_flags & UTSCSIG && ut->ut_error != 0) {

		/*
		 * While updating errno, we may fault.  If the reason
		 * for taking this path (ie. SGETOUT) is no longer valid
		 * when the fault is handled, then errno may not get updated,
		 * because the thread may be resumed as if it returned from 
		 * an interrupt.  Therefore, we need to force the system to 
		 * take this path again.  In addition, leave TSIGAVAIL 
		 * pending.  It will be cleared up in sigprocess. 
		 */ 
#ifdef _POWER_MP
		simple_lock(&proc_base_lock);
#endif
		t->t_flags |= TSIGAVAIL;
#ifdef _POWER_MP
		simple_unlock(&proc_base_lock);
#endif

		/*
		 * Set flag saying a signal is being delivered checked in
		 * p_slih() to determine what to do if an exception occurs.
		 * This field is usually updated with atomic primitives
		 * except in this case, because the thread is in user mode.
		 * Also set backtracking and disable the lru.
		 */
		t->t_atomic |= TSIGDELIVERY;
		mycsa->backt = 1;

		/*
		 * errnop is in user space. Always attach to assure the
		 * appropriate user privilege is followed. as_geth should
		 * be used but we are not at base level !!!
		 */
		sregval = as_getsrval(&U.U_adspace, ut->ut_errnopp);
		atterrnopaddr = (int **)vm_att(sregval, ut->ut_errnopp);
		errnoaddr = *atterrnopaddr;                     /* MAY FAULT */
		vm_det(atterrnopaddr);

		/*
		 * errno is in user space. Always attach to assure the
		 * appropriate user privilege is followed. as_geth should
		 * be used but we are not at base level !!!
		 */
		sregval = as_getsrval(&U.U_adspace, errnoaddr);
		atterrnoaddr = (int *)vm_att(sregval, errnoaddr);
		*atterrnoaddr = (int)ut->ut_error;              /* MAY FAULT */
		vm_det(atterrnoaddr);

		/*
		 * Reset "backtrack" state for our own mst;
		 * we're through page-faulting
		 */
		mycsa->backt = 0;
		t->t_atomic &= ~TSIGDELIVERY;
	}

	/* Check why sig_slih has been entered */

	/*
	 * We may be here due to a suspension request (SSUSP/TSUSP). This
	 * request may be reset, we still test it without lock. The condition
	 * will be rechecked in ksuspend.
	 */
	if ((p->p_int & (SSUSP|SSUSPUM)) || (t->t_flags & TSUSP))
		return(ksuspend());

	/*
	 * We may be here due to a signal (TSIGAVAIL). This may be reset (if
	 * the signal was posted at the process level and has already been
	 * delivered to another thread. We still test it without lock. 
	 * check_sig will recheck signal availability.  If TINTR has been
	 * set and this is an interrupted system call, then suspend the
	 * current thread for thread_setstate.
	 */
	if ((t->t_flags & TSIGAVAIL) ||
	    ((t->t_flags & TINTR) &&
	     (ut->ut_flags & UTSCSIG) &&
	     ((ut->ut_error == EINTR) || (ut->ut_error == ERESTART)))) {
		if (sig_process())
			return(ksuspend());
		return(0);
	}

	/*
	 * We may be here due to a swapout request (SGETOUT). This request may
	 * be reset, we still test it without lock. The condition will be
	 * rechecked in sched_swapout.
	 */
	if (p->p_int & SGETOUT) {
		ut->ut_flags &= ~UTSCSIG;
		return(sched_swapout());
	}

	/*
	 * The reason to run sig_slih has vanished. Just go on resuming the
	 * current thread.
	 */
	ut->ut_flags &= ~UTSCSIG;
	return(0);
}

/*
 *  Name:	sig_process() -- signal delivery processing
 *
 *  Function:
 *
 *        -- Copy thread-level mstsave area to thread's stack,
 *             which is presumed to be pointed to by GPR 1 as saved
 *             in the thread-level mstsave area.  (This may
 *             page-fault, hence the pager backtrack state setting.)
 *        -- When copy is complete, cancel back-track state in
 *             our mstsave area.
 *        -- Modify thread-level mstsave area so that:
 *               IAR points to entry point sig_deliver()
 *               R1 points to new save area on stack, beyond
 *                    copied mstsave
 *               R3 (first parameter) points to copied mstsave
 *                    (the "sigcontext" structure)
 *        -- Suppress signals for this thread by incrementing
 *             t_suspend
 *
 * Notes:
 *	  -- Only called by sig_slih().
 *
 */

int
sig_process()
{
	struct sigcontext *sc, *sc_local;	/* -> sigcontext structure */
	struct sigcontext tempsc;		/* temp sigcontext structure */
	char		  *usp;			/* user's stack pointer value */
	int		  signo;		/* the signal to be delivered */
	int		  sregval;		/* seg reg value for stack seg*/
	int		  signal_stack;		/* user specified signal stk */
	struct uthread	  *ut;			/* current uthread */
	struct thread	  *t;			/* current thread */
	struct mstsave	  *mycsa;		/* current csa */
	struct ppda	  *myppda = PPDA;	/* current ppda */
	int		  **atterrnopaddr;	/* attached address of errnop */
	int		  *errnoaddr;		/* real address of errno */
	int		  *atterrnoaddr;	/* attached address of errno */

	t = myppda->_curthread;
	ut = t->t_uthreadp;

	/* 
	 * Make sure that this thread's floating point state resides in the 
	 * mst, not in the FP registers 
	 */
	disown_fp(t->t_tid);

#ifdef _POWER_MP
	simple_lock(&proc_base_lock);
#endif

	/* 
 	 * Call check_sig (a smaller issig) to determine the sig 
	 * that will be sent. 
	 */
	if ((signo = check_sig(t, 0)) == 0)
	{
		t->t_flags &= ~TSIGAVAIL;
#ifdef _POWER_MP
		simple_unlock(&proc_base_lock);
#endif
		ut->ut_flags &= ~(UTSCSIG);
		goto out;
	}
#ifdef _POWER_MP
	simple_unlock(&proc_base_lock);
#endif

	if (signo == SIGKILL)
	{
		sig_setup(0, NULL);
		goto out;
	}

	mycsa = myppda->_csa;

	/* 
	 * Set flag saying a signal is being delivered checked in p_slih() 
	 * to determine what to do if an exception occurs.  This flag is
	 * typically set with atomic primitives except in this case,
	 * because the thread is in user mode.  Also set backtracking 
	 * and disable the lru.
	 */
	t->t_atomic |= TSIGDELIVERY;
#ifdef _POWER_MP
	myppda->lru = 1;
#endif
	mycsa->backt = 1;

	/* 
	 * errnop is in user space. Always attach to assure the
	 * appropriate user privilege is followed. as_geth should be used
	 * but we are not at base level !!!
	 */
	sregval = as_getsrval(&U.U_adspace, ut->ut_errnopp);
	atterrnopaddr = (int **)vm_att(sregval, ut->ut_errnopp);
	errnoaddr = *atterrnopaddr;                             /* MAY FAULT */
	vm_det(atterrnopaddr);

	/*
	 * errno is in user space. Always attach to assure the
	 * appropriate user privilege is followed. as_geth should be used
	 * but we are not at base level !!!
	 */
	sregval = as_getsrval(&U.U_adspace, errnoaddr);
	atterrnoaddr = (int *)vm_att(sregval, errnoaddr);

	/*
	 * errno is preserved across the signal handler.  It is 
	 * conditionally updated if the system call is restartable.  
	 */
	if (ut->ut_flags & UTSCSIG) 
	{
		if ( (ut->ut_error == ERESTART) &&
	     	    !(U.U_sigflags[signo] & SA_RESTART))
		{
			ut->ut_error = EINTR;
			/*
			 * No need to update errno since it will be restored
			 * from sc_uerror when returning from the handler.
			 */
			tempsc.sc_uerror = EINTR;
		}
		else if (ut->ut_error)
			tempsc.sc_uerror = ut->ut_error;
		else
			tempsc.sc_uerror = *atterrnoaddr;       /* MAY FAULT */

	        /*
		 * If the system call was interrupted and the thread is
		 * cancellable, cancel it now.
	         */
		if ((t->t_flags & TINTR) &&
		    ((ut->ut_error == EINTR) || (ut->ut_error == ERESTART)))
		{
			/* There won't be any return from handler,update errno*/
			*atterrnoaddr = ut->ut_error;  		/* MAY FAULT */
			vm_det(atterrnoaddr);
			/*
			 * Reset "backtrack" state for our own mst;
			 * we're through page-faulting
			 */
			mycsa->backt = 0;
#ifdef _POWER_MP
			myppda->lru = 0;
#endif
			t->t_atomic &= ~TSIGDELIVERY;
			ut->ut_flags &= ~(UTSCSIG);
			return 1;
		}

	}
	else
		tempsc.sc_uerror = *atterrnoaddr;		/* MAY FAULT */

	vm_det(atterrnoaddr);

        /* 
	 * Allocate sigcontext structure on signal handler stack.  User 
	 * specified signal stacks are not re-entrant.  ie. They are good for
	 * one signal at a time.  Switch to standard user stack if a signal
	 * is currently being delivered.  User stack is an offset from gpr1.
	 */
	if (!(ut->ut_flags & UTSIGONSTACK) &&
	     (U.U_sigflags[signo] & SA_ONSTACK) &&
	     (ut->ut_sigsp))
	{
		usp = (char *)ut->ut_sigsp;
		signal_stack = 1;
	}
	else
	{
		usp = (char *)ut->ut_save.gpr[STKP];
		signal_stack = 0;
	}
	sc = (struct sigcontext *)(usp - STACK_FLOOR-sizeof(struct sigcontext));
	sc = (struct sigcontext *)((ulong)sc & ~15);     /* quad-word aligned */

	/* Copy data from process-level mstsave to saved context */
	tempsc.sc_jmpbuf.jmp_context = ut->ut_save;

	/*
	 * Modify the msr field in the signal context presented to the
	 * user to reflect the current trap state as defined by fpinfo.
	 * sigreturn() will recompute fpinfo based upon this msr.  This
	 * allows the msr in the signal context structure to be the interface
	 * by which the user can modify the trapping state.
	 */
	tempsc.sc_jmpbuf.jmp_context.msr &=
		~((FP_IMP_INT | FP_SYNC_TASK) << FP_SYNC_IMP_S);
	tempsc.sc_jmpbuf.jmp_context.msr |=
		(tempsc.sc_jmpbuf.jmp_context.fpinfo &
			(FP_IMP_INT | FP_SYNC_TASK)) << FP_SYNC_IMP_S;

	tempsc.sc_mask = t->t_sigmask;
	tempsc.sc_onstack = 0;

        /*
         * If this signal is to be run on a signal stack, then mark it. 
         * Do not cleared until this signal is finished.  Run intermediate
         * signals on the user stack.  sc_onstack identifies the signal
         * associated with user defined stack.
         */
        if (signal_stack)
                tempsc.sc_onstack = UTSIGONSTACK;

	/* 
	 * Do the structure assignment of the sigcontext structure to the 
	 * user's address.  If this page faults, the data storage interrupt 
	 * handler will call the vmm, who will resolve the page fault and 
	 * restart the user mode context (from its process level mst).  The
	 * same path will be taken through resume to sig_slih.  This entire
	 * subroutine will be re-executed.  Therefore, it must be reentrant. 
	 * No state is changed until after the we are through page faulting.
	 * If the structure assignment below gets an exception, the program
	 * check handler will see that curthread->t_flags has TSIGDELIVERY 
	 * set and will complete the signal delivery mechanism for us by
	 * altering the process level mst to call kexit.  It should also
	 * be noted that U.U_adspace is used, because the mst may contain 
	 * stale values, since it is initialized at the time of the original 
	 * fault.  The U.U_adspace reference is not protected.  If it contains
	 * an invalid segment ID, then the process will be killed.  This
	 * is an application programming error. 
	 */
	sregval = as_getsrval(&U.U_adspace, sc);
	sc_local = (struct sigcontext *) vm_att(sregval, sc);
	bcopy(&tempsc, sc_local, sizeof(struct sigcontext));   /* PAGE FAULT */
	vm_det(sc_local);

	/* 
	 * Reset "backtrack" state for our own mst; 
	 * we're through page-faulting 
	 */
	mycsa->backt = 0;
#ifdef _POWER_MP
	myppda->lru = 0;
#endif
	t->t_atomic &= ~TSIGDELIVERY;
	ut->ut_flags &= ~(UTSCSIG);

	/*
	 * The following variables are preserved so that signal
	 * handler can be properly initialized without examining
	 * the user state in the signal context structure.
	 */
	ut->ut_scsave[0] = sregval; 
	ut->ut_scsave[1] = (ulong)sc - STKMIN;
	ut->ut_msr = ut->ut_save.msr;

	if (signal_stack)
               	ut->ut_flags |= UTSIGONSTACK;

	/* sigctx pointer for ptrace */
	t->t_scp = sc;

	/* 
	 * Store pointer to saved signal context into u_block; modify
	 * user-level mstsave to invoke sig_deliver() with proper stack.
	 */
	sig_setup(sc, NULL);

	/* 
	 * Increment the signal suspension count so that we won't try 
	 * this again before sig_deliver() has run. 
	 */
	t->t_suspend++;

out:
	/*
	TRCHKGT(HKWD_KERN_SIG_SLIH, curproc->p_pid, signo, NULL, NULL, NULL);
	*/

	return 0;
}

/*
 *  Name:       sig_deliver() -- Signal delivery
 *
 *  Function:
 *              Invoked under user process, in supervisor state, when
 *              a signal has been noticed by resume() and is to be
 *              delivered to the thread.
 *
 *		Checks the thread's active list of timer requests.  If
 *		the timer request at the head of the active list is
 *		expired, it is removed from the list and the thread's
 *		next timer request is made active (this way, each thread
 *		may have at most one active timer request).
 *
 *              Calls issig() to see if signal is actually to be delivered.
 *              Calls psig() to deliver the signal, if issig() returns non-zero.
 *              If psig() not called, call sigreturn() to clean up and resume 
 *              the user thread in user mode.
 *
 *  Note:	This routine may page fault.
 */

void
sig_deliver(struct sigcontext *sc)
{
	struct thread *t = curthread;

	/*
	 *  Call tsig() to perform any thread specific timer work such as
	 *  ensuring that the thread's next most immediate timeout is
	 *  made active.  Do that here because that type of thing should be
	 *  done under the thread while executing in supervisor state.
	 */
	tsig();

	if ((t->t_procp->p_int & STERM) || (t->t_flags & TTERM))
		thread_terminate();	/* does not return */

	if (issig()) {			/* decide whether to deliver signal, */
					/*   and which one if multiple exist */
		psig(sc);		/* deliver one of them */
	}				/*   psig does not return */
	sigreturn(sc);			/* cleanup after issig call */
					/*   we never left supervisor state */

	/* 
	 * Note:  sigreturn() doesn't return; it resumes the user 
	 * 	  thread in user mode. 
	 */
}

/*
 *  Name:   sigreturn() -- Finish with signal delivery
 *
 *  Function:
 *           -- Pin the part of the stack containing the saved signal context.
 *           -- Disable interrupts.
 *           -- Copy parts of stacked signal context back into u_block and
 *                user-level mstsave area.  Avoid copying back things that
 *                user-level code cannot legitimately modify, such as segment
 *                register contents and contents of MSR.
 *           -- Unpin the part of the stack pinned above.
 *           -- Exit to sig_slih_resume(), which will resume the thread
 */

void
sigreturn(struct sigcontext *sc)
{
	int                         op;    	/* old interrupt prio */
	register struct mstsave    *umst;  	/* -> user-level mstsave */
	register struct thread	   *th;		/* -> thread structure */
	register struct mstsave    *smst;  	/* -> stacked mstsave */
	struct sigcontext	    locsc;	/* local sigcontext */
	struct sigcontext	   *lsc;
	int			    nbytes;
	int			   *errnoaddr;

	th = curthread;		/* pointer to thread structure */
	ASSERT(!(th->t_flags & TKTHREAD));

	/*
	 * copyin the sigcontext structure, KILL the process on failure
	 * We return on failure here because vmcopyin() should only fail
	 * if we have been called as a system call.
	 */
	lsc = &locsc;
	nbytes = sizeof(locsc);
	if (copyin(sc, lsc, nbytes)) {
		kthread_kill(-1, SIGKILL); /* target curthread to help debug */
		return;
	}

	/* Reset errno from value saved in signal context structure. */
	if ((int)(errnoaddr = (int *)fuword(th->t_uthreadp->ut_errnopp)) == -1){
		kthread_kill(-1, SIGSEGV);
		return;
	}
	suword(errnoaddr, lsc->sc_uerror);

	/* 
	 * Disable interrupts so that we don't lose control with the 
	 * mstsave area and the u-block only partly updated.
	 */
	op = disable_ints();		/* disable interrupts */

	/* Make sure that this process has nothing in the FP registers */
	disown_fp(th->t_tid);
	
	/*
	 * Restore signal mask, onstack from context structure.
	 */
	SIGDELSET(lsc->sc_mask,SIGSTOP);
	SIGDELSET(lsc->sc_mask,SIGKILL);
#ifdef _POWER_MP
	simple_lock(&proc_base_lock);
#endif
	th->t_sigmask = lsc->sc_mask;
	if (SIG_AVAILABLE(th,th->t_procp))
		th->t_flags |= TSIGAVAIL;
#ifdef _POWER_MP
	simple_unlock(&proc_base_lock);
#endif
	if (lsc->sc_onstack)
		th->t_uthreadp->ut_flags &= ~UTSIGONSTACK;

	/* 
	 * Copy the user-modifiable parts of the stacked mstsave back 
	 * to the user-level mstsave, for resume() to load 
	 */
	umst = csa;				/* address of target */
	smst = &lsc->sc_jmpbuf.jmp_context; 	/* address of source */

	umst->iar    = smst->iar;
	umst->cr     = smst->cr;
	umst->lr     = smst->lr;
	umst->ctr    = smst->ctr;
	umst->xer    = smst->xer;
	umst->mq     = smst->mq;
	umst->intpri = op;
	
	/*
	 * Copy back user-modifiable floating point status/state.
	 * Note that the signal handler may alter fpeu or the fp enable
	 * bits in the msr (as well as the fpscr/fpscrx). This allows a
	 * signal handler to reset or set modes, etc as per the fp_cpusync()
	 * syscall. This is valid to do here, ONLY since the fpunit has been
	 * disowned first.
	 *
	 * fpinfo is reconstructed from the state of the floating point
	 * bits in the msr, since the msr is a "public" interface.
	 * Note also that fpinfo FP_INT_TYPE bit is always reset to zero,
	 * which is ok here.
	 *
	 * Also, the fpeu byte is copied back so that apps doing 
	 * multiprogramming within a single AIX process may, themselves,
	 * implement `lazy-state-save'.
	 */

	umst->fpscr  = smst->fpscr;
	umst->fpscrx = smst->fpscrx;
	umst->fpeu   = smst->fpeu;

	umst->fpinfo = UPDATE_FPINFO(smst->msr);
	umst->msr = smst->msr;
	SANITIZE_MSR(umst->msr);

	/* 
	 * Copy all the GP and FP regs.  Note that the floating-point 
	 * enable bit in the MSR will be OFF, since disown_fp() was called 
	 * before copying the mstsave area in sig_slih() above.  The possibly 
	 * modified FP registers copied here from the sigcontext structure 
	 * into the mstsave area, from where they will be reloaded if the 
	 * thread ever issues a floating point instruction. 
	 */

	bcopy(smst->gpr,		/* from */
	      umst->gpr,		/* to */
	      NGPRS*sizeof(ulong) +	/* length of GPRs */
	      NFPRS*sizeof(double));	/* length of FPRs */

	/* 
	 * Decrement signal suspension count, since we're not exiting via 
 	 * the svc handler
 	 */

	th->t_suspend--;

	/*
	 * The segment registers are loaded by resume, 
	 * when returning to user mode.
	 */

	sig_slih_resume();
}

/*
 * NAME: check_sig
 *
 * FUNCTION:
 * 	Check whether a signal is pending.  Called when doing a long 
 *	sleep and when determining which signal to deliver.
 *
 * RETURN VALUES:
 *      0 => no signal available for delivery
 *      non-zero => signal to deliver
 */

check_sig(struct thread *t, int deliver)
{
	register struct proc *p;
	register int signo;
	sigset_t sigbits, osigbits;
	int sigexist = 0;

	ASSERT(csa->intpri == INTMAX);
#ifdef _POWER_MP
	ASSERT(lock_mine(&proc_base_lock));
#endif

	p = t->t_procp;

recheck:

	/* Process/thread is terminating (exit, exec/thread_terminate*). */
	if ((p->p_int & STERM) || (t->t_flags & TTERM))
		return(SIGKILL);

	/* Process is core dumping. */ 
	if (p->p_int & SSUSPUM)
		return(SIGKILL);

	/* Thread is being thread_setstate, thread_setsched */
	if (t->t_flags & (TSUSP|TINTR))
		return(SIGSTOP);

	/*
	 * If SSUSP is set and the process is a kproc, then one
	 * of the stop signals has been sent to the kproc, since 
	 * kprocs cannot be ptraced.  The signal should still be 
	 * pending and will be found below.  We don't need to 
	 * stop the thread here.  This preserves the semantic
	 * for not taking the default action for stop when sig_chk
	 * is called.  The stop signal is returned. 
	 */
	else if (deliver && (p->p_int & SSUSP))
	{
       	       	stop_thread(t);       
		goto recheck;
	}
		
	if (t->t_cursig)
		return(t->t_cursig);

	/* get set of pending signals that are not masked */
	sigbits = osigbits = p->p_sig;
	SIGORSET(sigbits, t->t_sig);
	SIGMASKSET(sigbits, t->t_sigmask);
        if (!(p->p_flag & STRC))
              	SIGMASKSET(sigbits, p->p_sigignore);

	for(;;)
	{
		if (SIGSETEMPTY(sigbits))
		{
			if (sigexist)
			{
				/*
				 * The only signals we found are the ones
				 * below. We just need to return non-0 to
				 * proceed up to issig(), but we don't set
				 * t_cursig so we don't know what signal
				 * will be really delivered by issig().
				 * However, we do need the actual 
				 * signal number for sig_chk(). 
				 */
				return(fsig(osigbits));
			}
			return 0;
		}	
		sigexist = TRUE;

		/* get rightmost unmasked pending signal to deliver */
		signo = fsig(sigbits);
		SIGDELSET(sigbits, signo);

		if ((ulong)SIG_DFL == (ulong)U.U_signal[signo])
		{
			switch (signo) {
			/* 
			 * Action for these signals is either ignore,
			 * stop or will be done in issig()
			 */
			case SIGTSTP:
			case SIGTTIN:
			case SIGTTOU:
                                /*
                                 * POSIX 3.3.1.3: discard SIGTSTP, SIGTTIN,
                                 *                SIGTTOU if signal would have
                                 *                stopped the process.
                                 */
                                if (p->p_flag & SORPHANPGRP)
                                    continue;

                                /* keep going ... */
			case SIGSTOP:
				/* 
				 * Don't deliver signal.  This is provided
				 * for v3 compatibility with sig_chk (signals
				 * are not delivered to kthreads) and to
				 * prevent stop_thread from being called 
				 * at the interrupt level in sig_slih.
				 */ 
				if (!deliver)
					continue;
				/* 
				 * Don't stop traced process; we need to be in
				 * the procxmt loop to get debugger requests.
				 */
				if (p->p_flag & STRC)
					continue;

				/* Don't stop kthreads in long sleep */
				if (t->t_flags & TKTHREAD)
					continue; 

				/* 
				 * Stop user thread.  Recheck from top for 
				 * intra-process synchronization. 
				 */
				stop_thread(t);		
				goto recheck;
			case SIGCONT:
			case SIGURG:
                        case SIGWINCH:
			case SIGIO:
			case SIGCHLD:
			case SIGDANGER:
			case SIGPWR:
				continue;
			}
		}
		break;
	}
	/* set t_cursig to the signal chosen for delivery */
	t->t_cursig = signo;

	/* remove the signal from those that are pending */
	DEL_PENDING_SIG(t, p, signo, sigexist);	/* sigexist is void here ! */
	
	return (signo);
}

/*
 * sig_setup()
 * 
 * Function:  modify the user level mstsave area to run the 
 *		signal delivery code to handle the signal
 */

sig_setup(struct sigcontext *scp, struct thread *t)
{
	vmhandle_t	uh;		/* handle for user's private segment */
	struct mstsave	*sa;		/* its mst area	  */
	struct uthread 	*ut;
	struct proc	*p;
	boolean_t	seg_attach;

	if (t == NULL) {
		t = curthread;
		seg_attach = FALSE;
	}
	else
		seg_attach = TRUE;

	p = t->t_procp;
	ut = t->t_uthreadp;
	if (seg_attach)
		ut = (struct uthread *)vm_att(p->p_adspace, ut);

	sa = &ut->ut_save;

	/* 
	 * Set user mst so that sig_deliver() will be called with
	 * a parameter pointing to scp.  Also set kernel toc and
	 * kernel stack registers.
	 */
	sa->iar       = (ulong) deliver_ptr->entry_point;
	sa->gpr[TOC]  = (ulong) deliver_ptr->toc_ptr;
	sa->gpr[ARG1] = (ulong) scp;
	sa->gpr[STKP] = (ulong) ut->ut_kstack - STKMIN;

	/* 
	 * Give the signal handler a clean floating point state.
	 * Clear out hardware & software fpscr fields in u-block.
	 * Also, set pipelined mode for the handler, and disable
	 * imprecise interrupt mode (i.e. clear fpinfo).
	 * This way, if the handler uses floating point, and takes
	 * an fp_unavailable interrupt, it will run at machine speed
	 * with no previous exception history.
	 * 
	 * Note that disown_fp has already reset the FP, FE & IE bits.
	 */
	sa->fpscr = 0;
	sa->fpscrx = 0;
	sa->fpinfo = 0;
     
	sa->msr &= ~MSR_PR;	/* reset problem state bit */
	sa->msr |= MSR_AL;	/* alignment bit must be on in kernel mode */

	/* 
	 * Initialize the kernel-side address space in the 
	 * user-level mst (u_block) as if the user had issued svc 
	 * to call a kernel service 
	 */

	as_init(&sa->as);
	as_ralloc(&sa->as, KERNELORG);		
	as_seth(&sa->as, vm_setkey(vmker.kernsrval, VM_PRIV), KERNELORG);
	as_ralloc(&sa->as, KSTACKORG);
	as_seth(&sa->as, vm_setkey(p->p_kstackseg, VM_PRIV), KSTACKORG);
	as_ralloc(&sa->as, &U);
	uh = p->p_adspace;
	as_seth(&sa->as, vm_setkey(uh, VM_PRIV), &U);
	as_ralloc(&sa->as, KERNEXORG);
	as_seth(&sa->as, vm_setkey(vmker.kexsrval, VM_PRIV), KERNEXORG);

	/* 
	 * if scp is NULL, then we are going to kill the process.  One of
	 * the following is true.  Either 1) STERM is set.  2) SIGKILL is 
	 * set.  3) an exception occurred while delivering a signal.   psig
	 * will be called which attempts to deliver the signal.  We set
	 * SIGKILL to ensure that the appropriate default action is taken. 
	 */
	if (scp == NULL) {
		t->t_cursig = SIGKILL;
		t->t_suspend = 1;
	}

	if (seg_attach)
		vm_det(ut);
}
