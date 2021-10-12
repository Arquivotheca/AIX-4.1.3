/* @(#)51	1.2.1.1  src/bos/usr/ccs/lib/libdbx/keywords.h, libdbx, bos411, 9428A410j 2/19/92 14:23:21 */
#ifndef _h_keywords
#define _h_keywords
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: (macros)
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

#include "scanner.h"
#include "tree.h"

extern enterkeywords(/*  */);
extern keywords_free(/*  */);
extern Token findkeyword (/* n, def */);
extern boolean findalias (/* n, pl, str */);
extern String keywdstring (/* t */);
extern alias (/* newcmd, args, str */);
extern unalias (/* n */);
extern defvar (/* n, val */);
extern Node findvar (/* n */);
extern boolean varIsSet (/* s */);
extern undefvar (/* n */);
extern printmparams (/* File, pl */);
#endif /* _h_keywords */
