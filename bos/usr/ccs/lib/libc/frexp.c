static char sccsid[] = "@(#)18	1.13  src/bos/usr/ccs/lib/libc/frexp.c, libccnv, bos411, 9428A410j 12/7/93 08:11:14";
/*
 * COMPONENT_NAME: LIBCCNV frexp
 *
 * FUNCTIONS: frexp
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <math.h>
#include <limits.h>
#include <fp.h>

/*
 * NAME: frexp
 *                                                                    
 * FUNCTION: return mantissa of a double
 *                                                                    
 * NOTES:
 *      double frexp(double value, int *eptr)
 *
 *      frexp assumes that all floating point numbers can be
 *      be represented as: 0.ffff * 2^n. Where ffff is the fraction
 *      (or mantissa) and n is a power of 2. frexp returns the
 *      mantissa as the function value. The mantissa is normalized
 *      and thus guaranteed to be in the range: 0.5 <= |x| < 1.0.
 *      n is returned to the location pointed to by eptr.
 *
 * **********************************************************************
 *
 *      Portable version of frexp. This version should run on
 *      any hardware that supports IEEE standard double
 *      precision and has the Berkeley logb, scalb, and finite
 *      functions available.
 *
 *      It is quite possible that a faster version of this routine
 *      could be written for specific hardware.
 *
 *  *********************************************************************
 *
 * RETURNS: mantissa
 *
 *  The special cases are:
 *
 *      value == QNaN:         return QNaN, *eptr = INT_MIN
 *      value == SNaN:         return QNaN, *eptr = INT_MIN
 *      value == +/-infinity:  return +/-infinity, *eptr = INT_MAX
 *      value == +/-0:         return +/-0, *eptr = 0
 *
 */

static unsigned pzero[]={ INTS2DBL(0x00000000, 0x00000000)};  /* +0.0 */
static unsigned mzero[]={ INTS2DBL(0x80000000, 0x00000000)};  /* -0.0 */


double
frexp(double value, int *eptr)
{
	double dexp;		/* return from logb */
    
	if(!finite(value))
	{
		if (value != value) 
		{
			*eptr = LONG_MIN;   /* NaN */
			return(value + 1.0);  /* add of 1.0 quiets SNaN */
		}
		else 
		{
			if ( value > 0 ) 
			{
				*eptr = INT_MAX;   
				return value;
			}
			else 
			{
				*eptr = INT_MIN;   
				return value;
			}
		}
	}
	if (value == 0.0)           /* zero */
	{       
		*eptr = 0;
		return(value);
	}

	/* value is normalized or denormalized */

	dexp = logb(value);	/* extract IEEE exponent */
	dexp++;			/* incr it fro OLD System V exponent */
	*eptr = (int) dexp;	/* store it */
	return scalb(value, -dexp); /* make result -0.5 <= |x| < 1.0 */
}
