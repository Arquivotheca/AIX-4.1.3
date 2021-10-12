/* @(#)59	1.6.1.3  src/bos/usr/ccs/lib/libdbx/main.h, libdbx, bos411, 9434B411a 8/20/94 16:07:37 */
#ifndef _h_main
#define _h_main
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: (macros) isterm
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
#define isterm(file)	(interactive or isatty(fileno(file)))

extern boolean coredump;		/* true if using a core dump */
extern boolean runfirst;		/* run program immediately */
extern boolean interactive;		/* standard input IS a terminal */
extern boolean lexdebug;		/* trace scanner return values */
extern boolean tracebpts;		/* trace create/delete breakpoints */
extern boolean traceexec;		/* trace execution */
extern boolean tracesyms;		/* print symbols are they are read */
extern boolean traceblocks;		/* trace blocks while reading symbols */
extern boolean vaddrs;			/* map addresses through page tables */
extern boolean quiet;			/* don't print heading */
extern boolean attach;  		/* dbx has attached to a process */
extern boolean reattach;  		/* True if xde reattached a process */
extern fork_type multproc; 		/* Multi-process mode toggle. */
extern boolean norun;  			/* Do not allow run or rerun of proc */
extern boolean isXDE;			/* true if running XDE */
extern String  graphical_debugger;	/* name of the graphical debugger dbx is running under */
extern File corefile;			/* File id of core dump */
extern integer versionNumber ;
extern printheading (/*  */);
extern init(/*  */);
extern reinit(/* argv, infile, outfile */);
extern erecover(/*  */);
extern savetty(/* f, t */);
extern restoretty(/* f, t */);
extern quit(/* r */);
#endif /* _h_main */
