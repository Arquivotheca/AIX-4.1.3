static char sccsid[] = "@(#)51        1.2  src/bos/usr/lib/nls/loc/locale/zh_TW/__wctomb_euctw.c, libmeth, bos411, 9428A410j 9/17/93 09:35:36";
/*
 * COMPONENT_NAME: (LIBMETH) LIBC Code-Point/Process-Code Conversion Functions
 *
 * FUNCTIONS: wctomb_euctw
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
  Converts a process code to a string of characters for the TW_EUC codeset


  The algorithm for this conversion is:
  s[0] <= 0x7f:  PC = s[0]
  s[0] >= 0xa1:  PC = (((s[0] - 0xa1) * 94) | (s[1] - 0xa1) + 0x100);
  s[0] = 0x8e   s[1] = a2: PC = (((s[2] - 0xa1) * 94) | (s[3] - 0xa1) + 0x2384)
  s[0] = 0x8e   s[1] = a3: PC = (((s[2] - 0xa1) * 94) | (s[3] - 0xa1) + 0x4608)
  s[0] = 0x8e   s[1] = a4: PC = (((s[2]-- 0xa1) * 94) | (s[3] - 0xa1) + 0x688c)
  s[0] = 0x8e   s[1] = ac: PC = (((s[2] - 0xa1) * 94) | (s[3] - 0xa1) + 0x8b10)
  s[0] = 0x8e   s[1] = ad: PC = (((s[2] - 0xa1) * 94) | (s[3] - 0xa1) + 0xad94)

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
  than 0xf19b.

*/

int __wctomb_euctw(_LC_charmap_objhdl_t handle, char *s, wchar_t pwc)
{
    /**********
      if s is NULL or points to a NULL return 0
    **********/
    if (s == (char *)NULL)
        return(0);

    /**********
      Single Byte PC <= 7f
    **********/

    if (pwc <= 0x7f) {
        s[0] = (char) pwc;
        return(1);
    }


    /**********
      Double byte 0x0100 - 0x2383
    **********/
    else if ((pwc >=0x0100) && (pwc <= 0x2383)) {
        s[0] = (char) (((pwc - 0x0100) / 94 ) + 0x00a1);
        s[1] = (char) (((pwc - 0x0100) % 94) + 0x00a1);
        return(2);
    }

    /**********
      Quadruple      0x2384 - 0x4607
                     0x4608 - 0x688b
                     0x688c - 0x8b0f
                     0x8b10 - 0xad93
                     0xad94 - 0xd017
                     0xd018 - 0xf29b

    **********/
    else if ((pwc >=0x2384) && (pwc <= 0x4607)) {
        s[0] = 0x8e;
        s[1] = 0xa2;
        s[2] = (char) (((pwc - 0x2384) / 94) + 0x00a1);
        s[3] = (char) (((pwc - 0x2384) % 94) + 0x00a1);
        return(4);
    }

    else if ((pwc >=0x4608) && (pwc <= 0x688b)) {
        s[0] = 0x8e;
        s[1] = 0xa3;
        s[2] = (char) (((pwc - 0x4608) / 94) + 0x00a1);
        s[3] = (char) (((pwc - 0x4608) % 94) + 0x00a1);
        return(4);
    }
    else if ((pwc >=0x688c) && (pwc <= 0x8b0f)) {
        s[0] = 0x8e;
        s[1] = 0xa4;
        s[2] = (char) (((pwc - 0x688c) / 94) + 0x00a1);
        s[3] = (char) (((pwc - 0x688c) % 94) + 0x00a1);
        return(4);
    }
    else if ((pwc >=0x8b10) && (pwc <= 0xad93)) {
        s[0] = 0x8e;
        s[1] = 0xac;
        s[2] = (char) (((pwc - 0x8b10) / 94) + 0x00a1);
        s[3] = (char) (((pwc - 0x8b10) % 94) + 0x00a1);
        return(4);
    }
    else if ((pwc >=0xad94) && (pwc <= 0xd017)) {
        s[0] = 0x8e;
        s[1] = 0xad;
        s[2] = (char) (((pwc - 0xad94) / 94) + 0x00a1);
        s[3] = (char) (((pwc - 0xad94) % 94) + 0x00a1);
        return(4);
    }
    /**********
      otherwise it is an invaild process code
    **********/
    errno = EILSEQ;
    return(-1);
}
