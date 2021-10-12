static char sccsid[] = "@(#)29	1.1  src/bos/usr/lib/nls/loc/locale/ko_KR/__mblen_euckr.c, libmeth, bos411, 9428A410j 5/25/92 15:58:56";
/*
 * COMPONENT_NAME:	LIBMETH
 *
 * FUNCTIONS: __mblen_euckr
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
#include <stdlib.h>
#include <ctype.h>
#include <sys/errno.h>
/*
  Returns the number of bytes that comprise a multi-byte string for the KR_EUC codeset

  The algorithm for this conversion is:
  s[0] < 0x80:  1 byte
  s[0] > 0xa1   2 bytes

  +-----------------+-----------+-----------+
  |  process code   |   s[0]    |   s[1]    |
  +-----------------+-----------+-----------+
  | 0x0000 - 0x007f | 0x00-0x7f |    --     |
  | 0x0080 - 0x00ff |    --     |    --     |
  | 0x0100 - 0x2383 | 0xa1-0xfe | 0xa1-0xfe |
  +-----------------+-----------+-----------+
  
*/

size_t __mblen_euckr(_LC_charmap_objhdl_t handle, char *s, size_t maxlen)
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
      single byte (<=0x7f)
    **********/
    if ((unsigned char)s[0] <= 0x7f)
	return(1);

    /**********
      Double Byte [a1-fd][a1-fe]
    **********/
    else if ((unsigned char)s[0] >= 0xa1) {
	if (maxlen >=2 && ((unsigned char)s[0] <= 0xfe) && ((unsigned char)s[1] >=0xa1 && (unsigned char)s[1] <= 0xfe))
	    return(2);
    }

    /**********
      if we are here, then invalid multi-byte character
    **********/
    errno = EILSEQ;
    return((size_t)-1);
    
}
