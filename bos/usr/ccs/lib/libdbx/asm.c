static char sccsid[] = "@(#)28	1.9.1.4  src/bos/usr/ccs/lib/libdbx/asm.c, libdbx, bos411, 9428A410j 1/12/94 16:17:47";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: asm_buildaref, asm_evalaref, asm_foldnames, asm_hasmodules,
 *	      asm_init, asm_modinit, asm_passaddr, asm_printdecl, asm_printval,
 *	      asm_typematch
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

/*
 * Assembly language dependent symbol routines.
 */

#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
#include "defs.h"
#include "languages.h"
#include "eval.h"
#include "operators.h"
#include "mappings.h"
#include "process.h"
#include "runtime.h"
#include "decode.h"

/*  array is zeroed out because it is static  */
static short usetype[TP_NTYPES];

Language asmLang;


/*
 * Initialize assembly language information.
 */

void asm_init()
{
    asmLang = language_define("assembler", ".s");
    language_setop(asmLang, L_PRINTDECL, asm_printdecl);
    language_setop(asmLang, L_PRINTVAL, asm_printval);
    language_setop(asmLang, L_TYPEMATCH, asm_typematch);
    language_setop(asmLang, L_BUILDAREF, asm_buildaref);
    language_setop(asmLang, L_EVALAREF, asm_evalaref);
    language_setop(asmLang, L_MODINIT, asm_modinit);
    language_setop(asmLang, L_HASMODULES, asm_hasmodules);
    language_setop(asmLang, L_PASSADDR, asm_passaddr);
    language_setop(asmLang, L_FOLDNAMES, asm_foldnames);
}

/*
 * Test if two types are compatible.
 */

Boolean asm_typematch(Symbol type1, Symbol type2)
{
    return false;
}

void asm_printdecl(Symbol s)
{
    switch (s->class) {
	case CONST:
	    (*rpt_output)(stdout, "%s = %d", symname(s),
					s->symvalue.constval->value.lcon);
	    break;

	case VAR:
	case TOCVAR:
	case REF:
	    (*rpt_output)(stdout, "&%s = 0x%x", symname(s),
							s->symvalue.offset);
	    break;

	case PROC:
	case CSECTFUNC:
	case FUNC:
	    (*rpt_output)(stdout, "%s (0x%x):", symname(s), codeloc(s));
	    break;

	case TYPE:
	    (*rpt_output)(stdout, "%s", symname(s));
	    break;

	case ARRAY:
	    (*rpt_output)(stdout, "$string");
	    break;

	default:
	    (*rpt_output)(stdout, "[%s]", classname(s));
	    break;
    }
    (*rpt_output)( stdout, "\n");
}

/*
 * Print out the value on the top of the expression stack
 * in the format for the type of the given symbol.
 */

void asm_printval(Symbol s)
{
    register Symbol t;
    register Integer len;

    switch (s->class) {
	case ARRAY:
	    t = rtype(s->type);
	    if (t->class == RANGE and istypename(t->type, "$char")) {
		len = size(s);
		sp -= len;
		(*rpt_output)(stdout, "\"%.*s\"", len, sp);
	    } else {
		printarray(s);
	    }
	    break;

	default:
	    (*rpt_output)(stdout, "0x%x", pop(Integer));
	    break;
    }
}

/*
 * Treat subscripting as indirection through pointer to integer.
 */

Node asm_buildaref(Node a, Node slist)
{
    Symbol t, eltype;
    Node p, r;

    t = rtype(a->nodetype);
    eltype = t->type;
    p = slist->value.arg[0];
    r = build(O_MUL, p, build(O_LCON, (long) size(eltype)));
    r = build(O_ADD, build(O_RVAL, a), r);
    r->nodetype = eltype;
    return r;
}

/*
 * Evaluate a subscript index.  Assumes dimension is [0..n].
 */

void asm_evalaref(Symbol s, Address base, long i)
{
    Symbol t;

    t = rtype(s);
    push(long, base + i * size(t->type));
}

void asm_modinit ()
{
    addlangdefines(usetype);
}

boolean asm_hasmodules ()
{
    return false;
}

boolean asm_passaddr (Symbol param, Symbol exprtype)
{
    return false;
}

cases asm_foldnames ()
{
    return mixed;
}
