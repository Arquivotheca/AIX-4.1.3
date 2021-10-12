/* @(#)41       1.2.1.3  src/bos/usr/ccs/lib/libdbx/eval.h, libdbx, bos411, 9428A410j 6/3/93 17:52:53 */
#ifndef _h_eval
#define _h_eval
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: (macros) alignstack, pop, popn, push
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */

#include "machine.h"

#define STACKSIZE 20000

#define ISLONG             0x00
#define ISLONGLONG         0x01
#define ISUNSIGNED         0x02
#define ISUNSIGNEDLONG     0x02   /*  ISUNSIGNED | ISLONG      */
#define ISUNSIGNEDLONGLONG 0x03   /*  ISUNSIGNED | ISLONGLONG  */

typedef Char Stack;

#define push(type, value) { \
    ((type *) (sp += sizeof(type)))[-1] = (value); \
}

#define pop(type) ( \
    (*((type *) (sp -= sizeof(type)))) \
)

#define popn(n, dest) { \
    sp -= n; \
    bcopy(sp, dest, n); \
}

#define alignstack() { \
    sp = (Stack *) (( ((int) sp) + sizeof(int) - 1)&~(sizeof(int) - 1)); \
}

extern Stack stack[STACKSIZE];
extern Stack *sp ;
extern Boolean useInstLoc ;
extern Boolean ScrUsed;            /* "screen" command used flag */
extern Node topnode;
extern topeval (/* p */);
extern eval (/* p */);
extern evalcmdlist(/* cl */);
extern rpush(/* addr, len */);
extern Boolean canpush(/* n */);
extern pushsmall(/* t, v */);
extern pushregvalue (/* s, r, f, n */);
extern long popsmall(/* t */);
extern LongLong poplonglong(/* t */);
extern Boolean cond(/* p */);
extern Address lval(/* p */);
extern trace(/* p */);
extern stop(/* p */);
extern assign(/* var, exp */);
extern gripe(/*  */);
extern help(/*  */);
extern setout(/* filename */);
extern unsetout(/*  */);
extern Boolean isredirected(/*  */);
extern void dereference(/*  */);
#endif /* _h_eval */
