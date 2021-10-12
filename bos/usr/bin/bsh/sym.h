/* @(#)19	1.13  src/bos/usr/bin/bsh/sym.h, cmdbsh, bos411, 9428A410j 9/1/93 17:37:01 */
/*
 * COMPONENT_NAME: (CMDBSH) Bourne shell and related commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * 1.7  com/cmd/sh/sh/sym.h, cmdsh, bos324
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1
 */

/* symbols for parsing */
#define SYMFLG	0400
#define DOSYM	0405
#define BRSYM	0406
#define INSYM	0412
#define CASYM	0417
#define FISYM	0420
#define ELSYM	0421
#define EFSYM	0422
#define UNSYM	0427
#define WHSYM	0433
#define FORSYM	0435
#define IFSYM	0436
#define ODSYM	0441
#define THSYM	0444
#define ESSYM	0442
#define KTSYM	0450

#define SYMREP	04000
#define EOFSYM	02000

#define ECSYM	(SYMREP|';')
#define ANDFSYM	(SYMREP|'&')
#define ORFSYM	(SYMREP|'|')
#define APPSYM	(SYMREP|'>')
#define DOCSYM	(SYMREP|'<')

/* arg to `cmd' */
#define NLFLG	1
#define MTFLG	2

/* for peekc */
#define MARK	0100000

/* odd char */
#define DQUOTE	'"'
#define SQUOTE	'`'
#define LITERAL	'\''
#define DOLLAR	'$'
#define ESCAPE	'\\'
#define BRACE	'{'
#define COMCHAR '#'
