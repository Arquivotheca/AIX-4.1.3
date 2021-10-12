static char sccsid[] = "@(#)80	1.7  src/bos/usr/ccs/lib/libc/divide.c, libcgen, bos411, 9428A410j 6/16/90 01:09:17";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: div, ldiv
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

#include <stdlib.h>
#include <limits.h>

/*
 * NAME: div
 *
 * FUNCTION: Find a quotient and a remainder
 *
 * PARAMETERS: 
 *	     int num - number to be divided
 *	     int denom - denominator
 *
 * RETURN VALUE DESCRIPTIONS:
 *	     - returns a structure of type div_t (see stdlib.h)
 *	     - structure contains a quotient and a remainder
 *
 */


struct div_t	
div(int numer, int denom)
{
	struct div_t xdiv;

        if (denom == 0) {
		if (numer >= 0)
			xdiv.quot = INT_MAX;
		else
			xdiv.quot = INT_MIN;
		xdiv.rem = 0;
	} else {
		xdiv.rem = numer % denom;
		xdiv.quot = numer / denom;
	}
	return(xdiv);
}

/*
 * NAME: ldiv
 *
 * FUNCTION: Find a quotient and a remainder
 *
 * PARAMETERS: 
 *	     long num - number to be divided
 *	     long denom - denominator
 *
 * RETURN VALUE DESCRIPTIONS:
 *	     - returns a structure of type ldiv_t (see stdlib.h)
 *	     - structure contains a quotient and a remainder
 *
 */
#include <stdlib.h>
#include <limits.h>

struct ldiv_t
ldiv(long int numer, long int denom)
{
	struct ldiv_t xldiv;

        if (denom == 0) {
		if (numer >= 0)
			xldiv.quot = LONG_MAX;
		else
			xldiv.quot = LONG_MIN;
		xldiv.rem = 0;
	} else {
		xldiv.rem = numer % denom;
		xldiv.quot = numer / denom;
	}
	return(xldiv);
}
