static char sccsid[] = "@(#)61  1.58  src/bos/kernel/proc/strtdisp.c, sysproc, bos41J, 9513A_all 3/24/95 15:33:02";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: strtdisp
 *		strtwait_bs_proc
 *
 *   ORIGINS: 27, 83
 *
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#include <sys/param.h>
#include <sys/var.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/lockl.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/sysinfo.h>
#include <sys/syspest.h>
#include <sys/systm.h>
#include <sys/pri.h>
#include <sys/intr.h>
#include <sys/vmuser.h>
#include <sys/sysconfig.h>
#include <sys/sched.h>
#include <sys/systemcfg.h>
#include <sys/adspace.h>

extern void waitproc();			/* wait process function name */
extern void except_add();
extern struct proc *newproc();

extern struct pm_heap proc_cb;
extern struct pm_heap thread_cb;

/*
 * NAME: strtdisp
 *
 * FUNCTION:
 *	Initializes the process dispatcher.  This involves initializing
 *	the proc table and creating the first three processes
 *	(swapper, init, and wait).  Exits running under process zero.
 */
void
strtdisp()
{
	register struct proc *p;	/* swapper process ptr */
	register struct proc *c;	/* init process ptr */
	register struct proc *w;	/* wait process ptr */
	char		si_err;		/* return code from newproc() */
	int		pspid;
	extern struct	cfgncb	cb;	/* config notification control block */
	extern int child_adj();		/* config notification routine */
	int i, ipri;

	/* initialize sleep hash anchors */
	init_sleep();

	/* initialize process component dump tables */
	(void) proc_dump_init();
	(void) thread_dump_init();

	/* initialize table control blocks (cf. ipl.exp) */
	ASSERT((char *)&proc[NPROC] <= (char *)&thread);
	(void)pm_init(	&proc_cb,				       /*zone */
			(char *)&proc,				       /*start*/
			(char *)&proc[NPROC],			       /*end  */
			sizeof(struct proc),			       /*size */
			(char *)&proc[0].p_link - (char *)&proc[0],    /*link */
			PROC_ZONE_CLASS,			       /*class*/
			-1,					       /*occur*/
			PM_ZERO);				       /*flags*/
	/* note: &lock_pinned bounds the thread table below NTHREAD */
	(void)pm_init(	&thread_cb,				       /*zone */
			(char *)&thread,			       /*start*/
			(char *)&lock_pinned,			       /*end  */
			sizeof(struct thread),			       /*size */
			(char *)&thread[0].t_link - (char *)&thread[0],/*link */
			THREAD_ZONE_CLASS,			       /*class*/
			-1,					       /*occur*/
			PM_ZERO);				       /*flags*/

	/*
	 *  initialize proc[0] and make it the current process
	 */
	p = newproc(&si_err, SKPROC);	/* allocate a proc table entry */
	assert(p == &proc[0]);		/* better be the first one! */

	proc[0].p_pid = 0;		/* hardcode the PID */
	proc[0].p_threadlist->t_tid = SWAPPER_TID;/* same as si_thread (lock) */
	proc[0].p_threadlist->t_policy = SCHED_FIFO;
	proc[0].p_threadlist->t_pri = PRI_SCHED;
	proc[0].p_threadlist->t_sav_pri = PRI_SCHED;
	proc[0].p_threadlist->t_state = TSRUN;
	proc[0].p_threadlist->t_userdata = 0;
#ifdef _POWER_MP
	proc[0].p_threadlist->t_cpuid =
	proc[0].p_threadlist->t_scpuid = PROCESSOR_CLASS_ANY;
#endif
	proc[0].p_active = 1;
	proc[0].p_nice = PRI_SCHED;
	proc[0].p_stat = SACTIVE;
	proc[0].p_flag |= (SLOAD | SNOSWAP | SFIXPRI);
	proc[0].p_siblings = NULL;	/* only child */
	proc[0].p_child = p;		/* proc[0] is its own child */
	proc[0].p_uidl = p;		/* only process for UID 0 */
	proc[0].p_pgrpl = NULL;		/* only process for pgrp 0 */
	proc[0].p_ttyl = NULL;		/* really no session 0 for process 0 */

	i_enable(INTBASE);		/* enable interrupts */

	/* perform machine-dependent address space setup */
	proc0init(p);			/* init proc[0] address space */

	/* now we have a u-block; pin proc 0's ublock */
	ltpin(round_down(&__ublock, PAGESIZE), PAGESIZE);

	/*
	 *  We won't get an exception during this sequence, so it's OK
	 *  that interrupts are enabled.
	 *
	 *  This is the switch-over from the fake proc table entry and
	 *  save area to the real stuff.
	 *
	 *  Must set intpri, so we can page fault under this process.
	 */
	U.U_procp = p;
	i_disable(INTMAX);
	SET_CURTHREAD(p->p_threadlist);
	i_enable(INTBASE);

	uthr0.ut_kstack = (char *)&__ublock;		/* needed for dump */
	uthr0.ut_errnopp = (int **)0xC0C0FADE;	  /* never used by kthread */

	/* initialize uthread table control block and kernel stacks segment */
	(void)pm_init(	&U.U_uthread_cb,			  /* zone  */
			(char *)&__ublock.ub_uthr,		  /* start */
			(char *)&__ublock + sizeof(struct ublock),/* end   */
			sizeof(struct uthread),			  /* size  */
			(char *)&uthr0.ut_link - (char *)&uthr0,  /* link  */
			UTHREAD_ZONE_CLASS,			  /* class */
			0,					  /* occur */
			PM_FIFO);				  /* flags */
	p->p_kstackseg = NULLSEGVAL;

	/* allocate and initialize our private locks */
	lock_alloc(&U.U_handy_lock, LOCK_ALLOC_PIN, U_HANDY_CLASS, 0);
	lock_alloc(&U.U_timer_lock, LOCK_ALLOC_PIN, U_TIMER_CLASS, 0);
	lock_alloc(&U.U_fd_lock, LOCK_ALLOC_PAGED, U_FD_CLASS, 0);
	lock_alloc(&U.U_fso_lock, LOCK_ALLOC_PAGED, U_FSO_CLASS, 0);
	simple_lock_init(&U.U_handy_lock);
	simple_lock_init(&U.U_timer_lock);
	simple_lock_init(&U.U_fd_lock);
	simple_lock_init(&U.U_fso_lock);

	uthr0.ut_save.curid = proc[0].p_pid;
	uthr0.ut_save.intpri = INTBASE;	/* set interrupt priority */
	i_disable(INTMAX);
	SET_CSA(&uthr0.ut_save);	/* set up processor for a new csa */
	i_enable(INTBASE);

	/*
	 * Now create the init and wait processes
	 *   NOTE: we must reserve proc[1] for the init process before
	 *   creating any others, though the initp() of proc[1] is done
	 *   later in main() when we're ready to exec `/etc/init'.
	 */

	c = newproc(&si_err, SKPROC);	/* allocate the init process */
	assert(c == &proc[1]);

	proc[0].p_child = c;		/* proc[1] is the child of proc[0] */
	proc[1].p_pid = 1;		/* hardcode the PID */
	proc[1].p_siblings = NULL;	/* only child of proc[0] */
	SIGINITSET(proc[1].p_sigignore);

	/*
	 *  The wait process runs at the lowest possible priority, and
	 *  is always `ready'.  We need to set its nice value, which was
         *  inherited from proc[0], so that its priority will be set to the
         *  lowest possible value.
	 */
	/* We need one waitproc for every processor */
	for (i = 0; i < _system_configuration.ncpus; i++)
	{
		w = newproc(&si_err, SKPROC);	/* allocate the wait process */
		assert(w = &proc[2 + i]);
		w->p_nice = PIDLE;
		w->p_threadlist->t_sav_pri = PIDLE;
		initp(w->p_pid, waitproc, -1, 0,"wait"); /* ready to run */
#ifdef _POWER_MP
	 	/* bind it */
	 	w->p_threadlist->t_cpuid = w->p_threadlist->t_scpuid = (cpu_t)i;
#ifdef _SLICER
		/* See comments in waitproc().
		 * 
		 * We don't want the non-MP_MASTER waitproc()s to be bound at this
		 * point.  This lets them be instantiated into the system prior
		 * to actual slicing.  Full process initialization can be done
		 * without having to kludge it in.  Also, per-CPU initialization
		 * can then be done from the waitproc() without having to create
		 * new initialization routines.
		 */
		{
			struct mststack *p;

			if (i > 0) {
				/* only bind first waitproc */
				w->p_threadlist->t_cpuid = 
					w->p_threadlist->t_scpuid = PROCESSOR_CLASS_ANY;

				/* Initialize ppda specific entries */
				ppda[i]._curthread = w->p_threadlist;
				ppda[i]._csa = ppda[i].mstack;
				p = (struct mststack *)ppda[i].mstack;
				p--;
				ppda[i].mstack = (struct mstsave *)p;
			}
		}
#endif /* _SLICER */
#endif
	}

#ifdef _POWER_MP
#ifndef _SLICER
	/* Take all the non-MASTER wait threads off the runqueue */
	ipri = disable_lock(INTMAX, &proc_int_lock);
	for (i = 1; i < _system_configuration.ncpus; i++) {
		w = &proc[2 + i];
		ASSERT(w->p_threadcount == 1);
		remrq(w->p_threadlist);
	}
	unlock_enable(ipri, &proc_int_lock);
#endif
#endif 

	/*
	 * now need to change some of the 'init' process's attributes that
	 * were inherited from proc[0]
	 */

	proc[1].p_nice = P_NICE_DEFAULT; /* need to change the nice value
                                            since that is inherited from
                                            the parent (i.e. proc[0])      */
	proc[1].p_flag &= ~SFIXPRI; 	 /* init process should not run
					   with a a fixed priority (inherited
					   from proc[0]                    */
	proc[1].p_threadlist->t_policy = SCHED_OTHER;

        /*
	 * need to zero out 'sysinfo' and since two kernel processes 
	 * have been created, sysinfo.ksched needs to be initialized.  
	 */
        bzero(&sysinfo,sizeof(struct sysinfo));
	sysinfo.ksched = 1 + _system_configuration.ncpus;

	/*
	 * Add the system default exception handler
	 */
	except_add();

	/* Register process management's config notification routine on the
	   cfgncb list.  This will cause the cb.func routine to be called
	   when the sysconfig SYS_SETPARMS system call is invoked and will
	   not allow CHILD_MAX to be set lower than _POSIX_CHILD_MAX. */

	cb.cbnext = NULL;
	cb.cbprev = NULL;
	cb.func = child_adj; 
	cfgnadd(&cb);
}

#ifdef _POWER_MP
/*
 * NAME: strtwait_bs_proc
 *
 * FUNCTION: Start wait proc for boot-slave processor
 *	
 * INPUT: None
 *
 * RETURNS:  None
 *
 */
void
strtwait_bs_proc()
{
	struct proc *p;

	p = &proc[2+CPUID];
	SET_CURTHREAD(p->p_threadlist);
	SET_CSA(&uthr0.ut_save);	/* set up processor for a new csa */

	vm_seth(vm_setkey(p->p_adspace, VM_PRIV), &__ublock);

}
#endif /* _POWER_MP */
