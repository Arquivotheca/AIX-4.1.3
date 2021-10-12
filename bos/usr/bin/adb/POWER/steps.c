static char sccsid[] = "@(#)27  1.13  src/bos/usr/bin/adb/POWER/steps.c, cmdadb, bos411, 9428A410j 6/19/91 18:24:38";
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTIONS: alt_loc, delbp1, setbp1, single
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*              include file for message texts          */
#include "adb_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

/*
 *  Service routines for subprocess control
 */

#include "defs.h"
#define bits(x,n,m) ((x >> (31-m)) & ((1 << (m-n+1)) - 1))

#ifndef _NO_PROTO
long alt_loc();
#else
long alt_loc(int, long, long);
#endif
extern void bpwait();
extern void readregs();

void delbp1(bkptr)
BKPTR bkptr;
{
    unsigned long a;

    if ( pid == 0 )
	return;
    a = bkptr->loc;
    errno = 0;
    if ((-1 == ptrace(WIUSER, pid, a, bkptr->ins)) && (errno != 0))
	perror("delbp1");
    errno = 0;
}

#define BPT 0x7D821008

void setbp1(bkptr)
register BKPTR bkptr;
{
    register long a, ins;
    a = bkptr->loc;
    errno = 0;
    bkptr->ins = ins = ptrace(RIUSER, pid, a, 0);
    if ((errno != 0) && (-1 == bkptr->ins)) {
        prints( catgets(scmc_catd, MS_steps, E_MSG_67,
                "can't read instruction: ") );
        psymoff(bkptr->loc, ISYM, "\n");
	bkptr->flag=0;
	return;
    }
    if ( (-1 == ptrace(WIUSER, pid, a, BPT )) && (errno != 0) ) {
	prints( catgets(scmc_catd, MS_steps, E_MSG_56, 
		"cannot set breakpoint: ") );
	psymoff(bkptr->loc, ISYM, "\n");
    }
}

void single(pc, execsig)
unsigned long pc;
{

/*
 * Length of instruction with opcode op. (Treat execute instruction
 * and subject as single atomic unit.)
 */
#define INSTRLEN(op)    (4)
#define INSLEN(op)      (4)

    /* AIWS has no trace mode so ptrace(9) is not implemented */
    /* This routine emulates it by setting breakpoints around */
    /* instruction                                            */

    BKPT b1,b2;
    unsigned long instr; char opcode;

retry:
    if (pc < 0x10000000) {
	errflg= catgets(scmc_catd, MS_steps, M_MSG_59, 
		"Can't single step through protected code, use continue") ;
	chkerr();               /* never returns */
    }
    errno = 0;
    instr = ptrace(RIUSER, pid, pc, 0);
    if (errno != 0) {
	adbpr("(at pc=%R) ", pc);
	error( catgets(scmc_catd, MS_steps, E_MSG_67, 
			 "can't read instruction") );
    }
    if (instr == BPT) {
	 error( catgets(scmc_catd, MS_steps, E_MSG_68, 
		"cannot step through breakpoint instruction (tgte r2,r2)") );
    }
    opcode = bits(instr,0,5);
    b1.loc = pc+INSTRLEN(opcode);
    b2.loc = alt_loc(opcode, instr, pc);

    setbp1(&b1); 
    if ( b2.loc != -1 ) 
	setbp1(&b2);
    ptrace(CONTIN, pid, pc, execsig);
    bpwait();
    chkerr();
    if ( b2.loc != -1 ) 
	delbp1(&b2); 
    delbp1(&b1);
    readregs();
}


/*
 * Calculate & return the alternate pc for instructions
 * which modify it.  In some cases, e.g. bal and b, only
 * this path will be taken.
 *
 * Branches into segment zero are mistakes or calls
 * to floating point routines.  We set no alternate location
 * for these as we can't step through the kernel.
 */
long alt_loc(opcode, instr, pc)
int opcode;
long instr;
long pc;
{
	register long offset;
	unsigned dest;
	int immediate;
	BOOL absolute;
	int ext_op;

	absolute = (BOOL) ((instr >> 1) & 1);
	switch (opcode) {
	   case 18:  immediate = ((instr & ~3) << 6) >> 6;/* br unconditional */
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
			dest = LVADBREG(SYSREGNO(LR)) & ~3;
		     } else if (ext_op == 528) {/* Brnch condtnl to count reg */
			dest = LVADBREG(SYSREGNO(CTR)) & ~3;
		     } else return -1; 
		     break;
	
	   default: return -1;
	}
	return (dest < 0x10000000) ? -1 : dest;
}
