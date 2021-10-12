static char sccsid[] = "@(#)29	1.1.1.3  src/bos/usr/ccs/lib/libc/__mbstopcs_sb.c, libccppc, bos411, 9428A410j 2/16/93 16:26:30";
/*
 * COMPONENT_NAME: (LIBCCPPC) LIBC Code-Point/Process-Code Conversion Functions
 *
 * FUNCTIONS: __mbstopcs_sb
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991 , 1993
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
size_t __mbstopcs_sb(_LC_charmap_objhdl_t hdl, wchar_t *pwcs, size_t pwcs_len, 
		     const char *s, size_t s_len, int stopchr, char **endptr, 
		     int *err)
{
    int cnt;

    cnt = 0;

    *err = 0;

    while(1) {
	/**********
	  if we have hit the stopchr, set the endpointer and break
	  out of the while
	**********/
	if (s[cnt] == stopchr) {
	    pwcs[cnt] = (wchar_t) s[cnt];
	    cnt++;
	    *endptr = &(s[cnt]);
	    break;
	}

	/**********
	  otherwise set pwcs and increment cnt
	**********/
	pwcs[cnt] = (wchar_t) s[cnt];
	cnt++;

	/**********
	  if the end of either array has been reached, set the endpointer
	  and break out of the while loop
	**********/
	if (cnt >= pwcs_len || cnt >= s_len) {
	    *endptr = &(s[cnt]);
	    break;
	}
	
    }

    /**********
      Return the number of characters converted from s to pwcs
    **********/
    return(cnt);
}


