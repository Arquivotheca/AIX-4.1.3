/* @(#)87	1.8.1.5  src/bos/usr/ccs/lib/libdbx/tree.h, libdbx, bos411, 9428A410j 5/11/94 14:22:35 */
#ifndef _h_tree
#define _h_tree
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: (macros) cmdlist_append, evalcmd
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
#include "lists.h"

typedef struct Node *Node;
typedef Node Command;
typedef List Cmdlist;

#include "operators.h"
#include "symbols.h"
#include "events.h"
#include "resolve.h"

#define MAXNARGS 5

struct Node {
    Operator op;
    Symbol nodetype;
    union treevalue {
	Symbol sym;
	Name name;
	long lcon;
	LongLong llcon;
	double fcon;
	quadf qcon;
	struct {
	    double real;
	    double imag;
	} kcon;
	struct {
	    quadf real;
	    quadf imag;
	} qkcon;
	String scon;
	struct {
	    String scon;	/* fscon.strsize is used to store size */
	    int strsize;	/* of null-included string */
	} fscon;
	Node arg[MAXNARGS];
        struct {
            Node exp;
            Node place;
            Node cond;
            Boolean inst;
            Event event;
            Cmdlist actions;
        } trace;
	struct {
	    Boolean source;
	    Boolean skipcalls;
	} step;
	struct {
	    String mode;
	    Node beginaddr;
	    Node endaddr;
	    int count;
	} examine;
	cppSymList funcList;
    } value;
};

#define evalcmd(cmd) eval(cmd)
#define cmdlist_append(cmd, cl) list_append(list_item(cmd), nil, cl)

#ifdef _NO_PROTO
extern Node cons(/* va_alist */);
extern Node build(/* va_alist */);
#else
extern Node cons(Operator op, ...);
extern Node build(Operator op, ...);
#endif /* _NO_PROTO */
extern Node unrval (/* exp */);
extern Node ptrrename (/* p, t */);
extern Node amper (/* p */);
extern Node concrete (/* p */);
extern Cmdlist buildcmdlist (/* cmd */);
extern printcmd (/* f, cmd */);
extern prtree (/* f, p */);
extern tfree (/* p */);
extern boolean isarray (/* p */);
extern Node build_add_sub(/* op, n1, n2 */);
extern Node copynode(/* current */);
#endif /* _h_tree */
