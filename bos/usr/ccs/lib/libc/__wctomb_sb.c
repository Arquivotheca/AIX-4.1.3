static char sccsid[] = "@(#)57	1.2.1.1  src/bos/usr/ccs/lib/libc/__wctomb_sb.c, libccppc, bos411, 9428A410j 5/25/92 13:44:29";
/*
 * COMPONENT_NAME: (LIBCCCPPC) LIBC Code-Point/Process-Code Conversion Functions
 *
 * FUNCTIONS: __wctomb_sb
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
#include <stdlib.h>
#include <ctype.h>
int __wctomb_sb(_LC_charmap_objhdl_t hdl, char *s, wchar_t pwc)
{

    /**********
      if s is NULL or points to a NULL return 0
    **********/
    if (s == (char *)NULL)
	return(0);

    if (pwc > 255)
	return(-1);

    s[0]= (char) pwc;

    return(1);
}
    
		
