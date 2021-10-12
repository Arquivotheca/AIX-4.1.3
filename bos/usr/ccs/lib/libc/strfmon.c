static char sccsid[] = "@(#)64	1.3.1.3  src/bos/usr/ccs/lib/libc/strfmon.c, libcfmt, bos411, 9428A410j 10/20/93 14:31:54";
/*
 * COMPONENT_NAME: LIBCFMT
 *
 * FUNCTIONS:  strfmon
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/localedef.h>
#include <sys/lc_sys.h>
#include <monetary.h>
#include <stdarg.h>

/*
 * FUNCTION: strfmon() is a method driven fucntion where the actual monetary
 *	     formatting is done by a method pointed to by the current locale.
 *           The method formats a list of values and outputs them to the
 *           output buffer s. The values returned are affected by the format
 *           string and the setting of the locale category LC_MONETARY.
 *
 * PARAMETERS:
 *           char *s - location of returned string
 *           size_t maxsize - maximum length of output including the null
 *                            termination character.
 *           char *format - format that montary value is to be printed out
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - returns the number of bytes that comprise the return string
 *             excluding the terminating null character.
 *           - returns 0 if s is longer than maxsize
 */

ssize_t strfmon(char *s, size_t maxsize, const char *format, ...)
{
	va_list ap;
	int	i;

	va_start(ap, format);
	i = _CALLMETH(__lc_monetary,__strfmon)(__lc_monetary, s, maxsize,
					    format, ap);
	va_end(ap);
	return (i);
}

