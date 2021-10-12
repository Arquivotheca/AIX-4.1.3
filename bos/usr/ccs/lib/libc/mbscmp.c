static char sccsid[] = "@(#)38	1.8  src/bos/usr/ccs/lib/libc/mbscmp.c, libcnls, bos411, 9428A410j 2/17/93 14:09:22";
#pragma alloca
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: mbscmp
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */

#include <stddef.h>
#include <sys/types.h>

/*
 * NAME: mbscmp
 *                                                                    
 * FUNCTION: Compare characters (code points) of one multibyte character 
 *	     string to another multibyte character string.
 *
 * PARAMETERS: 
 *	     char *s1 - string
 *	     char *s2 - string
 *
 * RETURN VALUE DESCRIPTION: Returns s1>s2; >0  s1==s2; 0  s1<s2; <0.
 * NOTE:
 *  Two versions here:  mbscmp (operates on ASCII with embedded NLS
 *  code points) and wcscmp (operates on wchar_t).
 *
 */

int mbscmp(const char *s1, const char *s2)
{
    wchar_t *wcs1;
    wchar_t *wcs2;

    if ((wcs1 = (wchar_t *) alloca ((strlen(s1)+1) * sizeof(wchar_t))) == NULL)
	return(strcmp(s1,s2));
    if ((wcs2 = (wchar_t *) alloca ((strlen(s2)+1) * sizeof(wchar_t))) == NULL)
	return(strcmp(s1,s2));

    if (mbstowcs(wcs1, s1, strlen(s1)+1) == -1)
	return(strcmp(s1,s2));
    if (mbstowcs(wcs2, s2, strlen(s2)+1) == -1)
	return(strcmp(s1,s2));

    return(wcscmp(wcs1,wcs2));
}
