static char sccsid[] = "@(#)54	1.2.1.3  src/bos/usr/ccs/lib/libc/__wcswidth_latin.c, libccppc, bos411, 9428A410j 3/30/94 14:58:25";
/*
 * COMPONENT_NAME: (LIBCCPPC) LIBC Code-Point/Process-Code Conversion Functions
 *
 * FUNCTIONS: __wcswidth_latin
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991 , 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 *
 * FUNCTION: 
 *	    
 *
 * PARAMETERS: 
 *
 *
 * RETURN VALUE: 
 *
 *
 */
#define _ILS_MACROS
#include <stdlib.h>
#include <ctype.h>
/*
  returns the number of characters for a SINGLE-BYTE codeset
*/
int __wcswidth_latin(_LC_charmap_objhdl_t hdl, wchar_t *wcs, size_t n)
{
    int dispwidth;
    
    /**********
      if wcs is null or points to a null, return 0
    **********/
    if (wcs == (wchar_t *)NULL || *wcs == (wchar_t) '\0')
	return(0);

    /**********
      count the number of process codes in wcs, if
      there is a process code that is not printable, return -1
    **********/
    for (dispwidth=0; wcs[dispwidth] != (wchar_t)NULL && dispwidth<n; dispwidth++)
	if (!iswprint(wcs[dispwidth]))
	    return(-1);

    return(dispwidth); 
}
