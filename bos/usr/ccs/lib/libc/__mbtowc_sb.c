static char sccsid[] = "@(#)46	1.3.1.2  src/bos/usr/ccs/lib/libc/__mbtowc_sb.c, libccppc, bos411, 9428A410j 4/26/94 15:25:40";
/*
 * COMPONENT_NAME: (LIBCCCPPC) LIBC Code-Point/Process-Code Conversion Functions
 *
 * FUNCTIONS: __mbtowc_sb
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991 , 1994
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
#include <sys/errno.h>
int __mbtowc_sb(_LC_charmap_objhdl_t hdl, wchar_t *pwc, const char *s, 
	        size_t len)
{
    /**********
      if s is NULL return 0
    **********/
    if (s == (char *)NULL)
	return(0);

    /**********
      If length == 0 return -1
    **********/
    if (len < 1) {
	errno = EILSEQ;
	return((size_t)-1);
    }

    /**********
      if pwc is not NULL pwc to s
      length is 1 unless NULL which has length 0
    **********/
    if (pwc != (wchar_t *)NULL)
	*pwc = (wchar_t)*s;
    if (s[0] != '\0')
	return(1);
    else
        return(0);
}
    
		
