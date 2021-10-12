static char sccsid[] = "@(#)49	1.1  src/bos/usr/lib/nls/loc/locale/zh_TW/__wcstombs_euctw.c, libmeth, bos411, 9428A410j 5/25/92 16:04:24";
/*
 * COMPONENT_NAME: (LIBMETH) LIBC Code-Point/Process-Code Conversion Functions
 *
 * FUNCTIONS: wcstombs_euctw
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992
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
size_t  __wcstombs_euctw(_LC_charmap_objhdl_t handle, char *s, const wchar_t *pwcs, size_t n)
{
    int cnt=0;
    int len=0;
    int i=0;
    char tmps[5]; /* assume no more than a 4 byte character */

    /**********
      if s is a NULL pointer, then just count the number of bytes
      the converted string of process codes would require (does not
      include the terminating NULL)
    **********/
    if ( s == (char *)NULL) {
        cnt = 0;
        while (*pwcs != (wchar_t)'\0') {
           /* if ((len = wctomb(tmps, *pwcs)) == -1) */
            if ((len = __wctomb_euctw(handle,tmps, *pwcs)) == -1) /* modify By K. C. Lee */
                return(-1);
            cnt += len;
            pwcs++;
        }
        return(cnt);
    }

    if (*pwcs == (wchar_t) '\0') {
	*s = '\0';
	return(0);
    }

    while (1) {

        /**********
          get the length of the characters in the process code
        **********/
       /* if ((len = wctomb(tmps, *pwcs)) == -1) */
        if ((len = __wctomb_euctw(handle,tmps, *pwcs)) == -1) /* Modified by K. C. Lee */
            return(-1);

        /**********
          if there is no room in s for this character(s),
          set a null and break out of the while
        **********/
        else if (cnt+len > n) {
            *s = '\0';
            break;
        }

        /**********
          if a null was encounterd in pwcs, end s with a null and
          break out of the while
        **********/
        if (tmps[0] == '\0') {
            *s = '\0';
            break;

        }
        /**********
          Otherwise, append the temporary string to the
          end of s.
        ***********/
        for (i=0; i<len; i++) {
            *s = tmps[i];
            s++;
        }

        /**********
          incrent the number of bytes put in s
        **********/
        cnt += len;

        /**********
          if the number of bytes processed is
          n then time to return, do not null terminate
        **********/
        if (cnt == n)
            break;

        /**********
          increment pwcs to the next process code
        **********/
        pwcs++;
    }
    /*********
      if there was not enough space for even 1 char,
      return the len
    **********/
    if (cnt == 0)
        cnt = len;
    return (cnt);
}

