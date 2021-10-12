static char sccsid[] = "@(#)44	1.1  src/bos/usr/lib/nls/loc/locale/zh_CN/__mblen_euccn.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:25:26";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: __mblen_euccn
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
  Returns the number of bytes that comprise a multi-byte string for the IBM_eucCN codeset

  The algorithm for this conversion is:
  s[0] < 0x80:   1 byte
  s[0] >= 0xa1   2 bytes

	  |  process code   |   s[0]    |   s[1]    |
	  +-----------------+-----------+-----------+
 	  | 0x0000 - 0x007f | 0x00-0x7f |    --     |
  	  | 0x0080 - 0x04ff | 0xa1-0xf7 | 0xa1-0xfe |
  	  | 0x2000 - 0x32ff | 0xa1-0xf7 | 0xa1-0xfe |
	  | 0x4e00 - 0x9fac | 0xa1-0xf7 | 0xa1-0xfe |
	  | 0xe000 - 0xe291 | 0xf8-0xfe | 0xa1-0xfe | 
	  | 0xff00 - 0xfffd | 0xf8-0xfe | 0xa1-0xfe | 
	  +-----------------+-----------+-----------+
*/

size_t __mblen_euccn(_LC_charmap_objhdl_t handle, char *s, size_t maxlen)
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
      Double Byte [a1-fe][a1-fe]
    **********/
    else if ((unsigned char)s[0] >= 0xa1) {
        if (maxlen >=2 && (unsigned char)s[0] <= 0xfe && (unsigned char)s[1] >=0xa1 && (unsigned char)s[1] <= 0xfe)
            return(2);
    }

    /**********
      if we are here, then invalid multi-byte character
    **********/
    errno = EILSEQ;
    return((size_t)-1);

}


