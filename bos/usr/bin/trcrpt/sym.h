/* @(#)43	1.8  src/bos/usr/bin/trcrpt/sym.h, cmdtrace, bos411, 9428A410j 6/15/90 23:52:49 */

/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: header file for access to symbol table
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

struct symbol {
	struct symbol *s_next;
	int   s_type;		/* token IFORMAT, ... */
	char *s_name;
	char  s_fmtcode;	/* A, X, D, usually matching the format string itself */
	char  s_fld1;		/* 1,2,4 for fixed length formats */
	char  s_xcode;		/* 0, 1, or 2 */
};
typedef struct symbol symbol;
extern symbol *reslookup();
extern symbol *resinstall();
extern symbol *lookup();
extern symbol *install();

