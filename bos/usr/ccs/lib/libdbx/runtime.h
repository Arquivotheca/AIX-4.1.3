/* @(#)77	1.2.1.1  src/bos/usr/ccs/lib/libdbx/runtime.h, libdbx, bos411, 9428A410j 2/19/92 14:27:03 */
#ifndef _h_runtime
#define _h_runtime
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: (macros)
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
typedef struct Frame *Frame;

#include "machine.h"
extern Frame nextfunc (/* frp, fp */);
extern Frame findframe(/* f */);
extern Address return_addr (/*  */);
extern wherecmd (/*  */);
extern dump (/* func */);
extern dumpall (/*  */);
extern setcurfunc (/* f */);
extern Frame curfuncframe (/* &frame */);
extern up (/* n */);
extern down (/* n */);
extern Address firstline (/* f */);
extern runtofirst (/*  */);
extern Address lastaddr (/*  */);
extern Boolean isactive (/* f */);
extern callproc (/* exprnode, isfunc */);
extern integer evalargs (/* proc, arglist, thisptr */);
extern integer unsafe_evalargs (/* proc, arglist, thisptr */);
extern procreturn (/* f */);
extern pushenv (/*  */);
extern popenv (/*  */);
#endif /* _h_runtime */
