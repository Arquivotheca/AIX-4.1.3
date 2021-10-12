static char sccsid[] = "@(#)85	1.2  src/bos/usr/ccs/lib/libc/bcopy.c, libcmem, bos411, 9428A410j 3/4/94 10:22:42";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: bcopy
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <strings.h>

/*
 * NAME:	bcopy, ovbcopy
 *
 * FUNCTION:	bcopy - copy 'length' bytes from 'src' to 'dest'
 *
 * NOTES:	Bcopy copies 'length' bytes from 'src' to 'dest'.
 *		Overlapping 'src' and 'dest' are handled appropriately.
 *		Ovbcopy is provided as a compatibility interface.
 *
 * RETURN VALUE DESCRIPTION:	none
 */

void
bcopy(const void *src, void *dst, size_t length)
{
	/*
	 * let memmove do the work...
	 */

	(void) memmove( dst, src, length );
}

void
ovbcopy(char *src, char *dst, int length)
{
	/*
	 * let memmove do the work...
	 */

	(void) memmove( (void*)dst, (void*)src, (size_t)length );
}
