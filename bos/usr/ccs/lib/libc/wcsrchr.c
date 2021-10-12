static char sccsid[] = "@(#)26	1.1  src/bos/usr/ccs/lib/libc/wcsrchr.c, libcstr, bos411, 9428A410j 2/26/91 17:50:08";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library National Language Support
 *
 * FUNCTIONS: wcsrchr
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
 *  Return the ptr in sp at which the character c last appears; NULL
 *  if not found.
 */

#ifdef NULL
#undef NULL
#define NULL	0
#endif
/*
 * NAME: wcsrchr
 *
 * FUNCTION: look for the last occurrence of a wchar_t
 *
 * RETURN VALUE DESCRIPTION: returns either a NULL (failure) or a 
 * pointer to the occurrence of the char.
 *
 */

wchar_t * wcsrchr(wchar_t *sp, wchar_t c)
{
	register wchar_t *r;

	r = NULL;
	do {
		if(*sp == c)
			r = sp;
	} while(*sp++);
	return(r);
}
