static char sccsid[] = "@(#)67	1.3.1.3  src/bos/usr/ccs/lib/libc/strptime.c, libcfmt, bos411, 9428A410j 10/20/93 14:32:01";
/*
 * COMPONENT_NAME: LIBCFMT
 *
 * FUNCTIONS:  strptime
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
 * FUNCTION: strptime() is a method driven functions where the time formatting
 *	     processes are done in the method points by 
 *	     __lc_time->core.strptime.
 *           It parse the input buffer according to the format string. If
 *           time related data are recgonized, updates the tm time structure
 *           accordingly.
 *
 * PARAMETERS:
 *           const char *buf - the input data buffer to be parsed for any
 *                             time related information.
 *           const char *fmt - the format string which specifies the expected
 *                             format to be input from the input buf.
 *           struct tm *tm   - the time structure to be filled when 
 *                             appropriate time related information is found.
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - if successful, it returns the pointer to the character after
 *             the last parsed character in the input buf string.
 *           - if fail for any reason, it returns a NULL pointer.
 */

char *strptime(const char *buf, const char *fmt, struct tm *tm)
{
	return _CALLMETH(__lc_time,__strptime)(__lc_time,buf,fmt,tm); 
}
