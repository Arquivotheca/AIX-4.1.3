static char sccsid[] = "@(#)46        1.3  src/bos/usr/lib/nls/loc/locale/zh_TW/__mbtowc_euctw.c, libmeth, bos411, 9428A410j 4/26/94 15:34:21";
/*
 * COMPONENT_NAME: (LIBMETH) LIBC Code-Point/Process-Code Conversion Functions
 *
 * FUNCTIONS: mbtowc_euctw
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1992,1994
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
  Converts a multi-byte string to process code for the TW_EUC codeset

  The algorithm for this conversion is:
  s[0] <= 0x7f:  PC = s[0]
  s[0] >= 0xa1:  PC = (((s[0] - 0xa1) * 94) | (s[1] - 0xa1) + 0x100);
  s[0] = 0x8e   s[1] = a2: PC = (((s[2] - 0xa1) * 94) + (s[3] - 0xa1) + 0x2384)
  s[0] = 0x8e   s[1] = a3: PC = (((s[2] - 0xa1) * 94) + (s[3] - 0xa1) + 0x4608)
  s[0] = 0x8e   s[1] = a4: PC = (((s[2] - 0xa1) * 94) + (s[3] - 0xa1) + 0x688c)
  s[0] = 0x8e   s[1] = ac: PC = (((s[2] - 0xa1) * 94) + (s[3] - 0xa1) + 0x8b10)
  s[0] = 0x8e   s[1] = ad: PC = (((s[2] - 0xa1) * 94) + (s[3] - 0xa1) + 0xad94)

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

  This algorithm compresses all of code points to process codes less
  than 0xf29b.
*/

int __mbtowc_euctw(_LC_charmap_objhdl_t handle, wchar_t *pwc, const char *s, size_t maxlen)
{
    wchar_t dummy;

    /**********
      if s is NULL return 0
    **********/
    if (s == (char *)NULL)
        return(0);

    /**********
      If length == 0 return -1
    **********/
    if (maxlen < 1)
        return((size_t)-1);

    /*********
      if pwc is null, set it to dummy
    **********/
    if (pwc == (wchar_t *)NULL)
        pwc = &dummy;

    /**********
      single byte (<=0x7f)
    **********/
    if ((unsigned char)s[0] <= 0x7f) {
        *pwc = (wchar_t) s[0];
        if (s[0] != '\0')
            return(1);
        else
            return(0);
    }


    /**********
      Quadraple Byte 8e[a2,a3,ac,ad,af][a1-fe][a1-fe]
    **********/

    else if ((unsigned char)s[0] == 0x8e) {
        if (maxlen >= 4) {
          if ((unsigned char)s[1] == 0xa2 ) {
            if (((unsigned char)s[2] >=0xa1 && (unsigned char)s[2] <=0xfe) &&
                ((unsigned char)s[3] >=0xa1 && (unsigned char)s[3] <= 0xfe)) {
                *pwc = (wchar_t) (((unsigned char)s[2] - 0xa1) * 94) +
                       (wchar_t) (((unsigned char)s[3] - 0xa1)) + 0x2384;
                return(4);
            }
          }
          if ((unsigned char)s[1] == 0xa3 ) {
            if (((unsigned char)s[2] >=0xa1 && (unsigned char)s[2] <=0xfe) &&
                ((unsigned char)s[3] >=0xa1 && (unsigned char)s[3] <= 0xfe)) {
                *pwc = (wchar_t) (((unsigned char)s[2] - 0xa1) * 94) +
                       (wchar_t) (((unsigned char)s[3] - 0xa1)) + 0x4608;
                return(4);
            }
          }
          if ((unsigned char)s[1] == 0xa4 ) {
            if (((unsigned char)s[2] >=0xa1 && (unsigned char)s[2] <=0xfe) &&
                ((unsigned char)s[3] >=0xa1 && (unsigned char)s[3] <= 0xfe)) {
                *pwc = (wchar_t) (((unsigned char)s[2] - 0xa1) * 94) +
                       (wchar_t) (((unsigned char)s[3] - 0xa1)) + 0x688c;
                return(4);
            }
          }
          if ((unsigned char)s[1] == 0xac ) {
            if (((unsigned char)s[2] >=0xa1 && (unsigned char)s[2] <=0xfe) &&
                ((unsigned char)s[3] >=0xa1 && (unsigned char)s[3] <= 0xfe)) {
                *pwc = (wchar_t) (((unsigned char)s[2] - 0xa1) * 94) +
                       (wchar_t) (((unsigned char)s[3] - 0xa1)) + 0x8b10;
                return(4);
            }
          }

          if ((unsigned char)s[1] == 0xad ) {
            if (((unsigned char)s[2] >=0xa1 && (unsigned char)s[2] <=0xfe) &&
                ((unsigned char)s[3] >=0xa1 && (unsigned char)s[3] <= 0xfe)) {
                *pwc = (wchar_t) (((unsigned char)s[2] - 0xa1) * 94) +
                       (wchar_t) (((unsigned char)s[3] - 0xa1)) + 0xad94;
                return(4);
            }
          }
        }
    }


    /**********
      Double Byte [a1-fe][a1-fe]
    **********/
    else if ((unsigned char)s[0] >= 0xa1 && (unsigned char)s[0] <= 0xfe) {
        if (maxlen >= 2) {
            if  ((unsigned char)s[1] >=0xa1 && (unsigned char)s[1] <= 0xfe) {
                *pwc = (wchar_t) (((unsigned char)s[0] - 0xa1) * 94) +
                       (wchar_t) (((unsigned char)s[1] - 0xa1)) + 0x100;
                return(2);
            }
        }
    }

    /**********
      Not a valid character, return -1
    **********/
    errno = EILSEQ;
    return(-1);

}

