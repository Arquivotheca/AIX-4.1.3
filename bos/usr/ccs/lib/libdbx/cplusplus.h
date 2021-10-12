/* @(#)80	1.4.1.2  src/bos/usr/ccs/lib/libdbx/cplusplus.h, libdbx, bos411, 9428A410j 8/21/93 17:38:32 */
#ifndef _h_cpp
#define _h_cpp
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

typedef struct symlist {
  Symbol symbol;
  struct symlist *next;
} *SymList;

extern Boolean cppModuleSeen;
extern Language cppLang;
extern Name this;

extern void cpp_init();
extern void cpp_printval(/* Symbol s, int indent */);
extern void cpp_printtype(/* Symbol s, Symbol t, int indent */);
extern Boolean cpp_tempname(/* Name */);
extern void cpp_touchClass(/* Symbol */);
extern Boolean cpp_equivalent(/* Symbol, Symbol */);
extern Boolean cpp_typematch(/* Symbol, Symbol */);

extern void cpp_addToVirtualList(/* Node */);
extern Boolean cpp_isVirtual(/* Node */);
extern void cpp_emptyVirtualList();

extern void cpp_printqfuncname(/* Symbol, int */);
extern void cpp_printfuncname(/* Symbol, int */);

extern void cpp_printClass(/* Symbol, int, Name */);
extern void cpp_printPtrToMem(/* Symbol */);

extern void cpp_initreflist();
extern void cpp_clrreflist();

#endif /* _h_cpp */
