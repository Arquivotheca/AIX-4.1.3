static char sccsid[] = "@(#)75	1.12  src/bos/usr/ccs/lib/libc/ldexp.c, libccnv, bos411, 9428A410j 12/7/93 08:11:31";
/*
 * COMPONENT_NAME: LIBCCNV ldexp
 *
 * FUNCTIONS: ldexp
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

#include <errno.h>
#include <math.h>

/*
 * NAME: ldexp
 *                                                                    
 * FUNCTION: return value * 2 to the exponent
 *                                                                    
 * NOTES:
 *
 *      double ldexp(double value, int exp)
 *
 *      Ldexp returns value*2**exp, if that result is in range.
 *      If underflow occurs, it returns zero.  If overflow occurs,
 *      it returns an appropriate signed infinity and sets
 *      the external int "errno" is set to ERANGE.
 *
 *      Note that errno is
 *      not modified if no error occurs, so if you intend to test it
 *      after you use ldexp, you had better set it to something
 *      other than ERANGE first (zero is a reasonable value to use).
 *
 * **********************************************************************
 *
 *      Portable version of ldexp. This version should run on
 *      any hardware that supports IEEE standard double
 *      precision and has the Berkeley scalb and finite
 *      functions available.
 *
 *      It is quite possible that a faster version of this routine
 *      could be written for specific hardware.
 *
 *  *********************************************************************
 *
 * RETURNS: a double value
 *
 *  The special cases are:
 *
 *      if (value == NaN, infinity, 0.0 ) return(value)
 *      if (value == norm or denorm)
 *                  retval = value * 2^n
 *                  if (retval == infinity) set errno to ERANGE
 *                  return(retval)
 *
 */


double
ldexp(double value, int exp)
{
	double retval;

	/* infinities, QNaN's and Zeroes return themselves. SNaN -> QNaN  */
	if (value != value) 		/* SNaN or QNaN */
		return(value + 1.0);
	if ((!finite(value)) || (value == 0.0)) 
		return(value);

	/* Normalized or denormalized input */

	retval = scalb(value, (double) exp);
	if (finite(retval)) 
		return(retval);

	/* Result was infinity but "value" was norm or denorm. */
	/* Therefore, an overflow occurred                */

	errno = ERANGE;
	return(retval);
}
