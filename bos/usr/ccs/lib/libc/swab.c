static char sccsid[] = "@(#)94	1.2  src/bos/usr/ccs/lib/libc/swab.c, libcmem, bos411, 9428A410j 2/11/93 08:50:28";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: swab
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

#include <sys/limits.h>
#include <unistd.h>

/*
 * NAME:	swab
 *
 * FUNCTION:	swab - swap bytes in 16-bit [half-]words
 *
 * NOTES:	Swab copies 'ntypes' bytes from 'from' to
 *		'to' exchanging adjacent even and odd bytes.
 *
 * RETURN VALUE DESCRIPTION:	none
 */

void
swab(const void *src, const void *dest, ssize_t nbytes)
{
	unsigned short *from = (unsigned short *)src;
	unsigned short *to = (unsigned short *)dest;
	nbytes /= 2;			/* number of exchanges to make	*/

	while(--nbytes >= 0) {
		*to++ = (*from << CHAR_BIT) + ((*from >> CHAR_BIT) & CHAR_MAX);
		from++;
	}
}
