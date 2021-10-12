static char sccsid[] = "@(#)05	1.4  src/bos/usr/ccs/lib/libc/POWER/llabs.c, libccnv, bos411, 9428A410j 2/9/94 09:34:26";
/*
 * COMPONENT_NAME: (LIBCCNV) LIB C CoNVersion
 *
 * FUNCTIONS: llabs
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME: llabs
 *
 * FUNCTION:
 *      Absolute value of a 64-bit signed integer.
 *
 * EXECUTION ENVIRONMENT:
 *      This is public routine useable by any user.
 *      It is reentrant.  The input arguments are passed in (r3,r4).
 *      The output is returned in (r3,r4).  All quantities are
 *      64-bit signed ints in 2's complement format, done as a pair of
 *      32-bit ints.
 *
 * RECOVERY OPERATION:
 *      N/A.
 *
 * DATA STRUCTURES:
 *      No global data structures.
 *
 * RETURNS:
 *      The absolute value the 64-bit integer.  No indication of
 *      overflow on the case of the maximum negative number.
 *
 * NOTES:
 *      Could look into assembler.
 */

#include <stdlib.h>		/* make sure prototype matches code */

#ifdef llabs			/* stdlib.h may define llabs to __llabs */
#undef llabs
#endif /* ifdef llabs */

long long int
llabs(long long int i)
{
    if (i < 0LL) {		/* positive or zero is left alone */
	i = -i;			/* ignore overflow on max negative
				 * number */
    };
    return (i);
}
