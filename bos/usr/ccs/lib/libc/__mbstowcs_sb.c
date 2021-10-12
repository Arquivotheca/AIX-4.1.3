static char sccsid[] = "@(#)43	1.2.1.1  src/bos/usr/ccs/lib/libc/__mbstowcs_sb.c, libccppc, bos411, 9428A410j 5/25/92 13:43:24";
/*
 * COMPONENT_NAME: (LIBCCCPPC) LIBC Code-Point/Process-Code Conversion Functions
 *
 * FUNCTIONS: __mbstowcs_sb
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
size_t __mbstowcs_sb(_LC_charmap_objhdl_t hdl, wchar_t *pwcs, const char *s, 
		     size_t n)
{
    int len = n;
    char *s0 = s;

    /**********
      if pwcs is a null pointer, just count the number of characters
      in s
    **********/
    if (pwcs == (wchar_t *)NULL) {
	while (*s != '\0')
	      s++;
	return(s - s0);
    }
    
    /**********
      only do n or less characters
    **********/
    while (len-- > 0) {
	*pwcs = (wchar_t) *s;

	/**********
	  if s is null, return
	**********/
	if (*s == '\0')
	    return(s - s0);

	/**********
	  increment s to the next character
	**********/
	pwcs++;
	s++;
    }

    /**********
      Ran out of room in wcs before null was hit on s, return n
    **********/
    return(n);
}


