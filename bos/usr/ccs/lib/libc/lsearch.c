static char sccsid[] = "@(#)06	1.2  src/bos/usr/ccs/lib/libc/lsearch.c, libcsrch, bos411, 9428A410j 5/27/94 13:34:28";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: lsearch
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994 
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

#include <memory.h>			/* for memcpy */


/*
 * NAME:	lsearch
 *                                                                    
 * FUNCTION:	search/maintain a linear list.
 *                                                                    
 * NOTES:	Linear search algorithm, generalized from Knuth
 *		(6.1) Algorithm Q.  This version no longer has anything
 *		to do with Knuth's Algorithm Q, which first copies the
 *		new element into the table, then looks for it.  The
 *		assumption there was that the cost of checking for the
 *		end of the table before each comparison outweighed the
 *		cost of the comparison, which isn't true when an arbitrary
 *		comparison function must be called and when the copy itself
 *		takes a significant number of cycles.  Actually, it has
 *		now reverted to Algorithm S, which is "simpler."
 *
 *		'Compar' is a bsearch-type function.
 *		
 * RETURN VALUE DESCRIPTION:	either a pointer to the value it
 *		found within the list, or a pointer to the value it
 *		added within the list
 */  

/*
 * This function is now declared as specified by the XOPEN standard.
 */

typedef char *POINTER;			/* pointer type for lsearch() */

void *
lsearch(const void *key, void *base, size_t *nelp, size_t width,
        int (*compar)(const void *, const void *))
/* void *key;		Key to be located */
/* void *base;		Beginning of table */
/* size_t *nelp;	Pointer to current table size */
/* size_t width;	Width of an element (bytes) */
/* int (*compar)();	Comparison function */
{
	POINTER next = (char *)base + *nelp * width;	/* End of table */

	for ( ; (POINTER)base < next; base = (char *)base + width)
		if ((*compar)(key, base) == 0)
			return (base);	/* Key found */
	++*nelp;			/* Not found, add to table */

	return (memcpy(base, key, (size_t)width));	/* base now == next */
}
