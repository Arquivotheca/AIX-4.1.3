static char sccsid[] = "@(#)40	1.3.1.1  src/bos/usr/ccs/lib/libc/__mblen_sb.c, libccppc, bos411, 9428A410j 5/25/92 13:42:59";
/*
 * COMPONENT_NAME: (LIBCCCPPC) LIBC Code-Point/Process-Code Conversion Functions
 *
 * FUNCTIONS: __mblen_sb
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
#include <sys/errno.h>
/*
  returns the number of characters for a SINGLE-BYTE codeset
*/
size_t __mblen_sb(_LC_charmap_objhdl_t hdl, char *s, size_t len)
{
    /**********
      If length == 0 return -1
    **********/
    if (len < 1) {
	errno = EILSEQ;
	return((size_t)-1);
    }

    /**********
      if s is NULL or points to a NULL return 0
    **********/
    if (s == (char *)NULL || *s == '\0')
	return(0);

    return(1);
}
    
		
