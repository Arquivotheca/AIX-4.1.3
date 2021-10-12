static char sccsid[] = "@(#)92	1.2  src/bos/usr/ccs/lib/libc/memmove.c, libcmem, bos411, 9428A410j 8/2/91 16:00:53";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: memmove
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <string.h>			/* for size_t	*/

#ifdef memmove
#   undef memmove
#endif /* memmove */

/*
 *                                                                    
 * FUNCTION:	Copy s2 to s1, always copy n bytes.
 *		Handle overlapping s2 and s1 areas.
 *                                                                    
 * RETURN VALUE DESCRIPTION:	Return s1.
 *
 */  


void *
memmove(void *s1, const void *s2, size_t n)
{
	unsigned char *os1 = s1;
	const unsigned char *os2 = s2;

	/*
	 *	does the target overlap the source?
	 *	i.e. is the target starting address within the
	 *	source we want to copy?
	 */
	if (os2 < os1 && os1 < os2 + n) {
		/*
		 * target overlaps.  copy backwards (right to left) so
		 * that we will not destroy the source...
		 */
		os1 += n;
		os2 += n;

		while (n-- > 0)
			*--os1 = *--os2;
		}

	else
		/*
		 * target does not overlap.  do 'normal' copy (left to right)
		 */
		while (n-- > 0)
			*os1++ = *os2++;

	return (s1);
}
