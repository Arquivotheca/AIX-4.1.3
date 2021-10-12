static char sccsid[] = "@(#)84	1.3  src/bos/usr/ccs/lib/libc/NCstrncmp.c, libcnls, bos411, 9428A410j 6/6/91 18:42:39";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: NCstrncmp
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 , 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
#include <sys/localedef.h>
#include <stdlib.h>
#include <string.h>

/*
 *
 * FUNCTION: NCstrncmp
 *	    
 *
 * PARAMETERS: 
 *
 *
 * RETURN VALUE: 
 *
 *
 */
int NCstrncmp(const wchar_t *wcs1, const wchar_t *wcs2, int n)
{
    wchar_t *wcs1_ptr;
    wchar_t *wcs2_ptr;
    int  rc;

    /**********
      malloc the space for the temp strings
    **********/
    wcs1_ptr = (wchar_t *) malloc((n+1) * sizeof(wchar_t));
    wcs2_ptr = (wchar_t *) malloc((n+1) * sizeof(wchar_t));

    if (wcs1_ptr == (wchar_t *)NULL || wcs2_ptr == (wchar_t *)NULL ) {
	(void)free(wcs1_ptr);
	(void)free(wcs2_ptr);
	perror(NCstrncmp);
	return(0);
    }

    /*********
      copy only the first n process codes
      into the temp strings
    **********/
    wcsncpy(wcs1_ptr, wcs1, n);
    wcsncpy(wcs2_ptr, wcs2, n);

    /**********
      always add the terminating NULL
    **********/
    wcs1_ptr[n] = (wchar_t) '\0';
    wcs2_ptr[n] = (wchar_t) '\0';

    /*********
      collate the strings, free the space then return
    **********/
    rc = wcscoll(wcs1_ptr, wcs2_ptr);
    free(wcs1_ptr);
    free(wcs2_ptr);

    return( rc );
	

}
