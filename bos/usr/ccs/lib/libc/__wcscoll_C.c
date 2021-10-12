static char sccsid[] = "@(#)18	1.2.1.1  src/bos/usr/ccs/lib/libc/__wcscoll_C.c, libcstr, bos411, 9428A410j 5/25/92 14:07:36";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: __wcscoll_C
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
#include <string.h>

/*
 * FUNCTION: Compares the strings pointed to by s1 and s2, returning an
 *	     integer as follows:
 *
 *		Less than 0	If s1 is less than s2
 *		Equal to 0	If s1 is equal to s2
 *		Greater than 0	If s1 is greater than s2.
 *
 * PARAMETERS: (Uses file codes )
 *           _LC_collate_objhdl_t hdl - handle to collation information
 *	     wchar_t *wcs1       - first string
 *	     wchar_t *wcs2       - second string
 *
 * RETURN VALUE DESCRIPTIONS: Returns a negative, zero, or positive value
 *	     as described above.
 */
int __wcscoll_C(_LC_collate_objhdl_t hdl, const wchar_t *wcs1, 
		const wchar_t *wcs2)
{
    if(wcs1 == wcs2)
	return(0);
    while(*wcs1 == *wcs2++)
	if(*wcs1++ == '\0')
	    return(0);
    
    return(*wcs1 - *--wcs2);
}
