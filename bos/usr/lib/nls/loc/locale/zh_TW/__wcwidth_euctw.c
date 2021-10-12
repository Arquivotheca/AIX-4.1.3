static char sccsid[] = "@(#)52        1.3  src/bos/usr/lib/nls/loc/locale/zh_TW/__wcwidth_euctw.c, libmeth, bos411, 9428A410j 4/25/94 14:57:03";
/*
 * COMPONENT_NAME: (LIBMETH) LIBC Code-Point/Process-Code Conversion Functions
 *
 * FUNCTIONS: wcwidth_euctw
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
/*   Get the width of Wide Characters

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

int __wcwidth_euctw(_LC_charmap_objhdl_t hdl, const wchar_t wc)
{
    int len;

    /**********
      if wc is NULL, return 0
    **********/
    if (wc == (wchar_t )NULL)
        return(0);

    if (! iswprint(wc))
	return(-1);
    
    /**********
      Single Byte PC <= 7f (1 display widths)
    **********/
    if (wc <= 0x7f)
        return(1);

    /**********
      Double byte: 0x0100 - 0x2383 (2 display widths)
    **********/
    else if ((wc >= 0x0100) && (wc <= 0x2383))
        return(2);

    /**********
      Quadruple byte 0x2384 - 0x4607 (2 display widths)
    **********/
    else if ((wc >=0x2384) && (wc <= 0x4607))
        return(2);

    /**********
      Quardruple byte 0x4608 - 0x688b (2 display widths)
    **********/
    else if ((wc >=0x4608) && (wc <= 0x688b))
        return(2);


    /**********
      Quadruple byte 0x688c - 0x8b0f (2 display widths)
    **********/
    else if ((wc >=0x688c) && (wc <= 0x8b0f))
        return(2);

    /**********
      Quardruple byte 0x8b10 - 0xad93 (2 display widths)
    **********/
    else if ((wc >=0x8b10) && (wc <= 0xad93))
        return(2);

    /**********
      Quardruple byte 0xad94 - 0xd017 (2 display widths)
    **********/
    else if ((wc >=0xad94) && (wc <= 0xd017))
        return(2);

    /**********
      otherwise it is an invaild process code
    **********/

    return(-1);

}
