static char sccsid[] = "@(#)33	1.2  src/bos/usr/lib/nls/loc/locale/ko_KR/__mbtowc_euckr.c, libmeth, bos411, 9428A410j 4/26/94 15:31:13";
/*
 * COMPONENT_NAME:	LIBMETH
 *
 * FUNCTIONS: __mbtowc_euckr
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
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
  Converts a multi-byte string to process code for the KR_EUC codeset

  The algorithm for this conversion is:
  s[0] < 0x7f:  PC = s[0]
  s[0] > 0xa1   PC = (s[0] - 0xa1) * 94 + s[1] - 0xa1 + 0x100

  +-----------------+-----------+-----------+
  |  process code   |   s[0]    |   s[1]    |
  +-----------------+-----------+-----------+
  | 0x0000 - 0x007f | 0x00-0x7f |    --     |
  | 0x0080 - 0x00ff |   --      |    --     |
  | 0x0100 - 0x2383 | 0xa1-0xfe | 0xa1-0xfe |
  +-----------------+-----------+-----------+
  
  This algorithm compresses all of code points to process codes less
  than 0x5d5d.
*/

int __mbtowc_euckr(_LC_charmap_objhdl_t handle, wchar_t *pwc, const char *s, size_t maxlen)
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
      Double Byte [a1-fe][a1-fe]
    **********/
    else if ((((unsigned char)s[0] >= 0xa1) && ((unsigned char)s[0] <= 0xfe)) && (maxlen >= 2) &&
    ((unsigned char)(s[1] >=0xa1) && ((unsigned char)s[1] <= 0xfe))) {
	*pwc = (wchar_t) (((unsigned char)s[0] - 0xa1) * 94 +(unsigned char)s[1] - 0xa1 + 0x100) ;
	return(2);
    }

    /**********
      Not a valid character, return -1
    **********/
    errno = EILSEQ;
    return(-1);
    
}
    
		



