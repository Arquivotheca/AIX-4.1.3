static char sccsid[] = "@(#)56	1.1  src/bos/usr/ccs/lib/libc/ffs.c, libcenv, bos411, 9428A410j 2/26/91 17:52:14";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: ffs
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/*
 * FUNCTION: find first set bit in mask
 *
 * RETURN VALUE DESCRIPTION: 
 *      returns 0 if there is an error, otherwise the bit number
 */
/*LINTLIBRARY*/
/*
 * Berkeley 4.3 compatible routine written for AIX 3.0
 */

ffs(mask)
int mask;
{
	register int i;

	for(i = 0; i < (sizeof(int) * 8); i++) {
		if (mask & 1)
			return (i+1);
		mask >>= 1;
	}
	return (0);
}
