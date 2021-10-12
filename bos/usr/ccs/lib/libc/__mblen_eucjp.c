static char sccsid[] = "@(#)39	1.4.1.1  src/bos/usr/ccs/lib/libc/__mblen_eucjp.c, libccppc, bos411, 9428A410j 5/25/92 13:42:55";
/*
 * COMPONENT_NAME: (LIBCCCPPC) LIBC Code-Point/Process-Code Conversion Functions
 *
 * FUNCTIONS: __mblen_eucjp
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
  Returns the number of bytes that comprise a multi-byte string for the JP_EUC codeset

  The algorithm for this conversion is:
  s[0] < 0x9f:  1 byte
  s[0] = 0x8e:  2 bytes
  s[0] = 0x8f   3 bytes
  s[0] > 0xa1   2 bytes

  |  process code   |   s[0]    |   s[1]    |   s[2]    |
  +-----------------+-----------+-----------+-----------+
  | 0x0000 - 0x009f | 0x00-0x9f |    --     |    --     |
  | 0x00a0 - 0x00ff |   --      |    --     |    --     |
  | 0x0100 - 0x015d | 0x8e      | 0xa1-0xfe |    --     |
  | 0x015e - 0x303b | 0xa1-0xfe | 0xa1-0xfe |    --     |
  | 0x303c - 0x5f19 | 0x8f      | 0xa1-0xfe | 0xa1-0xfe |
  +-----------------+-----------+-----------+-----------+
  
*/

size_t __mblen_eucjp(_LC_charmap_objhdl_t hdl, char *s, size_t maxlen)
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
      single byte (<=0x8d)
    **********/
    if ((unsigned char)s[0] <= 0x8d)
	return(1);

    /**********
      Double Byte 8e[a1-fe]
    **********/
    else if ((unsigned char)s[0] == 0x8e) {
	if (maxlen >= 2 && (unsigned char)s[1] >=0xa1 && (unsigned char)s[1] <=0xfe)
	   return(2); 
    }

    /**********
      Triple Byte 8f[a1-fe][a1-fe]
    **********/
    else if ((unsigned char)s[0] == 0x8f) {
	if(maxlen >=3 && ((unsigned char)s[1] >=0xa1 && (unsigned char)s[1] <=0xfe) &&
	   ((unsigned char)s[2] >=0xa1 && (unsigned char)s[2] <= 0xfe))
	    return(3);
    }


    /**********
      Single Byte [90-9f]
    **********/
    else if ((unsigned char)s[0] <= 0x9f)
	return(1);

    
    /**********
      Double Byte [a1-fe][a1-fe]
    **********/
    else if ((unsigned char)s[0] >= 0xa1) {
	if (maxlen >=2 && ((unsigned char)s[0] <= 0xfe) &&
	    ((unsigned char)s[1] >=0xa1 && (unsigned char)s[1] <= 0xfe))
	    return(2);
    }

    /**********
      if we are here, then invalid multi-byte character
    **********/
    errno = EILSEQ;
    return((size_t)-1);
    
}
