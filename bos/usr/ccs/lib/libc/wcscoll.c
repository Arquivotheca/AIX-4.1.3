static char sccsid[] = "@(#)17	1.3.1.2  src/bos/usr/ccs/lib/libc/wcscoll.c, libcstr, bos411, 9428A410j 1/12/93 11:20:16";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: wcscoll
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <sys/localedef.h>
#include <sys/lc_sys.h>
#include <string.h>
#include <stdlib.h>

/*
 * FUNCTION: Compares the strings pointed to by ws1 and ws2, returning an
 *	     integer as follows:
 *
 *		Less than 0	If ws1 is less than ws2
 *		Equal to 0	If ws1 is equal to ws2
 *		Greater than 0	If ws1 is greater than ws2.
 *
 *           Calls the collation methods for the current locale.
 *
 * NOTES:    The ANSI Programming Language C standard requires this routine.
 *
 * PARAMETERS:
 *	     wchar_t *s1 - first string
 *	     wchar_t *s2 - second string
 *
 * RETURN VALUE DESCRIPTIONS: Returns a negative, zero, or positive value
 *	     as described above.
 */

int wcscoll(const wchar_t *wcs1, const wchar_t *wcs2)
{
    return _CALLMETH(__lc_collate,__wcscoll)(__lc_collate,wcs1, wcs2);
}    
