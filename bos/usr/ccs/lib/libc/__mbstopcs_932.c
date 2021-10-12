static char sccsid[] = "@(#)27	1.2.1.2  src/bos/usr/ccs/lib/libc/__mbstopcs_932.c, libccppc, bos411, 9428A410j 2/10/93 15:56:03";
/*
 * COMPONENT_NAME: (LIBCCPPC) LIBC Code-Point/Process-Code Conversion Functions
 *
 * FUNCTIONS: __mbstopcs_932
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
size_t __mbstopcs_932(_LC_charmap_objhdl_t hdl, wchar_t *pwcs, size_t pwcs_len, 
			const char *s, size_t s_len, int stopchr, char **endptr, int *err)
{
    int cnt = 0;
    int pwcs_cnt = 0;
    int s_cnt = 0;

    /**********
      err is 0 if everything works
    **********/
    *err = 0;
    
    while (1) {

	/**********
	  if there is no more room for process code
	  or all the characters in s have been processed
	  set the end pointer and break out of the while
	**********/
	if (pwcs_cnt >= pwcs_len || s_cnt >= s_len) {
	    *endptr = &(s[s_cnt]);
	    break;
	}

	/**********
	  convert s to process code and increment s by the number               
	  of bytes.  If the conversion failed, set the endpointer
	  the the start of the character that failed, and
	  break out of the while. err will be set by __mbtopc.
	**********/
	if ((cnt = __mbtopc(&(pwcs[pwcs_cnt]), &(s[s_cnt]), (s_len - s_cnt), err)) == 0) {
	    *endptr = &(s[s_cnt]);
	    break;
	}

	/**********
	  increment the process code counter
	**********/
	pwcs_cnt++;

	/**********
	  if we hit stopchr in s, Set endpointer to the character after
	  the stopchr and break out of the while 
        **********/
	if ( s[s_cnt] == stopchr) {
	    *endptr = &(s[s_cnt+1]);
	    break;
	}

	/**********
	  increment the string counter
	**********/
	s_cnt += cnt;
    }

    /**********
      ran out of space before hiting a stopchar in s
    **********/
    return (pwcs_cnt);
}
