static char sccsid[] = "@(#)01	1.3  src/bos/usr/ccs/lib/libc/fputwc.c, libcio, bos411, 9428A410j 6/16/90 01:17:21";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: fputwc 
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
 * Put a wide char to a stream.  producing either one or two bytes on output.
 *
 * RETURN VALUE DESCRIPTION:	
 * Returns EOF if an error occurs on the write.
 *
 */  

int fputwc(int c, FILE *fp)
{
	return(putwc(c, fp));
}
