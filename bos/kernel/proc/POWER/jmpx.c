static char sccsid[] = "@(#)47	1.19  src/bos/kernel/proc/POWER/jmpx.c, sysproc, bos411, 9428A410j 6/4/93 14:34:57";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: clrjmpx
 *		longjmpx
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*#print(off)*/
#include <sys/types.h>
#include <sys/mstsave.h>
#include <sys/low.h>
#include <sys/syspest.h>
#include <sys/intr.h>		/* condition reg is machine-dependent */
#include <sys/machine.h>
#include <sys/lockl.h>
#include <sys/dbg_codes.h>
#include <sys/pseg.h>
#include <sys/reg.h>
#include <sys/except.h>
/*#print(on)*/

/* Halt codes used by halt_display() routine */
#define HALT_DSI	0x30000000
#define HALT_ISI	0x40000000
#define HALT_EXI        0x50000000
#define HALT_PGM        0x70000000
#define HALT_FLP        0x80000000

/* 
 *  Name:      longjmpx -- kernel exception long jump
 *
 *  Function:
 *         -- longjmp() to buffer on top of stack anchored in u-block
 *
 */

int longjmpx(int rc)
{
	label_t *save_buf;
        int code; 		/* code to pass to debugger */
	int halt_code; 		/* code to display on LEDS  */
	/* 
	 * if no exception process or jump buffer
	 * trap the machine
	 */
        if(csa->kjmpbuf == NULL)
     	{
                i_disable(INTMAX);  

                j_errlog(rc); /* Do the error logging stuff */

		/* Restore IAR, TOC, and GPR3 that was trashed in  */
		/* exception(), if we got here that way.           */
		if(csa->iar == (ulong_t)(*(int *)longjmpx))
		{
			csa->iar = csa->o_iar;
			csa->gpr[TOC] = csa->o_toc;
			csa->gpr[ARG1] = csa->o_arg1;
                }

		/* Set up the halt and debugger codes for halt_display() */
		switch(rc) {
			case EXCEPT_FLOAT:	/* floating point exception */
				code = DBG_FPEN;
				halt_code = HALT_PGM;
				break;
			case EXCEPT_INV_OP:	/* invalid op-code */
				code = DBG_INVAL;
				halt_code = HALT_PGM;
				break;
			case EXCEPT_PRIV_OP:	/* priveleged op */
				code = DBG_PRIV;
				halt_code = HALT_PGM;
				break;
			case EXCEPT_IO_IOCC:
				code = DBG_DSI_IOCC;
				halt_code = HALT_DSI;
				break;
			case EXCEPT_IO_SLA:
				code = DBG_DSI_SLA;
				halt_code = HALT_DSI;
				break;
			case EXCEPT_IO_SCU:
				code = DBG_DSI_SCU;
				halt_code = HALT_DSI;
				break;
			default:
				code = DBG_UNK_PR;
				halt_code = 0;
		}

		/* Do a system-halt. Display the appropriate error */
                /* code in the led window */
		fst_halt_display(csa, halt_code, code);
	}
	
#ifdef _POWER
	/* we used to hold kernel-mode lock: */
	if ((csa->prev == NULL) && (csa->kjmpbuf->cr & CR_EQ))
	    lockl(&kernel_lock,LOCK_SHORT);
#endif _POWER

	/* enable to level when setjmpx called */
	if(csa->kjmpbuf->intpri >= csa->intpri)
		i_enable(csa->kjmpbuf->intpri);
	else
		i_disable(csa->kjmpbuf->intpri);
	
	save_buf = csa->kjmpbuf;	/* save current buffer address */
	clrjmpx(save_buf);		/* pop the buffer */
	
	longjmp(save_buf, rc);		/* go for it */
}
