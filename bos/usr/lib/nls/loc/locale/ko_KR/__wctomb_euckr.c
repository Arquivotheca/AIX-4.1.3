static char sccsid[] = "@(#)38	1.1  src/bos/usr/lib/nls/loc/locale/ko_KR/__wctomb_euckr.c, libmeth, bos411, 9428A410j 5/25/92 16:00:26";
/*
 * COMPONENT_NAME:	LIBMETH
 *
 * FUNCTIONS: __wctomb_euckr
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
#include <sys/errno.h>
/*
  Converts a process code to a string of characters for the KR_EUC codeset

  The algorithm for this conversion is:
  PC <= 0x007f:                 s[0] = PC
  PC >= 0x0100 and PC <=0x2383: s[1] = ((PC - 0x100) % 94) + 0xa1
                                s[0] = (PC - 0x100 -s[1] + 0xa1) / 94 + 0xa1

  +-----------------+-----------+-----------+
  |  process code   |   s[0]    |   s[1]    |
  +-----------------+-----------+-----------+
  | 0x0000 - 0x007f | 0x00-0x7f |    --     |
  | 0x0080 - 0x00ff |   --      |    --     |
  | 0x0100 - 0x2383 | 0xa1-0xfe | 0xa1-0xfe |
  +-----------------+-----------+-----------+

*/

int __wctomb_euckr(_LC_charmap_objhdl_t handle, char *s, wchar_t pwc)
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
      Double byte 0x0100 - 0x5d5d
    **********/
    else if ((pwc >=0x0100) && (pwc <= 0x2383)) {
	pwc -= 0x100;
	s[1] = (char) ((pwc % 94) + 0xa1);
	if( pwc < 94)		s[0] = 0xa1;
	else			s[0] = (char) ((pwc - s[1] + 0xa1)/94 + 0xa1);
	return(2);
    }

    /**********
      otherwise it is an invaild process code
    **********/
    errno = EILSEQ;
    return(-1);
}
