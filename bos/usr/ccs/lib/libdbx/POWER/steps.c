static char sccsid[] = "@(#)95	1.13.1.9  src/bos/usr/ccs/lib/libdbx/POWER/steps.c, libdbx, bos411, 9428A410j 4/27/94 10:26:35";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: bits, branch_dest, dojump, nextaddr, pstep
 *
 * ORIGINS: 26, 27, 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/

#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
/*              include file for message texts          */
#include "dbx_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
/*
 * Machine-dependent routines involved with the process.
 *
 */

#include "defs.h"
#include "process.h"
#include "execute.h"
#include "ops.h"
#include "main.h"
#include "envdefs.h"
#include <errno.h>
#include <sys/reg.h>

#define bits(x,n,m) ((x >> (31-m)) & ((1 << (m-n+1)) - 1))

private Address safety_net;
#ifdef K_THREADS
Boolean handler_signal = false;
#endif /* K_THREADS */


/*
 * Single step as best ptrace can.
 */

#ifdef KDBX
#define PSTEP_PTRACE	PT_STEP
#else
#define PSTEP_PTRACE	CONT
#endif

public pstep (p, signo)
Process p;
int signo;
{
    int s, status;
#ifdef K_THREADS
    extern tid_running_thread;            /* use by ptrace(PTT_CONTINUE)    */
    struct ptthreads buf_ptthreads;    /* buffer for ptrace(PTT_CONTINUE) */
#endif /* K_THREADS */
#ifndef KDBX
    Address bkpt1, bkpt2;
    Word instr, instrx, opcode, opcodex;

    s = signo;
/*  We have no ptrace single-step mode in AIX, so we must place breakpoints
 *  around the current instruction, then remove them.  For branches, the 
 *  next instruction is a little harder to compute, so we call branch_dest
 *  to figure it out. 
 */
 /*
  * p->reg[pc] is always valid, since it is set in getinfo().
  */
    if(p->reg[SYSREGNO(PROGCTR)] < LOWADDR)
	error( catgets(scmc_catd, MS_steps, MSG_405,
		"cannot step through protected code at 0x%x\n"),
						    p->reg[SYSREGNO(PROGCTR)]);
    iread(&instr,p->reg[SYSREGNO(PROGCTR)],4);
    if(instr == BPT)
	error( catgets(scmc_catd, MS_steps, MSG_407,
		"cannot step through breakpoint at 0x%x\n"),
						    p->reg[SYSREGNO(PROGCTR)]);
    opcode = instr >> 26;
    bkpt1 = p->reg[SYSREGNO(PROGCTR)] + INSLEN(opcode);
    bkpt2 = branch_dest( p, opcode, instr, p->reg[SYSREGNO(PROGCTR)] );
    if (bkpt2 == 0)
	bkpt2 = (Address) -1;
    /* Set a safety net in case we branch into the kernel. */
    safety_net = bkpt1;
    setbp(bkpt1);
    if (bkpt2 != (Address) -1)
	setbp(bkpt2);
#endif
    do {
	setinfo(p, s);
	if (traceexec) {
	    (*rpt_output)( stdout, "!! pstep from 0x%x with signal %d (%d)\n",
		reg(SYSREGNO(PROGCTR)), s, p->signo);
	    fflush(stdout);
	}
	sigs_off();
	errno = 0;
#ifdef K_THREADS
        buf_ptthreads.th[0] = NULL;
        if (traceexec) {
             (*rpt_output)(stdout, "!! ptrace(%d,0x%x,%d,%d,%x)\n",
                 PTT_CONTINUE, tid_running_thread, 1, p->signo, &buf_ptthreads);
        }

        /* call by beginproc : call func() or by dojump : next, step..  */
        /* resume only the running_thread                               */
        if (ptrace(PTT_CONTINUE,tid_running_thread, 1,
            p->signo,&buf_ptthreads) < 0) {
#else
	if (ptrace(PSTEP_PTRACE, p->pid, 1, p->signo, 0) < 0) {
#endif /* K_THREADS */
		    panic( catgets(scmc_catd, MS_steps, MSG_409,
				   "error %d trying to step process") , errno);
	}
	pwait(p->pid, &status);
        action_mask |= EXECUTION;
        action_mask &= ~CONTEXT_CHANGE;
        action_mask &= ~LISTING;
        action_mask &= ~ELISTING;
	sigs_on();
	getinfo(p, status);

        /*  if the user has set "debug 3" (traceexec is true) and
              and we are going to ignore this signal  */
	if (traceexec && (p->status == STOPPED) && !istraced(p)
         && !(p->is_bp)) {
            /*  print debug message  */
	    (*rpt_output)( stdout, "!! pstep ignored signal %d at 0x%x\n",
		p->signo, reg(SYSREGNO(PROGCTR)));
	    fflush(stdout);
	}
	s = p->signo;

    /*  while we want to ignore the signal and keep going  */
    } while ((p->status == STOPPED) && !istraced(p) && !(p->is_bp));

#ifndef KDBX
    unsetbp(bkpt1);
    if (bkpt2 != (Address) -1)
	unsetbp(bkpt2);
#endif
    if (traceexec) {
	(*rpt_output)( stdout, "!! pstep to 0x%x on signal %d\n",
	    reg(SYSREGNO(PROGCTR)), p->signo);
	fflush(stdout);
    }
    if (p->status != STOPPED) {
	if (p->exitval == 0) {
	    error( catgets(scmc_catd, MS_steps, MSG_413,
							 "program exited\n") );
	} else {
	    error( catgets(scmc_catd, MS_steps, MSG_414,
				"program exited with code %d\n") , p->exitval);
	}
    }
}

/*
 * Calculate the destination of a branch/jump.  Return -1 if not a branch.
 */
branch_dest( p, opcode, instr, pc )
 Process p;
 int opcode, instr, pc;
{
	register long offset;
	unsigned dest;
	int immediate;
	boolean absolute;
	int ext_op;

	absolute = (boolean) ((instr >> 1) & 1);
	switch (opcode) {
	   case 18:  immediate = ((instr & ~3) << 6) >> 6;/* br unconditionl */
	   case	16:  if (opcode != 18)		          /* br conditional */
			immediate = ((instr & ~3) << 16) >> 16;
		     if (absolute) {
			dest = immediate;	
		     } else {
			dest = pc + immediate;
		     }
		     break;

	   case	19:  ext_op = (instr>>1) & 0x3ff;
		     if (ext_op == 16) {     /* Branch conditional register */
			dest = reg(SYSREGNO(LR)) & ~3;
		     } else if (ext_op == 528) { /* Brnch cond to count reg */
			dest = reg(SYSREGNO(CTR)) & ~3;
		     } else return -1; 
		     break;
	
	   default: return -1;
	}
        if (dest < LOWADDR) {
           /* if we are branching into the kernel, try  */
           /* return_addr for the actual return address */
           Address r_addr = return_addr();
           return (r_addr) ? r_addr : safety_net;
        } else {
           return dest;
        }
}

/*
 * Compute the next address that will be executed from the given one.
 * If "isnext" is true then consider a procedure call as straight line code.
 *
 * We must unfortunately do much of the same work that is necessary
 * to print instructions.  In addition we have to deal with branches.
 * Unconditional branches we just follow, for conditional branches
 * we continue execution to the current location and then single step
 * the machine.  We assume that the last argument in an instruction
 * that branches is the branch address (or relative offset).
 */

typedef List Bplist;
extern Bplist bplist;                  /* List of active breakpoints */
extern boolean catchbp;

public Address dojump (startaddr, isnext)
Address startaddr;
Boolean isnext;
{
    Address addr;
    register Breakpoint bp;
    Address orgin;

    if (catchbp and isnext) {
      orgin = reg(SYSREGNO(PROGCTR));
      foreach (Breakpoint, bp, bplist)
          if (bp->bpaddr != orgin)
              setbp(bp->bpaddr);
      endfor
    }
    stepto(startaddr);
    if (catchbp and isnext)
    {
        foreach (Breakpoint, bp, bplist)
            if (bp->bpaddr != orgin)
                unsetbp(bp->bpaddr);
        endfor
    }
    /* If current pc != startaddr, it means that we */
    /* stopped at one of the breakpoints set above. */
    /* And we don't want to step again!             */
    if (reg(SYSREGNO(PROGCTR))==startaddr)
       pstep(process, DEFSIG);
    pc = addr = reg(SYSREGNO(IAR));
    bpact();
    if (not isbperr()){
	printstatus();
    }
    return addr;
}

public Address nextaddr (startaddr, isnext)
Address startaddr;
Boolean isnext;
{
    Address addr;
    YOpCode inst;
    int info;
    int opcode, ext_op;
    Boolean absolute, link;

    addr = usignal(process);
#ifdef K_THREADS
    handler_signal = false;
#endif /* K_THREADS */

    if (addr != 0 && addr != 1 && !isnext) {
#ifdef K_THREADS
        handler_signal = true;
#endif /* K_THREADS */
	dread(&addr, addr, sizeof(addr));
    } else {
	iread(&inst, addr = startaddr, sizeof(inst));
	/* Check for branches, all types... */
	opcode = bits(inst,0,5);
	link = (boolean) bits(inst,31,31);
	switch(opcode) {
	   case 16: 		/* unconditional branch */
	   case 18: 		/* conditional branch */
		absolute = (boolean) bits(inst,30,30);
		if ((isnext && link) || absolute) {
			addr = startaddr + INST_SZ;
		} else {
			addr = dojump(startaddr, isnext);
		}
		break;
	   case 19: 		/* check for branch register */
		ext_op = bits(inst,21,30);
		if (((ext_op != 16) && (ext_op != 528)) || (isnext && link)) {
			addr = startaddr + INST_SZ;
		} else {
			addr = dojump(startaddr, isnext);
		}
		break;
	   default:
		addr = startaddr + INST_SZ;
		break;
	}
    }
    return addr;
}
