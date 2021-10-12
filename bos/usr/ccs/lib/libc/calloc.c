static char sccsid[] = "@(#)87	1.2  src/bos/usr/ccs/lib/libc/calloc.c, libcmem, bos411, 9428A410j 9/2/93 10:41:31";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: calloc, cfree
 *
 * ORIGINS: 3 26 27
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <stdlib.h>
#include <string.h>

/*
 * FUNCTION: -  Allocate and clear memory block.
 *      	Note that the multiplication of num by size,
 *      	necessary to determine the number of bytes
 *      	to malloc, may result in truncation if the
 *      	request is VERY large.  If the result of this
 *      	multiplication is less than the requested number
 *      	of elements, and no zeros were passed in as
 *      	parameters, then overflow is assumed and NULL
 *      	is returned.
 *
 *
 * PARAMETERS: nmemb  - number of members to allocate for
 *
 *		size   - size of objects
 *
 * RETURN VALUE DESCRIPTIONS:
 *		either a pointer to the allocated space or a NULL pointer
 */



void *
calloc(size_t nmemb, size_t size)
{
	unsigned total;
	void *mp;

	total = nmemb * size;
	if (total < nmemb && nmemb != 0 && size != 0)    /* overflow */
		return(NULL);

	mp = malloc((size_t)total);
	if(mp != NULL)
		(void) memset(mp, 0, (size_t)total);

	return(mp);
}

void *
__calloc(size_t nmemb, size_t size)
{
	return(calloc(nmemb, size));
}

/*
 * NAME:	cfree
 *
 * FUNCTION:	cfree - free memory allocated by calloc
 *
 * NOTES:	Cfree frees memory previously allocated by
 *		calloc.  It's retained for backward compatibility.
 *
 * RETURN VALUE DESCRIPTION:	none
 */

/*ARGSUSED*/

void
cfree(void *p, unsigned nmemb, unsigned size)
{
	free(p);
}
