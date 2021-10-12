static char sccsid[] = "src/bos/diag/tu/tok/detrace.c, tu_tok, bos411, 9428A410j 6/20/91 12:41:36";
/*
 * COMPONENT_NAME: (TU_TOK) Token Test Unit
 *
 * FUNCTIONS: detrace
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************

Function(s) Trace Function

Module Name :  detrace.c
SCCS ID     :  1.4

Current Date:  6/20/91, 10:14:44
Newest Delta:  1/19/90, 16:48:35

Function used for debugging code.  Outputs print statement to stderr
and optionally prompts user depending on first arg passed (prompts
if non-zero, else does not prompt).

*****************************************************************************/
#include <stdio.h>
#include <varargs.h>

void detrace (va_alist)
   va_dcl
   {
	va_list args;
	int pause_flg;
	char *fmt_string;
	char buf[80];

	va_start(args);
	pause_flg = va_arg(args, int);
	fmt_string = va_arg(args, char *);
	vfprintf(stderr, fmt_string, args);
	fflush(stderr);
	if (pause_flg)
	   {
		fprintf(stderr,"\n======> Hit RETURN to continue ======>\n");
		fflush(stderr);
		(void) fgets(buf, 80, stdin);
	   }
	va_end(args);
   }
