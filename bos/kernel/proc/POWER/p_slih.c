static char sccsid[] = "@(#)84	1.48.2.13  src/bos/kernel/proc/POWER/p_slih.c, sysproc, bos41B, 9506A 1/27/95 17:22:13";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: default_uexcp
 *		j_errlog
 *		p_slih
 *		
 *   ORIGINS: 27,83
 *
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


/*
 * Function: This is the Program Second Level Interrupt Handler, which is
 *           called from the Program FLIH (entry address 0x700) when a
 *           Program Interrupt occurs.
 *
 *           p_slih() determines what type of Program Interrupt occurred,
 *           and determines what action is necessary to handler the exception
 *
 * Note: This code resides in pinned memory and executes with
 *       interrupts disabled.
 */

#include <sys/types.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/mstsave.h>
#include <sys/machine.h>
#include <sys/signal.h>
#include <sys/low.h>
#include <sys/dbg_codes.h>
#include <sys/lldebug.h>
#include <sys/adspace.h>
#include <sys/except.h>
#include <sys/intr.h>
#include <sys/reg.h>
#include <sys/errno.h>
#include <sys/seg.h>
#include <sys/trchkid.h>
#include <sys/errids.h>
#include <sys/syspest.h>
#include <sys/systemcfg.h>

extern	struct uexcepth *uexcept_anchor;

/* the lower order bit must be set to enter the debugger on exception */
#define ENTER_DBG	1	
int enter_dbg = 0;

struct progerr {
	struct err_rec0 sproc;
	ulong sreg;
	ulong srr0;
	ulong srr1;
	ulong msr;
	} prog_log = {ERRID_PROGRAM_INT,"SYSPROC",0,0,0};

struct ioexcept {
	struct err_rec0 ioe;
	ulong data1;
	ulong data2;
	ulong data3;
	ulong data4;
	} ioe_log = {0,"SYSPROC",0,0,0,0};

struct sigerr {
	struct err_rec0 sproc;
	ulong sreg;
	ulong addr;
	ulong msr;
	} sig_log = {ERRID_PSIGDELIVERY,"SYSPROC",0,0,0};

/*
 * Contents of instructions used as kernel debugger breakpoints:
 */
#define BREAK_TRAP		0x7C800008
#define STATIC_BREAK_TRAP	0x7C810808
#define HALT_DSI		0x30000000
#define HALT_ISI		0x40000000
#define HALT_EXI		0x50000000
#define HALT_PGM		0x70000000
#define HALT_FLP		0x80000000

#ifdef _POWER_MP /* lldb & SMP */
extern int db_is_to_call();
#endif /* POWER_MP */
extern int db_active();


/*
 * p_slih()
 *
 * Parameters on entry:
 *      struct mstsave *mst;   Address of interrupted mst, which contains
 *                               the state of the interrupted program.
 *      ulong          addr;   Contents of addr at time of interrupt.
 *      ulong          except;   Contents of except at time of interrupt.
 *
 * Returns:
 *      Nothing.
 *
 * this program is called when a program-interrupt occurs (see low.s)
 * it is also called for synchronous io_exceptions (see below).
 * and it is also called for data and instruction storage interrupts
 * that are exceptions which are not fatal (i.e. kernel panics.. see
 * v_exception.c ).
 *
 * It is callable from the process and interrupt environments.
 *
 */

void
p_slih(struct mstsave *mst, ulong addr, ulong except, struct thread *threadp)
{
	int   code;			/* Code to pass to debugger           */
	ulong instr;			/* Failing instruction                */
	ulong srval;			/* Value in segment register 	      */
	int   intlev, rc;
	ulong *p;			/* Computed address of failing instr  */
	struct uexcepth *excp;
	int   halt_code=HALT_PGM;	/* error code to pass system halt pgm */
	int   exceptok;			/* 1 if exception can be handled by   */
				 	/* kernel exception handlers	      */
	struct mstsave *mycsa=csa;	/* current save area                  */

#ifdef _POWER_MP
	/* Enable lru, if the process was trying to deliver a signal. */
	if (threadp->t_atomic & TSIGDELIVERY)
		PPDA->lru = 0;
#endif

	TRCHKGT(HKWD_KERN_P_SLIH, addr, except, NULL, NULL, NULL);

	/*
 	 * Process exception type:  need to determine what the exception was
 	 * and what codes/signals to generate. the default case handles a
 	 * variety of data and instruction storage exceptions which should
 	 * be handled (i.e either user-mode or not a kernel panic).
 	 */

	/* get the segment register value of the faulting address */
	srval = as_getsrval(&mst->as, addr);
	exceptok = 0;
	mst->excp_type = except;
	switch(except)
	{
		case EXCEPT_FLOAT:		/* Floating point exception */
			code = DBG_FPEN;
			mst->fpinfo |= 
				FP_INT_TYPE;	/* set type to 1 (precise) */
			break;
		case EXCEPT_FLOAT_IMPRECISE:	/* Imprecise Flt point except */
			code = DBG_FPEN;
			mst->fpinfo &= 
				~FP_INT_TYPE; 	/* set type to 0 (imprecise) */
			break;
		case EXCEPT_INV_OP:		/* Invalid op-code */
			code = DBG_INVAL;
			break;
		case EXCEPT_PRIV_OP:		/* Priveleged operation */
			code = DBG_PRIV;
			break;
		case EXCEPT_TRAP:		/* A trap instruction */
                        srval = SRVAL(SRTOSID(srval),0,0);
			p = (ulong*) vm_att(	/* Load a sr for our access */
			    srval,		
			    addr);		
#ifdef _POWER_MP
			mycsa->backt = 1;	/* page might be stolen */
#endif 
			instr = *p;		/* Fetch the instr; note that */
						/* addr should be valid, else */
						/* interupt would be for bad */
						/* addr, not trap instruction */
#ifdef _POWER_MP
			mycsa->backt = 0;	/* clear backtracking */
#endif 
			vm_det(p);		/* Free the segment register */
	
			code = DBG_TRAP;
#ifdef _POWER_MP
        		if (instr == STATIC_BREAK_TRAP)
            		{
            			/* Invoke kernel debugger with failing mst */
            			if (debugger(mst,DBG_STATIC_TRAP) != NODEBUGRET)
                		{
                			return;
                		}
             		}
             		else 
			if ((instr == BREAK_TRAP) || (db_is_to_call(addr, srval)))
#else /* _POWER_MP */
        		if ((instr == BREAK_TRAP) ||
		            (instr == STATIC_BREAK_TRAP))
#endif /* _POWER_MP */
                        {
        			/* Invoke kernel debugger with failing mst */
        			if (debugger(mst,DBG_TRAP) != NODEBUGRET)
                		{
                			return;
                		}
        		}
        		code = DBG_HTRAP;
        		break;
		case EXCEPT_IO:
#ifdef _POWER_601
			if ( __power_601() && (mst->msr & MSR_PR) )
				(void)undo_601_dse( mst, addr, threadp );
#endif /* _POWER_601 */

			code = DBG_DSI_IOCC;
			halt_code = HALT_DSI | (srval & 0x0FF00000);
			exceptok  = 1;
			break;
		case EXCEPT_IO_SLA:
			code = DBG_DSI_SLA;
			halt_code = HALT_DSI | (srval & 0x0FF00000);
			exceptok  = 1;
			break;
		case EXCEPT_IO_SCU:
			code = DBG_DSI_SCU;
			halt_code = HALT_DSI;
			exceptok  = 1;
			break;
		case EXCEPT_IO_SGA:
			code = DBG_DSI_SGA;
			halt_code = HALT_DSI | (srval & 0x0FF00000);
			exceptok = 1;
			break;
		default:
			code = DBG_UNK_PR;
			exceptok  = 1;
	}

	/* The debugger is only entered, if the appropriate bit is set */ 
	if (enter_dbg & ENTER_DBG)
	{
		/* 
		 * To aid in debugging call debugger first.
		 * go from debugger will be normal subroutine return.
		 */
		intlev = i_disable(INTMAX);
		debugger(mst,code,except);
		i_enable(intlev);
	}


	/* 
	 * If the exception occurred while delivering a signal to a
	 * thread, call sig_setup() with a NULL signal context which
	 * means call core and exit.
	 */
	if (threadp->t_atomic & TSIGDELIVERY)
	{
		sig_log.sreg = srval;
		sig_log.addr = addr;
		sig_log.msr = mst->msr;
		errsave(&sig_log, sizeof(sig_log));

		sig_setup(0, threadp);

		/* 
		 * This field is updated exclusively with atomic primitives
		 * except in this case, because the thread is in user mode.
		 */
		threadp->t_atomic &= ~TSIGDELIVERY;
		return;
	}

	/* 
	 * If user mode call exception handlers. the default 
	 * handler signals the thread and returns handled.
	 * so we should never get to the assert(0).
	 */
	if ((mst->prev == NULL) && (mst->msr & MSR_PR))
	{
		for(excp = uexcept_anchor; excp  ; excp = excp->next)
		{
			rc = (excp->handler)(excp, 
			      except, 
			      threadp->t_tid, mst);
			if (EXCEPT_HANDLED == rc)
				return;
		}
		assert(0);
	}

	/* Note: The following code calls the exception handler defined
	 * in the current mst.  However, we may get here because of a
	 * problem (e.g. DSI) that occurred in the kernel debugger.  This
	 * should always be a fatal error - the debugger does not use any
	 * exception handlers.  However, if the debugger was entered via
	 * a console key sequence (e.g. CTL_ALT_NUM4), we are using whatever
	 * happened to be the current mst, which may have had a handler
	 * defined for some totally different purpose.  Therefore, we call
	 * db_active to determine whether the debugger was running on this
	 * processor when this exception occurred.
	 */

	/* Kernel mode. If its ok to take the exception and there
	 * is an exeception handler let it have a go at it.
	 */
	if (exceptok && (mst->kjmpbuf != NULL || mst->excbranch != 0) && !db_active())
	{
		exception(mst, except);
		return;
	}

	/*  exception is fatal (no handler or exceptiok is false).  */
	switch(except)
	{
	case EXCEPT_FLOAT:
		prog_log.sreg = srval;
		prog_log.srr0 = addr;
		prog_log.srr1 = SRR_PR_FPEN;
		prog_log.msr  = mst->msr;
		errsave(&prog_log,sizeof(prog_log));
		break;
	case EXCEPT_FLOAT_IMPRECISE:
		/* We will turn on the Imprecise bit (SRR_PR_IMPRE)
		 * even on Rios-II even though the bit doesn't actually
		 * exist in the hardware.  This allows the user to see
		 * in the error log that it was actually a imprecise
		 * FP exception.
		 */
		prog_log.sreg = srval;
		prog_log.srr0 = addr;
		prog_log.srr1 = SRR_PR_FPEN | SRR_PR_IMPRE;
		prog_log.msr  = mst->msr;
		errsave(&prog_log,sizeof(prog_log));
		break;
	case EXCEPT_INV_OP:
		prog_log.sreg = srval;
		prog_log.srr0 = addr;
		prog_log.srr1 = 0x00080000;
		prog_log.msr  = mst->msr;
		errsave(&prog_log,sizeof(prog_log));
		break;
	case EXCEPT_PRIV_OP:
		prog_log.sreg = srval;
		prog_log.srr0 = addr;
		prog_log.srr1 = 0x00040000;
		prog_log.msr  = mst->msr;
		errsave(&prog_log,sizeof(prog_log));
		break;
	case EXCEPT_TRAP:
		prog_log.sreg = srval;
		prog_log.srr0 = addr;
		prog_log.srr1 = 0x00020000;
		prog_log.msr  = mst->msr;
		errsave(&prog_log,sizeof(prog_log));
		break;
	case EXCEPT_IO_SCU:
		/* io exception error log */
		ioe_log.ioe.error_id = ERRID_DSI_SCU;
		ioe_log.data1 = mst->except[1];
		ioe_log.data2 = mst->except[3];
		ioe_log.data3 = mst->except[4];
		ioe_log.data4 = srval;
		errsave(&ioe_log,sizeof(ioe_log));
		break;
	case EXCEPT_IO:
		/* iocc io exception error log */
		ioe_log.ioe.error_id = ERRID_DSI_IOCC;
		ioe_log.data1 = mst->except[1];
		ioe_log.data2 = mst->except[3];
		ioe_log.data3 = srval;
		ioe_log.data4 = mst->except[0];
		errsave(&ioe_log,sizeof(ioe_log));
		break;
	default:
		break;
	}

	halt_display(mst,halt_code,code);
	return;
}	

default_uexcp(struct uexcepth *exp, int except, tid_t tid, struct mstsave *mst)
{
	int signo;
	struct thread *threadp;

	/*
 	 * Process exception type:  need to determine what the exception was
 	 * and what signal to generate
 	 */
	threadp = THREADPTR(tid);

	switch(except)
	{
		case EXCEPT_FLOAT:	/* Floating point exception */
		case EXCEPT_FLOAT_IMPRECISE:
			signo = SIGFPE;
			break;
		case EXCEPT_INV_OP:	/* Invalid op-code */
		case EXCEPT_IFETCH_IO:	/* or instruc fetch exceptions*/
		case EXCEPT_IFETCH_SPEC:
		case EXCEPT_ISI:
			signo = SIGILL;
			break;
		case EXCEPT_PRIV_OP:	/* Priveleged operation */
			signo = SIGILL;
			break;
		case EXCEPT_TRAP:	/* A trap instruction */
			signo = SIGTRAP;
			break;
		case EFAULT:		/* Invalid address access */
		case EXCEPT_ALIGN:	/* such as alignment problems */
		case EXCEPT_INV_ADDR:	/* or invalid address */
		case EXCEPT_PROT:	/* or protection exceptions */
		case EXCEPT_DATA_LOCK:
		case EXCEPT_FP_IO:
		case EXCEPT_MIXED_SEGS:
		case EXCEPT_PFTLOOP:
		case EXCEPT_DSI:
			signo = SIGSEGV;
			break;
		case EIO:		/* IO exceptions */
		case EXCEPT_IO:
		case EXCEPT_IO_SCU:
		case EXCEPT_IO_SLA:
		case EXCEPT_IO_SGA:
		case EXCEPT_EOF:	/* mmap() ref beyond end-of-file */
		case ENOSPC:		/* no space in file system */
		case EDQUOT:		/* file system quota exceeded */
		case ESTALE:		/* file system unmounted */
		case EXCEPT_GRAPHICS_SID:
			signo = SIGBUS;
			break;
		case EFBIG:		/* ulimit exceeded */
			signo = SIGXFSZ;
			break;
		default:
	    		signo = SIGKILL;
	}

	/* Signal the thread */
	tidsig(threadp->t_tid, signo);

	return(EXCEPT_HANDLED);
}

/*
 * NAME: j_errlog  -- longjmp exception  error logging 
 *
 * FUNCTION: 
 *       -- generates an errsave template for the error
 *	 -- calls errsave
 *
 * NOTES:
 *     
 *	
 * RETURNS:
 *       Does not return a meaningful value      
 *
 */

int
j_errlog(int rc)
{
	struct mstsave *mycsa=csa;

        
	switch(rc) {
		case EXCEPT_FLOAT:
		case EXCEPT_FLOAT_IMPRECISE:
			prog_log.sreg = mycsa->except[2];
			prog_log.srr0 = mycsa->except[1];
			prog_log.srr1 = 0x00100000;
			prog_log.msr  = mycsa->msr;
			errsave(&prog_log,sizeof(prog_log));
			break;
		case EXCEPT_INV_OP:
			prog_log.sreg = mycsa->except[2];
			prog_log.srr0 = mycsa->except[1];
			prog_log.srr1 = 0x00080000;
			prog_log.msr  = mycsa->msr;
			errsave(&prog_log,sizeof(prog_log));
			break;
		case EXCEPT_PRIV_OP:
			prog_log.sreg = mycsa->except[2];
			prog_log.srr0 = mycsa->except[1];
			prog_log.srr1 = 0x00040000;
			prog_log.msr  = mycsa->msr;
			errsave(&prog_log,sizeof(prog_log));
			break;
		case EXCEPT_TRAP:
			prog_log.sreg = mycsa->except[2];
			prog_log.srr0 = mycsa->except[1];
			prog_log.srr1 = 0x00020000;
			prog_log.msr  = mycsa->msr;
			errsave(&prog_log,sizeof(prog_log));
			break;
		case EXCEPT_PROT:
		case EXCEPT_ALIGN:
		case EXCEPT_IO_SCU:
		case EXCEPT_INV_ADDR:
			/* io exception error log */
			ioe_log.data1 = mycsa->except[1];
			ioe_log.data2 = mycsa->except[3];
			ioe_log.data3 = mycsa->except[4];
			ioe_log.data4 = 0;
			errsave(&ioe_log,sizeof(ioe_log));
			break;
		case EXCEPT_IO:
			/* iocc io exception error log */
			ioe_log.ioe.error_id = ERRID_DSI_IOCC;
			ioe_log.data1 = mycsa->except[1];
			ioe_log.data2 = mycsa->except[3];
			ioe_log.data3 = 0;
			ioe_log.data4 = mycsa->except[0];
			/* Need to build the error code w/ the BUID */
			errsave(&ioe_log,sizeof(ioe_log));
			break;
		}

	return;

}
