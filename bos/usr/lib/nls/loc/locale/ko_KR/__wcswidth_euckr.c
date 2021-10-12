static char sccsid[] = "@(#)37	1.2  src/bos/usr/lib/nls/loc/locale/ko_KR/__wcswidth_euckr.c, libmeth, bos411, 9428A410j 4/25/94 14:59:19";
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
  PC >= 0x0100 and PC <=0x2383: s[0] = (PC - 0x100) % 94 + 0xa1)
				s[1] = (PC - 0x100 - s[1] + 0xa1) / 94 + 0xa1 

  +-----------------+-----------+-----------+
  |  process code   |   s[0]    |   s[1]    |
  +-----------------+-----------+-----------+
  | 0x0000 - 0x007f | 0x00-0x7f |    --     |
  | 0x0080 - 0x00ff |   --      |    --     |
  | 0x0100 - 0x2383 | 0xa1-0xfe | 0xa1-0xfe |
  +-----------------+-----------+-----------+

*/

int __wcswidth_euckr(_LC_charmap_objhdl_t hdl, const wchar_t *wcs, size_t n)
{
    int	len;
    int i;

    /**********
      if wcs is NULL, return 0
    **********/
    if (wcs == (wchar_t *)NULL || *wcs == (wchar_t)NULL)
	return(0);
    
    len = 0;
    for (i=0; wcs[i] != (wchar_t)NULL && i<n; i++) {
	
	/**********
	  if any character is non-printing, return -1
	**********/
	if (!iswprint(wcs[i]))
		return(-1);

	/**********
	  Single Byte PC <= 7f (1 display widths)
	**********/
	if (wcs[i] <= 0x7f)
	    len += 1;
	
	/**********
	  Double byte 0x0100 - 0x5d5d (2 display widths)
	**********/
	else if ((wcs[i] >=0x0100) && (wcs[i] <= 0x2383))
	    len += 2;
	
	/**********
	  otherwise it is an invaild process code
	**********/
	else
	    return(-1);
    }

    return(len);
    
}
