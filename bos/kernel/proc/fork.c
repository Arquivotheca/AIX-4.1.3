static char sccsid[] = "@(#)51	1.98.1.36  src/bos/kernel/proc/fork.c, sysproc, bos41J, 9522A_all 5/30/95 11:47:39";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: creatp
 *		initp
 *		kforkx
 * 
 *   ORIGINS: 27, 3, 26, 83
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


#include <sys/acct.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/var.h>
#include <sys/user.h>
#include <sys/mstsave.h>
#include <sys/seg.h>
#include <sys/systm.h>
#include <sys/lockl.h>
#include <sys/vnode.h>
#include <sys/sysinfo.h>
#include <sys/intr.h>
#include <sys/utsname.h>
#include <mon.h>
#include <sys/pseg.h>
#include <sys/adspace.h>
#include <sys/syspest.h>
#include <sys/malloc.h>
#include <sys/timer.h>
#include <sys/prio_calc.h>
#include <sys/trchkid.h>
#include <sys/xmem.h>
#include <sys/sleep.h>
#include <sys/audit.h>
#include <sys/uio.h>
#include <sys/sched.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/atomic_op.h>
#include "swapper_data.h"
#include "ld_data.h"

extern struct	proch *proch_anchor;
extern int      proch_gen;
extern struct	proc  *newproc();
extern void	schedfork();
extern struct	ublock *procdup();

/*
 * NAME: kforkx()
 *
 * FUNCTION:
 *	fork() system call; creates a new process.
 *
 * EXECUTION ENVIRONMENT:
 *      Preemptable
 *      May Page Fault (except within disabled critical section)
 *
 * PROCESS STATE TRANSITIONS:
 * 	not created (SNONE)	==> not quite created (SIDL)
 *	not quite created (SIDL)==> ready             (SRUN)
 *
 * PARENT PROCESS RETURNS:
 *	child's PID, if successful;
 *	-1, if unable to create child (u.u_error has the reason).
 *
 * CHILD PROCESS RETURNS:
 *	0, if successful
 */
pid_t
kforkx()
{
	register struct proc *parent;	/* parent proc structure */
	register struct proc *child;	/* child proc structure */
	register struct thread *nth;	/* child thread structure */
	register struct proc *g;	/* process group proc structure */
	register struct ublock *ubchild;/* child's u_block (in parent) */

	register int	ipri;		/* saved interrupt priority */
	struct proch	*proch;
	register struct proc *q;	/* used to search uid list */
	int		uid_cnt;	/* number of processes in uid list */
	static int	svcnum = 0;
        int		old_gen;        /* old proch generation */
	struct loader_anchor *la=NULL;	/* per-process loader anchor */

	struct thread	*t;		/* current thread */
	struct uthread	*ut;		/* current uthread */
	struct proc	*p;		/* current process */
	char		*errorp;	/* current error storage */

	t = curthread;
	p = t->t_procp;
	ut = t->t_uthreadp;

	errorp = &ut->ut_error;

	/* make sure maximum processes for this uid has not been reached */
	simple_lock(&proc_tbl_lock);
	for (parent = p, q = parent->p_uidl, uid_cnt = 1;  
	     parent != q; 
	     q = q->p_uidl, uid_cnt++);
	simple_unlock(&proc_tbl_lock);

	if ((uid_cnt > (v.v_maxup - 1)) && privcheck(SET_PROC_RAC)) {
		*errorp = EAGAIN;
		return(-1);
	}

	/*
	 * allocate a new process structure and initialize it
	 */
	if ((child = newproc(errorp, 0)) == NULL)
		return(-1);

	parent = p;			/* parent's proc pointer */

        /* 
	 * Pick this up regardless of possible 
	 * audit suspension 
 	 */

	if (ut->ut_audsvc)
		ut->ut_audsvc->svcnum = 1;

	if (audit_flag && audit_svcstart("PROC_Create", &svcnum, 1, child->p_pid)){

		audit_svcfinis();

		/* 
		 * Commit immediately
	 	 */

		auditscall();

	}

	/* special case for multi-process debug, inherit flags, signal both */
	simple_lock(&ptrace_lock);
	if ((parent->p_flag & (STRC | SMPTRACE)) == (STRC | SMPTRACE))
	{
		if (*errorp = ptrace_attach(parent->p_ipc->ip_dbp, child))
			goto ptrace_unlock;


		ipri = disable_lock(INTMAX, &proc_base_lock);

#ifdef _POWER_MP
		simple_lock(&proc_int_lock);

		fetch_and_or((int *)&parent->p_atomic, SFWTED);
		fetch_and_or((int *)&child->p_atomic, SFWTED);
#else
		parent->p_atomic |= SFWTED;
		child->p_atomic |= SFWTED;
#endif

		child->p_flag |= (SMPTRACE /*| STRC*/);
		pidsig(child->p_pid, SIGTRAP);
		pidsig(parent->p_pid, SIGTRAP);

#ifdef _POWER_MP
		simple_unlock(&proc_int_lock);
#endif
		unlock_enable(ipri, &proc_base_lock);
	}
	simple_unlock(&ptrace_lock);

	/* get per-process loader lock in a multi-threaded process */
	if (parent->p_active > 1) {
		la = (struct loader_anchor *)(U.U_loader);
		(void) lockl(&la->la_lock, LOCK_SHORT);
	}

	/*
	 * Call a machine-dependent function to build the child
	 * process image.  If unsuccessful, it returns -1.  Otherwise
	 * it returns twice: first in the parent process with a pointer
	 * to the child's u-block, which is temporarily attached to the
	 * parent's address space; and later in the child process
	 * with a zero function value.
	 */
	switch ((int)(ubchild = procdup(child)))  {

	      case -1:			/* procdup() failed? */
	
                /* 
		 * Unlock parent's per-process loader lock for 
		 * multi-threaded process.  
		 */
		if (la)
			unlockl(&la->la_lock);

		simple_lock(&ptrace_lock);

		if (child->p_flag & STRC) {
			int	rc;
			rc = ptrace_detach(child);
			assert(!rc);
		}

ptrace_unlock:
		simple_unlock(&ptrace_lock);

		ipri = disable_lock(INTMAX, &proc_base_lock);

		/* give back the proc struct */
		parent->p_child = child->p_siblings;

		/* 
		 * Must find child entry process group list.
		 * It could be anywhere, since the parent may 
		 * go to sleep, due to paced forks. 
		 */
		if (parent->p_pgrp)
		{
			g = VALIDATE_PID(parent->p_pgrp);
			for (g = g->p_ganchor; 
				g && g->p_pgrpl && g->p_pgrpl != child;
					g = g->p_pgrpl);
			g->p_pgrpl = child->p_pgrpl;
		}
		else				
		{
			/* 
			 * init has a process grp 0, but is not chained 
			 * to the swapper's process group list.
			 */
			parent->p_pgrpl = child->p_pgrpl;
		}

		child->p_pgrpl = NULL;
		unlock_enable(ipri, &proc_base_lock);

		simple_lock(&proc_tbl_lock);
		freeproc(child);
		simple_unlock(&proc_tbl_lock);


		return(-1);		/* u.u_error was set in procdup() */

	      case 0:			/* normal return in CHILD process */

		U.U_start = time;	/* fill in rest of child's u_block*/
		U.U_ticks = lbolt;
		U.U_acflag = AFORK;
		U.U_ior = U.U_iow = U.U_ioch = 0;
		bzero(&U.U_cru, sizeof(struct rusage));
		bzero(&U.U_ru, sizeof(struct rusage));
#ifdef  _LONG_LONG
                U.U_irss = 0;
#else
                U.U_irss[0] = U.U_irss[1] = 0;
#endif
		U.U_semundo = NULL;
		U.U_lock = 0;
                U.U_ulocks = EVENT_NULL;

		/*
		 * If parent was profiling, child must as well.  Pin childs
		 * profiling buffer.  This does not need to be serialized with
		 * the sys_timer, because there is only one thread and it is
		 * in kernel mode.
		 */
		if (U.U_prof.pr_scale) {
			if (pinu((char *)U.U_pprof->pin_buf, U.U_pprof->pin_siz,
				(short)UIO_USERSPACE)) {
				/* pinu failed  so turn profiling off */
				bzero(&U.U_prof, sizeof(U.U_prof));
				U.U_pprof = NULL;
			} else {
				U.U_pprof->count++;
			}
		} else {
			/* we are not profiling */
			bzero(&U.U_prof, sizeof(U.U_prof));
			U.U_pprof = NULL;
		}
			
		tfork(&U, &uthr0);	/* initialize timers */
		uthr0.ut_audsvc = NULL;	/* initialize per-thread auditing */

		return(0);		/* return as child */

	      default:			/* normal return in PARENT process */
		break;			/* continue rest of fork()... */
	}

	/*
	 * from here on, no more errors will occur.
	 */
	sysinfo.sysfork++;		/* count fork's */
	cpuinfo[CPUID].sysfork++;
	TRCHKL2(HKWD_SYSC_FORK, child->p_pid, child->p_threadlist->t_tid);

	/*
	 * Set up important fields in new process ublock.
	 */
	ubchild->ub_user.U_procp = child;
	ubchild->ub_uthr0.ut_save.curid = child->p_pid;
	ubchild->ub_uthr0.ut_flags = ut->ut_flags & UTSIGONSTACK;
	ubchild->ub_user.U_auditstatus = AUDIT_RESUME;

	/*
	 * Reset kernel stack to default but keep inherited parent's errno
	 * and user data.
	 * Note that when the child will start executing, it will be on the
	 * special (fork) kernel stack. However that's OK because ut_kstack
	 * is not used to know where the current kernel stack is (this
	 * information is in r1 of course) but to choose a kernel stack when
	 * entering the kernel (SVC or signal delivery).
	 */
	ubchild->ub_uthr0.ut_kstack = (char *)&__ublock;
	 
	/*
	 * Reset the uthread table control block and the kernel stacks segment.
	 *
	 * Note: The PM_ZERO flag MUST NOT be specified because we are freeing
	 * entries of zombies before they are actually no more needed. Therefore
	 * they can be reallocated but still in use by the previous owner and
	 * thus must not be altered.
	 */
	(void)pm_init(	&ubchild->ub_user.U_uthread_cb,		  /* zone  */
			(char *)&__ublock.ub_uthr,		  /* start */
			(char *)&__ublock + sizeof(struct ublock),/* end   */
			sizeof(struct uthread),			  /* size  */
			(char *)&uthr0.ut_link - (char *)&uthr0,  /* link  */
			UTHREAD_ZONE_CLASS,			  /* class */
			child-proc,				  /* occur */
			PM_FIFO);				  /* flags */
	child->p_kstackseg = NULLSEGVAL;

	/*
	 * Allocate and initialize the private locks.
	 * (U_fd_lock and U_fso_lock are initialized by fs_fork().
	 */
	lock_alloc(&ubchild->ub_user.U_handy_lock,
		LOCK_ALLOC_PIN, U_HANDY_CLASS, child-(struct proc *)&proc);
	lock_alloc(&ubchild->ub_user.U_timer_lock,
		LOCK_ALLOC_PIN, U_TIMER_CLASS, child-(struct proc *)&proc);
	simple_lock_init(&ubchild->ub_user.U_handy_lock);
	simple_lock_init(&ubchild->ub_user.U_timer_lock);

        /*
         * Loop through the registered resource handlers
         * Start over if the list changes.  The serialization of this
	 * list is strange because the resource handlers can sleep
	 * causing deadlock problems if any other lock but the kernel
	 * lock is used.  Since the kernel lock may be given up, the
	 * list can change which is why the generation count is used.
	 * Both of these techniques must be used to ensure list integrity.
         */
        do {
		(void) lockl(&kernel_lock, LOCK_SHORT);
                old_gen = proch_gen;
                for (proch = proch_anchor; proch != NULL; proch = proch->next) {
                        /* call resource handler with parms */
                        (proch->handler)(proch, PROCH_INITIALIZE, child->p_pid);
			(proch->handler)(proch, THREAD_INITIALIZE,
						    child->p_threadlist->t_tid);
                        if (proch_gen != old_gen)
                                break;
		    }
	} while (proch_gen != old_gen);
	unlockl(&kernel_lock);

	ld_usecount(1);			/* increment loader use counts */

	/* unlock per-process loader lock for multi-threaded process */
	if (la) {
		unlockl(&la->la_lock);
		/* Initialize lock in ublock of child process */
		la = (struct loader_anchor *)(ubchild->ub_user.U_loader);
		la->la_lock = LOCK_AVAIL;
	}

	assert(ld_ptracefork(child->p_pid) == 0);

	/* perform all fork related file system operations */
	fs_fork(&ubchild->ub_user);

	/* increment the reference count on the credentials structure */
	crhold(U.U_cred);

	/* perform all fork related adspace operations */
	vmm_fork(U.U_map, &ubchild->ub_user);

	vm_det(ubchild);		/* detach child's ublock */

	/*
	 * Put child on ready queue.  A context switch occurs during the
	 * unlock_enable call. The child is run before the parent.  It starts 
	 * at the return from forksave().  The call to setrq() changes the 
	 * process state from SIDL to SRUN.
	 */
	ipri = disable_lock(INTMAX, &proc_int_lock); /* for the dispatcher */

	nth = child->p_threadlist;

	/* 
	 * Need to reinitialize a couple of fields, since they are updated
	 * outside of the context of the thread.  The lock owner's t_boosted 
	 * field is referenced in the course of priority promotion.  The 
	 * previous thread's t_lockcount is referenced by the dispatcher.
	 * These fields can go to -1, if the scheduler harvests the thread
	 * just before the update.  
	 */ 
        nth->t_boosted = 0;    
        nth->t_lockcount = 0; 
        nth->t_sav_pri = nth->t_pri = prio_calc(nth);
	setrq(nth, E_WKX_PREEMPT, RQTAIL);		   /* add child to rq */
	child->p_active = 1;		 /* the (unique) thread is now active */
	schedfork(child);		     /* mark as candidate for swapout */
	INC_RUNRUN(1);			              /* force context switch */

	unlock_enable(ipri, &proc_int_lock);/* enable ints, switch contexts */

	return(child->p_pid);		/* parent returns pid of child */
}


/*
 * NAME: creatp
 *
 * FUNCTION: create a kernel process
 *
 * RETURNS:	process ID of newly created process, if successful;
 *		-1, if unsuccessful (u.u_error has the reason).
 *
 * NOTES:
 *
 *	This will create a kernel process that will eventually be
 *	initialized and dispatched by initp().  The caller can
 *	set up queues and other resources prior to the process
 *	being readied.
 *
 *	The SIDL state is needed for exit() to realize that this
 *	is not yet a full process.  It is hooked onto the parent's
 *	chain only for purposes of exit() remembering to throw
 *	away the child pid if the parent gets killed before it finishes
 *	initializing the child.  Processes in the SIDL state cannot
 *	be sent signals.
 *
 * Process State Transitions:
 *	SNONE ==> SIDL
 */
pid_t 
creatp()
{
	register struct proc *child;	/* new proc pointer */

	/* allocate a new process structure */
	if ((child = newproc(&u.u_error, SKPROC)) == NULL) {
		if (u.u_error == EAGAIN)  /* a proc threshold was reached */
                     sysinfo.koverf++;    /* increment counter */
		return(-1);
        }

	sysinfo.ksched++;		/* increment kernel process counter*/

	return(child->p_pid);
}

/*
 * NAME: initp
 *
 * FUNCTION: Initialize a kernel process.
 *
 *	This function completes the work of building a kernel
 *	process that was started in creatp().  The caller
 *	must be the same process that originally called creatp().
 *
 * NOTES:
 *	Process State Transitions:
 * 		not quite created (SIDL) ==> ready (SRUN)
 * RETURNS:
 *	0 if successful;
 *	errno value otherwise.
 */
initp(pid_t pid, int (*init_func)(), char *init_data_addr, 
	int init_data_length, char name[])
/* register pid_t	pid;		Process ID */
/* register int	(*init_func)();		Pointer to initial function */
/* register char *init_data_addr;	Initialization data, address */
/* register int	init_data_length;	- and length */
/* register char   name[4];		name of the process */
{
	register struct proc *c;	/* new proc table pointer */
	register struct proc *p;	/* calling proc table pointer */
	register struct ublock *nub;	/* new proc's ublock pointer */
	register struct thread *nth;	/* new thread's thread pointer */
	register int	ipri;		/* saved interrupt priority */
	register int	klock_rc;	/* lockl() return value */
	struct proch	*proch;
	int             old_gen;        /* old proch gen */
	
	/*
	 * validate the input pid, and make sure the input proc is in
	 * the SIDL state.
	 */
	c = PROCPTR(pid);
	ASSERT(c->p_pid == pid && c->p_stat == SIDL);

	/*
	 * perform platform-dependent machine state initialization
	 */
	nub = (struct ublock *)procinit(c, init_func,
					   init_data_addr, init_data_length);
	if (nub == NULL)		/* unable to create process? */
		return(ENOMEM);
		
	/*
 	 * inherit attributes from parent process
	 * Note:  the u.u_cdir and u.u_cred fields referenced below will
	 *	  always be set except when the wait proc (proc 2) is
	 *	  initp-ed.  In this case, the vfs and cred structures
	 *	  have not yet been initialized, so the fields are invalid.
	 *	  As a result the code was put in 'if' statements.  This
	 *	  is a sign that the current implementation of the system
	 *	  initialization is flawed and should be readdressed:
	 *	  ie., why is a child process of proc 0 getting set up
	 *	  before proc 0 is even initialized?
 	 */
	bcopy(&U.U_rlimit, &nub->ub_user.U_rlimit, sizeof(U.U_rlimit));

	/*
	 * Kernel processes will not get a root or current directory
	 * They must do an explicit chdir() or chroot() to have
	 * these set up.
	 * The filesystem knows what to do when it sees a NULL cdir
	 */
        nub->ub_user.U_cdir = NULL;
        nub->ub_user.U_rdir = NULL;

        nub->ub_user.U_cmask = U.U_cmask;        /* file creation mask */
        nub->ub_user.U_maxofile = U.U_maxofile;

	if (U.U_cred) {
		nub->ub_user.U_cred = U.U_cred; /* copy cred pointer */
		crhold(U.U_cred);		/* and up use count */
	}

        nub->ub_uthr0.ut_save.prev = NULL;  /* set up back chain */
        nub->ub_user.U_procp = c;       /* save pointer to proc in u-block */
        nub->ub_uthr0.ut_save.curid = c->p_pid;

	/*
	 * Save the name of the kernel process in the u_block
	 */
        nub->ub_user.U_comm[0] = name[0];
        nub->ub_user.U_comm[1] = name[1];
        nub->ub_user.U_comm[2] = name[2];
        nub->ub_user.U_comm[3] = name[3];

	/*
	 * complete proc and u-block initialization
	 */
        nub->ub_uthr0.ut_save.intpri = INTBASE;  /* init int pri to base */
        nub->ub_uthr0.ut_save.stackfix = NULL;   /* zero out stack fix */
        nub->ub_user.U_pprof = NULL;    /* This process will not be profiling */
        nub->ub_user.U_dp = NULL;

	/* initialize per-thread timers */
	tfork(&nub->ub_user, &nub->ub_uthr0);

	/* initialize per-thread auditing */
	nub->ub_uthr0.ut_audsvc = NULL;

        /* void errnop address since it's a kproc */
        nub->ub_uthr0.ut_errnopp = (int **)0xC0C0FADE;

	/* initialize uthread table control block and kernel stacks segment */
	(void)pm_init(	&nub->ub_user.U_uthread_cb,		  /* zone  */
			(char *)&__ublock.ub_uthr,		  /* start */
			(char *)&__ublock + sizeof(struct ublock),/* end   */
			sizeof(struct uthread),			  /* size  */
			(char *)&uthr0.ut_link - (char *)&uthr0,  /* link  */
			UTHREAD_ZONE_CLASS,			  /* class */
			c-proc,					  /* occur */
			PM_FIFO);				  /* flags */
	c->p_kstackseg = NULLSEGVAL;

	/* allocate and initialize the private locks */
	lock_alloc(&nub->ub_user.U_handy_lock,LOCK_ALLOC_PIN,
			U_HANDY_CLASS,c-(struct proc *)&proc);
	lock_alloc(&nub->ub_user.U_timer_lock,LOCK_ALLOC_PIN,
			U_TIMER_CLASS,c-(struct proc *)&proc);
        lock_alloc(&nub->ub_user.U_fd_lock,LOCK_ALLOC_PAGED,
			U_FD_CLASS,c-(struct proc *)&proc);
        lock_alloc(&nub->ub_user.U_fso_lock,LOCK_ALLOC_PAGED,
			U_FSO_CLASS,c-(struct proc *)&proc);
        simple_lock_init(&nub->ub_user.U_handy_lock);
        simple_lock_init(&nub->ub_user.U_timer_lock);
        simple_lock_init(&nub->ub_user.U_fd_lock);
        simple_lock_init(&nub->ub_user.U_fso_lock);

	/* initailize VMM U_adspace_lock  */
	nub->ub_user.U_lock_word = -1;
	nub->ub_user.U_vmm_lock_wait = NULL;

        nub->ub_user.U_start = time;
        nub->ub_user.U_ticks = lbolt;
        nub->ub_user.U_ior = U.U_iow = U.U_ioch = 0;
	bzero(&nub->ub_user.U_cru, sizeof(U.U_cru));
	bzero(&nub->ub_user.U_ru, sizeof(U.U_ru));
#ifdef  _LONG_LONG
        nub->ub_user.U_irss = 0;
#else
        nub->ub_user.U_irss[0] = nub->ub_user.U_irss[1] = 0;
#endif

	vm_det(nub);			/* release child's ublock */

	/* process calling initp() must have been the caller of creatp() */
	p = curproc;                    /* get current proc ptr */
	assert(c->p_ppid == p->p_pid);

        /*
         * Loop through the registered resource handlers
         * Start over if the list changes.
         */
	klock_rc = lockl(&kernel_lock,LOCK_SHORT);
        do {
            old_gen = proch_gen;
            for (proch = proch_anchor; proch != NULL; proch = proch->next) {
                /* call resource handler with parms */
                (proch->handler)(proch, PROCH_INITIALIZE, c->p_pid);
		(proch->handler)(proch, THREAD_INITIALIZE,
							c->p_threadlist->t_tid);
                if (proch_gen != old_gen) {
		    (void) lockl(&kernel_lock,LOCK_SHORT);
                    break;
		}
            }
        } while (proch_gen != old_gen);
        if (klock_rc != LOCK_NEST)
            unlockl(&kernel_lock);

	/*
	 * make process ready to execute.
	 */
	ipri = disable_lock(INTMAX, &proc_int_lock);

	nth = c->p_threadlist;

        /*
         * Need to reinitialize a couple of fields, since they are updated
         * outside of the context of the thread.  The lock owner's t_boosted
         * field is referenced in the course of priority promotion.  The
         * previous thread's t_lockcount is referenced by the dispatcher.
         * These fields can go to -1, if the scheduler harvests the thread
         * just before the update.
         */
        nth->t_boosted = 0;
        nth->t_lockcount = 0;
	nth->t_sav_pri = nth->t_pri = prio_calc(nth);
	setrq(nth, E_WKX_PREEMPT, RQTAIL);	      /*p_stat:SIDL==>SACTIVE */
	c->p_active = 1;		  /*the (unique) thread is now active */

	unlock_enable(ipri, &proc_int_lock);

	return(0);
}
