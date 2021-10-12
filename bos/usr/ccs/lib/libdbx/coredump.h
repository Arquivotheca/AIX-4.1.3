/* @(#)36	1.3.1.1  src/bos/usr/ccs/lib/libdbx/coredump.h, libdbx, bos411, 9428A410j 10/2/92 16:10:15 */
#ifndef _h_coredump
#define _h_coredump
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: (macros) coredump_readin, coredump.h
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
#define coredump_readin(r, f, s) coredump_xreadin(r, f, &(s))

#include "machine.h"
extern coredump_getkerinfo (/*  */);
extern coredump_xreadin(/* mask, reg, signo */);
extern coredump_close(/*  */);
extern coredump_readtext(/* buff, addr, nbytes */);
extern coredump_readdata(/* buff, addr, nbytes */);
extern copyregs(/* savreg, reg, savfps, fpregs */);
#endif /* _h_coredump */
