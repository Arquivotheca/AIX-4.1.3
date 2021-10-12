static char sccsid[] = "@(#)88	1.7  src/bos/usr/ccs/lib/libc/NCisshift.c, libcnls, bos411, 9428A410j 2/26/91 12:43:56";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCisshift
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdlib.h>
#include <ctype.h>

int NCisshift(int c)
{
    int len;
    if (MB_CUR_MAX == 1)
	return(0);
    else {
	
	len = 0;
	/**********
	  Double byte [0x81-0x9f] [0x40-0x7e]
	  [0x81-0x9f] [0x80-0xfc]
	**********/
	if ((c >= 0x81) && (c <=0x9f))  
	    len = 2;
	
	/**********
	  Double byte [0xe0-0xfc] [0x40-0x7e]
	  [0xeo-0xfc] [0x80-0xfc]
	**********/
	else if (((c >= 0xe0) && (c <=0xfc))) 
	    len = 2;
	
	/**********
	  return the length of the multibyte character
	**********/
	return(len);
	
    }
}

