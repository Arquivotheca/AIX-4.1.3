static char sccsid[] = "@(#)86    1.29.3.16  src/bos/usr/ccs/lib/libdbx/tree.c, libdbx, bos411, 9428A410j 5/11/94 14:22:22";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: amper, build, buildcmdlist, concrete, isarray,
 *	      isfunction, notregsym, prtree,
 *	      ptrrename, tfree, unrval, unrval_routine,
 *	      build_add_sub, copynode
 *
 * ORIGINS: 26, 27, 83
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
 * Parse tree management.
 */

#ifdef _NO_PROTO
#include <varargs.h>
#else
#include <stdarg.h>
#endif /* _NO_PROTO */
#include "defs.h"
#include "tree.h"
#include "operators.h"
#include "debug.h"
#include "eval.h"
#include "events.h"
#include "symbols.h"
#include "scanner.h"
#include "source.h"
#include "object.h"
#include "mappings.h"
#include "process.h"
#include "machine.h"

extern boolean hexaddr;
extern int sourcefiles;
extern int inclfiles;
extern Language cobLang;

#define is_pointer(node) \
   (((node)->nodetype->class == PTR) || \
    (((node)->nodetype->class == VAR || (node)->nodetype->class == MEMBER) \
   && ((node)->nodetype->type->class == PTR)))

/*
 * Strip away indirection from a node, thus returning a node for
 * interpreting the expression as an lvalue.
 */

public Node unrval (exp)
Node exp;
{
    Node p;

    if (exp->op == O_RVAL) {
	p = exp->value.arg[0];
	dispose(exp);
    } else if ((exp->op == O_INDIR) || (exp->op == O_INDIRA)) {
	p = exp->value.arg[0];
	if (p->op == O_RVAL) {
	    p->op = O_INDIR;
	    p->nodetype = exp->nodetype;
	}
	dispose(exp);
    } else {
	p = exp;
    }
    return p;
}

/*
 * Special case: Strip away indirection from a node, AND if
 * it is a routine return a node for interpreting the 
 * expression as an lvalue. Else just leave it alone.
 */

public Node unrval_routine (exp)
Node exp;
{
    Node p, saved;

    saved = exp;
    if (exp->op == O_RVAL)
    {
	p = exp->value.arg[0];
	if (isroutine(p->value.sym))
	{
	   dispose(exp);
	}
	else 
	{
	   p = saved;
	}
    }
    else 
    {
	p = exp;
    }
    return p;
}

/*
 * Create a node for renaming a node to a pointer type.
 */

public Node ptrrename (p, t)
Node p;
Node t;
{
    t->nodetype = newSymbol(nil, 0, PTR, t->nodetype, nil);
    return build(O_TYPERENAME, p, t);
}

private notregsym (s)
Symbol s;
{
    if (s->storage == INREG || (s->param && preg(s, nil) != -1)) 
    {
	error(catgets(scmc_catd, MS_tree, MSG_379,
	      "cannot take address of register"));
    }
}

private notenumsym(s)
Symbol s;
{
    if (s->class == CONST && s->type->class == SCAL) 
    {
	error(catgets(scmc_catd, MS_tree, MSG_622,
              "cannot take address of enumeration constant"));
    }
}

/*
 * Return the tree for a unary ampersand operator.
 */

public Node amper (p)
register Node p;
{
    register Node r;
    Symbol s;
    extern Language pascalLang;

    checkref(p);
    switch (p->op) {
	case O_RVAL:
            if (p->nodetype->class == LABEL && 
		p->nodetype->language == pascalLang)
            {
               r = copynode(p->nodetype->symvalue.constval);
               r->nodetype = t_addr;
               dispose(p);
               break;
            }
                                      /* if we have a pointer to member that */
                                      /* is a function, do not modify the    */
                                      /* tree because this is printed the    */
                                      /* same whether or not 'address of'    */
                                      /* was specified                       */
            if ((rtype(p->nodetype)->class == FFUNC) && 
                 (p->value.arg[0]->op == O_DOTSTAR)) {
                r = p;
                break;
            }
	case O_INDIR:
	case O_INDIRA:
	    r = p->value.arg[0];
	    if ((r->op == O_SYM) && (p->op != O_INDEX)) {
		notregsym(r->value.sym);
	    }
	    r->nodetype = t_addr;
	    dispose(p);
	    break;

	case O_INDEX:
            r = p;
            break;

	case O_SYM:
	    s = p->value.sym;
	    if (isblock(s)) {
		if (prolloc(s)) {
		   r = build(O_LCON, prolloc(s));
		} else 			/* unnamed blocks have prolloc == 0 */
		   r = build(O_LCON, codeloc(s));
	    } else {
		notregsym(s);
                notenumsym(s);
		r = build(O_LCON, address(s, nil));
	    }
	    r->nodetype = t_addr;
	    dispose(p);
	    break;

	case O_DOT:
	    r = p;
	    r->nodetype = t_addr;
	    break;

	default:
	    beginerrmsg();
	    (*rpt_error)(stderr,  catgets(scmc_catd, MS_tree, MSG_381,
					       "expected variable, found \""));
	    prtree( rpt_error, stderr, p);
	    (*rpt_error)(stderr, "\"");
	    tfree(p);
	    enderrmsg();
	    /* NOTREACHED */
    }
    return r;
}

/*
 * Create a command list from a single command.
 */

public Cmdlist buildcmdlist (cmd)
Command cmd;
{
    Cmdlist cmdlist;

    cmdlist = list_alloc();
    cmdlist_append(cmd, cmdlist);
    return cmdlist;
}

/*
 * Print out a tree.
 */

public prtree( report, f, p)
int (*report)( );
File f;
register Node p;
{
    Operator op;

    if (p != nil) {
	op = p->op;
	if (ord(op) > ord(O_LASTOP)) {
	    panic( catgets(scmc_catd, MS_tree, MSG_425,
	     "Cannot display evaluation tree for unknown operation %d"), p->op);
	    return;
	}
	switch (op) {
	    case O_NAME:
		(*report)( f, "%s", ident(p->value.name));
		break;

	    case O_SYM:
		printname( report, f, p->value.sym, false);
		break;

	    case O_QLINE:
		if ((sourcefiles > 1) || inclfiles) {
		    prtree( report, f, p->value.arg[0]);
		    (*report)( f, ":");
		}
		prtree( report, f, p->value.arg[1]);
		break;

	    case O_CLEAR:
		if ((sourcefiles > 1) || inclfiles) {
		    prtree( report, f, p->value.arg[0]);
		    (*report)( f, ":");
		}
		prtree( report, f, p->value.arg[1]);
		break;

	    case O_LCON:
		if (hexaddr) {
		    (*report)( f, "0x");
		    addrprint( report, f,p->value.lcon,false,0);
	    	} else
		    (*report)( f, "%d", p->value.lcon);
		break;

	    case O_ULCON:
		(*report)( f, "%u", (unsigned int) p->value.lcon);
		break;

	    case O_LLCON:
                (*report)(f, "%lld", p->value.llcon);
                break;

	    case O_ULLCON:
                (*report)(f, "%llu", p->value.llcon);
                break;

	    case O_CCON:
		(*report)( f, "'%c'", p->value.lcon);
		break;

	    case O_FCON:
		(*report)( f, "%g", p->value.fcon);
		break;

	    case O_QCON:
		printquad (report, f, p->value.qcon);
		break;

	    case O_KCON:
		(*report)( f, "(%g,%g)", p->value.kcon.real,
						p->value.kcon.imag);
		break;

            case O_QKCON:
                (*report)(f, "(");
                printquad (report, f, p->value.qkcon.real);
                (*report)(f, ",");
                printquad (report, f, p->value.qkcon.imag);
                (*report)(f, ")");

	    case O_SCON:
		(*report)(f, "\"%s\"", p->value.scon);
		break;

	    /* Pascal set constant */
	    case O_SETCON:
		(*report)(f, "[");
		prtree( report, f, p->value.arg[0]);
		(*report)(f, "]");
		break;

	    case O_ADDR:
		(*report)( f, "&");
		prtree( report, f, p->value.arg[0]);
		break;

	    case O_INDEX:
		prtree( report, f, p->value.arg[0]);
		(*report)( f, "[");
		prtree( report, f, p->value.arg[1]);
		(*report)( f, "]");
		break;

	    case O_COMMA:
		prtree( report, f, p->value.arg[0]);
		if (p->value.arg[1] != nil) {
		    (*report)( f, ", ");
		    prtree( report, f, p->value.arg[1]);
		}
		break;

	    case O_SCOPE:
		prtree( report, f, p->value.arg[0]);
		(*report)( f, "::");
		prtree( report, f, p->value.arg[1]);
		break;

	    case O_NAMELIST:
                printmparams( f, p->value.arg[0] );
		break;

	    case O_UNRES:
		if (p->value.arg[0] != nil)
		{
		    prtree( report, f, p->value.arg[0]);
		    (*report)( f, "::");
		}
		prtree( report, f, p->value.arg[1]);
		break;

	    case O_BSET:
		(*report)( f, "build set[");
		prtree( report, f, p->value.arg[0]);
		(*report)( f, "]");
		break;

	    case O_RVAL:
	    case O_CPPREF:
	    case O_ITOF:
		prtree( report, f, p->value.arg[0]);
		break;

	    case O_CALL:
		prtree( report, f, p->value.arg[0]);
		if (p->value.arg[1]!= nil) {
		    (*report)( f, "(");
		    prtree( report, f, p->value.arg[1]);
		    (*report)( f, ")");
		}
		break;

	    case O_INDIR:
	    case O_INDIRA:
		prtree( report, f, p->value.arg[0]);
		(*report)( f, "^");
		break;

	    case O_DOT:
		prtree(report, f, p->value.arg[0]);
		(*report)( f, ".%s", symname(p->value.arg[1]->value.sym));
		break;

	    case O_DOTSTAR:
		prtree( report, f, p->value.arg[0]);
		(*report)( f, ".*");
		prtree(report, f, p->value.arg[1]);
		break;

	    case O_FREE:
		prtree( report, f, p->value.arg[0]);
		break;

	    case O_PTRRENAME:
		prtree( report, f, p->value.arg[1]);
		(*report)( f, "(");
		prtree( report, f, p->value.arg[0]);
		(*report)( f, " *)");
		break;

	    case O_TYPERENAME:
		prtree( report, f, p->value.arg[1]);
		(*report)( f, "(");
		prtree( report, f, p->value.arg[0]);
		(*report)( f, ")");
		break;

	    default:
		switch (degree(op)) {
		    case BINARY:
			prtree( report, f, p->value.arg[0]);
			(*report)( f, "%s", opinfo[ord(op)].opstring);
			prtree( report, f, p->value.arg[1]);
			break;

		    case UNARY:
			(*report)( f, "%s", opinfo[ord(op)].opstring);
			prtree( report, f, p->value.arg[0]);
			break;

		    default:
			if (opinfo[ord(op)].opstring == nil) {
			    (*report)( f, "[op %d]", ord(op));
			} else {
			    (*report)( f, "%s", opinfo[ord(op)].opstring);
			}
			break;
		}
		break;
	}
    }
}

/*
 * Free storage associated with a tree.
 */

public tfree (p)
Node p;
{
    int i;
    short	numb_args;

    if (p == nil) {
	return;
    }
    switch (p->op) {
	case O_CLEAR:
	    dispose(p->value.arg[0]->value.scon);
	    dispose(p->value.arg[0]);
	    tfree(p->value.arg[1]);
	    break;

	case O_SCON:
	    dispose(p->value.scon);
	    break;

	case O_LCON:
	case O_ULCON:
	case O_LLCON:
	case O_ULLCON:
	    break;

	default:
	    numb_args = nargs(p->op);
	    for (i = 0; i < numb_args; i++) {
		tfree(p->value.arg[i]);
	    }
	           break;
    }
    dispose(p);
}
/* 
 * Determine whether or not a symbol is an array symbol instead of a function.
 */

public Boolean isarray(n)
Node n;
{
    Symbol s;

    if (n->op == O_DOT)
      n = n->value.arg[1];
    if (n->op == O_SYM) {
	s = n->value.sym;
	if(s->language == cobLang) {
	    if (!((s->class == VAR) || (s->class == TOCVAR) ||
		  (s->class == REF) || (s->class == REFFIELD))) {
	        return false;
	    } else {
		Symbol p;
	        for (p = s; p->class == REFFIELD;
						p = p->symvalue.field.parent) {
		    if (p->type->class == ARRAY)
			return true;
		}
		return (Boolean) (p->type->class == ARRAY);
	    }
	} else {
	    return (((s->class == VAR) || (s->class == TOCVAR) ||
		     (s->class == REF) || (s->class == REFFIELD) ||
		     (s->class == MEMBER)) &&
		    (s->type->class == ARRAY));
	}
    } else
	return false;
}

/* 
 * Determine whether or not a symbol is a function.
 */

public Boolean isfunction(n)
Node n;
{
    Symbol s;

    if (n->op == O_SYM) {
	s = n->value.sym;
	return ((s->class == FUNC) || (s->class == PROC) ||
						(s->class == CSECTFUNC));
    } else
	return false;
}

/*
 * Build a tree.
 */

#ifdef _NO_PROTO
/*VARARGS1*/
public Node build(va_alist)
     va_dcl
#else
public Node build(Operator op, ...)
#endif /* _NO_PROTO */
{
#ifdef _NO_PROTO
    Operator op;
#endif /* _NO_PROTO */
    register va_list ap;
    register Node p, q;
    Symbol s, t;
    int i;
    short	numb_args;

#ifdef _NO_PROTO
    va_start(ap);
    op = va_arg(ap, Operator);
#else
    va_start(ap, op);
#endif /* _NO_PROTO */
    p = new(Node);
    p->op = op;
    p->nodetype = nil;
    switch (op) {
	case O_NAME:
	    p->value.name = va_arg(ap, Name);
	    break;

	case O_SYM:
	case O_PRINTCALL:
	case O_PRINTRTN:
	case O_PROCRTN:
	    p->value.sym = va_arg(ap, Symbol);
	    break;

	/* Pascal Set Constant */
	case O_SETCON:
            p->value.lcon = va_arg(ap, long);
            p->nodetype = newSymbol( identname("$setcon",true), 0, SET,
						      va_arg(ap, Symbol), nil);
            break;

	case O_DEBUG:
	case O_LCON:
	case O_CCON:
	case O_CASE:
	case O_TRACEOFF:
	    p->value.lcon = va_arg(ap, long);
	    break;

	case O_ULCON:
	    p->value.lcon = va_arg(ap, unsigned long);
	    break;

	case O_LLCON:
	    p->value.llcon = va_arg(ap, LongLong);
	    break;

	case O_ULLCON:
	    p->value.llcon = va_arg(ap, uLongLong);
	    break;

	case O_FCON:
	    p->value.fcon = va_arg(ap, double);
	    break;

	case O_QCON:
	    p->value.qcon = va_arg(ap, quadf);
	    break;

	case O_KCON:
	    p->value.kcon.real = va_arg(ap, double);
	    p->value.kcon.imag = va_arg(ap, double);
	    break;

        case O_QKCON:
            p->value.qkcon.real = va_arg(ap, quadf);
            p->value.qkcon.imag = va_arg(ap, quadf);
            break;

#ifdef KDBX
        case O_LLDB:
#endif  /* KDBX */
	case O_SCON:
	case O_CHFILE:
	case O_EDIT:
	case O_SOURCE:
	case O_PROMPT:
	    p->value.scon = va_arg(ap, String);
	    if (op == O_SCON)
	      p->value.fscon.strsize = va_arg(ap, int);
	    break;

	case O_RVAL:
	case O_INDIR:
	case O_INDIRA:
	case O_CPPREF:
	    p->value.arg[0] = va_arg(ap, Node);
	    break;

	case O_CALL:
	case O_CALLPROC:
	    q = va_arg(ap, Node);
	    p->value.arg[0] = q;
	    p->value.arg[1] = va_arg(ap, Node);
	    if (q->op == O_SYM) {
		s = q->value.sym;
		if (s->class == TYPE or s->class == TAG) {
		    p->op = O_TYPERENAME;
		    p->value.arg[0] = p->value.arg[1];
		    p->value.arg[1] = q;
		    q = p->value.arg[0];
		    if (q == nil) {
			error( catgets(scmc_catd, MS_resolve, MSG_615,
					 "Too few arguments to type rename."));
		    }
		    if (q->value.arg[1] != nil) {
			error( catgets(scmc_catd, MS_resolve, MSG_616,
					 "Too many arguments to type rename."));
		    }
		    p->value.arg[0] = q->value.arg[0];
		} else if (s->class == MODULE) {
		    for (t = lookup(s->name); t != nil; t = t->next_sym) {
			if (t->block == s and isroutine(t)) {
			    q->value.sym = t;
			    q->nodetype = t;
			    break;
			}
		    }
		}
	    }
	    break;

	case O_ADDEVENT:
	case O_ONCE:
	case O_IF:
	    p->value.trace.exp = NULL;
	    p->value.trace.place = NULL;
	    p->value.trace.cond = va_arg(ap, Node);
	    p->value.trace.inst = false;
	    p->value.trace.event = NULL;
	    p->value.trace.actions = va_arg(ap, Cmdlist);
	    break;

	case O_TRACEON:
	    p->value.trace.exp = NULL;
	    p->value.trace.place = NULL;
	    p->value.trace.cond = NULL;
	    p->value.trace.inst = va_arg(ap, Boolean);
	    p->value.trace.event = NULL;
	    p->value.trace.actions = va_arg(ap, Cmdlist);
	    break;

	case O_STOPIFCHANGED:
	    p->value.trace.exp = va_arg(ap, Node);
	    p->value.trace.place = NULL;
	    p->value.trace.cond = va_arg(ap, Node);
	    p->value.trace.inst = NULL;
	    p->value.trace.event = NULL;
	    p->value.trace.actions = NULL;
	    break;

	case O_STEP:
	    p->value.step.source = va_arg(ap, Boolean);
	    p->value.step.skipcalls = va_arg(ap, Boolean);
	    break;

	case O_EXAMINE:
	    p->value.examine.mode = va_arg(ap, String);
	    p->value.examine.beginaddr = va_arg(ap, Node);
	    p->value.examine.endaddr = va_arg(ap, Node);
	    p->value.examine.count = va_arg(ap, int);
	    break;

	default:
	    numb_args = nargs(op);
	    for (i = 0; i < numb_args; i++) {
		p->value.arg[i] = va_arg(ap, Node);
	    }
	    break;
    }
    va_end(ap);

    /* Check for basic types here */
    if ((int) p->op >= (int) O_ALIAS) {
        check(p);
        assigntypes(p);
    }
    else
    switch(p->op) {
        case O_LCON:
            p->nodetype = t_int;
            break;

        case O_ULCON:
            p->nodetype = dt_uint;
            break;

        case O_LLCON:
            p->nodetype = t_longlong;
            break;

        case O_ULLCON:
            p->nodetype = t_ulonglong;
            break;

        case O_CCON:
            p->nodetype = t_char;
            break;

        case O_FCON:
            p->nodetype = t_real;
            break;

        case O_QCON:
            p->nodetype = t_quad;
            break;

        case O_KCON:
            p->nodetype = t_complex;
            break;

        case O_QKCON:
            p->nodetype = t_qcomplex;
            break;

        case O_SETCON:
            break;

        default:
            check(p);
            assigntypes(p);
            break;
    }

    if (tracetree) {
        (*rpt_output)(stdout,
                      "built %s node 0x%x with arg[0] 0x%x arg[1] 0x%x\n",
                       opname(p->op), p, p->value.arg[0], p->value.arg[1]);
        fflush(stdout);
    }

    return p;
}

/*
 * Build a tree without performing "check" and "assigntypes"
 */

#ifdef _NO_PROTO
/*VARARGS1*/
public Node cons(va_alist)
     va_dcl
#else
public Node cons(Operator op, ...)
#endif /* _NO_PROTO */
{
#ifdef _NO_PROTO
    Operator op;
#endif /* _NO_PROTO */
    register va_list ap;
    register Node p;
    int i;
    short numb_args;

#ifdef _NO_PROTO
    va_start(ap);
    op = va_arg(ap, Operator);
#else
    va_start(ap, op);
#endif /* _NO_PROTO */
    p = new(Node);
    p->op = op;
    p->nodetype = nil;
    switch (op) {
	case O_NAME:
	    p->value.name = va_arg(ap, Name);
	    break;

	case O_SYM:
	case O_PRINTCALL:
	case O_PRINTRTN:
	case O_PROCRTN:
	    p->value.sym = va_arg(ap, Symbol);
	    break;

	/* Pascal Set Constant */
	case O_SETCON:
            p->value.lcon = va_arg(ap, long);
            p->nodetype = newSymbol(identname("$setcon", true), 0, SET,
				    va_arg(ap, Symbol), nil);
            break;

	case O_DEBUG:
	case O_LCON:
	case O_CCON:
	case O_CASE:
	case O_TRACEOFF:
	    p->value.lcon = va_arg(ap, long);
	    break;

        case O_ULCON:
            p->value.lcon = va_arg(ap, unsigned long);
            break;

        case O_LLCON:
            p->value.llcon = va_arg(ap, LongLong);
            break;

        case O_ULLCON:
            p->value.llcon = va_arg(ap, uLongLong);
            break;

	case O_FCON:
	    p->value.fcon = va_arg(ap, double);
	    break;

	case O_QCON:
	    p->value.qcon = va_arg(ap, quadf);
	    break;

	case O_KCON:
	    p->value.kcon.real = va_arg(ap, double);
	    p->value.kcon.imag = va_arg(ap, double);
	    break;

	case O_QKCON:
	    p->value.qkcon.real = va_arg(ap, quadf);
	    p->value.qkcon.imag = va_arg(ap, quadf);
	    break;

	case O_SCON:
	case O_CHFILE:
	case O_EDIT:
	case O_SOURCE:
	case O_PROMPT:
	    p->value.scon = va_arg(ap, String);
	    if (op == O_SCON)
	        p->value.fscon.strsize = va_arg(ap, int);
	    break;

	case O_RVAL:
	case O_INDIR:
	case O_INDIRA:
	case O_CPPREF:
	    p->value.arg[0] = va_arg(ap, Node);
	    break;

	case O_CALL:
	case O_CALLPROC:
	    p->value.arg[0] = va_arg(ap, Node);
	    p->value.arg[1] = va_arg(ap, Node);
	    break;

	case O_ADDEVENT:
	case O_ONCE:
	case O_IF:
	    p->value.trace.exp = NULL;
	    p->value.trace.place = NULL;
	    p->value.trace.cond = va_arg(ap, Node);
	    p->value.trace.inst = false;
	    p->value.trace.event = NULL;
	    p->value.trace.actions = va_arg(ap, Cmdlist);
	    break;

	case O_TRACEON:
	    p->value.trace.exp = NULL;
	    p->value.trace.place = NULL;
	    p->value.trace.cond = NULL;
	    p->value.trace.inst = va_arg(ap, Boolean);
	    p->value.trace.event = NULL;
	    p->value.trace.actions = va_arg(ap, Cmdlist);
	    break;

	case O_STOPIFCHANGED:
	    p->value.trace.exp = va_arg(ap, Node);
	    p->value.trace.place = NULL;
	    p->value.trace.cond = va_arg(ap, Node);
	    p->value.trace.inst = NULL;
	    p->value.trace.event = NULL;
	    p->value.trace.actions = NULL;
	    break;

	case O_STEP:
	    p->value.step.source = va_arg(ap, Boolean);
	    p->value.step.skipcalls = va_arg(ap, Boolean);
	    break;

	case O_EXAMINE:
	    p->value.examine.mode = va_arg(ap, String);
	    p->value.examine.beginaddr = va_arg(ap, Node);
	    p->value.examine.endaddr = va_arg(ap, Node);
	    p->value.examine.count = va_arg(ap, int);
	    break;

	default:
	    numb_args = nargs(op);
	    for (i = 0; i < numb_args; i++) {
		p->value.arg[i] = va_arg(ap, Node);
	    }
	    break;
    }
    va_end(ap);

    if (tracetree) {
        (*rpt_output)(stdout,
                      "built %s node 0x%x with arg[0] 0x%x arg[1] 0x%x\n",
                       opname(p->op), p, p->value.arg[0], p->value.arg[1]);
        fflush(stdout);
    }

    return p;
}

/*
 * Build an O_ADD or O_SUB node, possibly scaling one of the operands
 * appropriately for pointer arithmetic.
 */
public Node build_add_sub(op, n1, n2)
     Operator op;
     Node n1, n2;
{
  long TypeSize;
  Node ntemp, r;

  if (op == O_ADD &&		/* swap nodes in the case of (i + ptr) */
      (n2->nodetype->class == VAR || n2->nodetype->class == MEMBER) &&
      n2->nodetype->type->class == PTR) {
    ntemp = n1;
    n1 = n2;
    n2 = ntemp;
  }

  /*  if n1 is a pointer and n2 is an "int" type  */  
  /*  NOTE : second part of "if" test is important because
             pointers are compatible with ints  */
  if (is_pointer(n1) && !is_pointer(n2) && compatible(n2->nodetype, t_int))
  {
    Symbol n1_type = (n1->nodetype->class == PTR) ? n1->nodetype
                                                  : n1->nodetype->type;
    /*  build a node so n2 gets multiplied by 4  */
    TypeSize = size(n1_type->type);
    n2 = build(O_MUL, n2, build(O_LCON, TypeSize));
    r = build(op, n1, n2);
    r->nodetype = n1_type;
  }
  else 
  {
    r = build(op, n1, n2);
  }

  return r;
}

/*
 * NAME: copynode
 *
 * FUNCTION: make a copy of a node 
 *
 * PARAMETERS:
 *      current - input Node
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: a new copy of the node
 */

Node copynode( Node current )
{
    Node	current_copy;
    short	index;
    short	num_args;

    if( current == NULL ) return NULL;

    current_copy = new(Node);
    current_copy->op = current->op;
    current_copy->nodetype = current->nodetype;

    num_args = nargs( current->op );

    for( index = 0; index < num_args; index++ ) {
	    current_copy->value.arg[index] =
					copynode( current->value.arg[index] );
    }

    if( index == 0 ) {
	switch( current->op ) {
	    case O_SCON:
		current_copy->value.scon =
		      malloc( (strlen(current->value.scon) + 1) * sizeof(char));
		strcpy(current_copy->value.scon, current->value.scon);
		break;
	    default:
		current_copy->value = current->value;
		break;
	}
    }
    return current_copy;
}
