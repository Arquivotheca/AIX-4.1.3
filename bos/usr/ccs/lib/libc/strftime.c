static char sccsid[] = "@(#)68	1.3.1.3  src/bos/usr/ccs/lib/libc/strftime.c, libcfmt, bos411, 9428A410j 10/20/93 14:31:57";
/*
 * COMPONENT_NAME: LIBCFMT
 *
 * FUNCTIONS:  strftime
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

#include <sys/lc_sys.h>
#include <sys/localedef.h>
#include <time.h>

/*
 * FUNCTION: strftime() is a method driven function where the time formatting
 *	     processes are done the method points by __lc_time->core.strftime.
 *	     It formats the date and ouput to the output buffer s. The values
 *           returned are affected by the setting of the locale category
 *           LC_TIME and the time information in the tm time structure.
 *
 * PARAMETERS:
 *           char *s - location of returned string
 *           size_t maxsize - maximum length of output string
 *           char *format - format that date is to be printed out
 *           struct tm *timeptr - date to be printed
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - returns the number of bytes that comprise the return string
 *             excluding the terminating null character.
 *           - returns 0 if s is longer than maxsize
 */

size_t strftime(char *s, size_t maxsize, const char *format, 
                const struct tm *timeptr)
{
	return _CALLMETH(__lc_time,__strftime)(__lc_time, s, 
					    maxsize, format, 
					    timeptr);
}
