static char sccsid[] = "@(#)88	1.48  src/bos/kernel/proc/POWER/m_fork.c, sysproc, bos41J, 9513A_all 3/27/95 15:25:59";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: procdup
 *		procentry
 *		procinit
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


#include <sys/types.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/pseg.h>
#include <sys/intr.h>
#include <sys/syspest.h>
#include <sys/vmuser.h>
#include <sys/vmker.h>
#include <sys/adspace.h>
#include <sys/lockl.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include "ld_data.h"

#define FORK_RETRIES 5

/*
 *  Machine-dependent #define's
 */
#ifdef	_POWER				/* R2 System machine-dependency */
/*
 *  The following bits are set in the MSR:
 *	EE=1 External Int. Enable
 *	ME=1 Machine Check Enable
 *	AL=1 Align. Check  Enable
 *	IP=0 Interrupt Prefix
 *	I and D relocate on
 */
#define DEFAULT_MSR 0x90B0
#endif	_POWER				/* R2 System machine-dependency */

int pacefork=10;

extern struct vmmdseg vmmdseg;
extern union ptaseg ptaseg;
extern procentry();

/*
 * NAME: procdup
 *
 * FUNCTION: Duplicate process address space
 *
 * NOTES:
 *	This routine is platform specific.  It is called by fork()
 *	to set up an address space for the child process.
 *
 * RETURNS (in PARENT):
 *	pointer to the new process's ublock structure, which is attached
 *		to the current address space in the kernel.  The caller
 *		is expected to vm_det() this structure when finished.
 *	-1, if unsuccessful (u.u_error has the reason).
 *		No vm_det() is required in this case.
 *
 * RETURNS (in CHILD):
 *	0, when successful; never returns in the child when unsuccessful.
 */

struct ublock *
procdup(struct proc *child)
/* register struct proc *child;	address of child's proc structure */
{
	register struct thread *t;	/* current thread */
	register struct ublock *nub;	/* address of child's ublock
					   (in parent) */
	vmid_t		vm_id;		/* ID of child's private VM object */
	register vmhandle_t vm_handle;	/* child's VM handle while in kernel */
	int		i;		/* retry counter */
	char		*errorp;	/* current error storage */
	int		sregval;	/* seg reg value for shared data */
	struct loader_anchor *la;	/* used to detect overflow segment */

	t = curthread;
	errorp = &t->t_uthreadp->ut_error;

        /*
	 * Call disown_fp() to ensure that our thread's floating point
	 * state (if any) is entirely stored in the mst (in the u-block)
	 * and our thread does not currently have access to the floating
	 * point hardware.  This is required because
	 *  (1) The child must be given a faithful copy of the entire
	 *      thread state, including floating point registers.
	 *  (2) If our thread owns floating point access at the time
	 *      that the child thread is copied from it, then we will
	 *      have two threads (parent, child) which "own" the
	 *      floating point hardware.  At the very least, this causes
	 *      rampant confusion.
	 */
	disown_fp(t->t_tid);
	   
	/*
	 * Copy the parent's process image.  The parent process cannot 
	 * be swapped out while in vm_forkcopy(). If the thread goes 
	 * to sleep, the vmm puts it in a page wait state (t_wtype == TWPGIN).
	 * Currently, the swapper ( process 0 ) does not perform real IO 
	 * swapping, nor does it unpin pages, so no coordination is needed.  
	 * However, if the swapper changes, this code must also be changed 
	 * to prevent the parent's u_block from being swapped out.
	 *
	 * Because paging space is not preallocated, paging space can be
	 * over committed.  The vmm kills processes when paging space reaches
	 * a low threshold.  Process creation is paced below, not because 
	 * it is a good indication of how much memory the process is going 
	 * to use, but rather because it is an interesting checkpoint.  
	 * Consequently, this is not a definitive solution, but should work 
	 * for homogenous process environments, where processes are created 
	 * to perform small tasks and do not page fault much. 
	 */
	for (i=0; i<FORK_RETRIES; i++) 
	{
		/*
		 * If vm_forkcopy returns ENOSPC, then delay and retry.
		 */
		if ((*errorp = vm_forkcopy
			(SRTOSID(t->t_procp->p_adspace),&vm_id)) == ENOSPC)
		{
			delay(pacefork);
		}
		else
			break;
	}
	if (*errorp == ENOSPC)
		*errorp = ENOMEM;
	if (*errorp)
		return((struct ublock *)(-1));
	
        /*
         * Get addressability to the new proc's ublock.
         * The ublock page was long-term pinned in vm_forkcopy()
         * already.
         */
	vm_handle = SRVAL(vm_id, 0, 0);
	nub = (struct ublock *)(vm_att(vm_handle, (caddr_t)(&__ublock)));

	/*
	 * The thread currently forking may have its uthread block anywhere,
	 * however the resulting child thread will use the default uthread
	 * block (uthr0). To enforce the semantics of fork(), we must therefore
	 * copy it.
	 */
	if (t->t_uthreadp == &uthr0)
		; /* same place, vm_forkcopy has already done the job */
	else
		bcopy(t->t_uthreadp, &nub->ub_uthr0, sizeof(struct uthread));

	/*
	 * initialize the child's address space map
	 */
	child->p_adspace = vm_handle;	/* store u-block handle in proc table */
	as_fork(&nub->ub_uthr0.ut_save.as);/* copy the kernel address space */
	as_det(&nub->ub_uthr0.ut_save.as, nub);
	as_seth(&nub->ub_uthr0.ut_save.as, vm_setkey(vm_handle, VM_PRIV), &U);
	as_seth(&nub->ub_user.U_adspace, vm_setkey(vm_handle, VM_UNPRIV), &U);
	if (nub->ub_uthr0.ut_save.as.alloc & ((unsigned)0x80000000>>SHDATASEG))
		as_det(&nub->ub_uthr0.ut_save.as, SHDATAORG);

	/*
	 * forkcopy the shared library data segment.  The segment register is
	 * protected by the per process loader lock.
	 */
	if (U.U_adspace.alloc & ((unsigned)0x80000000 >> SHDATASEG)) {
		sregval = as_getsrval(&U.U_adspace, SHDATAORG);
		*errorp = vm_forkcopy(SRTOSID(sregval), &vm_id);
		if (*errorp) {
			/*
			 * Detach from segment containing the new ublock
			 * and destroy it.
			 */
			vm_det(nub);
			vms_delete(SRTOSID(child->p_adspace));
			return((struct ublock *)(-1));
		}
		/* compute the new srval for the child, privileged key */
		vm_handle = SRVAL(vm_id,0,0);
		as_ralloc(&nub->ub_user.U_adspace, SHDATAORG);
		as_seth(&nub->ub_user.U_adspace,vm_setkey(vm_handle,VM_UNPRIV),
			SHDATAORG);
		as_ralloc(&nub->ub_uthr0.ut_save.as, SHDATAORG);
		as_seth(&nub->ub_uthr0.ut_save.as,vm_setkey(vm_handle,VM_PRIV),
			SHDATAORG);
	}

	/* If an overflow segment was created for this process,  then it must
	 * be fork copied also.
	 */
	la = (struct loader_anchor *)(U.U_loader);
	if (OVFL_EXISTS(la)) {
		if (*errorp = vm_forkcopy(la->la_ovfl_srval, &vm_id)) {
			/* Error path must undo everything up to this point.
			 * Delete the shared library data segment.  No locking
			 * needed for child's address space.
			 */
			if (nub->ub_user.U_adspace.alloc &
			  ((unsigned)0x80000000 >> SHDATASEG))
				vms_delete(SRTOSID(as_getsrval(
					&nub->ub_user.U_adspace, SHDATAORG)));
			/*
			* Detach from segment containing the new ublock
			* and destroy it.
			*/
			vm_det(nub);
			vms_delete(SRTOSID(child->p_adspace));
			return((struct ublock *)(-1));
		}
		/* Save segement in loader anchor of child */
		la = (struct loader_anchor *)nub->ub_user.U_loader;
		la->la_ovfl_srval = SRVAL(vm_id, 0, 0);
	}

	/* loop through the secondary data segments forkcopying each one */
	for (i = BDATASEG; i <= BDATASEGMAX; i++)
	{
		if (!(U.U_segst[i].segflag & SEG_WORKING))
			break;

		/*
		* If vm_forkcopy is failing due to a lack of paging
		* space, let's nip this big data process in the bud
		* rather than retrying the operation.  If the process
		* has any more segments, then the next call to vm_forkcopy
		* is bound to fail.
		*/
		if (*errorp =vm_forkcopy(SRTOSID(U.U_segst[i].ss_srval),&vm_id))
		{
			/* 
			* Double back and destroy any segments created
			* so far.  We know that all segment numbers from
			* BDATASEG to i-1 are valid SEG_WORKING segments.
			*/
			int j;

			for (j = BDATASEG; j < i; j++)
			  vms_delete(SRTOSID(nub->ub_user.U_segst[j].ss_srval));

			/* Delete the shared library data segment.  No locking
			 * is needed for the child's address space.
			 */
			if (nub->ub_user.U_adspace.alloc &
				((unsigned)0x80000000 >> SHDATASEG))
			  vms_delete(SRTOSID(as_getsrval(
				&nub->ub_user.U_adspace, SHDATAORG)));

			/* Delete overflow segment if one was created */
			la = (struct loader_anchor *)nub->ub_user.U_loader;
			if (OVFL_EXISTS(la))
				vms_delete(SRTOSID(la->la_ovfl_srval));

			/*
			* Detach from segment containing the new ublock
			* and destroy it.
			*/
			vm_det(nub);
			vms_delete(SRTOSID(child->p_adspace));
			return((struct ublock *)(-1));
		}

		/* compute the new srval for the child, unprivileged key */
		vm_handle = SRVAL(vm_id,0,0);
		as_seth(&nub->ub_user.U_adspace,
			vm_setkey(vm_handle,VM_UNPRIV),i<<SEGSHIFT);

		/* set up the segstate in the child */
		nub->ub_user.U_segst[i].segflag = SEG_WORKING;
		nub->ub_user.U_segst[i].num_segs = 1;
		nub->ub_user.U_segst[i].ss_srval = vm_handle;
	}

	
	/* child inherits upfbuf */
	if (U.U_message != NULL)
	{
		nub->ub_user.U_message = U.U_message;
		U.U_message = NULL;
	}

	/*
	 * forksave() is a little like setjmp().  It saves the current
	 * machine state in the new u-block, returning 0 to the
	 * parent and setting up the child to return 1 when it is
	 * dispatched, which happens after fork() calls setrq().
	 */
	if (forksave(&nub->ub_uthr0.ut_save))
		return(0);		/* child process return */
	else
		return(nub);		/* parent process return */
}

/*
 * NAME: procinit
 *
 * FUNCTION: Initialize kernel process machine state.
 *
 * NOTES:
 *	This routine is platform specific.  It is called by initp() during
 *	kernel process start up to initialize the machine state for this
 *	platform.
 *
 *	Since it is cheap to have an address space on this machine,
 *	we go ahead and allocate a process private segment even for
 *	kernel processes.  Note that this would not necessarily be
 *	done for all platforms;  kernel processes can be allocated
 *	completely within the kernel global address space.
 *
 * RETURNS:
 *	pointer to the new process's ublock structure, which is attached
 *		to the current address space in the kernel.  The caller
 *		is expected to vm_det() this structure when finished.
 *	NULL, if unsuccessful.  In this case, there is no child u-area
 *		to be detached by the caller.
 */

struct ublock *
procinit(p, init_func, init_data, length)
register struct proc	*p;	/* process to initialize */
int (*init_func)();		/* pointer to initial function */
register char	*init_data;	/* init data pointer */ 
register int	length;		/* length of init data */
{
	register struct ublock	*nub;	/* new proc's u-block	*/
	register int	i;		/* register index */
	register char	*stack_data;	/* address of data in stack */
	vmid_t		vm_id;		/* process private VM ID */
	register vmhandle_t vm_handle;	/* process private VM handle */

	struct func_desc *proc_desc = (struct func_desc *)procentry;
	int dsize, smax, size;

	/*
 	 * Allocate a process private VM object, including u-block.
	 * The units of smax and dsize are in bytes. Calculate the 
	 * boundaries of the stack. 
 	 */
	dsize = U.U_dsize < SEGSIZE ? U.U_dsize : U.U_sdsize;
	smax = SEGSIZE - K_REGION_SIZE - dsize;

	/* calculate down limits */
	if ((unsigned)U.U_smax < smax)
		smax = U.U_smax;

        size = (p == &proc[1]) ? U_REGION_SIZE : 0;
	if (vms_create(&vm_id, V_WORKING|V_PRIVSEG, 0, 
			size, dsize, smax + K_REGION_SIZE) != 0)
		return(NULL);		
	vm_handle = SRVAL(vm_id, 0, 0);

	/*
 	 * Get addressability to the new proc's ublock and pin it.
 	 */
	nub = (struct ublock *)(vm_att(vm_handle, (caddr_t)(&__ublock)));
	if (ltpin(round_down(nub, PAGESIZE), PAGESIZE) != 0) {
		vm_det(nub);		/* detach from failed segment */
		vms_delete(vm_id);
		return(NULL);
	}

	p->p_adspace = vm_handle;       /* store u-block handle in proc table */

	/*
	 * initialize the new uthread structure (including MST save area)
	 */
	threadinit(&nub->ub_uthr0, NULLSEGVAL, vm_handle, init_func, init_data,
			length, (void *)(DATAORG+USTACK_TOP),
			((char *)nub-(char *)&__ublock));

	/*
	 * build an empty user address space
	 */
	as_init(&nub->ub_user.U_adspace);
	as_ralloc(&nub->ub_user.U_adspace, KERNELORG);
	as_seth(&nub->ub_user.U_adspace, vmker.ukernsrval, KERNELORG);
	as_ralloc(&nub->ub_user.U_adspace, &U);
	as_seth(&nub->ub_user.U_adspace, vm_setkey(vm_handle, VM_UNPRIV), &U);

	/*
	 * The first thread calls procentry, not threadentry.
	 */
#ifdef _POWER
	nub->ub_uthr0.ut_save.gpr[2] = (ulong_t) proc_desc->toc_ptr;
	nub->ub_uthr0.ut_save.iar    = (ulong_t) proc_desc->entry_point;
#endif _POWER

	return(nub);
}

/*
 * FUNCTION: procentry()
 *
 * This routine is the front end of all kernel processes.
 * If the kproc returns, call kexit()
 */
procentry(int data, char *stack, int length, int (*ep)())
{
	int rc = 0;

	/* Call the kproc entry point */
	rc = (*ep)(data, stack, length);

	/* Cause the kernel process to exit */
	kexit(rc);
}
