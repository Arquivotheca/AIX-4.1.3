static char sccsid[] = "@(#)39	1.2  src/bos/usr/lib/nls/loc/locale/ko_KR/__wcwidth_euckr.c, libmeth, bos411, 9428A410j 4/25/94 14:59:49";
/*
 * COMPONENT_NAME:	LIBMETH
 *
 * FUNCTIONS: __wcwidth_euckr
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1994
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
  Converts a process code to a string of characters for the KR_EUC codeset

  The algorithm for this conversion is:
  PC <= 0x007f:                 s[0] = PC
  PC >= 0x0100 and PC <=0x2383: s[1] = (PC - 0x100) % 94 + 0xa1
                                s[0] = (PC - 0x100 -s[1] +0xa1)/94 +0xa1 

  +-----------------+-----------+-----------+
  |  process code   |   s[0]    |   s[1]    |
  +-----------------+-----------+-----------+
  | 0x0000 - 0x007f | 0x00-0x7f |    --     |
  | 0x00a0 - 0x00ff |   --      |    --     |
  | 0x0100 - 0x2383 | 0xa1-0xfe | 0xa1-0xfe |
  +-----------------+-----------+-----------+

*/

int __wcwidth_euckr(_LC_charmap_objhdl_t hdl, const wchar_t wc)
{
    int	len;
    
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
      Double byte 0x0100 - 0x2383 (2 display widths)
    **********/
    else if ((wc >=0x0100) && (wc <= 0x2383))
	return(2);

    /**********
      otherwise it is an invaild process code
    **********/

    return(-1);
    
}
