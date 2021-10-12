static char sccsid[] = "@(#)91	1.36.1.20  src/bos/kernel/si/aixinit.c, syssi, bos41J, 9507A 2/8/95 14:29:43";
/*
 * COMPONENT_NAME: (SYSSI) System Initialization
 *
 * FUNCTIONS:	proc1init, proc1restart
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/systm.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/seg.h>
#include <sys/sched.h>
#include <sys/conf.h>
#include <sys/pin.h>
#include <sys/xmem.h>
#include <sys/syspest.h>
#include <sys/lockl.h>
#include <sys/sleep.h>
#include <sys/errno.h>
#include <sys/var.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>

extern void p2vmminit();
extern void p2fsinit();
extern struct var v;
extern Simple_lock cs_lock;

void (*p2init_tbl[])() = {	/* phase 2 init. functions */
	p2vmminit,
	p2fsinit,
	NULL
};

struct fcndes {
	ulong addr;
	ulong toc;
	ulong rsvd;
};

/*
 * NAME: csinit
 *
 * FUNCTION:
 *	called during system initialization to find the system call
 *	index for the cmp_swap() system call.  cmp_swap() is used
 * 	by the Power PC implementation of cs().
 *
 * EXECUTION ENVIRONMENT:
 *	system initialization only.
 *
 *	must be after init_ldr()
 *
 * RETURNS: None
 */
void
csinit()
{
	struct fcndes *scp;
	extern uint cmp_swap_index;

	/*
	 * look up loaders entry for cmp_swap() in it sc table
	 */
	scp = (struct fcndes *)ld_svcaddress("cmp_swap");
	assert(scp != NULL);

	/*
	 * save index away for easy access by cs()
	 */
	cmp_swap_index = scp->toc;

	lock_alloc(&cs_lock, LOCK_ALLOC_PAGED, CS_LOCK_CLASS, -1);
	simple_lock_init(&cs_lock);

}


/*
 * NAME: phasetwoinit
 *
 * FUNCTION: called when paging space has been defined, to allow
 *	kernel components to grow their data structures to run time
 *	levels
 *
 * EXECUTION ENVIRONMENT:
 *	called from defineps
 *
 * RETURNS: NONE
 */

void
phasetwoinit()
{
	int i;

	for (i = 0 ; i < sizeof(p2init_tbl) / sizeof(p2init_tbl[0]) ; i++)
		if (p2init_tbl[i])
			(*p2init_tbl[i])();

}

proc1init()
{
	struct initargs
	{
		char	* argv[3];
		char	pname[16];
		char	param[4];
		int	* errnop;
	} * argptr;
	int (*execptr)();
	volatile register int junk;

	/* construct arguments to execve in user storage */
	argptr = (void *) PRIVORG;
	argptr->argv[0] = argptr->pname;
	argptr->argv[1] = NULL;
	argptr->argv[2] = NULL;
	bcopy ("/etc/init", argptr->pname, 16);

	if( strcmp(v.v_initlvl,"") ) {
		argptr->argv[1] = argptr->param;
		bcopy(v.v_initlvl, argptr->param,4);
		v.v_initlvl[0] = (char) 0;
	}

	argptr->errnop = &errno;	/* don't use standard errnop to */
					/* avoid an extra page fault */
	tfork(&U, &uthr0);		/* initialize timers */
	ASSERT(uthr0.ut_audsvc == NULL);/* initialize per-thread audit  */

	/* initialize vmm fields in u block */
	U.U_lock_word = -1;
	U.U_vmm_lock_wait = NULL;

	/* establish a current directory for init */
	if (U.U_cdir == NULL)
	{
		U.U_cdir = rootdir;
		VNOP_HOLD(U.U_cdir);
	}
	U.U_compatibility = 0;
	U.U_ulocks = EVENT_NULL;

	/*
	 * At this point, process 1 changes from a kernel process into
	 * a user process, and exec's /etc/init.
	 */
	proc[1].p_flag &= ~SKPROC;	/* turn into a user process */
	proc[1].p_threadlist->t_flags &= ~TKTHREAD; /* user thread */

	/* As a user thread, we will need correct ut_kstack and ut_errnopp */
	uthr0.ut_kstack = (char *)&__ublock;
	uthr0.ut_errnopp = &argptr->errnop;

	execptr = ld_svcaddress("execve");

	/*
	 * Touch the stack.  There is nothing else running so it
	 * will stay in memory
	 */
	i_enable(i_disable(INTIODONE));

	/*
	 * Touch the page of code we're about to branch to.
	 */
	junk = *((int *) execptr);

	/*
	 * DANGER ZONE: now in user mode, but still in kernel space.
	 * Since the stack was touched by the i_disable/i_enable
	 * and interrupts are disabled with the MSR
	 * there will be no interrupts or exceptions until
	 * the SC instructions is executed.
	 *
	 * setting a break point between here and the SVC instruction
	 * will cause the system to crash
	 */

	disable();
	(proc[1].p_threadlist->t_suspend)--;
	usermode();			/* give up kernel privileges */

	/* DANGER ZONE: now in user mode, but still in kernel space */

	(*execptr)(argptr->pname, argptr, NULL);

	/*
	 * The exec should not fail and hence should never return.
	 * If it does, we're in deep trouble...
	 */
	panic("exec of /etc/init failed.");
}

proc1restart()
{
	register struct proc *p1;	/* init process ptr */
	char		err;		/* return code from newproc() */

	/* allocate a proc table slot */
	p1 = (struct proc *) newproc(&err, SKPROC); 

	assert(p1 == &proc[1]);		/* it better be slot 1 */

	proc[0].p_child = p1;		/* proc[1] is the child of proc[0] */
	p1->p_pid = 1;			/* hardcode the PID */
	p1->p_siblings = NULL;		/* only child of proc[0] */

        /*  
	 * since process 1 inherited some attributes from process 0, which
	 * are not valid, need to reset those attributes.
	 */
        p1->p_flag &= ~SFIXPRI;     /* 'init' is not a fixed priority process */
        p1->p_nice = P_NICE_DEFAULT;
	p1->p_threadlist->t_policy = SCHED_OTHER;

	initp(1, proc1init, -1, 0, "init");
}
