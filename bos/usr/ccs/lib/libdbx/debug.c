static char sccsid[] = "@(#)37	1.8.1.4  src/bos/usr/ccs/lib/libdbx/debug.c, libdbx, bos411, 9428A410j 9/27/93 18:10:39";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: debug, opname
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

/*
 *  Debug routines
 */

#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
#include "defs.h"
#include "tree.h"
#include "operators.h"
#include "eval.h"
#include "events.h"
#include "symbols.h"
#include "scanner.h"
#include "source.h"
#include "object.h"
#include "main.h"
#include "mappings.h"
#include "process.h"
#include "machine.h"
#include "debug.h"
#include <signal.h>

public boolean tracetree;	/* trace building of parse trees */
public boolean traceeval;	/* trace tree evaluation */

/*
 * Dynamically turn on/off a debug flag, or display some information.
 */

public debug (p)
Node p;
{
    int code;

    code = p->value.lcon;
    switch (code) {
	case 0:
	    puts("debugging flags:");
	    puts("    1        trace scanner return values");
	    puts("    2        trace breakpoints");
	    puts("    3        trace execution");
	    puts("    4        trace tree building");
	    puts("    5        trace tree evaluation");
	    puts(" -[12345]  turns off corresponding flag");
	    puts("    6        dump function table");
	    puts("    7        print ptrace information");
	    puts("   10        dump loader information");
	    puts("   11        dump file table");
	    puts("   12        dump symbol table (whatis)");

	    break;

	case 1:
	case -1:
#           ifdef LEXDEBUG
		lexdebug = (boolean) (code > 0);
#           else
		error("cannot debug scanner (not compiled with LEXDEBUG)");
#           endif
	    break;

	case 2:
	case -2:
	    tracebpts = (boolean) (code > 0);
	    break;

	case 3:
	case -3:
	    traceexec = (boolean) (code > 0);
	    break;

	case 4:
	case -4:
	    tracetree = (boolean) (code > 0);
	    break;

	case 5:
	case -5:
	    traceeval = (boolean) (code > 0);
	    break;

	case 6:
	    dumpfunctab();
	    break;

	case 7:
	    printptraceinfo();
	    break;

	case 10:
	    printloaderinfo();
	    break;

        case 11:
	    dumpFileTable();
	    break;

        case 12:
            dumpSymbolTable();
            break;

	default:
	    error("unknown debug flag");
	    break;
    }
}

private String leafname[] = {
    "nop", "name", "sym", "lcon", "ulcon", "llcon", "ullcon", 
    "ccon", "fcon", "qcon", "kcon", "qkcon", "scon",
    "setcon", "rval", "unrval", "index"
};

public String opname (op)
Operator op;
{
    String s;
    static char buf[100];

    switch (op) {
	case O_ITOF:
	    s = "itof";
	    break;

	case O_ITOQ:
	    s = "itoq";
	    break;

	case O_FTOQ:
	    s = "ftoq";
	    break;

	case O_BSET:
	    s = "bset";
	    break;

	case O_FREE:
	    s = "free";
	    break;

	case O_ENDX:
	    s = "endx";
	    break;

	case O_QLINE:
	    s = "qline";
	    break;

	default:
	    if (ord(op) <= ord(O_INDEX)) {
		s = leafname[ord(op)];
	    } else {
		s = opinfo[ord(op)].opstring;
		if (s == nil) {
		    sprintf(buf, "[op %d]", op);
		    s = buf;
		}
	    }
	    break;
    }
    return s;
}
