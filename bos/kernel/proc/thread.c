static char sccsid[] = "@(#)53	1.25.1.26  src/bos/kernel/proc/thread.c, sysproc, bos41J, 9520A_all 5/16/95 16:43:58";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: thread_create
 *		thread_setsched
 *		thread_setstate
 *		kthread_start
 *		thread_terminate
 *		thread_terminate_ack
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


#include <sys/errno.h>
#include <sys/proc.h>
#include <sys/lockl.h>
#include <sys/user.h>
#include <sys/sched.h>
#include <sys/sleep.h>
#include <sys/intr.h>
#include <sys/syspest.h>
#include <sys/prio_calc.h>
#include <sys/pseg.h>
#include <sys/machine.h>
#include <sys/atomic_op.h>
#include <sys/audit.h>
#include <sys/malloc.h>
#include <sys/trchkid.h>
#include <sys/lockname.h>
#include <sys/adspace.h>
#include "swapper_data.h"
#include "sig.h"

extern struct	proch *proch_anchor;
extern int	proch_gen;
extern struct	thread *newthread();
extern void	tidsig();

/*
 * NAME: thread_create
 *
 * FUNCTION:
 *	creates a new thread
 *
 * EXECUTION ENVIRONMENT:
 *	only callable by a thread
 *	cannot page fault
 *
 * NOTES:
 *	This routine is NEVER called for the first thread of the process.
 *	thread state transition :
 * 		not created (TSNONE) ==> created but not started (TSIDL)
 *
 * RETURNS:
 *	tid of newly created thread if successful
 *	-1, otherwise, u.u_error set to
 *		EAGAIN, if no thread slot left
 *		ENOMEM, if no pinnable memory left
 */
tid_t
thread_create()
{
	struct thread	*t;		/* current thread	 */
	struct proc	*p;		/* current process	 */
	char		*errorp;	/* current error storage */
	struct thread	*nt;		/* new thread structure	 */
	struct uthread	*ut;		/* new uthread structure */
	struct uthread	*used_uts;	/* list of unusable uthread structures*/
	vmid_t		vm_id;
	int		klock_rc;
	struct proch	*proch;
	int		old_gen;
	int		ipri;

	t = curthread;
	p = t->t_procp;
	errorp = &t->t_uthreadp->ut_error;

	ASSERT(csa->prev == NULL);

	/* Allocate new thread structure and initialize it */
	if (!(nt = newthread(errorp)))
		return(-1);

	/*
	 * Allocate new uthread structure.
	 *
	 * Note: The uthread pmzone is managed on a FIFO basis and the latency
	 * between setting and resetting UTSTILLUSED is very low, therefore we
	 * should succeed on the first try almost always.
	 */
	used_uts = NULL;
	while ((ut = (struct uthread *)pm_alloc(&U.U_uthread_cb, errorp)) &&
					(ut->ut_flags & UTSTILLUSED)) {
		ut->ut_link = used_uts;
		used_uts = ut;
	}
	nt->t_uthreadp = ut;
	while (used_uts) {
		ut = used_uts;
		used_uts = ut->ut_link;
		pm_free(&U.U_uthread_cb, (char *)ut);
	}
        if (!nt->t_uthreadp) {
		freethread(nt);
		return(-1);
	}
	/* jfs requirement; saves bzero of uthread */
	nt->t_uthreadp->ut_fstid = 0;

	/*
	 * Allocate the kernel stack segment for user processes if it does not
         * already exist.  Also allocate zone for asynchronous cancelation.
	 *
	 * NOTE: If p_kstackseg is NULLSEGVAL then we obviously are the only
	 * thread in the process, therefore we can go ahead and change it 
	 * without any locks to protect it.
	 */
	if (!(p->p_flag & SKPROC) && (p->p_kstackseg == NULLSEGVAL)) {
		if (vms_create(&vm_id, V_WORKING|V_PRIVSEG, 0, 0, 0, SEGSIZE)) {
			pm_free(&U.U_uthread_cb, (char *)nt->t_uthreadp);
			freethread(nt);
			*errorp = ENOMEM;
			return(-1);
		}
		p->p_kstackseg = SRVAL(vm_id, 0, 0);
		vm_seth(vm_setkey(p->p_kstackseg, VM_PRIV), KSTACKORG);
		t->t_uthreadp->ut_kstack = (char *)KSTACKTOP;
                (void)pm_init(&U.U_cancel_cb,                     /* zone  */
                     (char *)KSTACKORG,                           /* start */
                    round_up(KSTACKORG+2*(sizeof(struct tstate)*(MAXTHREADS+1)),
			      PAGESIZE),			  /* end */
                     sizeof(struct tstate),                       /* size  */
                     0,                                           /* link  */
                     UTHREAD_ZONE_CLASS,                          /* class */
                     p-proc+1000000,                              /* occur */
                     PM_FIFO);                                    /* flags */
	}

	/* Link the new thread to the owning process' thread list */
	ipri = disable_lock(INTMAX, &proc_base_lock);
	nt->t_procp = p;
	nt->t_state = TSIDL;
	nt->t_prevthread = t;
	nt->t_nextthread = t->t_nextthread;
	nt->t_prevthread->t_nextthread = nt;
	nt->t_nextthread->t_prevthread = nt;
	p->p_threadcount++;
	TRCHKL4T(HKWD_SYSC_CRTHREAD,p->p_pid,nt->t_tid,nt->t_pri,nt->t_policy);
	unlock_enable(ipri, &proc_base_lock);

	/*
	 * Loop through the registered resource handlers.
	 * Start over if the list changes.  The serialization of this
	 * list is strange because the resource handlers can sleep
	 * causing deadlock problems if any other lock but the kernel
	 * lock is used.  Since the kernel lock may be given up, the
	 * list can change which is why the generation count is used.
	 * Both of these techniques must be used to ensure list integrity.
	 */
	klock_rc = lockl(&kernel_lock, LOCK_SHORT);
	do {
		old_gen = proch_gen;
		for (proch = proch_anchor; proch != NULL; proch = proch->next) {
			/* call resource handler with parms */
			(proch->handler)(proch, THREAD_INITIALIZE, nt->t_tid);
			/* somebody changed the list, restart */
			if (proch_gen != old_gen) {
				lockl(&kernel_lock, LOCK_SHORT);
				break;
			}
		}
	} while (proch_gen != old_gen);
	if (klock_rc != LOCK_NEST)
		unlockl(&kernel_lock);
	
	return(nt->t_tid);
}

/*
 * NAME: kthread_start
 *
 * FUNCTION:
 *	starts a newly created thread as a kernel thread
 *
 * EXECUTION ENVIRONMENT:
 *	only callable by a thread
 *	cannot page fault
 *
 * NOTES:
 *	caller must belong to the same process as the thread being started
 *	thread state transition :
 *		created but not started (TSIDL) ==> ready (TSRUN)
 *
 * RETURNS:
 *	0, if successful
 *	ESRCH, if tid is invalid
 */
int
kthread_start(tid_t tid, void (*init_func)(void *), void *init_data_addr,
	size_t init_data_length, void *init_stk_addr,
	sigset_t *init_signal_mask)
/* register tid_t	tid;		thread ID */
/* register void (*init_func)(void *);	pointer to initial function */
/* register void *init_data_addr;	initialization data, address */
/* register size_t init_data_length;	- and length */
/* register void *init_stk_addr;	kernel stack base */
/* sigset_t *init_signal_mask;		initial signal mask */
{
	struct thread	*t;		/* current thread	*/
	struct proc	*p;		/* owning process	*/
	struct thread	*nt;		/* thread to be started	*/
	struct uthread	*ut;		/* its uthread structure*/
	int		ipri;

	t = curthread;
	p = t->t_procp;

	ASSERT(csa->prev == NULL);

	ipri = disable_lock(INTMAX, &proc_base_lock);

	/*
	 * Validate the input tid, make sure the input thread is in
	 * the TSIDL state and not undergoing another start, and is
	 * part of this process.
	 */
	nt = VALIDATE_TID(tid);
	if (!nt || (nt->t_state != TSIDL)    || 
	           (nt->t_flags & TSETSTATE) || (nt->t_procp != p)) {
		unlock_enable(ipri, &proc_base_lock);
		return(ESRCH);
	}

	TRCHKL5T(HKWD_KERN_KTHREADSTART,p->p_pid, nt->t_tid, t->t_pri,
		 t->t_policy, init_func);

	/* A setstate is now in progress */
	nt->t_flags |= TSETSTATE;

	unlock_enable(ipri, &proc_base_lock);

	/* Use an allocated kernel stack if this is a user process */
	ut = nt->t_uthreadp;
	if (!(p->p_flag & SKPROC))
		init_stk_addr = (void *)(KSTACKTOP - KSTACKSIZE -
				      (ut - &__ublock.ub_uthr[0]) * KSTACKSIZE);

	/* Perform platform-dependent machine state initialization */
	threadinit(ut, (vmhandle_t)p->p_kstackseg, (vmhandle_t)p->p_adspace,
	    init_func, init_data_addr, (int)init_data_length, init_stk_addr, 0);

	/* Complete uthread initialization */
	ut->ut_save.prev = NULL;		/* set up back chain */
	ut->ut_save.curid = p->p_pid;		/* OBSOLETE field */
	ut->ut_save.intpri = INTBASE;		/* init int pri to base */
	ut->ut_save.stackfix = NULL;		/* zero out stack fix */

	tfork(NULL, ut);			/* init per-thread timers */
	ut->ut_audsvc = NULL;			/* init per-thread auditing */

	ut->ut_errnopp = (int **)0xC0C0FADE;	/* never used by a kthread */

	ipri = disable_lock(INTMAX, &proc_base_lock);
#ifdef _POWER_MP
	simple_lock(&proc_int_lock);
#endif

	/* Set up important fields in new thread slot from parent thread */
	nt->t_policy = t->t_policy;
	nt->t_boosted = 0;	   /* asynchronously updated by pri-boosting */
	nt->t_lockcount = 0;	   /* asynchronously updated by dispatcher */
	nt->t_sav_pri = t->t_sav_pri;		/* seed */
	nt->t_sav_pri = nt->t_pri = prio_calc(nt);
	nt->t_sigmask = *init_signal_mask;	/* init signal mask but */
	nt->t_suspend = 1;			/* no sig delivery for kthread*/
	nt->t_flags |= TKTHREAD;		/* this is a kernel thread */
	nt->t_userdata = 0;

	/* Make thread ready to execute */
	setrq(nt, E_WKX_PREEMPT, RQTAIL);	/* t_state : TSIDL ==> TSRUN */
	p->p_active++;
	ASSERT(p->p_active <= p->p_threadcount);

	/* No longer setstating */
	nt->t_flags &= ~TSETSTATE;
        e_wakeup((int *)&nt->t_synch); 		/* TSETSTATE reset */

#ifdef _POWER_MP
	simple_unlock(&proc_int_lock);
#endif
	unlock_enable(ipri, &proc_base_lock);

	return(0);
}

/*
 * NAME: thread_terminate
 *
 * FUNCTION: 
 *	terminates the current thread
 *
 * EXECUTION ENVIRONMENT: 
 *	only callable by a thread
 *	cannot page fault
 *
 * N0TES:
 *	thread state transition :
 *		running (TSRUN) ==> zombie (TSZOMB)
 *                                                                              
 * RETURNS:
 *	This routine does NOT return.
 */
void
thread_terminate()
{
	struct thread	*t;		/* current thread	*/
	struct uthread	*ut;		/* current uthread      */
	struct proc	*p;		/* owning process	*/
	int		klock_rc;
	struct proch	*proch;
	int		old_gen;
	int		ipri;
	struct ptipc	*ipc;	
	int		state_change;

	ASSERT(csa->prev == NULL);
	ASSERT(!IS_LOCKED(&kernel_lock));

	t = curthread;
	p = t->t_procp;

        /* We shouldn't be on an event list or have any locks */
        assert(t->t_eventlist == NULL);
        assert(t->t_lockcount == 0);

	TRCHKL2T(HKWD_SYSC_TERMTHREAD, p->p_pid, t->t_tid);

	ipri = disable_lock(INTMAX, &proc_base_lock);

	/* Check whether we are the only active thread not terminating */
	if (p->p_terminating + 1 == p->p_active) {
		unlock_enable(ipri, &proc_base_lock);
		kexit(0); 		/* normal exit; does not return */
	}

	/* Mark as terminating to stop tracing and signals */
	t->t_flags |= TTERM;
	e_wakeup((int *)&t->t_synch);		/* TTERM set */
	p->p_terminating++;
	ASSERT(p->p_terminating < p->p_active);

	unlock_enable(ipri, &proc_base_lock);

	/*
	 * Loop through the registered resource handlers
	 * Start over if the list changes.  The serialization of this
	 * list is strange because the resource handlers can sleep
	 * causing deadlock problems if any other lock but the kernel
	 * lock is used.  Since the kernel lock may be given up, the
	 * list can change which is why the generation count is used.
	 * Both of these techniques must be used to ensure list integrity.
	 */
	klock_rc = lockl(&kernel_lock, LOCK_SHORT);
	do {
		old_gen = proch_gen;
		for (proch = proch_anchor; proch != NULL; proch = proch->next) {
			/* call resource handler with parms */
			(proch->handler)(proch, THREAD_TERMINATE, t->t_tid);
			/* somebody changed the list, restart */
			if (proch_gen != old_gen) {
				(void) lockl(&kernel_lock, LOCK_SHORT);
				break;
			}
		}
	} while (proch_gen != old_gen);
	if (klock_rc != LOCK_NEST) 
		unlockl(&kernel_lock);

        ASSERT(t->t_lockcount == 0);

	/* Relase the cancellation buffer */
	if (t->t_cancel)
		pm_free(&U.U_cancel_cb, (char *)t->t_cancel);

	/* Release floating point H/W */
	disown_fp(t->t_tid);

	/* Release the audit buffers */
	ut = t->t_uthreadp;
	if (ut->ut_audsvc) {
		if (ut->ut_audsvc->audbuf)
			free(ut->ut_audsvc->audbuf);
		xmfree((void *)ut->ut_audsvc, kernel_heap);
		ut->ut_audsvc = NULL;
	}

	/* Clean up outstanding timer requests */
	texit(NULL, ut);

	/* If thread still has locks, panic, this is an error */
	assert(t->t_lockcount == 0);

	/* Mark uthread block still in use and free it. It won't be reused */
	ut->ut_flags |= UTSTILLUSED;
	if (ut != &uthr0)
		pm_free(&U.U_uthread_cb, (char *)ut);
	
	ipri = disable_lock(INTMAX, &proc_base_lock);
#ifdef _POWER_MP
	simple_lock(&proc_int_lock);
#endif

	/*
	 * Unmark the uthread entry.
	 *
	 * Note: The uthread (and thus the kernel stack too) are now reusable.
	 * However, they will only be altered by kthread_start/thread_setstate,
	 * and those routines are synchronized with effective death of zombies
	 * through the proc_base_lock.
	 */
	ut->ut_flags &= ~UTSTILLUSED;

	p->p_terminating--;
	p->p_active--;
	ASSERT(p->p_active > 0);

	/* Wake up waiters on "all active threads are suspended" */
	if (p->p_suspended == p->p_active) {

               /*
                * Suspension may be caused by signals or intra-process
                * synchronization.  Only the former involves the POSIX
                * semantic of sending signals, posting events, ..
                */
		state_change = (int)p->p_synch != EVENT_NULL;

		if (state_change)
			e_wakeup((int *)&p->p_synch);
		else {
			/* Suspension caused by a signal */
			if (p->p_int & SSUSP) {
                                if (p->p_flag & STRC) {
                                        ipc = p->p_ipc;
                                        e_wakeup (&ipc->ip_event);
                                        ep_post (EVENT_CHILD, ipc->ip_dbp->p_pid);
                                }
                                else
                                {
                                        struct  proc    *pp;

                                        /*
                                         * POSIX 3.3.4: Unless parent process set
                                         *      SA_NOCLDSTOP, send SIGCHLD to parent
                                         *      process.
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
			}
			/* Suspension caused by load control */
			else if (p->p_int & SGETOUT) {
				p->p_stat = SSWAP;
			}
		}
	}

	/* Mark as zombie */
	t->t_state = TSZOMB;

#ifdef _POWER_MP
	simple_unlock(&proc_int_lock);
        assert(t->t_lockcount == 1);   /* To account for the proc_base_lock */
	locked_swtch();
#else
        assert(t->t_lockcount == 0);
        swtch();                 	/* choose another thread to run */
#endif
	/* Does not return */
}

/*
 * NAME: ksuspend
 *
 * FUNCTION:
 *	suspends the current thread
 *
 * EXECUTION ENVIRONMENT:
 *	Cannot page fault.
 *	Interrupts are disabled by caller.
 *
 * N0TES:
 *	thread state transition :
 *		running (TSRUN) ==> stopped (TSSTOP)
 *
 * RETURNS:
 *	1 (with proc_base_lock held) if the suspension occurred
 *	0 (without proc_base_lock) otherwise
 */
int
ksuspend()
{
	struct thread	*t;			   /* current thread	     */
	struct thread   *th;			   /* temp thread ptr        */
        struct uthread *ut;                        /* current uthread ptr    */
	struct proc	*p;			   /* current process	     */
	struct ptipc	*ipc;			   /* ptrace control block   */
        struct tstate   *nstate;                   /* cancelation state      */
	int 		state_change;		   /* process state 	     */
        int             t_flagsmask, t_flags;      /* local thread bit masks */

	t = curthread;
	p = t->t_procp;

	ASSERT(csa->intpri == INTMAX);
	ASSERT(t->t_lockcount == 0);
#ifdef _POWER_MP
	simple_lock(&proc_base_lock);
	simple_lock(&proc_int_lock);
#endif

	/* 
	 * Reset boosted count to avoid swtch in svcret.  We are in
	 * user mode so we don't have any base level locks.
	 */
	t->t_boosted = 0;

        /*
         * This path is taken because of a signal.  It should take precedence
         * over thread suspension so that ptrace/core can see the state of each
         * thread before it's state is changed (TINTR).  TINTR should take
         * precedence over TSUSP.
         */
        if (p->p_int & (SSUSP|SSUSPUM)) {

                /* release floating point hw  */
                disown_fp(t->t_tid);

                /* Suspend oneself */
                p->p_suspended++;
                ASSERT(p->p_suspended <= p->p_active);

                TRCHKL3T(HKWD_KERN_KSUSPEND, t->t_tid,
                         p->p_suspended, p->p_active);

                /* if everybody is suspended */
                if (p->p_suspended == p->p_active) {

                        /*
                         * Suspension may be caused by signals or intra-process
                         * synchronization.  Only the former involves the POSIX
                         * semantic of sending signals, posting events, ..
                         */
			state_change = (int)p->p_synch != EVENT_NULL;

			if (state_change)
                		e_wakeup((int *)&p->p_synch);
			else {

                        	/* Suspension caused by a signal */
                        	if (p->p_int & SSUSP) {
                                	if (p->p_flag & STRC) {
                                             ipc = p->p_ipc;
                                             e_wakeup (&ipc->ip_event);
                                             ep_post (EVENT_CHILD, ipc->ip_dbp->p_pid);
                                	}
                                	else
                                	{
                                             struct  proc    *pp;

                                             /*
                                              * POSIX 3.3.4: Unless parent process 
					      *              set SA_NOCLDSTOP, 
					      *              send SIGCHLD to parent
                                              *              process.
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
                        	}
			}
                }
		e_wakeup((int *)&t->t_synch);	/* TSSTOP set */
                t->t_state = TSSTOP;
        }
        else if (t->t_flags & TINTR) {

                ut = t->t_uthreadp;
                nstate = (struct tstate *)vm_att(p->p_kstackseg, t->t_cancel);

                /* Screen MSR bits */
                nstate->mst.msr &= MSR_IE | MSR_FE | MSR_AL;
                ut->ut_save.msr &= ~(MSR_IE | MSR_FE | MSR_AL);
                ut->ut_save.msr |= nstate->mst.msr;

                /* Overwrite MST with new state as requested */
                bcopy(nstate->mst.gpr, ut->ut_save.gpr, sizeof(nstate->mst.gpr));
                bcopy(nstate->mst.fpr, ut->ut_save.fpr, sizeof(nstate->mst.fpr));
                ut->ut_save.iar = nstate->mst.iar;
                ut->ut_save.cr = nstate->mst.cr;
                ut->ut_save.lr = nstate->mst.lr;
                ut->ut_save.ctr = nstate->mst.ctr;
                ut->ut_save.xer = nstate->mst.xer;
                ut->ut_save.mq = nstate->mst.mq;
                ut->ut_save.fpscr = nstate->mst.fpscr;
                ut->ut_save.fpscrx = nstate->mst.fpscrx;
                ut->ut_save.fpeu = nstate->mst.fpeu;

                /* Change errno address and user data */
		ut->ut_error = 0;
                ut->ut_errnopp = nstate->errnop_addr;
		t->t_userdata = nstate->userdata;

                /* Change sigmask. Don't forget to screen it. */
                SIGDELSET(nstate->sigmask, SIGKILL);
                SIGDELSET(nstate->sigmask, SIGSTOP);
                t->t_sigmask = nstate->sigmask;

                /* Change pending sig. */
                t->t_sig = nstate->psig;

                /* Change policy */
                t->t_policy = nstate->policy;

                /* Change priority */
                t->t_sav_pri = nstate->priority;

                t->t_sav_pri = t->t_pri = prio_calc(t);

                /* Change flags. Don't forget to screen them. */
                if (t->t_flags & TLOCAL)
                        p->p_local--;

                /* translation */
                t_flagsmask = t_flags = 0;
                if (nstate->flagmask & TSTATE_LOCAL)
                        t_flagsmask |= TLOCAL;
                if (nstate->flags & TSTATE_LOCAL)
                        t_flags |= TLOCAL;

                /* insertion */
                t->t_flags &= ~t_flagsmask;
                t->t_flags |= t_flags;

                if (t->t_flags & TLOCAL)
                        p->p_local++;

                /*
                 * Cannot call pm_free because we are at the
                 * interrupt level.  Leave t_cancel pointing
                 * to structure.
                 */
                vm_det(nstate);

                t->t_flags &= ~TINTR;

                if (SIG_MAYBE(t,p))
                        t->t_flags |= TSIGAVAIL;
        }
        else if (t->t_flags & TSUSP) {
                p->p_suspended++;
                e_wakeup((int *)&t->t_synch);         /* TSSTOP set */
                t->t_state = TSSTOP;
        }


#ifdef _POWER_MP
	simple_unlock(&proc_int_lock);
#endif
	if (t->t_state == TSRUN) {
#ifdef _POWER_MP
		simple_unlock(&proc_base_lock);
#endif
		return 0;
	}
	return 1;
}

/*
 * NAME: thread_setstate
 *
 * FUNCTION:
 *	starts a newly created thread as a user thread
 *	changes the computational state of a started user thread
 *
 * EXECUTION ENVIRONMENT:
 *	only callable by a thread
 *	cannot page fault
 *
 * NOTES:
 *	Thread state transition :
 *		created but not started (TSIDL) ==> ready (TSRUN)
 *
 * RETURNS:
 *	0, if successful
 *	-1, otherwise, u.u_error set to
 *		ESRCH, if tid is invalid
 *		EINVAL, if policy or priority are invalid
 *		EFAULT, if nstate or ostate are not in the process address space
 *		EPERM, if not enough privilege to change policy or priority
 *		ENOMEM, if no more pinable memory to hold the cancellation str.
 */
int
thread_setstate(tid_t tid, struct tstate *nstate, struct tstate *ostate)
/* register tid_t	   tid;		thread ID */
/* register struct tstate *nstate;	new state to be set */
/* register struct tstate *ostate;	old state */
{
	struct thread	*t;		/* current thread		*/
	struct proc	*p;		/* current process		*/
	struct thread	*nt;		/* thread to be reset		*/
	struct uthread	*ut;		/* its uthread structure	*/
	char		*errorp;	/* current error location	*/
	struct tstate	knstate, kostate;
	char		*kstack;
	int		ipri;
	int		t_flagsmask, t_flags;
	struct tstate   *stateptr = NULL;

	t = curthread;
	p = t->t_procp;
	errorp = &t->t_uthreadp->ut_error;

	ASSERT(csa->prev == NULL);

        /* Get the new state from the user space */
	if (copyin((caddr_t)nstate, (caddr_t)&knstate, sizeof(struct tstate))) {
		*errorp = EFAULT;
		return(-1);
	}

        /* Test the validity of the new errnop address */
	if (copyin((caddr_t)knstate.errnop_addr,(caddr_t)&ipri,sizeof(int))) {
		*errorp = EINVAL;
		return(-1);
        }

	/* Check policy and priority */
	switch (knstate.policy) {
	case SCHED_OTHER:
		/* 
		 * The priority parameter is ignored.  
		 * prio_calc reloads from p_nice.
		 */
		break;
	case SCHED_FIFO :
	case SCHED_RR   :
		/* Check the privileges */
		if (privcheck(SET_PROC_RAC) == EPERM) {
			*errorp = EPERM;
			return(-1);
		}
		/* Check the priority */
		if ((knstate.priority < PRIORITY_MIN) ||
		    (knstate.priority > PRIORITY_MAX)) {
			*errorp = EINVAL;
			return(-1);
		}
		break;
	default:
		*errorp = EINVAL;
		return(-1);
	}

        /*
         * Preallocate a tstate structure for thread cancelation.
         */
        if (knstate.flags & TSTATE_INTR) {
                if (p->p_kstackseg != NULLSEGVAL) {
                        if (!(stateptr = (struct tstate *)
				    pm_alloc(&U.U_cancel_cb, (char *)errorp))) {
				ASSERT(*errorp == ENOMEM);
				return(-1);
			}
                        /*
                         * Need to pin structure so that copy to
                         * stateptr can be serialized with dispatcher.
                         */
			if (pin(&knstate, sizeof(knstate))) {
				pm_free(&U.U_cancel_cb, (char *)stateptr);
				*errorp = ENOMEM;
				return(-1);
			}
                }
                else {
                        *errorp = ESRCH;
                        return(-1);
                }
        }

	ipri = disable_lock(INTMAX, &proc_base_lock);

	/*
	 * Validate the input tid, make sure the input thread is a
	 * non-terminating user thread, part of the current process but 
	 * not the current one for synchronous setstate.
	 */
        nt = VALIDATE_TID(tid);
        if ((!nt) || (nt->t_state == TSNONE)
                  || (nt->t_flags & (TTERM|TKTHREAD))
                  || (nt->t_procp != p)
                  || ((nt == t) && !(knstate.flags & TSTATE_INTR))) {
                unlock_enable(ipri, &proc_base_lock);
                if (stateptr) {
                        pm_free(&U.U_cancel_cb, (char *)stateptr);
                        unpin(&knstate, sizeof(knstate));
                }
                *errorp = ESRCH;
                return(-1);
        }

	TRCHKL5T(HKWD_SYSC_THREADSETSTATE,tid,nt->t_state,nt->t_flags,
		knstate.priority, knstate.policy);

        /*
         * If the request is asynchronous (TINTR), then copy the data.  It
         * is retrieved by ksuspend, when the thread next returns to user
         * mode.  The parameter ostate is ignored.
         */
        if (knstate.flags & TSTATE_INTR) {
                nt->t_flags |= TINTR;
                if (nt->t_cancel == NULL)
                        nt->t_cancel = stateptr;
                bcopy(&knstate, nt->t_cancel, sizeof(struct tstate));
                switch(nt->t_wtype) {
                case TNOWAIT :
                case TWCPU :
                        break;
                case TWPGIN :
                        vcs_interrupt(nt);
                        break;
                default :
                        if (nt->t_flags & TWAKEONSIG) {
#ifdef _POWER_MP
				simple_lock(&proc_int_lock);
#endif
                                if (nt->t_state == TSSLEEP)
                                        setrq(nt, E_WKX_PREEMPT, RQHEAD);
                                else
                                        nt->t_wtype = TNOWAIT;
#ifdef _POWER_MP
				simple_unlock(&proc_int_lock);
#endif
                        }
                        break;
                }
                unlock_enable(ipri, &proc_base_lock);
                if (nt->t_cancel != stateptr)
                        pm_free(&U.U_cancel_cb, (char *)stateptr);
                unpin(&knstate, sizeof(knstate));
                return(0);
        }

	/*
	 * Serialize with other state changes of the same thread and
	 * check whether the targetted thread is stopped in kernel mode.
	 * We should suspend ourself because we cannot change a kernel
	 * MST anyway and we have to wait for the targetted thread to
	 * be resumed so that we can stop it again in user mode.
	 *
	 * Note : We can't first try to get the TSETSTATE flag and set it
	 * because a set TSETSTATE flag would prevent a stopped thread to
	 * restart, thus defeating the second loop. Moreover each loop
	 * may release the proc_int_lock which may void the state of
	 * their condition, so they are to be intermixed.
	 */
	while((nt->t_flags & TSETSTATE) ||
	      ((nt->t_state == TSSTOP) && (nt->t_suspend))) {

		if (nt->t_flags & TSETSTATE) {
			e_sleep_thread((int *)&nt->t_synch, &proc_base_lock,
							LOCK_HANDLER);

			if ((nt->t_tid != tid)      || (nt->t_flags & TTERM) ||
		            (nt->t_state == TSNONE) || (p->p_int & STERM)) { 
				unlock_enable(ipri, &proc_base_lock);
				*errorp = ESRCH;
				return(-1);
			}
		}

		if ((nt->t_state == TSSTOP) && (nt->t_suspend)) {

			stop_thread(t);

			if ((nt->t_tid != tid)      || (nt->t_flags & TTERM) ||
			    (nt->t_state == TSNONE) || (p->p_int & STERM)) {
				unlock_enable(ipri, &proc_base_lock);
				*errorp = ESRCH;
				return(-1);
			}
		}
	}
	nt->t_flags |= TSETSTATE;

	/*
	 * Request suspension if necessary.
	 *
	 * Note: If the thread is in the TSSWAP state, we will wait till it's
	 * swapped in and stopped. We therefore are virtually swapped out
	 * ourself. Alternatively, we could have treated TSSWAP like TSSTOP
	 * and changed the MST, restarted the thread which would have swapped
	 * itself out again on resume and we would have swapped ourself on
	 * SVC return. The generated activity is a bit contradictory with the
	 * swapping philosophy, hence our choice.
	 * Note also that we have been able to make that choice because nobody
	 * ever waits for swapping completion unlike stopping completion.
	 * Note also that as long as the thread has been stopped for us,
	 * it's entirely up to us to resume it, therefore we must not abort
	 * before restarting it.
	 */
	if ((nt->t_state != TSIDL) && (nt->t_state != TSSTOP)) {

		do {
			nt->t_flags |= TSUSP;
                        switch(nt->t_wtype) {
                        case TNOWAIT :
                        case TWCPU :
                                break;
                        case TWPGIN :
                                vcs_interrupt(nt);
                                break;
                        default :
#ifdef _POWER_MP
				simple_lock(&proc_int_lock);
#endif
                                if (nt->t_flags & TWAKEONSIG) {
                                        if (nt->t_state == TSSLEEP)
                                               setrq(nt, E_WKX_PREEMPT, RQHEAD);
                                        else
                                               nt->t_wtype = TNOWAIT;
                                }
#ifdef _POWER_MP
				simple_unlock(&proc_int_lock);
#endif
                                break;
                        }

			e_sleep_thread((int *)&nt->t_synch, &proc_base_lock, 
								LOCK_HANDLER);

			if ((nt->t_tid != tid) || (nt->t_flags & TTERM) ||
			    (nt->t_state == TSNONE)) {
				unlock_enable(ipri, &proc_base_lock);
				*errorp = ESRCH;
				return(-1);
			}
		} while (nt->t_state != TSSTOP);
		nt->t_flags &= ~TSUSP;
	}

	unlock_enable(ipri, &proc_base_lock);
		
	/* Copy the old state to the user space */
	if (ostate != NULL) {
		kostate.mst = nt->t_uthreadp->ut_save;
		kostate.errnop_addr = nt->t_uthreadp->ut_errnopp;
		kostate.userdata = nt->t_userdata;
		kostate.sigmask = nt->t_sigmask;
		kostate.psig = nt->t_sig;
		kostate.policy = (int)nt->t_policy;
		kostate.priority = (int)nt->t_pri;
		if (nt->t_flags & TLOCAL)
			kostate.flags |= TSTATE_LOCAL;

		if (copyout((caddr_t)&kostate, (caddr_t)ostate,
						sizeof(struct tstate))){
			*errorp = EFAULT;
			ipri = disable_lock(INTMAX, &proc_base_lock);
			goto run;
		}
	}

	ut = nt->t_uthreadp;

	/*
	 * Change the MST.
	 *
	 * For an idle thread, we need to allocate the stack and 
	 * initialize the MST.
	 *
	 * Note: The setup is for kernel mode, we need to reset MSR,
	 * and fetch the segment registers from our fellow thread.
	 * Everything else is overwritten.
	 */
	if (nt->t_state == TSIDL) {
		kstack = (char *)(KSTACKTOP - KSTACKSIZE -
				     (ut - &__ublock.ub_uthr[0]) * KSTACKSIZE);
		threadinit(ut, (vmhandle_t)p->p_kstackseg,
						(vmhandle_t)p->p_adspace, NULL,
						NULL, 0, (void *)kstack, 0);
		ut->ut_save.msr = DEFAULT_USER_MSR;
		ut->ut_save.as = t->t_uthreadp->ut_save.as;

		/* Complete uthread initialization */
		ut->ut_save.prev = NULL;	/* set up back chain */
		ut->ut_save.curid = p->p_pid;
		ut->ut_save.intpri = INTBASE;	/* set intpri to base */
		ut->ut_save.stackfix = NULL;	/* zero out stack fix */
		tfork(NULL, ut);		/* per-thread timers */
		ut->ut_audsvc = NULL;		/* per-thread audit */
		ut->ut_kstack = kstack;
	}

	/* Screen MSR bits */
	knstate.mst.msr &= MSR_IE | MSR_FE | MSR_AL;
	ut->ut_save.msr &= ~(MSR_IE | MSR_FE | MSR_AL);
	ut->ut_save.msr |= knstate.mst.msr;

	/* Overwrite MST with new state as requested */
	bcopy(knstate.mst.gpr,ut->ut_save.gpr,sizeof(knstate.mst.gpr));
	bcopy(knstate.mst.fpr,ut->ut_save.fpr,sizeof(knstate.mst.fpr));
	ut->ut_save.iar = knstate.mst.iar;
	ut->ut_save.cr = knstate.mst.cr;
	ut->ut_save.lr = knstate.mst.lr;
	ut->ut_save.ctr = knstate.mst.ctr;
	ut->ut_save.xer = knstate.mst.xer;
	ut->ut_save.mq = knstate.mst.mq;
	ut->ut_save.fpscr = knstate.mst.fpscr;
	ut->ut_save.fpscrx = knstate.mst.fpscrx;
	ut->ut_save.fpeu = knstate.mst.fpeu;

	/* Change errno address and user data */
	ut->ut_error = 0;
	ut->ut_errnopp = knstate.errnop_addr;
	nt->t_userdata = knstate.userdata;

	ipri = disable_lock(INTMAX, &proc_base_lock);

	/* Change sigmask. Don't forget to screen it. */
	SIGDELSET(knstate.sigmask, SIGKILL);
	SIGDELSET(knstate.sigmask, SIGSTOP);
	nt->t_sigmask = knstate.sigmask;

	/* Change pending sig. */
	nt->t_sig = knstate.psig;

	if (SIG_AVAILABLE(nt,p))
		nt->t_flags |= TSIGAVAIL;

	/* Change policy */
	nt->t_policy = knstate.policy;

	/* Change flags. Don't forget to screen them. */
	if (nt->t_flags & TLOCAL)
		p->p_local--;

	/* translation */
	t_flagsmask = t_flags = 0;
	if (knstate.flagmask & TSTATE_LOCAL)
		t_flagsmask |= TLOCAL;
	if (knstate.flags & TSTATE_LOCAL)
		t_flags |= TLOCAL;

	/* insertion */
	nt->t_flags &= ~t_flagsmask;
	nt->t_flags |= t_flags;

	if (nt->t_flags & TLOCAL)
		p->p_local++;

run:

#ifdef _POWER_MP
	simple_lock(&proc_int_lock);
#endif
	/* 
	 * For consistency reset t_boosted field under the same lock
	 * as the dispatcher, since the dispatcher updates it asynchronously.
	 * This field is used when calculating priorities, but doesn't
	 * formally need to need to be done under this lock because the
	 * thread is not in kernel mode.
	 */ 
	if (*errorp == 0) {
		nt->t_lockcount = 0;
		nt->t_boosted = 0;
		nt->t_sav_pri = knstate.priority;
		nt->t_sav_pri = nt->t_pri = prio_calc(nt);
	}

	switch (nt->t_state) {
	case TSIDL:
		/* Start the thread only if the system call succeeded */
		if (*errorp == 0) {
			nt->t_suspend = 0;	/* start in user mode */
			setrq(nt, E_WKX_NO_PREEMPT, RQTAIL);
			p->p_active++;
			ASSERT(p->p_active <= p->p_threadcount);
		}
		break;
	case TSSTOP:
		/* Always restart a stopped thread, even if failure */
		setrq(nt, E_WKX_PREEMPT, RQTAIL);
		p->p_suspended--;
		ASSERT(p->p_suspended >= 0);
		break;
	default:
		assert(FALSE);
	}

	nt->t_flags &= ~TSETSTATE;
	e_wakeup((int *)&nt->t_synch); 		/* TSETSTATE reset */
	
#ifdef _POWER_MP
	simple_unlock(&proc_int_lock);
#endif
	unlock_enable(ipri, &proc_base_lock);

	return(*errorp ? -1 : 0);
}

/*
 * NAME: thread_setsched
 *
 * FUNCTION:
 *	changes the policy and priority of a thread (and userdata if not 0)
 *
 * EXECUTION ENVIRONMENT:
 *	only callable by a thread
 *	cannot page fault
 *
 * RETURNS:
 *	0, if successful
 *	-1, otherwise, u.u_error set to
 *		ESRCH, if tid is invalid
 *		EINVAL, if policy or priority are invalid
 *		EPERM, if not enough privilege to change policy or priority
 */
int
thread_setsched(tid_t tid, int priority, int policy, int userdata)
{
	struct thread	*t;		/* current thread        */
	struct proc	*p;		/* current process       */
	struct thread	*nt;		/* thread to be affected */
	int		ipri;
	int		rc;

	ASSERT(csa->prev == NULL);

	t = curthread;
	p = t->t_procp;

	/* Check the priorities & policy */
	switch (policy) {
		case SCHED_LOCAL:
		case SCHED_GLOBAL:
			/* Must be the current thread */
			if (tid != -1 && tid != t->t_tid) {
				u.u_error = EINVAL;
				return(-1);
			}
			/* Don't allow local fixed priority threads. */
			if (policy == SCHED_LOCAL && t->t_policy != SCHED_OTHER) {
				u.u_error = EPERM;
				return(-1);
			}
		case SCHED_OTHER:
			break;
		case SCHED_FIFO :
		case SCHED_RR   :

			/* Check the privileges */
			if (privcheck(SET_PROC_RAC) == EPERM) {
				u.u_error = EPERM;
				return(-1);
			}

			/* Check the priority */
			if ((priority < 0) || (priority > PRI_LOW)) {
				u.u_error = EINVAL;
				return(-1);
			}
			break;
		default:
			u.u_error = EINVAL;
			return(-1);
	}

	ipri = disable_lock(INTMAX, &proc_base_lock);

	/*
	 * Validate the input tid, make sure the input thread is active
	 * or not terminating, and is in the current process.
	 */
	if (tid == -1)
		nt = t;
	else {
		nt = VALIDATE_TID(tid);
		if ((!nt) || (nt->t_state == TSNONE) || (nt->t_flags & TTERM)) {
			unlock_enable(ipri, &proc_base_lock);
			u.u_error = ESRCH;
			return(-1);
		}
		if (nt->t_procp != p) {
			unlock_enable(ipri, &proc_base_lock);
			u.u_error = EPERM;
			return(-1);
		}
	}

	TRCHKL4T(HKWD_SYSC_THREADSETSCHED,p->p_pid,nt->t_tid,priority,policy);

	/* Serialize with other state changes of the same thread */
	while (nt->t_flags & TSETSTATE) {

		e_sleep_thread((int *)&nt->t_synch, &proc_base_lock, 
						LOCK_HANDLER);

		if ((nt->t_tid != tid)      || (nt->t_flags & TTERM) ||
	            (nt->t_state == TSNONE) || (p->p_int & STERM)) {
			unlock_enable(ipri, &proc_base_lock);
			u.u_error = ESRCH;
			return(-1);
		}
	}
	nt->t_flags |= TSETSTATE;
	rc = 0;

	/*
	 * Change the userdata
	 */
	if (userdata)
		nt->t_userdata = userdata;

#ifdef _POWER_MP
	simple_lock(&proc_int_lock);
#endif
	/* Change the policy  */
	switch(policy) {
		case SCHED_LOCAL:
                        /* Don't allow local fixed priority threads. */
                        if (nt->t_policy != SCHED_OTHER) {
                                u.u_error = EPERM;
                                rc = -1;
                                break;
                        }
			if (!(nt->t_flags & TLOCAL)) {
				nt->t_flags |= TLOCAL;
				p->p_local++;
			}
			break;
		case SCHED_GLOBAL:
			if (nt->t_flags & TLOCAL) {
				nt->t_flags &= ~TLOCAL;
				p->p_local--;
				if (p->p_local == 0)
					pidsig(p->p_pid, SIGWAITING);
			}
			break;
		case SCHED_FIFO :
		case SCHED_RR   :
			/* Don't allow local fixed priority threads. */
			if (nt->t_flags & TLOCAL) {
				u.u_error = EPERM;
				rc = -1;
				break;
			}

			/* Change the priority */
			nt->t_sav_pri = priority;

			/* fall through to update thread policy */
		default:
			nt->t_policy = policy;
			prio_requeue(nt, prio_calc(nt));
			if (!nt->t_boosted)
				nt->t_sav_pri = nt->t_pri;

			break;
	}

	/* Notify other state changers */
	nt->t_flags &= ~TSETSTATE;
	e_wakeup((int *)&nt->t_synch); 		/* TSETSTATE reset */

#ifdef _POWER_MP
	simple_unlock(&proc_int_lock);
#endif
	unlock_enable(ipri, &proc_base_lock);

	return(rc);
}

/*
 * NAME: thread_terminate_ack
 *
 * FUNCTION: 
 *	terminates a user thread
 *
 * EXECUTION ENVIRONMENT: 
 *	only callable by a thread
 *	cannot page fault
 *
 * N0TES:
 *	thread state transition :
 *		running (TSRUN) ==> zombie (TSZOMB)
 *                                                                              
 * RETURNS:
 *	may not return if self-termination has been requested
 *	0, if successful
 *	-1, otherwise, u.u_error set to
 *		ESRCH, if tid is invalid
 *		EPERM, if tid denotes a kernel thread 
 */
int
thread_terminate_ack(tid_t tid)
/* register tid_t	   tid;		thread ID */
{
	struct thread	*t;		/* current thread		*/
	struct proc	*p;		/* current process		*/
	struct thread	*nt;		/* thread to be reset		*/
	int		ipri;

	t = curthread;
	p = t->t_procp;

	TRCHKL2T(HKWD_SYSC_THREADTERM_ACK, t->t_tid, tid);

	ASSERT(csa->prev == NULL);

	ipri = disable_lock(INTMAX, &proc_base_lock);

        /*
         * Validate the input tid, make sure the input thread is not a zombie
         * or idle thread, is part of the current process but not the current
         * thread.
         */
	nt = VALIDATE_TID(tid);
	if ((!nt) || (nt->t_state == TSNONE) || (nt->t_state == TSIDL) ||
	             (nt->t_flags & TTERM)   || (nt->t_procp != p)     || (nt == t)) {
		unlock_enable(ipri, &proc_base_lock);
		u.u_error = ESRCH;
		return(-1);
	}

        /* Ensure that the targetted thread is a user thread */
	if (nt->t_flags & TKTHREAD) {
		unlock_enable(ipri, &proc_base_lock);
		u.u_error = EPERM;
		return(-1);
	}

	/* Request termination and wake up the targetted thread */
	nt->t_flags |= TTERM;

        switch(nt->t_wtype) {
        case TNOWAIT :
        case TWCPU :
                break;
        case TWPGIN :
                vcs_interrupt(nt);
                break;
        default :
                if (nt->t_flags & TWAKEONSIG) {
#ifdef _POWER_MP
			simple_lock(&proc_int_lock);
#endif
                        if (nt->t_state == TSSLEEP)
                                setrq(nt, E_WKX_PREEMPT, RQHEAD);
                        else
                                nt->t_wtype = TNOWAIT;
#ifdef _POWER_MP
			simple_unlock(&proc_int_lock);
#endif
                }
                break;
        }

	/* Wait for the targetted thread to terminate itself */
	for(;;) {

                /*
                 * We must be sure that the thread we are trying to terminate
                 * is not trying to terminate us at the same time, otherwise
                 * that would deadlock. So we abort ourself if somebody else
                 * has requested our termination.
                 */
                if (t->t_flags & TTERM)         /* self-termination requested */
                        break;

		e_sleep_thread((int *)&nt->t_synch, &proc_base_lock, 
							LOCK_HANDLER);
		if ((nt->t_tid != tid)      || (nt->t_flags & TTERM) ||
	            (nt->t_state == TSNONE) || (p->p_int & STERM)) {
			break;
		}
	}

	unlock_enable(ipri, &proc_base_lock);

	return(0);
}
