static char sccsid[] = "@(#)54	1.1  src/bos/usr/lib/nls/loc/locale/zh_CN/__wcwidth_euccn.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:25:47";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: __wcwidth_euccn
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

/*   Get the width of Wide Characters


  |  process code   |   s[0]    |   s[1]    |
  +-----------------+-----------+-----------+
  | 0x0000 - 0x007f | 0x00-0x7f |    --     |
  | 0x0080 - 0x04ff | 0xa1-0xfe | 0xa1-0xfe |
  | 0x2000 - 0x32ff | 0xa1-0xfe | 0xa1-0xfe |
  | 0x4e00 - 0x9fa8 | 0xa1-0xf7 | 0xa1-0xfe |
  | 0xe000 - 0xe291 | 0xf8-0xfe | 0xa1-0xfe |
  | 0xff00 - 0xfffd | 0xa1-0xfe | 0xa1-0xfe |
  +-----------------+-----------+-----------+

*/

int __wcwidth_euccn(_LC_charmap_objhdl_t handle, const wchar_t wc)
{
    int len;

    /**********
      if wc is NULL, return 0
    **********/
    if (wc == (wchar_t )NULL)
        return(0);

    /**********
      Single Byte PC <= 7f (1 display widths)
    **********/
    if (wc <= 0x7f)
        return(1);

    /**********
      Double byte: 0x4e00 - 0x9fa8 (2 display widths)
    **********/
    else if ( (wc >= 0x4e00) && (wc <= 0x9fa8) )
        return(2);

    /**********
      Non Chinese glyph
    ***********/
    else if ( (wc >= 0x0080) && (wc <= 0x0d17) )
        return(2);

    /**********
      Non Chinese glyph
    ***********/
    else if ( (wc >= 0x2000) && (wc <= 0x32ff) )
        return(2);


    /**********
      Non Chinese glyph
    ***********/
    else if ( (wc >= 0xff00) && (wc <= 0xfffd) )
        return(2);

    /**********
      Double byte 0xe000 - 0xe54f (2 display widths)
    **********/
    else if ((wc >= 0xe000) && (wc <= 0xe54f))
        return(2);

    /**********
      otherwise it is an invaild process code
    **********/

    return(-1);

}
