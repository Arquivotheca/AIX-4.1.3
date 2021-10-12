static char sccsid[] = "@(#)54	1.2  src/bos/usr/lpp/Unicode/methods/__wcswidth_unistd.c, cfgnls, bos411, 9428A410j 4/20/94 10:41:49";
/*
 *   COMPONENT_NAME: CFGNLS
 *
 *   FUNCTIONS: __wcswidth_unistd
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

int __wcswidth_unistd( _LC_charmap_objhdl_t hdl, const wchar_t *wcs, size_t n)
{
    	int	width, rc;
    	int 	i;
	wchar_t	wc;

    /**********
      if wcs is NULL, return 0
    **********/
    if (wcs == (wchar_t *)NULL || *wcs == (wchar_t)NULL)
	return(0);
    
    rc = 0;

    for (i=0; ((wc = wcs[i]) != (wchar_t)NULL ) && ( i < n) ; i++) {
	if ((width = __wcwidth_unistd(hdl, wc)) < 0) {
	    return (-1);
	}
	rc += width;
    }

    return(rc);
}

