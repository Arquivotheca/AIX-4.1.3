static char sccsid[] = "@(#)51	1.2.1.3  src/bos/usr/ccs/lib/libc/__wcswidth_932.c, libccppc, bos411, 9428A410j 3/30/94 14:57:44";
/*
 * COMPONENT_NAME: (LIBCCPPC) LIBC Code-Point/Process-Code Conversion Functions
 *
 * FUNCTIONS: __wcswidth_932.c
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991 , 1992
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
#define	_ILS_MACROS
#include <stdlib.h>
#include <ctype.h>
/*

  The algorithm for this conversion is:
  PC <=  0x007f:                  s[0] = PC 
  PC >= 0x00a1 and PC <= 0x00df:  s[0] = PC
  PC >= 0x0100 and PC <= 0x189e:  s[0] = ((PC - 0x0100) & 0x001f) + 0x81
                                  s[1] = ((PC - 0x0100) >> 5) + 0x40
  PC >= 0x189f and PC <= 0x303b:  s[0] = ((PC - 0x189f) & 0x001f) + 0xe0
                                  s[1] = ((PC - 0x189f >> 5) + 0x40

  |  process code   |   s[0]    |   s[1]    |
  +-----------------+-----------+-----------+
  | 0x0000 - 0x007f | 0x00-0x7f |    --     |
  | 0x007e - 0x00a0 |    --     |    --     |
  | 0x00a1 - 0x00df | 0xa1-0xdf |    --     |
  | 0x00e0 - 0x00ff |    --     |    --     |
  | 0x0100 - 0x189e | 0x81-0x9f | 0x40-0xfc (excluding 0x7f)
  | 0x189f - 0x303b | 0xe0-0xfc | 0xa1-0xfe (excluding 0x7f)
  +-----------------+-----------+-----------+
  
*/

int __wcswidth_932(_LC_charmap_objhdl_t hdl, wchar_t *wcs, size_t n)
{
    int i;
    int	len;
    
    /**********
      if wcs is NULL  return 0
    **********/
    if (wcs == (wchar_t *)NULL || *wcs == (wchar_t) NULL)
	return(0);

    len = 0;
    for (i=0; wcs[i] != (wchar_t)NULL && i<n; i++) {

	/**********
	  check if it is a printing character
	**********/
	if (!iswprint(wcs[i]))
		return(-1);

	/**********
	  Single Byte PC <= 7f or PC >= a1 and PC <= df
	**********/
	if (wcs[i] <= 0x7f || (wcs[i] >=0xa1 && wcs[i] <= 0xdf))
	    len += 1;
	
	/**********
	  Double Byte PC >= e0 and PC <= 189e
	**********/
	else if ( (wcs[i] >= 0x0100) && (wcs[i] <= 0x189e) )
	    len += 2;
	
	
	/**********
	  Double Byte PC >=189f and PC <=303b
	**********/
	else if ( (wcs[i] >=0x189f) && (wcs[i] <= 0x303b) )
	    len += 2;

	/**********
	  invalid process code
	**********/
	else
	    return(-1);
    }

    return(len);
    
}
