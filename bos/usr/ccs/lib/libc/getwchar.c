static char sccsid[] = "@(#)02	1.3  src/bos/usr/ccs/lib/libc/getwchar.c, libcio, bos411, 9428A410j 2/26/91 13:41:14";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: getwchar 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

/*
 * FUNCTION:	
 * Get a one or multi byte character from stdin and return it as a wide char.
 *
 * RETURN VALUE DESCRIPTION:	
 * Returns wide char read or WEOF. 
 *
 */  
#undef getwchar

wint_t getwchar ()
{
	return(getwc(stdin));
}
