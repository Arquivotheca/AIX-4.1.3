static char sccsid[] = "@(#)30	1.1  src/bos/usr/lib/nls/loc/locale/ZH_CN/__mblen_utf8cn.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:24:24";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: __mblen_utf8cn
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdlib.h>
#include <ctype.h>
#include <sys/errno.h>

/*
  Returns the number of bytes that comprise a multi-byte string for the UCS-UTF2 codeset

  The algorithm for this conversion is:

	  |  process code   |   s[0]    |   s[1]    |   s[2]    |
	  +-----------------+-----------+-----------+-----------+
 	  | 0x0000 - 0x007f | 0x00-0x7f |    --     |    --     |
  	  | 0x0080 - 0x07ff | 0xc2-0xdf | 0x80-0xbf |    --     |
	  | 0x0800 - 0x4dff | 0xe0-0xe2 | 0x80-0xbf | 0x80-0xbf |
	  | 0x4e00 - 0x9fff | 0xe4-0xe9 | 0x80-0xbf | 0x80-0xbf |
	  | 0xe000 - 0xfffd | 0xee-0xef | 0x80-0xbf | 0x80-0xbf |
	  +-----------------+-----------+-----------+-----------+
*/

size_t 
__mblen_utf8cn(_LC_charmap_objhdl_t handle, char *s, size_t maxlen)
{
    unsigned int *utfin;
    unsigned int store_space;

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
      Double Byte [c2-df][80-bf]
    **********/
    else if ((unsigned char)s[0] <= 0xdf) {
        if (maxlen >= 2 && (unsigned char)s[0] >= 0xc2 && ((unsigned char)s[1] >= 0x80 && (unsigned char)s[1] <= 0xbf))
            return(2);
    }

    /***********
       Triple Byte [e0-ef][a0-bf][80-bd]
    ***********/
    else if( maxlen >= 3 ) {
        utfin = &store_space;
        *((unsigned char *)utfin) = 0x00;
        *((unsigned char *)utfin +1) = s[0];
        *((unsigned char *)utfin +2) = s[1];
	*((unsigned char *)utfin +3) = s[2];

	if( *utfin >= 0xe0a080 && *utfin <= 0xefbfbd )  return(3);
    }

    /**********
      if we are here, then invalid multi-byte character
    **********/
    errno = EILSEQ;
    return((size_t)-1);

}


