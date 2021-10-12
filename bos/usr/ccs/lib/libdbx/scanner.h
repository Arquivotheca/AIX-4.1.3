/* @(#)79	1.2  src/bos/usr/ccs/lib/libdbx/scanner.h, libdbx, bos411, 9428A410j 6/15/90 20:41:59 */
#ifndef _h_scanner
#define _h_scanner
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: 
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
typedef int Token;

#define MAXLINESIZE 10240

extern String initfile ;
extern boolean chkalias;
extern char scanner_linebuf[MAXLINESIZE];
extern scanner_init(/*  */);
extern Token yylex(/*  */);
extern insertinput (/* s */);
extern yyerror(/* s */);
extern gobble (/*  */);
extern setinput(/* filename */);
extern Boolean popinput(/*  */);
extern Boolean isstdin(/*  */);
extern shellline(/*  */);
extern beginshellmode(/*  */);
extern print_token(/* f, t */);
#endif /* _h_scanner */
