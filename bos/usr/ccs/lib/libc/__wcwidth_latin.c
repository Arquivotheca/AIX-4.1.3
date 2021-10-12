static char sccsid[] = "@(#)62	1.2.1.3  src/bos/usr/ccs/lib/libc/__wcwidth_latin.c, libccppc, bos411, 9428A410j 3/30/94 14:59:08";
/*
 * COMPONENT_NAME: (LIBCCPPC) LIBC Code-Point/Process-Code Conversion Functions
 *
 * FUNCTIONS: __wcwidth_latin
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
int __wcwidth_latin(_LC_charmap_objhdl_t hdl, wchar_t wc)
{
    /**********
      if wc is null, return 0
    **********/
    if (wc == (wchar_t) '\0')
	return(0);

    if (! iswprint(wc))
	return(-1);
    
    /**********
      single-display width
    **********/
    return(1);

}
    
		
