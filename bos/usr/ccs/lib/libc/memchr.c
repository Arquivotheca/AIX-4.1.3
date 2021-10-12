static char sccsid[] = "@(#)90	1.2  src/bos/usr/ccs/lib/libc/memchr.c, libcmem, bos411, 9428A410j 8/2/91 15:59:38";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: memchr
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
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

#include <string.h>			/* for NULL and size_t	*/

#ifdef memchr
#   undef memchr
#endif /* memchr */

/*
 * FUNCTION:	Return the ptr in s at which the character c appears;
 *		NULL if not found in n chars; don't stop at \0.
 *                                                                    
 * RETURN VALUE DESCRIPTION:	
 *
 *		Returns a pointer to the located character, or a
 *		null pointer if the character does not occur in the
 *		subject.
 */  


void *
memchr(const void *s, int c, size_t n)
{
	/* required to do pointer arithmetic for void * */
	unsigned char *s1 = (unsigned char *)s;

 	while (n-- > 0)
		if (*s1++ == (unsigned char)c) {
			s = --s1;
			return ((void*)s);
		}

	return (NULL);
}
