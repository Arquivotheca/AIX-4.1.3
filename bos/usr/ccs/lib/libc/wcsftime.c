static char sccsid[] = "@(#)69	1.3.1.2  src/bos/usr/ccs/lib/libc/wcsftime.c, libcfmt, bos411, 9428A410j 1/12/93 11:20:20";
/*
 * COMPONENT_NAME: LIBCFMT
 *
 * FUNCTIONS:  wcsftime
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/lc_sys.h>
#include <sys/localedef.h>
#include <time.h>

/*
 * FUNCTION: wcsftime() is a method driven function where the time formatting
 *	     processes are done in the method poninted by 
 *	     __lc_time->core.wcsftime.
 *           This function behaves the same as strftime() except the
 *           ouput buffer is wchar_t. Indeed, wcsftime_std() calls strftime()
 *           which performs the conversion in single byte first. Then the
 *           output from strftime() is converted to wide character string by
 *           mbstowcs().
 *
 * PARAMETERS:
 *           const char *ws  - the output data buffer in wide character
 *                             format.
 *           size_t maxsize  - the maximum number of wide character including
 *                             the terminating null to be output to ws buffer.
 *           const char *fmt - the format string which specifies the expected
 *                             format to be output to the ws buffer.
 *           struct tm *tm   - the time structure to provide specific time
 *                             information when needed.
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - if successful, it returns the number of bytes placed into the
 *             ws buffer not including the terminating null byte.
 *           - if fail for any reason, it returns 0.
 */


size_t wcsftime(wchar_t *ws, size_t maxsize, 
                    const char *format, const struct tm *timeptr)
{
	return _CALLMETH(__lc_time,__wcsftime)(__lc_time, ws, maxsize, 
					    format, timeptr);
}
