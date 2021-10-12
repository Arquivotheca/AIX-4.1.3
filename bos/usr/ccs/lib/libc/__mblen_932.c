static char sccsid[] = "@(#)38	1.5.1.1  src/bos/usr/ccs/lib/libc/__mblen_932.c, libccppc, bos411, 9428A410j 5/25/92 13:42:52";
/*
 * COMPONENT_NAME: (LIBCCCPPC) LIBC Code-Point/Process-Code Conversion Functions
 *
 * FUNCTIONS: __mblen_932
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
  Returns the number of bytes that comprise a multi-byte string for the PC932 codeset

  |  process code   |   s[0]    |   s[1]    |
  +-----------------+-----------+-----------+
  | 0x0000 - 0x007f | 0x00-0x7f |    --     |
  | 0x007e - 0x00a0 |    --     |    --     |
  | 0x00a1 - 0x00df | 0xa1-0xdf |    --     |
  | 0x00e0 - 0x00ff |    --     |    --     |
  | 0x0100 - 0x189e | 0x81-0x9f | 0x40-0xfc (excluding 0x7f)
  | 0x189f - 0x303b | 0xe0-0xfc | 0xa1-0xfe (excluding 0x7f)
  +-----------------+-----------+-----------+
  
*/

size_t __mblen_932(_LC_charmap_objhdl_t hdl, char *s, size_t maxlen)
{
    /**********
      If length == 0 return -1
    **********/
    if (maxlen < 1) {
    	errno = EILSEQ;
	return((size_t)-1);
    }

    /**********
      if s is NULL or points to a NULL return 0
    **********/
    if (s == (char *)NULL || *s == '\0')
	return(0);

    /**********
      single byte (<0x7f)
    **********/
    if ((unsigned char)s[0] <= 0x7f)
	return(1);

    /**********
      Double byte [0x81-0x9f] [0x40-0x7e]
                  [0x81-0x9f] [0x80-0xfc]
    **********/
    else if ((unsigned char)s[0] >= 0x81 && (unsigned char)s[0] <=0x9f) {
	if ((maxlen >=2) && ((unsigned char)s[1] >= 0x40 && (unsigned char)s[1] <= 0xfc && (unsigned char)s[1] != 0x7f))
	    return(2);
    }

    /**********
      Double byte [0xe0-0xfc] [0x40-0x7e]
                  [0xeo-0xfc] [0x80-0xfc]
    **********/
    else if ((unsigned char)s[0] >= 0xe0 && (unsigned char)s[0] <=0xfc) {
	if ((maxlen >= 2) && ((unsigned char)s[1] >= 0x40 && (unsigned char)s[1] <= 0xfc && (unsigned char)s[1] != 0x7f))
	    return(2);
    }

    /**********
      Single Byte 0xa1 - 0xdf
    **********/
    else if ((unsigned char)s[0] >= 0xa1 && (unsigned char)s[0] <= 0xdf) {
	return(1);
    }

    /**********
      If we are here, then this is an invalid multi-byte character
    **********/
    errno = EILSEQ;
    return(-1);
    
}
    
		
