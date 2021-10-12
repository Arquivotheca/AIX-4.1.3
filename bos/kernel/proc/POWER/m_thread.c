static char sccsid[] = "@(#)31	1.7  src/bos/kernel/proc/POWER/m_thread.c, sysproc, bos41J, 9517B_all 4/27/95 13:43:40";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: threadentry
 *		threadinit
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
#include <sys/machine.h>

extern void waitproc(void);
extern struct vmmdseg vmmdseg;
extern union ptaseg ptaseg;
extern threadentry();

/*
 * NAME: threadinit
 *
 * FUNCTION: Initialize kernel thread machine state.
 *
 * NOTES:
 *	This routine is platform specific.  It is called by procinit() and
 *	kthread_start() during kernel thread start up to initialize the
 *	machine state for this platform. It is also called by thread_setstate()
 *	during user thread startup.
 *
 *	Offset is used to access the intended kernel stack (init_stk) from the
 *	current address space. Init_stk is the address where the newly
 *	initialized thread will see its stack, however this thread may belong
 *	to another process and the segment about to contain the kernel stack
 *	may have been attached at a different address to the current address
 *	space. Offset is the offset between this temporary address and the
 *	intended address.
 *	
 * RETURNS:
 *	None.
 */

void
threadinit(ut, kstksr, pprivsr, init_func, init_data, length, init_stk, offset)
register struct uthread *ut;	/* uthread block to initialize	*/
register vmhandle_t kstksr;	/* kernel stack seg VM handle	*/
register vmhandle_t pprivsr;	/* process private seg VM handle*/
void (*init_func)(void *);	/* pointer to initial function	*/
register void	*init_data;	/* init data to copy in init_stk*/ 
register int	length;		/* length of init data		*/
register void	*init_stk;	/* kernel stack base		*/
register int	offset;		/* to reach init_stk from here	*/
{
	register int	i;		/* register index */
	register char	*stack_data;	/* address of data in stack */
	register char	*stack_top;

	struct func_desc *thread_desc = (struct func_desc *)threadentry;

	/*
	 * initialize the new kernel address space
	 */
	as_init(&ut->ut_save.as);	/* initialize new address space */
	as_ralloc(&ut->ut_save.as, KERNELORG);
	as_seth(&ut->ut_save.as, vm_setkey(vmker.kernsrval,VM_PRIV), KERNELORG);
	as_ralloc(&ut->ut_save.as, &U);
	as_seth(&ut->ut_save.as, vm_setkey(pprivsr, VM_PRIV), &U);
	as_ralloc(&ut->ut_save.as, KERNEXORG);
	as_seth(&ut->ut_save.as, vm_setkey(vmker.kexsrval, VM_PRIV), KERNEXORG);
	
	/*
	 * complete kernel address space
	 */
	as_ralloc(&ut->ut_save.as, TEXTORG);
	as_ralloc(&ut->ut_save.as, &vmmdseg);
	as_ralloc(&ut->ut_save.as, &ptaseg);
	as_ralloc(&ut->ut_save.as, SHTEXTORG);

	/*
	 * give access to the kernel stack segment if it exists (it's TEXTORG)!
	 */
	if (kstksr != NULLSEGVAL)
		as_seth(&ut->ut_save.as, vm_setkey(kstksr, VM_PRIV), KSTACKORG);

        /*
         * If this is a wait thread, then reset the kernel stack to the 
         * default kernel stack, of which the first 0x400 bytes is pinned.  
	 * The waitproc cannot page fault.  This leads to the following 
	 * restriction: the waitproc must operate in 0x400 bytes of stack 
	 * space.  The ublock is not page aligned.
         */
        if ((ulong)init_func == (ulong)waitproc)
		init_stk = &__ublock;
	
	/*
	 * Copy initialization data to stack.
	 * Note that the stack must be quadword aligned.
	 */
	ut->ut_error = 0;
	ut->ut_kstack = init_stk;
	stack_data = (char *)(((int)init_stk - length) & ~15);
	bcopy(init_data, stack_data + offset, length);

	/*
	 * Initialize GPRs 
	 */
	for (i = 0; i < NGPRS; i++)
		ut->ut_save.gpr[i] = DEFAULT_GPR;

#ifdef	_POWER				/* R2 System machine-dependency */
	/* 
	 * Initialize FPRs by clearing to binary zeros
	 */
	bzero(ut->ut_save.fpr, NFPRS*sizeof(double));
#endif	

	/*
 	 * Initialize parameters.  Stack is set to stack_data less STKMIN
	 * (stack_data was aligned on double word boundary above).
 	 */
#ifdef	_POWER				/* R2 System machine-dependency */
	ut->ut_save.gpr[3] = 0;
	ut->ut_save.gpr[4] = (ulong_t) stack_data;
	ut->ut_save.gpr[5] = (ulong_t) length;
	ut->ut_save.gpr[6] = (ulong_t) init_func;
	ut->ut_save.gpr[1] = ((ulong_t) stack_data - STKMIN);
	ut->ut_save.gpr[2] = (ulong_t) thread_desc->toc_ptr;
	ut->ut_save.iar    = (ulong_t) thread_desc->entry_point;
	ut->ut_save.msr    = DEFAULT_MSR;
	ut->ut_save.fpscr  = 0;
	ut->ut_save.fpscrx = 0;
	ut->ut_save.fpinfo = 0;
	ut->ut_save.cr     = 0;
	ut->ut_save.ctr    = 0;
	ut->ut_save.xer    = 0;
#endif

}

/*
 * FUNCTION: threadentry()
 *
 * This routine is the front end of all kernel threads.
 * If the kthread returns, call thread_terminate()
 */
threadentry(int param, void *data, int length, void (*func)(void *))
{
	/* Call the kthread entry point */
	(*func)(data);

	/* Cause the kernel thread to terminate */
	thread_terminate();
}
