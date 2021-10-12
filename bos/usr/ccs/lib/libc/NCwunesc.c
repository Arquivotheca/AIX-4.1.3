static char sccsid[] = "@(#)36	1.3  src/bos/usr/ccs/lib/libc/NCwunesc.c, libcnls, bos411, 9428A410j 6/11/91 09:46:09";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCwunesc
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <NLctype.h>
#include <stdlib.h>

/*
 * FUNCTION:
 * 	Translate the character escape sequence at src to a single NLchar at dest.
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	Return the number of NLchars translated.
 */


int
NCwunesc(src, dest)
register NLchar *src;
register NLchar *dest;
{
    char carray[NLESCMAX];
    int length;

    /**********
      convert src to chars
    **********/
    (void)wcstombs(carray, src, NLESCMAX);

    return(NCunesc (carray, dest));
}
 
