/* @(#)38	1.3  src/bos/usr/ccs/lib/libdbx/debug.h, libdbx, bos411, 9428A410j 9/27/93 18:11:23 */
#ifndef _h_debug
#define _h_debug
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
extern boolean tracetree;	/* trace building of parse trees */
extern boolean traceeval;	/* trace tree evaluation */
extern debug (/* p */);
extern String opname (/* op */);
extern void dumpSymbolTable();
#endif /* _h_debug */
