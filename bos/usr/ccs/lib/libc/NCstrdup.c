static char sccsid[] = "@(#)81	1.1  src/bos/usr/ccs/lib/libc/NCstrdup.c, libcnls, bos411, 9428A410j 2/26/91 17:39:19";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: NCstrdup
 *
 * ORIGINS: 3 27
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
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */

#include <string.h>
#include <ctype.h>

/*
 *  returns a pointer to a wchar_t string which is a duplicate of the wchar 
 *  string pointed to by s1.  Space for the new string is allocated by using 
 *  MALLOC (BA_OS).  When a new string cannot be created a NULL pointer is 
 *  returned.
 */
wchar_t *NCstrdup(wchar_t *s1)
{
	register wchar_t *ns;

	if ((ns = (wchar_t *)malloc((size_t)(NCstrlen (s1) * sizeof (wchar_t)))) == NULL)
	    return (NULL);
	
	wcscpy(ns, s1);
	return(ns);
}
