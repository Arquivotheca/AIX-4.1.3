static char sccsid[] = "@(#)39	1.7  src/bos/usr/ccs/lib/libc/mbscpy.c, libcnls, bos411, 9428A410j 7/22/91 10:14:41";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: mbscpy
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
#include <sys/types.h>

/*
 * NAME: mbscpy
 *                                                                    
 * FUNCTION: Copy characters (code points) from one multibyte character 
 *	     string to another multibyte character string.
 *
 * PARAMETERS: 
 *	     char *s1 - overlaid string
 *	     char *s2 - copied string
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer equal to s1.
 */

char  *
mbscpy(char *s1, const char *s2)
{
	strcpy(s1,s2);
	return(s1);
}
