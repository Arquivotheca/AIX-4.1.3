static char sccsid[] = "@(#)54	1.1  src/bos/usr/ccs/lib/libc/mbstomb.c, libcnls, bos411, 9428A410j 4/1/91 11:23:02";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: mbstomb
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <mbstr.h>
#include <stdlib.h>

/*
 *
 * FUNCTION: mbstomb
 *	    
 *
 * PARAMETERS: 
 *
 *
 * RETURN VALUE: 
 *
 *
 */

mbchar_t mbstomb(const char *s)
{
    int len;
    mbchar_t rc = 0;
    
    if (s == (char *)NULL)
	return ((mbchar_t)NULL);

    /**********
      if an invalid character is encountered, return
      the pointer to it
    **********/
    if ((len = mblen(s, MB_CUR_MAX)) == -1)
	return(0);

    while (len--) {
	rc = (rc << 8) | (unsigned char)*s++;
    }

    return (rc);
}
	    

    
    
    
    
