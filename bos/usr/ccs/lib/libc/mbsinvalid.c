static char sccsid[] = "@(#)53	1.1  src/bos/usr/ccs/lib/libc/mbsinvalid.c, libcnls, bos411, 9428A410j 4/1/91 11:23:00";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: mbsinvalid
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
 * FUNCTION: mbsinvalid
 *	    
 *
 * PARAMETERS: 
 *
 *
 * RETURN VALUE: 
 *
 *
 */

char *mbsinvalid(const char *s)
{
    int len;
    int rc;
    
    if (s == (char *)NULL)
	return ((char *)NULL);

    /**********
      if an invalid character is encountered, return
      the pointer to it
    **********/
    while (*s) {
	if ((rc = mblen(s, MB_CUR_MAX)) == -1)
	    return(s);
	s += rc;
    }

    /**********
      if we made it here, all of the characters are valid
    **********/
    return ((char *)NULL);
}
	    

    
    
    
    
