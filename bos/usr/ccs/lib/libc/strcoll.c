static char sccsid[] = "@(#)38	1.21.1.2  src/bos/usr/ccs/lib/libc/strcoll.c, libcstr, bos411, 9428A410j 1/12/93 11:19:19";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: strcoll
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
#include <sys/lc_sys.h>		/* not shipped */
#include <string.h>
/*
 * FUNCTION: Compares the strings pointed to by s1 and s2, returning an
 *	     integer as follows:
 *
 *		Less than 0	If s1 is less than s2
 *		Equal to 0	If s1 is equal to s2
 *		Greater than 0	If s1 is greater than s2.
 *
 *           Calls the collation methods for the current locale.
 *
 * NOTES:    The ANSI Programming Language C standard requires this routine.
 *
 * PARAMETERS: (Uses file codes )
 *	     char *s1 - first string
 *	     char *s2 - second string
 *
 * RETURN VALUE DESCRIPTIONS: Returns a negative, zero, or positive value
 *	     as described above.
 */

int strcoll(const char *s1, const char *s2)
{
    return _CALLMETH(__lc_collate,__strcoll)(__lc_collate,s1, s2);
}    
