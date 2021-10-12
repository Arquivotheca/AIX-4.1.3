/* @(#)92	1.6.1.2  src/bos/usr/ccs/lib/libdbx/POWER/frame.h, libdbx, bos411, 9428A410j 1/27/94 16:28:02 */
#ifndef _h_frame
#define _h_frame
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: endofstack, frameeq
 *
 * ORIGINS: 26, 27
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
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */

#define frameeq(f1, f2) (((f1)->save_pc == (f2)->save_pc) && \
			 ((f1)->arsize == (f2)->arsize) && \
			 ((f1)->save_fp == (f2)->save_fp))

#include "machine.h"
#include <sys/debug.h>

#define endofstack(frp)	((frp->caller_fp == 0) || (frp->save_lr == 0))

#define NSAVEREG 19
#define NSAVEFREG 18
#define MAXSAVEREG 32
#define MAXSAVEFREG 32

typedef enum { FIXED_PARM, FLOAT_PARM, DOUBLE_PARM } parm_type;

struct Frame {
    struct tbtable_short tb;	/* Copy of traceback table information */
    unsigned parminfo;		/* parameter information encoding */
    unsigned locals_reg;	/* register for automatic storage */
    Address caller_fp;		/* frame pointer for caller */
    Address save_fp;		/* frame pointer */
    Address save_pc;		/* program counter */
    Address orig_loc;           /* pre-fdpr loc of save_pc */
    Address save_lr;		/* link register */
    Address save_lp;		/* base to locals, usually same as fp */ 
    Word save_cr;		/* saved condition register */
    Address save_TOC;		/* saved TOC address */
    Word save_reg[MAXSAVEREG];	   /* not necessarily there */
    double save_freg[MAXSAVEFREG]; /* not necessarily there */
    int arsize;			/* size of frame minus REGARGSIZE */
    short nparams;		/* number of words of parameters */
    boolean prolog;		/* Is the stack in prolog code? */
    Address regoff;		/* Offset to gpregs */
    Address fregoff;		/* Offset to fpregs */
    char *name;			/* name if tb.name_present == true */
};

extern Frame curframe ;
extern struct Frame curframerec;
extern getcurframe (/* frp */);
extern Frame nextframe (/* frp */);
extern int preg (/* p, offset, optfrp */);
extern Address args_base (/* frp */);
extern Address locals_base (/* optfrp */);
extern Word savereg (/* n, frp */);
extern double savefreg (/* n, frp */);
extern getnewregs (/* addr */);
extern setsavereg(/* n, frp, w */);
extern Word argn (/* n, optfrp */);
extern integer pushargs (/* proc, arglist, thisptr */);
extern passparam (/* actual, formal */);
extern pushretval (/* len, isindirect */);
extern flushoutput(/*  */);
#endif /* _h_frame */ 
