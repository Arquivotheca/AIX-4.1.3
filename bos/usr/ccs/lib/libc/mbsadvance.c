static char sccsid[] = "@(#)52	1.1  src/bos/usr/ccs/lib/libc/mbsadvance.c, libcnls, bos411, 9428A410j 4/1/91 11:22:59";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: mbsadvance
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
 * FUNCTION: mbsadvance
 *	    
 *
 * PARAMETERS: 
 *
 *
 * RETURN VALUE: 
 *
 *
 */

char *mbsadvance(const char *s)
{
    int len;
    
    if ((s == (char *)NULL) || (*s == '\0'))
	return ((char *)NULL);

    len = mblen(s, MB_CUR_MAX);

    /**********
      if s is not a valid mbs, then assume a length of one
    **********/
    if (len > 0)
	s += len;
    else
	s++;

    return (s);
}
	    

    
    
    
    
