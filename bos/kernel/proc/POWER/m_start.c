static char sccsid[] = "@(#)99  1.20.1.15  src/bos/kernel/proc/POWER/m_start.c, sysproc, bos412, 9445C412a 10/25/94 12:22:23";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: si_context_init
 *              init_flihs
 *		proc0init
 *
 *   ORIGINS: 27, 83
 *
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#include <sys/types.h>
#include <sys/systm.h>
#include <sys/intr.h>
#include <sys/mstsave.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/pseg.h>
#include <sys/syspest.h>
#include <sys/vmuser.h>
#include <sys/adspace.h>
#include <sys/errno.h>
#include <sys/low.h>
#include <sys/processor.h>
#include <sys/systemcfg.h>

struct mstsave	si_save={0};		/* temporary mstsave area for SI */
struct proc	si_proc={0};		/* temporary proc table for SI */
struct thread	si_thread={0};		/* temporary thread table for SI */

/*
 * NAME: si_context_init
 *
 * FUNCTION:
 *	set up context at boot time
 *
 * NOTES:
 */
si_context_init(id)
{
	SET_CURTHREAD(&si_thread);	/* fake thread table for SI */

	/* link fake proc entry and fake thread entry together */
	si_proc.p_threadlist = &si_thread;
	si_thread.t_procp = &si_proc;

	si_thread.t_uthreadp = &uthr0;	/* fixed address (for "u" macro) */
	si_thread.t_userp = &U;		/* fixed address (for "u" macro) */

	/* Swapper uses PID 0, but can't use TID 0 because TIDs are odd */
	si_thread.t_tid = id;

	/*
	 * Prevent delivery of signals. Not that we can receive any, but
	 * t_suspend is also used to know if we are in kernel mode or not
	 * and resume fetches the segment registers from MST or U accordingly.
	 * This behavior may change with the new graphics requirements.
	 */
	si_thread.t_suspend = 1;

	si_proc.p_adspace = NULLSEGVAL;	/* don't have a data segment */
	si_proc.p_uidl = &si_proc;	/* initial UID list */
 	si_proc.p_threadcount = 1;	/* only one thread */

	si_save.intpri = INTMAX;	/* allow disabled faults later */
	as_init(&si_save.as);		/* initialize the address space */
}

/*
 * NAME: init_flihs
 *
 * FUNCTION:
 *	Initializes the first level interrupt handler (FLIH) save areas.
 *
 * NOTES:
 *	This routine provides a temporary proc structure and save area so
 *	we can take interrupts before strtdisp() is called to build the
 *	first processes.
 *
 *	MP_MASTER's csa has already been set up in hardinit() as being
 *	si_save, therefore there is no need to invoke SET_CSA(). As for
 *	the other processors, start_bs_proc() will do the SET_CSA().
 */
init_flihs()
{
	register struct mststack *mst;	/* mstsave stack pointer */
	register struct mststack *next;	/* next mstsave stack pointer */
	register struct mststack *mst1, *mst_top;
	register struct ppda	 *p_da;	/* ppda pointer */
	register int		 Cpu;	/* processor loop index */
	register int		 NCpu;	/* number of processors */

	/* Swapper uses PID 0, but can't use TID 0 because TIDs are odd */
	si_context_init(SWAPPER_TID);

	/* link the backchains of the successive stack entries */

	for (NCpu = _system_configuration.ncpus, Cpu = 0; Cpu < NCpu; Cpu += 1)
	{
		p_da	= &ppda[Cpu];
		mst1	= &mststack[FRAME_1(Cpu)];
		mst_top	= &mststack[NUMBER_OF_FRAMES * Cpu];

		p_da->mstack	= &mst1->save;	/* first FLIH stack area */
		p_da->_csa	= &si_save;	/* setup CPU to si save area */
		mst1->save.prev	= &si_save;	/* set initial backchain */

		for (mst = mst1; mst > mst_top; mst = next)
		{
			next = mst - 1;
			as_init(&next->save.as);/* initialize the & space */
			next->save.prev = &mst->save;
		}

		/*
		 * Set address of per-processor V=R pmap stack.
		 */
		p_da->pmapstk 	= &pmap_stack[PMAP_STK_SIZE * (Cpu + 1)];
	}
	return 0;
}


/*
 * NAME: proc0init
 *
 * FUNCTION: Initialize process 0 address space.
 *
 *	This program allocates a u-block and kstack for process 0.
 *	For this platform, a process private segment is created.
 *	On machines for which addressing is expensive, this would
 *	not be necessary.
 *
 * NOTES:
 *	This routine is platform-specific.  It is called by strtdisp()
 *	at boot time to build the proc[0] address space.  When called,
 *	only the kernel global addressing is enabled.
 */

void 
proc0init(struct proc *p)
/* register struct proc	*p;	proc structure pointer */
{
	register long	dmax;		/* user data region limit (in bytes) */
	register long	smax;		/* user stack region limit (in bytes) */
	vmid_t		vm_id;		/* process private VM ID */
	register vmhandle_t vm_handle;	/* process private VM handle */
	register int	rc;		/* vms_create return code */
	register int	i;		/* loop counter (resource limits) */

	dmax = PAGESIZE;	/* proc1init uses some storage at PRIVORG*/
	smax = DFLSSIZ;

	/*
	 * create a private segment for proc[0]'s u-block, and get
	 * addressability to it
	 */
	rc = vms_create( &vm_id, V_WORKING|V_PRIVSEG, NULL, 
				U_REGION_SIZE, dmax, K_REGION_SIZE+smax );
	assert(rc == 0);

	vm_handle = SRVAL(vm_id,0,0);
	p->p_adspace = vm_handle;	/* store u-block handle in proc table */
	vm_seth(vm_setkey(vm_handle, VM_PRIV), &U);

	/*
	 * Initialize the new address space.  Also initialize the user 
	 * side for ps statistics.
	 */
	as_init(&uthr0.ut_save.as);
	as_ralloc(&uthr0.ut_save.as, KERNELORG);
	as_ralloc(&uthr0.ut_save.as, KERNEXORG);
	as_ralloc(&uthr0.ut_save.as, &U);

	as_init(&U.U_adspace);
	as_ralloc(&U.U_adspace, &U);
	as_seth(&U.U_adspace, vm_setkey(p->p_adspace, VM_UNPRIV), &U);

	/* now we have a u-block. */
	for (i = 0; i < RLIM_NLIMITS; i++)
		U.U_rlimit[i].rlim_cur = U.U_rlimit[i].rlim_max = RLIM_INFINITY;

	/* initialize the timer fields */
	tfork(&U, &uthr0);

	/* initialize the audit pointer */
	uthr0.ut_audsvc = NULL;

	U.U_dsize = dmax;
	U.U_rlimit[RLIMIT_DATA ].rlim_max = U.U_dmax  = DFLDSIZ;
	U.U_rlimit[RLIMIT_STACK].rlim_max = U.U_smax  = U.U_ssize = smax;

	/* process 0 ignores all signals */
	SIGFILLSET(p->p_sigignore);
	/* except death of child which requires default action */
	SIGDELSET(p->p_sigignore,SIGCHLD);
	/* but never deliver a signal to proc 0 */
	p->p_threadlist->t_suspend = 1;
}
