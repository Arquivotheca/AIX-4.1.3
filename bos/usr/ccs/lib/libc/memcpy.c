static char sccsid[] = "@(#)95	1.2  src/bos/usr/ccs/lib/libc/memcpy.c, libcmem, bos411, 9428A410j 8/2/91 16:00:27";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: memcpy
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

#include <string.h>			/* for size_t	*/

#ifdef memcpy
#   undef memcpy
#endif /* memcpy */

/*
 *                                                                    
 * FUNCTION:	Copy s2 to s1, always copy n bytes.
 *                                                                    
 * RETURN VALUE DESCRIPTION:	Returns s1.
 *
 */  


void *
memcpy(void *s1, const void *s2, size_t n)
{
	/* required for void * pointer arithmetic */
	unsigned char *os1 = s1;
	const unsigned char *os2 = s2;

	while (n-- > 0)
		*os1++ = *os2++;
	return (s1);
}
