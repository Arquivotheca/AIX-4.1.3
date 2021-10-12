static char sccsid[] = "@(#)40	1.1  src/bos/usr/lib/nls/loc/locale/ZH_CN/__wcwidth_utf8cn.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:24:41";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: __wcwidth_utf8cn
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

  |  process code   |   s[0]    |   s[1]    |    s[2]   |  wcwidth  |
  +-----------------+-----------+-----------+-----------+-----------+
  | 0x0000 - 0x007f | 0x00-0x7f |    --     |    --     |     1     |
  | 0x0080 - 0x07ff | 0xc2-0xdf | 0x80-0xbf |    --     |     2     |
  | 0x0800 - 0x4dff | 0xe0-0xe2 | 0x80-0xbf | 0x80-0xbf |     3     |
  | 0x4e00 - 0x9fff | 0xe4-0xe9 | 0x80-0xbf | 0x80-0xbf |     3     |
  | 0xe000 - 0xfffd | 0xee-0xef | 0x80-0xbf | 0x80-0xbf |     3     |
  +-----------------+-----------+-----------+-----------+-----------+
*/

int __wcwidth_utf8cn(_LC_charmap_objhdl_t handle, const wchar_t wc)
{
    int len;

    /**********
      If wc is NULL, return 0
    **********/
    if (wc == (wchar_t )NULL)
        return(0);

    /**********
      Single Byte PC <= 7f (1 display widths)
    **********/
    if (wc <= 0x7f)
        return(1);

    /************
      Double byte: 0x0080 - 0x07ff (2 display widths)
    *************/
    else if ( (wc >= 0x0080) && (wc <= 0x07ff) )
        return(2);

    /**********
      Double byte 0x0800 - 0xfffd (3 display widths)
    **********/
    else if ((wc >= 0x0800) && (wc <= 0xfffd))
        return(2);

    /**********
      otherwise it is an invaild process code
    **********/

    return(-1);

}
