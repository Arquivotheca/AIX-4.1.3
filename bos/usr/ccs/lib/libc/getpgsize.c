static char sccsid[] = "@(#)49	1.5  src/bos/usr/ccs/lib/libc/getpgsize.c, libcsys, bos411, 9428A410j 6/16/90 01:33:26";
/*
 * COMPONENT_NAME: (LIBCSYS) Standard C Library System Functions 
 *
 * FUNCTIONS: getpagesize 
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

#include <sys/param.h>

/*
 * NAME:	getpagesize
 *
 * FUNCTION:	getpagesize(void) - return number of bytes in a page
 *
 * NOTES:	Getpagesize returns the number of bytes in a system
 *		page.
 *
 * RETURN VALUE DESCRIPTION:	numbert of bytes in a system page
 */

int
getpagesize(void)
{
	return (PAGESIZE);
}
