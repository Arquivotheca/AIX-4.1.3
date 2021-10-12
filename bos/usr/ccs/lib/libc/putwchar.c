static char sccsid[] = "@(#)03	1.3  src/bos/usr/ccs/lib/libc/putwchar.c, libcio, bos411, 9428A410j 6/16/90 01:19:07";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: putwchar 
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

#include <stdio.h>

/*
 * FUNCTION:	
 * Put a wide char to stdout.  (A version of putwc for stdout)
 *
 * RETURN VALUE DESCRIPTION:	
 * Returns EOF on any error condition.
 *
 */

int putwchar(int c)
{
	return(putwc(c, stdout));
}
