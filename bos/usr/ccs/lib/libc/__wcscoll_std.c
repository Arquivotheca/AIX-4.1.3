static char sccsid[] = "@(#)71	1.6.2.1  src/bos/usr/ccs/lib/libc/__wcscoll_std.c, libcstr, bos411, 9428A410j 10/8/92 21:31:20";

#pragma alloca
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: __wcscoll_std.c
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/localedef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

/*
 * FUNCTION: Compares the strings pointed to by wcs1 and wcs2, returning an
 *	     integer as follows:
 *
 *		Less than 0	If wcs1 is less than wcs2
 *		Equal to 0	If wcs1 is equal to wcs2
 *		Greater than 0	If wcs1 is greater than wcs2.
 *
 *	     The comparison is based on the collating sequence specified
 *	     by the locale category LC_COLLATE affected by the setlocale
 *	     function.
 *
 * NOTES:    The ANSI Programming Language C standard requires this routine.
 *
 * PARAMETERS: (Uses file codes )
 *	     char *wcs1 - first string
 *	     char *wcs2 - second string
 *
 * RETURN VALUE DESCRIPTIONS: Returns a negative, zero, or positive value
 *	     as described above.
 */
/**********
  __wcscoll_std is calling strcoll due to the fact that regular expressions
  cannot handle process code.
**********/

int __wcscoll_std(_LC_collate_objhdl_t hdl, const wchar_t *wcs1, const wchar_t *wcs2)
{
    char *str1;
    char *str2;
    int  len1;
    int  len2;
    int  rc;
    
    /**********
      alloca the space for the multi-byte wcs1
    **********/
    len1=wcslen(wcs1)*MB_CUR_MAX + 1;
    if ((str1 = (char *)alloca( (len1) )) == NULL) {
	perror("__wcscoll_std:alloca");
	_exit (-1);
    }
    len2=wcslen(wcs2)*MB_CUR_MAX + 1;
    if ((str2 = (char *)alloca( (len2) )) == NULL) {
	perror("__wcscoll_std:alloca");
	_exit (-1);
    }

    if (wcstombs(str1, wcs1, len1) == -1) 
	errno = EINVAL;
    if (wcstombs(str2, wcs2, len2) == -1) 
	errno = EINVAL;

    rc = strcoll(str1, str2);

    return(rc);

}
