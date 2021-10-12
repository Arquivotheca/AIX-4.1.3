static char sccsid[] = "@(#)40	1.1  src/bos/diag/tu/fddi/detrace.c, tu_fddi, bos411, 9428A410j 7/9/91 12:38:56";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: detrace
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************

Function(s) Trace Function

Module Name :  detrace.c
SCCS ID     :  1.5

Current Date:  5/23/90, 11:17:44
Newest Delta:  1/19/90, 16:24:00

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
