static char sccsid[] = "@(#)42	1.3.1.2  src/bos/usr/ccs/lib/libc/__mbstowcs_eucjp.c, libccppc, bos411, 9428A410j 1/12/93 11:09:53";
/*
 * COMPONENT_NAME: (LIBCCPPC) LIBC Code-Point/Process-Code Conversion Functions
 *
 * FUNCTIONS: __mbstowcs_eucjp
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
size_t __mbstowcs_eucjp(_LC_charmap_objhdl_t hdl, wchar_t *pwcs, const char *s, 
			size_t n)
{
    int len = n;
    int rc;
    int cnt;
    wchar_t *pwcs0 = pwcs;

    /**********
      if pwcs is a null pointer, just count the number of characters
      in s
    **********/
    if (pwcs == (wchar_t *)NULL) {
	cnt = 0;
	while (*s != '\0') {
	      if ((rc = mblen(s, MB_CUR_MAX)) == -1)
		    return(-1);
	      cnt++;
	      s += rc;
	}
	return(cnt);
    }

    while (len-- > 0) {

	/**********
	  if we hit a NULL in s, add a NULL to pwcs and return
	  the length of pwcs
        **********/
	if ( *s == '\0') {
	    *pwcs = (wchar_t) '\0';
	    return (pwcs - pwcs0);
	}

	/**********
	  convert s to process code and increment s by the number
	  of bytes is took
	**********/
	if ((cnt = mbtowc(pwcs, s, MB_CUR_MAX)) < 0)
	    return(-1);
	s += cnt;
	++pwcs;
    }

    /**********
      ran out of space before hiting a NULL in s, return n
    **********/
    return (n);
}
