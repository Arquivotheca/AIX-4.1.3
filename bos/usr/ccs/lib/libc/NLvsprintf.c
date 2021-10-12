static char sccsid[] = "@(#)61	1.4  src/bos/usr/ccs/lib/libc/NLvsprintf.c, libcprnt, bos411, 9428A410j 6/16/90 01:29:19";
/*
 * COMPONENT_NAME: (LIBCPRNT) Standard C Library Print Functions 
 *
 * FUNCTIONS: NLvsprintf 
 *
 * ORIGINS: 3, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1987, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <varargs.h>
#include <values.h>

extern int _doprnt();

/*                                                                    
 * FUNCTION: Writes output to to the stream pointed to by stream, under
 *           control of the string pointed to by format, that specifies
 *           how subsequent argumnts are converted for output.
 *
 * PARAMETERS: string - string to be printed to
 *             format - format used to print arguments
 *	       args   -   arguments to be printed
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	      If successful, returns number of characters printed
 *	      Otherwise returns negative value
 */

int
NLvsprintf (char *string, char *format, va_list args)
{
	FILE siop;		/* psuedo file for _doprnt */
	register int rc;	/* return code */

	siop._cnt = MAXINT;
	siop._base = siop._ptr = (unsigned char *)string;
	siop._flag = (_IOWRT|_IONOFD);
	rc = _doprnt(format, args, &siop);
	*siop._ptr = '\0';
	return (rc);
}
