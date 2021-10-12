static char sccsid[] = "@(#)08	1.4.1.2  src/bos/usr/ccs/lib/libc/__is_wctype_sb.c, libcchr, bos411, 9428A410j 1/12/93 11:09:22";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: __is_wctype_sb
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 *
 * FUNCTION: Determines if the process code, wc, has the properties of mask
 *	    
 *
 * PARAMETERS: hdl -- The ctype info for the current locale
 *             wc  -- Process code of character to classify
 *            mask -- Mask of property to check for
 *
 *
 * RETURN VALUE: 0 -- wc does not contain the property of mask
 *              >0 -- wc has the property in mask
 *
 *
 */

#include <sys/localedef.h>
#include <ctype.h>

int __is_wctype_sb(_LC_ctype_objhdl_t hdl, wint_t wc, wctype_t mask)
{
    /**********
      if wc is outside of the bounds, or 
      if the mask is -1, then return 0
    **********/
    if ((wc > __OBJ_DATA(hdl)->max_wc || wc < __OBJ_DATA(hdl)->min_wc) 
	|| (mask == -1) )
	return(0);

    return(__OBJ_DATA(hdl)->mask[wc] & mask);
}
