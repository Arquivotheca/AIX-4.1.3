static char sccsid[] = "@(#)49	1.2.1.1  src/bos/usr/ccs/lib/libc/__wcstombs_sb.c, libccppc, bos411, 9428A410j 5/25/92 13:44:07";
/*
 * COMPONENT_NAME: (LIBCCCPPC) LIBC Code-Point/Process-Code Conversion Functions
 *
 * FUNCTIONS: __wcstombs_sb
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
size_t __wcstombs_sb(_LC_charmap_objhdl_t hdl, char *s, const wchar_t *pwcs, 
		     size_t n)
{
    int len = n;
    char *pwcs0 = pwcs;

    /**********
      if s is a null pointer, just count the number of characters
      in pwcs
    **********/
    if (s == (char *)NULL) {
	while (*pwcs != '\0')
	      pwcs++;
	return(pwcs - pwcs0);
    }
    
    /**********
      only do n or less characters
    **********/
    while (len-- > 0) {
	*s = (char) *pwcs;

	/**********
	  if pwcs is null, return
	**********/
	if (*pwcs == '\0')
	    return(pwcs - pwcs0);

	/**********
	  increment s to the next character
	**********/
	s++;
	pwcs++;
    }

    /**********
      Ran out of room in wcs before null was hit on s, return n
    **********/
    return(n);
}


