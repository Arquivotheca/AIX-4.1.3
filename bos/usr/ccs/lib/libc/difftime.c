static char sccsid[] = "@(#)13	1.9  src/bos/usr/ccs/lib/libc/difftime.c, libctime, bos411, 9428A410j 8/21/92 14:19:10";
/*
 * COMPONENT_NAME: (LIBCTIME) Standard C Library Time Management Functions 
 *
 * FUNCTIONS: difftime 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <time.h>
#include <stdlib.h>

/*
 * FUNCTION: Calculates the difference between two calandar times
 *
 * PARAMETERS: 
 *	     long time1  - one time value
 *	     long time0  - other time value
 *
 * RETURN VALUE DESCRIPTIONS:
 *	     - returns the difference in the two time values
 *
 */

double 	
difftime(time_t time1, time_t time0)
{
	return((double)(time1 - time0));
}
