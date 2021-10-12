static char sccsid[] = "@(#)bcmp.c	1.6  com/lib/c/gen,3.1.1,9021 6/16/90 01:08:31";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: bcmp 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <strings.h>

/*
 * NAME:	bcmp
 *
 * FUNCTION:	bcmp - compare 2 blocks of byte strings
 *
 * NOTES:	Bcmp compares 2 blocks of bytes strings, 'b1' and 'b2'
 *		for 'length' bytes.
 *
 * RETURN VALUE DESCRIPTION:	returns	< 0 if *b2 < *b1,
 *					> 0 if *b2 > *b1,
 *					0 if *b2 == *b1
 *
 */

int bcmp(const void *b1, const void *b2, size_t length)
{
	/*
	 * let memcmp do the work...
	 */

	return( memcmp( b2, b1, length ) );
}
