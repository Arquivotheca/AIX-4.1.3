static char sccsid[] = "@(#)32	1.16.2.6  src/bos/usr/ccs/lib/libdbx/check.c, libdbx, bos411, 9428A410j 11/22/93 12:25:45";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: chkaddr, chkblock, chkline, chkstop, chktrace, chkwatch,
 *	      fortchararray, isanaddr
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
#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
/*              include file for message texts          */
#include "dbx_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

/*
 * Check a tree for semantic correctness.
 */

#include "defs.h"
#include "tree.h"
#include "operators.h"
#include "events.h"
#include "symbols.h"
#include "scanner.h"
#include "source.h"
#include "object.h"
#include "mappings.h"
#include "process.h"
#include <signal.h>

#define fortchararray(t) ((t->class != CHARSPLAT) && (t->class != FSTRING))

private chktrace();
private chkwatch();
private chkstop();
private chkblock();
private chkline();
private chkaddr();
private boolean isanaddr();

/*
 * Check that the nodes in a tree have the correct arguments
 * in order to be evaluated.  Basically the error checking here
 * frees the evaluation routines from worrying about anything
 * except dynamic errors, e.g. subscript out of range.
 */

public check(p)
register Node p;
{
    Node p1, p2;
    Address addr;
    Symbol f;

    checkref(p);
    switch (p->op) {
	case O_ASSIGN:
	    p1 = p->value.arg[0];
	    p2 = p->value.arg[1];

            /*  if the types are not compatible  */
            if (!compatible(p1->nodetype, p2->nodetype))
            {
              /*  if $unsafeassign is set  */
	      if (varIsSet("$unsafeassign")) 
              {
		if (size(p1->nodetype) != size(p2->nodetype))
                {
		   f = p1->nodetype;
		   while ((f->class != TYPE) && fortchararray(f)
			  && f->class != PIC && f->class != RPIC)
		     f = f->type; 
		   if (fortchararray(f) || (size(p1->nodetype) == 1))
			error( catgets(scmc_catd, MS_check, MSG_0,
						       "incompatible sizes"));
		}
	      }
              else
              {
                error(catgets(scmc_catd, MS_check, MSG_1,
                              "incompatible types"));
              }
	    }
	    break;

	case O_CATCH:
	case O_IGNORE:
	    if (p->value.arg[0]->value.lcon < 0 or 
                p->value.arg[0]->value.lcon >= NSIG) 
	    {
		error( catgets(scmc_catd, MS_check, MSG_3,
						    "invalid signal number"));
	    }
	    break;

	case O_CONT:
	    if (p->value.arg[0]->value.lcon != DEFSIG and 
                (p->value.arg[0]->value.lcon < 0 or 
		 p->value.arg[0]->value.lcon >= NSIG)) 
	    {
		error( catgets(scmc_catd, MS_check, MSG_3,
						    "invalid signal number"));
	    }
	    break;

	case O_DUMP:
	    if (p->value.arg[0] != nil) 
	    {
		/* allow for C++ member functions */
		Node q;

		q = p->value.arg[0];
		if (q->op == O_RVAL)
		    q = q->value.arg[0];
		if (q->op == O_DOT)
		    q = q->value.arg[1];
		if (q->op == O_SYM) 
		{
		    f = q->value.sym;
		    if (not isblock(f)) 
		    {
			error(catgets(scmc_catd, MS_check, MSG_5,
			              "\"%s\" is not a block"), symname(f));
		    }
		} 
                else 
		{
	            beginerrmsg();
		    (*rpt_error)(stderr, catgets(scmc_catd, MS_check, MSG_8, 
					 "expected a symbol, found \""));
		    prtree(rpt_error, stderr, p->value.arg[0]);
		    (*rpt_error)(stderr, "\"");
		    enderrmsg();
		}
	    }
	    break;

	case O_LIST:
	case O_LISTI:
	case O_MOVE:
	{
	    /* allow for C++ member functions */
	    Node q;

	    q = p->value.arg[0];
	    if (q->op == O_RVAL)
		q = q->value.arg[0];
	    if (q->op == O_DOT)
		q = q->value.arg[1];

	    if (q->op == O_SYM) 
	    {
		f = q->value.sym;
		if (not isblock(f) or ismodule(f)) 
		{
		    error(catgets(scmc_catd, MS_check, MSG_10,
			  "\"%s\" is not a procedure or function"), symname(f));
		}
		addr = firstline(f);
		if (addr == NOADDR) 
		{
		    error(catgets(scmc_catd, MS_check, MSG_13,
                          "\"%s\" is empty"), symname(f));
		}
	    }
	    else 
	    {
		if (p->value.arg[0]->op == O_RVAL &&
		    p->value.arg[0]->value.arg[0]->op == O_DOT &&
		    not isfunction(p)) 
	 	{
		    error(catgets(scmc_catd, MS_check, MSG_10,
			  "\"%s\" is not a procedure or function"),
			  p->value.arg[0]->nodetype->name->identifier);
		}
	    }

	    break;
	}

	case O_WATCH:
	    chkwatch(p);

	case O_TRACE:
	case O_TRACEI:
	    chktrace(p);
	    break;

	case O_STOP:
	case O_STOPI:
	    chkstop(p);
	    break;

	case O_CALLPROC:
	case O_CALL:
	    if (not isroutine(p->value.arg[0]->nodetype) && 
		/* not pure virtual call */
		(p->value.arg[0]->nodetype->class != MEMBER ||
		 p->value.arg[0]->nodetype->symvalue.member.type != FUNCM))
	    {
		beginerrmsg();
		(*rpt_error)(stderr, "\"");
		prtree( rpt_error, stderr, p->value.arg[0]);
		(*rpt_error)(stderr,  catgets(scmc_catd, MS_check, MSG_15,
							 "\" not call-able"));
		enderrmsg();
	    }
	    break;

	default:
	    break;
    }
}

/*
 * Check arguments to a trace command.
 */

private chktrace(p)
Node p;
{
    Node exp, place;

    exp = p->value.arg[0];
    place = p->value.arg[1];
    if (exp == nil) {
	if ((place != nil) && (place->op == O_LCON or place->op == O_QLINE)) {
	    if (p->op == O_TRACE) {
	        chkline(place);
	    } else {
	        chkaddr(place);
	    }
	} else {
	    chkblock(place);
	}
    } else if (exp->op == O_LCON or exp->op == O_QLINE) {
	if (place != nil and exp->op == O_QLINE) {
	    error( catgets(scmc_catd, MS_check, MSG_17,
					       "unexpected \"at\" or \"in\""));
	}
	if (p->op == O_TRACE) {
	    chkline(exp);
	} else {
	    chkaddr(exp);
	}
    } else if (place != nil && (place->op == O_QLINE || place->op == O_LCON)) {
	if (p->op == O_TRACE) {
	    chkline(place);
	} else {
	    chkaddr(place);
	}
    } else {
	if (exp->op != O_RVAL and exp->op != O_SYM and 
	    exp->op != O_CALL and exp->op != O_DOT) {
	    error( catgets(scmc_catd, MS_check, MSG_20,
						  "cannot trace expressions"));
	}
	chkblock(place);
    }
}

/*
 * Check arguments to a watch command.
 */

private chkwatch (p)
Node p;
{
    Node exp, place, left;

    exp = p->value.arg[0];
    place = p->value.arg[1];
    if (exp == nil) {
	error( catgets(scmc_catd, MS_check, MSG_22,
						"missing variable to watch"));
    } else if (exp->op == O_LCON or exp->op == O_QLINE) {
	    beginerrmsg();
	    (*rpt_error)(stderr,  catgets(scmc_catd, MS_check, MSG_23,
				       "expected variable to watch, found "));
	    prtree( rpt_error, stderr, exp);
	    enderrmsg();
    } else {
	if ((exp->op != O_SYM) && (exp->op != O_RVAL) && (exp->op != O_INDEX)){
	    beginerrmsg();
	    (*rpt_error)(stderr,  catgets(scmc_catd, MS_check, MSG_23,
					"expected variable to watch, found "));
	    prtree( rpt_error, stderr, exp);
	    enderrmsg();
	}
	left = exp;
	if (left->op == O_RVAL or left->op == O_CALL)
	    left = left->value.arg[0];
	if (left->op == O_SYM and isblock(left->value.sym)) {
	    beginerrmsg();
	    (*rpt_error)(stderr,  catgets(scmc_catd, MS_check, MSG_23,
					"expected variable to watch, found "));
	    prtree( rpt_error, stderr, exp);
	    enderrmsg();
	}
        if (place->op == O_LCON or place->op == O_QLINE) {
	    chkline(place);
	} else {
	    chkblock(place);
	}
    }
}

/*
 * Check arguments to a stop command.
 */

private chkstop(p)
Node p;
{
    Node exp, place;

    exp = p->value.arg[0];
    place = p->value.arg[1];
    if (exp != nil) 
    {
   
        /*  O_RVAL and O_SYM are valid for O_STOP,
            O_RVAL, O_SYM and O_LCON are valid for O_STOPI -
            return an error for anything else  */
        if (((p->op == O_STOP) || (exp->op != O_LCON))
         && (exp->op != O_RVAL) && (exp->op != O_SYM))
	{
	    beginerrmsg();
	    (*rpt_error)(stderr, catgets(scmc_catd, MS_check, MSG_26,
				 "expected variable, found "));
	    prtree(rpt_error, stderr, exp);
	    enderrmsg();
	}
    } 

    if (place != nil)
    {
	if (place->op == O_SYM or place->op == O_RVAL or place->op == O_DOT)
	    chkblock(place);
	else
	    if (p->op == O_STOP)
	        chkline(place);
	    else
	        chkaddr(place);
    }
}

/*
 * Check to see that the given node specifies some subprogram.
 * Nil is ok since that means the entire program.
 */

private chkblock(b)
Node b;
{
    Symbol p, outer;

    if (b != nil) {
	/* accept lists of C++ functions - they've already been validated */
	if (b->nodetype->class == CPPSYMLIST)
	    return;

	/* single C++ functions - handle member functions */
	if (b->op == O_CPPREF)
	    b = b->value.arg[0];
	while (b->op == O_RVAL)
	    b = b->value.arg[0];
	if (b->op == O_DOT)
	    b = b->value.arg[1];

	if (b->op != O_SYM) {
	    beginerrmsg();
	    (*rpt_error)(stderr,  catgets(scmc_catd, MS_check, MSG_27,
					       "expected subprogram, found "));
	    prtree( rpt_error, stderr, b);
	    enderrmsg();
	} 
	
	if (ismodule(b->value.sym)) 
	{
	    outer = b->value.sym;
				
	    while (outer != nil) {     /* handle things like printf.printf */
		find(p, outer->name) 
		     where ((p->block == outer) && (isblock(p)))
		endfind(p);
		if (p == nil) {
		    /* Couldn't find printf.printf, look for randname.printf */
		    outer = b->value.sym;
		    find (p, outer->name)
			 where ((isroutine(p)) && (p->level == program->level))
		    endfind(p);
		    if (p != nil) {
			 outer = nil;
		         b->value.sym = p;
		    } else {
		       error( catgets(scmc_catd, MS_check, MSG_28,
			 "\"%s\" is not a subprogram"), symname(b->value.sym));
		    }
		} else if (ismodule(p)) {
		    outer = p;
		} else {
		    outer = nil;
		    b->value.sym = p;
		}
	    }
	} 
	else if ((b->value.sym->class == VAR or b->value.sym->class == TOCVAR) 
		 and b->value.sym->name == b->value.sym->block->name 
		 and b->value.sym->block->class == FUNC) 
	{
	    b->value.sym = b->value.sym->block;
	} 
	else if (not isblock(b->value.sym)) 
	{
	    error( catgets(scmc_catd, MS_check, MSG_28,
			 "\"%s\" is not a subprogram"), symname(b->value.sym));
	}
    }
}

/*
 * Check to make sure a node corresponds to a source line.
 */

private chkline(p)
Node p;
{
    if (p == nil) {
	error( catgets(scmc_catd, MS_check, MSG_30, "missing line"));
    } else if (p->op != O_QLINE and p->op != O_LCON) {
	error( catgets(scmc_catd, MS_check, MSG_31,
			      "expected source line number, found \"%t\""), p);
    }
}

/*
 * Check to make sure a node corresponds to an address.
 */

private chkaddr(p)
Node p;
{
    if (p == nil) 
    {
	error( catgets(scmc_catd, MS_check, MSG_32, "missing address"));
    }
    else if (p->op != O_LCON  && p->op != O_QLINE && p->op != O_MUL &&
	     p->op != O_INDIR && p->op != O_INDIRA &&
	     p->op != O_ADD && p->op != O_SUB && p->op != O_NEG)
    {
	if (!isanaddr(p))
        {
	    beginerrmsg();
	    (*rpt_error)(stderr,  catgets(scmc_catd, MS_check, MSG_33,
						"expected address, found \""));
	    prtree( rpt_error, stderr, p);
	    (*rpt_error)(stderr, "\"");
	    enderrmsg();
        }
    }
}

private boolean isanaddr(p)
Node p;
{
    if (p == nil) {
       return false;
    } else if (p->nodetype == t_addr) {
	return true;
    } else if ((p->op == O_ADD) || (p->op == O_SUB) || (p->op == O_MUL)) {
	return (isanaddr(p->value.arg[0]) && (isanaddr(p->value.arg[1])));
    } else if ((p->op == O_INDIR) || (p->op == O_INDIRA) || (p->op == O_NEG)) {
	return isanaddr(p->value.arg[0]);
    } else
	return false;
}
