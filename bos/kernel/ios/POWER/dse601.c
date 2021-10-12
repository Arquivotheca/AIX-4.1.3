static char sccsid[] = "@(#)46	1.4  src/bos/kernel/ios/POWER/dse601.c, sysios, bos411, 9428A410j 12/7/93 15:50:45";
/*
 *   COMPONENT_NAME: SYSIOS
 *
 *   FUNCTIONS: backout_safe       (private)
 *              undo_601_dse       (exported)
 *   DATA:      dse_601_total      (exported) - unsigned long integer
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifdef _POWER_601

/*
 * This file contains the logic to back out 601 Direct Store Errors (DSEs),
 * which are an asynchronous exception and, thus, destroy potentially important
 * registers before it is known that the instruction will cause an exception.
 * All DSEs are considered Data Storage Interrupts (DSIs), and only occur on
 * instructions which access memory.
 *
 * The primary mechanism for recovering destroyed registers is algebraic
 * manipulation of the Data Address Register (DAR).  This, in combination
 * with unaltered registers, allows all instructions to be restarted.
 */

#include <sys/types.h>
#include <sys/syspest.h>
#include <sys/mstsave.h>
#include <sys/adspace.h>
#include "dse601.h"

/*
 * The following table lists the Primary and Secondary OpCodes which will be
 * handled in the backout procedure.  Primary OpCodes are listed sequentially,
 * Secondary sequentially by functional group.
 *
 * Special Cases:
 *
 * Although this table lists sixteen (16) floating point Load/Store operations,
 * these OpCodes are just passed through with no register modification.  They
 * are only received if the Alignment Handler gets a DSE while repairing the
 * floating move.  (The mst points to the FP op, not the Alignment Handler op)
 *
 * Load and Store Multiple Word instructions pass through unmodified as well.
 * These instructions do not update their addressing component register, if
 * specified, until the entire instruction completes.
 *
 * Load and Store String Indexed/Immediate instructions also require no backout.
 * Like the multi-word instructions, a target register overlapping an addressing
 * component will not be updated until the instruction completes.
 *
 ******************************************************************************
 *
 * Primary Opcode	Secondary Opcode	Type	Instruction
 *
 * 31.	1F   01 1111	
 *			 23.  017  00 0001 0111	X	lwzx	rT, rA, rB
 *			 55.  037  00 0011 0111	X	lwzux	rT, rA, rB
 *			 87.  057  00 0101 0111	X	lbzx	rT, rA, rB
 *			119.  077  00 0111 0111	X	lbzux	rT, rA, rB
 *			151.  097  00 1001 0111	X	stwx	rS, rA, rB
 *			183.  0B7  00 1011 0111	X	stwux	rS, rA, rB
 *			215.  0D7  00 1101 0111	X	stbx	rS, rA, rB
 *			247.  0F7  00 1111 0111	X	stbux	rS, rA, rB
 *			279.  117  01 0001 0111	X	lhzx	rT, rA, rB
 *			311.  137  01 0011 0111	X	lhzux	rT, rA, rB
 *			343.  157  01 0101 0111	X	lhax	rT, rA, rB
 *			375.  177  01 0111 0111	X	lhaux	rT, rA, rB
 *			407.  197  01 1001 0111	X	sthx	rS, rA, rB
 *			439.  1B7  01 1011 0111	X	sthux	rS, rA, rB
 *
 *			534.  216  10 0001 0110	X	lwbrx	rT, rA, rB
 *			662.  296  10 1001 0110	X	stwbrx	rT, rA, rB
 *			790.  316  11 0001 0110	X	lhbrx	rT, rA, rB
 *			918.  396  11 1001 0110	X	sthbrx	rT, rA, rB
 *
 *			533.  215  10 0001 0101 X	lswx	rT, rA, rB
 *			597.  255  10 0101 0101 X	lswi	rT, rA, NB
 *			661.  295  10 1001 0101 X	stswx	rS, rA, rB
 *			725.  2D5  10 1101 0101 X	stswi	rS, rA, NB
 *
 *			535.  217  10 0001 0111 X	lfsx	frT, rA, rB
 *			567.  237  10 0011 0111 X	lfsux	frT, rA, rB
 *			599.  257  10 0101 0111 X	lfdx	frT, rA, rB
 *			631.  277  10 0111 0111 X	lfdux	frT, rA, rB
 *			663.  297  10 1001 0111 X	stfsx	frS, rA, rB
 *			695.  2B7  10 1011 0111 X	stfsux	frS, rA, rB
 *			727.  2D7  10 1101 0111 X	stfdx	frS, rA, rB
 *			759.  2F7  10 1111 0111 X	stfdux	frS, rA, rB
 *
 * 32.  20   10 0000				D	lwz	rT, D(rA)
 * 33.  21   10 0001				D	lwzu	rT, D(rA)
 * 34.  22   10 0010				D	lbz	rT, D(rA)
 * 35.  23   10 0011				D	lbzu	rT, D(rA)
 * 36.  24   10 0100				D	stw	rS, D(rA)
 * 37.  25   10 0101				D	stwu	rS, D(rA)
 * 38.  26   10 0110				D	stb	rS, D(rA)
 * 39.  27   10 0111				D	stbu	rS, D(rA)
 * 40.  28   10 1000				D	lhz	rT, D(rA)
 * 41.  29   10 1001				D	lhzu	rT, D(rA)
 * 42.  2A   10 1010				D	lha	rT, D(rA)
 * 43.  2B   10 1011				D	lhau	rT, D(rA)
 * 44.  2C   10 1100				D	sth	rS, D(rA)
 * 45.  2D   10 1101				D	sthu	rS, D(rA)
 * 46.	2E   10 1110				D	lmw	(not handled)
 * 47.	2F   10 1111				D	stmw	(not handled)
 *
 * 48.	30   11 0000				D	lfs	frT, D(rA)
 * 49.	31   11 0001				D	lfsu	frT, D(rA)
 * 50.	32   11 0010				D	lfd	frT, D(rA)
 * 51.	33   11 0011				D	lfdu	frT, D(rA)
 * 52.	34   11 0100				D	stfs	frS, D(rA)
 * 53.	35   11 0101				D	stfsu	frS, D(rA)
 * 54.	36   11 0110				D	stfd	frS, D(rA)
 * 55.	37   11 0111				D	stfdu	frS, D(rA)
 */

/*
 * The exported dse_601 statistics.  From this file, run at a priority of at
 * least INTPAGER, they are not volatile.
 */

unsigned long		dse_601_total  = 0;

/******************************************************************************/
/*
 * NAME: backout_safe
 *
 * FUNCTION: Decode the passed instruction, determine if it requires
 *	its General Purpose Registers to be manipulated to their pre-
 *	execution state.  All backouts are accurate, since the DAR is.
 *	This backout service runs on DD2.1 and later 601 chips, which
 *	is checked at runtime.
 *
 * EXECUTION ENVIRONMENT: Runs during exception handling, called by
 *	undo_601_dse() below only on the 601 chip.  A full context save
 *	has been performed, available in the passed MST.
 *
 * NOTES: This code runs, so far, only on the __power_601.
 *	There is only one type of instruction which will have different
 *	(albeit equivalent) registers when restarted versus first run.
 *
 *      In MP environment, we can not assert as is done for UP
 *      because a different segment may be mapped into the thread's address
 *      space and occupy the same effective segment.
 *      This may occur if another thread of the process performs as_det()
 *      and as_att() after this thread page faulted in fetching the
 *      instruction in undo_601_dse().
 *
 * RETURNS: Nothing.
 *
 * EXTERNAL FUNCTIONS: None.
 */

void	  backout_safe( struct mstsave	*p_mst,
			unsigned long	 DAR,
			t_power_i	 inst )
{
unsigned long			 rA;		/* Base Address Register  */

	rA = inst.d_inst.rA;

	/* Catch out-of-range, low, Primary OpCodes */
#ifndef _POWER_MP
	assert( inst.d_inst.op >= LDST_XF );
#else
        if (inst.d_inst.op < LDST_XF)	return;
#endif

	if ( inst.x_inst.op == LDST_XF )
	{
		unsigned long		rB;

		/*
		 * Parse the Secondary OpCode into the four allowed classes
		 * and 'others', which assert.
		 */

		if ( ( ( inst.x_inst.xop & XF_NORMAL_MSK )   != XF_NORMAL ) &&
		     ( ( inst.x_inst.xop & XF_REVERSED_MSK ) != XF_REVERSED ) )
		{
		    if ( ( ( inst.x_inst.xop & XF_FLOAT_MSK )  == XF_FLOAT ) ||
		         ( ( inst.x_inst.xop & XF_STRING_MSK ) == XF_STRING ) )
			    return;
		    else
#ifndef _POWER_MP
		    	{ assert(0); }
#else
		        { return; }
#endif
	    	}

		rB = inst.x_inst.rB;

		/* This has to be handled before rA == rB */

		if ( rA == 0 )
			p_mst->gpr[rB] = DAR;

		else if ( rA == rB )
			p_mst->gpr[rA] = DAR >> 1;

		else if ( rB != inst.x_inst.rT )
			p_mst->gpr[rA] = DAR - p_mst->gpr[rB];

		else if ( ( inst.x_inst.xop & LDST_XF_UPD ) == 0 )
			p_mst->gpr[rB] = DAR - p_mst->gpr[rA];

		else if ( ( inst.x_inst.xop & STORE_XF ) == 0 )
			p_mst->gpr[rB] = 0;

		else
			p_mst->gpr[rA] = DAR - p_mst->gpr[rB];

		return;		/* X-Form */
	}

	else if ( inst.d_inst.op <= DF_MAX_FX )
	{
		long			displacement;

		/*
		 * Again, rather than make complicated and time consuming
		 * decisions about whether there was a register killed, we
		 * just set rA unless it was not an address component (r0).
		 */

		if ( rA != 0 )
		{
			displacement = (long)( (signed short)inst.d_inst.off );
			p_mst->gpr[rA] = DAR - displacement;
		}

		return;		/* D-Form */
	}

	/*
	 * If you skip directly from D-Form Fixed Point L/S operations to the
	 * floating ones, you inadvertently catch 'STMw' and 'LMw' also.
	 * This is desired behavior because they, like all Floating Point
	 * memory operations, require no backout.
	 */

	else if ( inst.d_inst.op <= DF_MAX_FP )
		return;

	/*
	 * Any instruction arriving here was neither a D-Form or X-Form
	 * Load or Store instruction.
	 */

#ifndef _POWER_MP
	else
		{ assert(0); }			/* Does not return */
#endif
	/* Each if/else returns separately. */
}

/******************************************************************************/
/*
 * NAME: undo_601_dse
 *
 * FUNCTION: Makes a DSE excepting instruction addressable, fetches it,
 *	releases addressability, and calls backout_dse.  A global count
 *	of DSEs is kept for debugging purposes.
 *
 * EXECUTION ENVIRONMENT: Runs during exception handling, called from p_slih
 *	for EXCEPT_IO errors on the 601 chip only.  A full context save has
 *	already taken place and the passed MST contains all GP and Segment
 *	registers.
 *
 * NOTES: This code runs, so far, only on the __power_601.
 *	In the UP environment, the Process/Thread pointer is not used.
 *	Under MP, this function becomes extremely complicated because of the
 *	potential that the excepting instruction was paged out.
 *
 * EXTERNAL FUNCTIONS:
 *	as_geth()
 *	vm_att()
 *	vm_det()
 *	as_puth()
 */

void	undo_601_dse( struct mstsave	*p_mst,
		      unsigned long	 DAR,
		      struct thread	*p_thread )
{
unsigned long		 eaddr;
unsigned long		 srval;
unsigned long		*p_inst;	/* Ptr, Excepting instruction */
t_power_i		 instr;		/* Actual excepting instruction */
int			 rc_backout;
struct uthread          *uthreadp;
struct mstsave          *csap;

	eaddr = p_mst->iar;			/* Addr, excepting instr */
	srval = p_mst->as.srval[eaddr>>SEGSHIFT];
	p_inst = (unsigned long *)vm_att( srval, eaddr );

	/*
	 * On UP systems, the PAGER cannot have run since we took this
	 * exception, thus the excepting instruction may be fetched
	 * directly from the IAR without regard for page faulting during
	 * that operation.
	 */

#ifndef _POWER_MP
	instr.opcode = *p_inst;
#else
	/* On MP, the excepting instruction may be long gone. */

	uthreadp = p_thread->t_uaddress.uthreadp;

        /* save iar. msr, DAR, and except[]				     
	 */  								     
	uthreadp->ut_scsave[0] = p_mst->iar;                                 
	uthreadp->ut_scsave[1] = p_mst->msr;				     
	uthreadp->ut_scsave[2] = p_mst->except[0];                           
	uthreadp->ut_scsave[3] = p_mst->except[1];                           
	uthreadp->ut_scsave[4] = p_mst->except[2];                           
	uthreadp->ut_scsave[5] = DAR; /* except[3] */                        
                                                                             
	/* set up process level mstsave so it will backtrack to dse_backt()  
	 * in case of normal page fault                                      
	 */								     
	p_mst->iar = dse_backt;						     
	p_mst->msr = (p_mst->msr | MSR_IR | MSR_DR) & ~(MSR_EE | MSR_PR);    
	p_mst->as.srval[0] = KERNELSEGVAL;				     

        /* increment to signal suspend count to guarantee:
	 * (1) no more user mode instruction is executed before this DSE
	 *     interrupt is resolved; and 
	 * (2) resume() will restore segment registers' content from the
	 *     modified process level mstsave (instead of from u.u_adspace)
	 *     when resuming to dse_backt() or dse_ex_handler().
	 */       
        p_thread->t_suspend++;

        /* set up an exception handler
	 */  
        p_mst->excbranch = dse_ex_handler;

	csap = csa;
	/* may get a page fault and/or an exception.
	 * In case of a normal page fault, it will resume at dse_backt().
	 * In case an exception occurs, it will resume at dse_ex_handler().
         */ 
        csap->backt = 1;

        /* Do not need an isync here because DSI is precise.
	 * Do not need a sync here because state_save() will have a sync
	 * (eventually), which guarantees that the other processor will see
	 * the updates in case this thread is resumed at a different processor.
	 */  
        instr.opcode = *p_inst;  

	csap->backt = 0;

	p_mst->excbranch = 0;

	p_thread->t_suspend--;

	/* restore iar, msr, and sr0           				    
	 */								     
	p_mst->iar = uthreadp->ut_scsave[0];                                 
	p_mst->msr = uthreadp->ut_scsave[1];                                 
	p_mst->as.srval[0]  = U.U_adspace.srval[0];			     

#endif
        vm_det( p_inst );

	backout_safe( p_mst, DAR, instr );

	++dse_601_total;
}

#endif  /* _POWER_601 */
