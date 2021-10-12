static char sccsid[] = "@(#)33	1.1.1.1  src/bos/usr/ccs/lib/libc/__mbtopc_sb.c, libccppc, bos411, 9428A410j 5/25/92 13:43:38";
/*
 * COMPONENT_NAME: (LIBCCCPPC) LIBC Code-Point/Process-Code Conversion Functions
 *
 * FUNCTIONS: __mbtopc_sb
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
size_t __mbtopc_sb(_LC_charmap_objhdl_t hdl, wchar_t *pwc, char *s, size_t len, 
		   int *err)
{
    /**********
      If length == 0 return -1
    **********/
    if (len < 1) {
	*err = 1;
	return((size_t)0);
    }

    /**********
      if s is NULL or points to a NULL return 0
    **********/
    if (s == (char *)NULL || *s == '\0') {
	*err = -1;
	return(0);
    }

    /**********
      If pwc is NULL, just return the number of bytes
      otherwise set pwc to s
    **********/
    if (pwc != (wchar_t *)NULL)
	*pwc = (wchar_t)*s;
    return(1);
}
    
		



