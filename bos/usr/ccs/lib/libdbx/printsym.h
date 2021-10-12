/* @(#)73	1.6.1.3  src/bos/usr/ccs/lib/libdbx/printsym.h, libdbx, bos411, 9428A410j 6/8/93 14:10:46 */
#ifndef _h_printsym
#define _h_printsym
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: (macros) FPUTS, PUTCHAR
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
extern String classname (/* s */);
extern printentry (/* s */);
extern printexit (/* s */);
extern printcall (/* s, t */);
extern printrtn (/* s */);
extern printparams (/* f, frame */);
extern Boolean should_print (/* s */);
extern printparamv (/* p, frame */);
extern printv (/* s, frame */);
extern printname (/* f, s */);
extern printwhich (/* f, s */);
extern printwhereis (/* f, s */);
extern printouter (/* f, s */);
extern printdecl (/* s */);
extern psym (/* s */);
extern printval (/* t, indent */);
extern printrecord (/* s */);
extern printarray (/* a */);
extern printchar (/* c */);
extern printRangeVal (/* val, t */);
extern printEnum (/* i, t */);
extern printString (/* addr, quotes */);
extern printquad (/* report, f, q */);

#define PRINTF		(*(rpt_output)) (stdout,
#define PUTCHAR(cc)	(*(rpt_output)) (stdout, "%c", cc)
#define FPUTS(ss)	(*(rpt_output)) (stdout, "%s", ss)
#endif /* _h_printsym */
