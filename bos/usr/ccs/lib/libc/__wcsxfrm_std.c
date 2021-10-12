static char sccsid[] = "@(#)72	1.6.2.3  src/bos/usr/ccs/lib/libc/__wcsxfrm_std.c, libcstr, bos411, 9428A410j 2/11/94 14:43:55";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: __wcsxfrm_std.c
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#pragma alloca

#include <sys/localedef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

/*
 * FUNCTION: Converts the wchar_t string pointed to by wcs_in to collation 
 *	     weights and places them in wcs_out.  If n is zero, wcs_out can
 *	     be NULL.
 *
 *	     The conversion is based on the collating sequence specified
 *	     by the locale category LC_COLLATE affected by the setlocale
 *	     function.
 *
 * NOTES:    XPG4 requires this function.
 *
 * PARAMETERS: (Uses file codes )
 *	     char *wcs_out - first string
 *	     char *wcs_in - second string
 *	     size_t n - number of wchars to place in wcs_out
 *
 * RETURN VALUE DESCRIPTIONS: 
 *		-1 - wcs_out id null and n != 0
 *		     wcstombs fails
 *		     alloca fails
 */
/**********
  __wcsxfrm_std is calling strxfrm due to the fact that regular expressions
  cannot handle process code.
**********/

size_t __wcsxfrm_std(_LC_collate_objhdl_t hdl, wchar_t *wcs_out, const wchar_t *wcs_in, size_t n)
{
    char *str_in;
    int  len_in;
    int  rc;
    
    if((wcs_out == (wchar_t *)NULL) && (n != 0)) 
	return((size_t)-1);
    /**********
      alloca the space for the multi-byte wcs_in
    **********/
    len_in=wcslen(wcs_in)*MB_CUR_MAX + 1;
    if ((str_in = (char *)alloca( (len_in) )) == NULL) {
	errno = ENOMEM;
	perror("__wcsxfrm_std:alloca");
	return(-1);
    }

    /**********
      convert the process code to file code
    **********/
    if (wcstombs(str_in, wcs_in, len_in) == -1)  {
	errno = EINVAL;
	return(-1);
    }

    rc = strxfrm((char *) wcs_out, str_in, n*2);

    /**********
      since only one null is placed at the end of wcs_out, always
      put a null wchar null if n is non-zero
    **********/
    rc /= 2;

    if (n != 0) {
    	if (rc >= n)
		wcs_out[n-1] = 0x00;
    	else
		wcs_out[rc] = 0x00;
    }

    return(rc);

}
