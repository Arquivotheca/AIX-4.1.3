static char sccsid[] = "@(#)42        1.2  src/bos/usr/lib/nls/loc/locale/zh_TW/__mblen_euctw.c, libmeth, bos411, 9428A410j 9/17/93 09:35:22";
/*
 * COMPONENT_NAME: (LIBMETH) LIBC Code-Point/Process-Code Conversion Functions
 *
 * FUNCTIONS: mblen_euctw
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1992,1993
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
  Returns the number of bytes that comprise a multi-byte string for the TW_EUC codeset

  The algorithm for this conversion is:
  s[0] < 0x80:   1 byte
  s[0] = 0x8e:   4 bytes
  s[0] >= 0xa1   2 bytes


  |  process code   |   s[0]    |   s[1]    |   s[2]    |    s[3]   |
  +-----------------+-----------+-----------+-----------+-----------+
  | 0x0000 - 0x007f | 0x00-0x7f |    --     |    --     |     --    |
  | 0x0080 - 0x00ff |   --      |    --     |    --     |     --    |
  | 0x0100 - 0x2383 | 0xa1-0xfe | 0xa1-0xfe |    --     |     --    |
  | 0x2384 - 0x4607 | 0x8e      | 0xa2      | 0xa1-0xfe | 0xa1-0xfe |
  | 0x4608 - 0x688b | 0x8e      | 0xa3      | 0xa1-0xfe | 0xa1-0xfe |
  | 0x688c - 0x8b0f | 0x8e      | 0xa4      | 0xa1-0xfe | 0xa1-0xfe |
  | 0x8b10 - 0xad93 | 0x8e      | 0xac      | 0xa1-0xfe | 0xa1-0xfe |
  | 0xad94 - 0xd017 | 0x8e      | 0xad      | 0xa1-0xfe | 0xa1-0xfe |
  +-----------------+-----------+-----------+-----------+-----------+

*/

size_t __mblen_euctw(_LC_charmap_objhdl_t handle, char *s, size_t maxlen)
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
      Four Byte 8e[a2,a3,a4,ac,ad,af][a1-fe][a1-fe]
    **********/
    else if ((unsigned char)s[0] == 0x8e) {
        if (maxlen >= 4 && ((unsigned char)s[1] == 0xa2 || (unsigned char)s[1] == 0xa3 || (unsigned char)s[1] == 0xa4 || (unsigned char)s[1] == 0xac || (unsigned char)s[1] == 0xad  ) && ( (unsigned char)s[2] >= 0xa1 && (unsigned char)s[2] <= 0xfe) && ((unsigned char)s[3] >= 0xa1 && (unsigned char)s[3] <= 0xfe))
           return(4);
    }

    /**********
      Double Byte [a1-fe][a1-fe]
    **********/
    else if (s[0] >= 0xa1) {
        if (maxlen >=2 && ((unsigned char)s[0] >= 0xa1 && (unsigned char)s[0] <= 0xfe) && ((unsigned char)s[1] >=0xa1 && (unsigned char)s[1] <= 0xfe))
            return(2);
    }

    /**********
      if we are here, then invalid multi-byte character
    **********/
    errno = EILSEQ;
    return((size_t)-1);

}





