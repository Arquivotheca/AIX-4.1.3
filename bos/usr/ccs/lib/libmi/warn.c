static char sccsid[] = "@(#)97	1.1  src/bos/usr/ccs/lib/libmi/warn.c, cmdpse, bos411, 9428A410j 5/7/91 13:08:43";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1990  Mentat Inc.
 ** warn.c 2.2, last change 4/9/91
 **/


#include <pse/common.h>
#include <stdio.h>
#ifdef	USE_STDARG
#include <stdarg.h>
#else
#include <varargs.h>
#endif

extern	void	exit(   int status   );
extern	char	* get_program_name();

void
#ifdef	USE_STDARG
warn (char * fmt, ... )
#else
warn (fmt, va_alist)
	char	* fmt;
	va_dcl
#endif
{
	va_list	ap;

	if (fmt  &&  *fmt) {
#ifdef	USE_STDARG
		va_start(ap, fmt);
#else
		va_start(ap);
#endif
		(void)fprintf(stderr, "%s: ", get_program_name());
		(void)vfprintf(stderr, fmt, ap);
		va_end(ap);
		while (*fmt)
			fmt++;
		if (fmt[-1] != '\n')
			(void)fprintf(stderr, "\n");
	}
}
