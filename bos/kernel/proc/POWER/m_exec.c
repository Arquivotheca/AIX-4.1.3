static char sccsid[] = "@(#)78  1.15  src/bos/kernel/proc/POWER/m_exec.c, sysproc, bos411, 9428A410j 2/3/94 15:58:04";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: setregs
 *		
 *   ORIGINS: 27,83
 *
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
 * NOTES:
 *	This is the machine-dependent part of exec[2].  The common,
 *	platform-independent code is in exec.c.
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/pseg.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/machine.h>
#include <execargs.h>
#include "exec.h"

extern   void disown_fp();

/*
 * NAME: setregs
 *
 * FUNCTION: Set user register context at top of stack.
 *
 *	The top_of_stack structure will be passed to execexit(),
 *	which uses it to load the user context before branching to
 *	the main entry point.
 *
 *      Also, if the process doing the exec() is running with 
 *      floating point traps enabled, this is revoked.
 *
 * EXECUTION ENVIRONMENT:
 *	Preemptable
 *	May page fault
 *
 * RETURNS:	0, if successful;
 *		errno value, if failure.
 */

int
setregs(register struct xargs *xp)
/* xp	 exec argument structure */
{
	register struct thread	*t;	/* current thread */
	register int		i;
	struct top_of_stack	tos;	/* local copy of user context */
	struct func_desc	fdes;	/* function descriptor */

	/* mustn't pass user programs uninitialized storage */
	bzero(&tos, sizeof(tos));

	for (i = 0; i < NGPRS; i++)
		tos.main_reg[i] = DEFAULT_GPR;


	xp->ucp = TopOfBaseStack;

	xp->ap = (caddr_t) ((int) xp->ucp - xp->nc
		 - XA_ARGSIZE(xp) - XA_ENVSIZE(xp));

	/* set up main() arguments: argc, argv, and envp */
	tos.main_reg[ARG1] = xp->na;
	tos.main_reg[ARG2] = (ulong) xp->ap;
	tos.main_reg[ARG3] = (ulong) (xp->ap + XA_ARGSIZE(xp));
	tos.environ = (char **) (xp->ap + XA_ARGSIZE(xp));

	/* round first stack frame to a 16-byte boundary. */
	tos.main_reg[STKP] = ((ulong)xp->ap - STKMIN - 15) & ~15;

	/*
	 * Set up toc reg and link reg for main entry point.
	 *
	 * NOTE: execexit() should be changed to pick up
	 *	the function pointer in user mode, avoiding
	 *	this copyin().
	 */
	if (copyin((caddr_t) xp->ep, &fdes, sizeof(fdes)))
		return(EFAULT);
	tos.main_reg[TOC] = (ulong) fdes.toc_ptr;

#ifdef	_POWER
	tos.lr = (ulong) fdes.entry_point;
#endif	_POWER

	/* copy user context to top of user stack area. */
	if (copyout(&tos, (caddr_t) xp->ucp, sizeof(struct top_of_stack)))
		return(EFAULT);		/* shouldn't happen, but... */


	/*
	 * The exec'd program must start with a clear floating point
	 * state (FPSCR and FPSCRX zero, traps disabled).  This is
	 * accomplished by setting the appropriate stuff in u.u_save.
	 * Calling disown_fp is necessary because if this thread 
	 * is now the FP owner, if not done it will continue to run with the
	 * current hardware FPSCR without reloading, which is wrong.
	 * Also, all FPR's are set to zero.
	 */
	t = curthread;
	disown_fp(t->t_tid);
	t->t_uthreadp->ut_save.fpinfo = 0;
	t->t_uthreadp->ut_save.fpscr = 0;
	t->t_uthreadp->ut_save.fpscrx = 0;
	t->t_uthreadp->ut_save.fpeu = 0;
	bzero(t->t_uthreadp->ut_save.fpr, sizeof(t->t_uthreadp->ut_save.fpr));

	return(0);			/* success */
}
