static char sccsid[] = "@(#)32	1.1  src/bos/usr/lib/nls/loc/locale/ko_KR/__mbtopc_euckr.c, libmeth, bos411, 9428A410j 5/25/92 15:59:26";
/*
 * COMPONENT_NAME:	LIBMETH
 *
 * FUNCTIONS: __mbtopc_euckr
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
/*
  Converts a multi-byte string to process code for the KR_EUC codeset

  The algorithm for this conversion is:
  s[0] < 0x7f:  PC = s[0]
  s[0] > 0xa1   PC = (s[0] - 0xa01) * 94 + s[1] - 0xa1 + 0x100

  +-----------------+-----------+-----------+
  |  process code   |   s[0]    |   s[1]    |
  +-----------------+-----------+-----------+
  | 0x0000 - 0x007f | 0x00-0x7f |    --     |
  | 0x0080 - 0x00ff |    --     |    --     |
  | 0x0100 - 0x2383 | 0xa1-0xfe | 0xa1-0xfe |
  +-----------------+-----------+-----------+
  
  This algorithm compresses all of code points to process codes less
  than 0x5d5d.
*/

size_t __mbtopc_euckr(_LC_charmap_objhdl_t handle, wchar_t *pwc, char *s, size_t maxlen, int *err)
{
    wchar_t dummy;

    /**********
      if s is NULL or points to a NULL return 0
    **********/
    if (s == (char *)NULL)
	return(0);

    /**********
      if pwc is null, then set it to dummy
    **********/
    if (pwc == (wchar_t *)NULL)
	pwc = &dummy;
    
    /**********
      assume it is a bad character
    **********/
    *err = 0;
    
    /**********
      single byte (<=0x7f)
    **********/
    if ((unsigned char)s[0] <= 0x7f) {
	if (maxlen < 1) {
	    *err = 1;
	    return(0);
	}
	else {
	    *pwc = (wchar_t) s[0];
	    return(1);
	}
    }

    /**********
      Double Byte [a1-fd][a1-fe]
    **********/
    else if ((unsigned char)s[0] >= 0xa1 && (unsigned char)s[0] <= 0xfe) {
	if (maxlen >= 2) {
	    if  ((unsigned char)s[1] >=0xa1 && (unsigned char)s[1] <= 0xfe) {
		*pwc = (wchar_t) ( ((unsigned char)s[0] - 0xa1) * 94 + (unsigned char)s[1] - 0xa1 + 0x100);
		return(2);
	    }
	}
	else {
	    *err = 2;
	    return(0);
	}
    }

    /**********
      If we are here invalid mb character
    **********/
    *err = -1;
    return(0);
    
}
    
		



