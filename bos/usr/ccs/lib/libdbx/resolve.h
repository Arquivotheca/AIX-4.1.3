/* @(#)68	1.1.1.5  src/bos/usr/ccs/lib/libdbx/resolve.h, libdbx, bos411, 9428A410j 6/30/93 13:02:19 */
#ifndef _h_resolve
#define _h_resolve
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: none.
 *
 * ORIGINS: 27
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

#include "mappings.h"

typedef struct AccessList {
    Symbol baseClass;
    struct AccessList *next;
} *AccessList;

typedef struct cppSymList {
    Symbol sym;
    Boolean file_list;
    char *filename;
    Address line_addr;
    struct cppSymList *next;
    struct cppSymList *chain;
} *cppSymList;

#define free_element_chain(list)    \
  {                                 \
    cppSymList temp_list;           \
    while (list != NULL)            \
    {                               \
      temp_list = list;             \
      list = list->chain;           \
      free(temp_list);              \
    }                               \
  }

#ifndef _NO_PROTO
extern Symbol		findmember(Symbol, Name, AccessList *, int);
extern Node		traverse(Node, unsigned long);
extern Node		buildAccess(AccessList, Symbol, Node);
extern Node		resolveQual(Node, Node, Boolean);
extern Node		resolveName(Node, Node, Node, unsigned long, Boolean);
extern Node		findQual(Node, Node, Boolean);
extern Symbol		isMember(Symbol, Name, unsigned long);
extern Symbol		isStaticMember(Symbol, Name, int);
#else
extern Symbol		findmember();
extern Node		traverse();
extern Node		buildAccess();
extern Node		resolveQual();
extern Node		resolveName();
extern Node		findQual();
extern Symbol		isMember();
extern Symbol		isStaticMember();
#endif

#endif /* _h_resolve */
