static char sccsid[] = "@(#)52	1.1  src/bos/usr/lib/nls/loc/locale/zh_CN/__wcswidth_euccn.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:25:44";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: __wcswidth_euccn
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
/*

  |  process code   |   s[0]    |   s[1]    | 
  +-----------------+-----------+-----------+
  | 0x0000 - 0x007f | 0x00-0x7f |    --     |
  | 0x0080 - 0x0d17 | 0xa1-0xfe | 0xa1-0xfe |
  | 0x2000 - 0x32ff | 0xa1-0xfe | 0xa1-0xfe |
  | 0x4e00 - 0x9fa8 | 0xa1-0xf7 | 0xa1-0xfe |
  | 0xe000 - 0xe291 | 0xf8-0xfe | 0xa1-0xfe |
  | 0xff00 - 0xfffd | 0xa1-0xfe | 0xa1-0xfe |
  +-----------------+-----------+-----------+


*/

int __wcswidth_euccn(_LC_charmap_objhdl_t handle, const wchar_t *wcs, size_t n)
{
    int len;
    int i;

    /**********
      if wcs is NULL, return 0
    **********/
    if ( wcs == (wchar_t *)NULL || *wcs == (wchar_t)NULL )
        return(0);

    len = 0;
    for (i=0; wcs[i] != (wchar_t)NULL && i<n; i++) {

        /**********
          Single Byte PC <= 7f (1 display widths)
        **********/
        if (wcs[i] <= 0x7f)
            len += 1;

	/**********
	  Non Chinese glyph
	***********/
        else if ( (wcs[i] >= 0x0080) && (wcs[i] <= 0x0d17) )
            len += 2;

	/**********
	  Non Chinese glyph
	***********/
        else if ( (wcs[i] >= 0x2000) && (wcs[i] <= 0x32ff) )
            len += 2;

        /**********
          Double byte: 0x4e00 - 0x9fa8 (2 display widths)
        **********/
        else if (wcs[i] >= 0x4e00 && wcs[i] <= 0x9fa8)
            len += 2;

        /**********
          Double byte: 0xe000 - 0xe54f (2 display widths)
        **********/
        else if (wcs[i] >= 0xe000 && wcs[i] <= 0xe54f)
            len += 2;

	/**********
	  Non Chinese glyph
	***********/
        else if (wcs[i] >= 0xff00 && wcs[i] <= 0xfffd)
            len += 2;

        /**********
          otherwise it is an invaild process code
        **********/
        else
            return(-1);
    }

    return(len);
}
