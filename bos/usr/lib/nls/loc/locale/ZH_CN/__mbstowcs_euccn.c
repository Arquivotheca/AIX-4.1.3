static char sccsid[] = "@(#)46	1.1  src/bos/usr/lib/nls/loc/locale/zh_CN/__mbstowcs_euccn.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:25:30";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: __mbstowcs_euccn
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

size_t __mbstowcs_euccn(_LC_charmap_objhdl_t handle, wchar_t *pwcs, const char *s, size_t n)
{
    int len = n;
    int rc;
    int cnt;
    wchar_t *pwcs0 = pwcs;

    /**********
      if pwcs is a null pointer, just count the number of characters
      in s
    **********/
    if (pwcs == (wchar_t *)NULL) {
        cnt = 0;
        while (*s != '\0') {
              if ((rc = mblen(s, MB_CUR_MAX)) == -1)
                    return(-1);
              cnt += rc;
              s += rc;
        }
        return(cnt);
    }

    while (len-- > 0) {

        /**********
          if we hit a NULL in s, add a NULL to pwcs and return
          the length of pwcs
        **********/
        if ( *s == '\0') {
            *pwcs = (wchar_t) '\0';
            return (pwcs - pwcs0);
        }

        /**********
          convert s to process code and increment s by the number
          of bytes is took
        **********/
        /*if ((cnt = mbtowc(pwcs, s, MB_CUR_MAX)) < 0)*/
        if ((cnt = __mbtowc_euccn(handle,pwcs, s, MB_CUR_MAX)) < 0)
            return(-1);
        s += cnt;
        ++pwcs;
    }

    /**********
      ran out of space before hiting a NULL in s, return n
    **********/
    return (n);
}
