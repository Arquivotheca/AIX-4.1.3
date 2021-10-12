static char sccsid[] = "@(#)19	1.11  src/bos/usr/ccs/lib/libc/modf.c, libccnv, bos411, 9428A410j 5/28/91 15:16:20";
/*
 * COMPONENT_NAME: LIBCCNV modf
 *
 * FUNCTIONS: modf
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

#include <float.h>
#include <math.h>
#include <fp.h>

/*
 * NAME: modf
 *                                                                    
 * FUNCTION: split a fp number into an integral part and a fraction
 *                                                                    
 * NOTES:
 *
 *      double modf(double value, double *iptr)
 *
 *      Split a floating point number into an integral part and a
 *      fraction. The fraction is the function return value. The
 *      integral part is stored indirectly thru iptr. Both are
 *      double results.
 *
 * **********************************************************************
 *
 *      Portable version of modf. This version should run on
 *      any hardware that supports IEEE standard double
 *      precision and has the Berkeley rint function available.
 *      It also requires a machine specific routine (fp_swap_rnd) that
 *      will have to be provided by the porter. "fp_swap_rnd" writes
 *      the IEEE style rounding mode and returns the previous mode.
 *      The encodings for the rounding mode are those specified by
 *      the ANSI C standard.
 *
 *      It is quite possible that a faster version of this routine
 *      could be written for specific hardware.
 *
 *  *********************************************************************
 *
 * RETURNS: a double which is the fractional part
 *
 *      Special cases:
 *
 *        if (value == + or - infinity)
 *              fraction = +0.0 or -0.0
 *              integral part = + or - infinity
 *        if (value == Nan)
 *              fraction = NaN
 *              integral part = Nan
 *
 *
 */

static unsigned long msign=0x7fffffff;

double
modf(value, iptr)
double value, *iptr;

{
	fprnd_t	old_round;
	double	retval;
	static unsigned mzero[] = { INTS2DBL(0x80000000, 0x00000000)};

	/* save current rounding mode and change it to Round toward Zero */
	old_round = fp_swap_rnd(FP_RND_RZ);
	*iptr = rint(value);            /* extract the integral part      */
	(void) fp_swap_rnd(old_round);  /* restore caller's rounding mode */
	if (value == HUGE_VAL)  
		return(0.0);
	if (value == -HUGE_VAL) 
		return( *((double *)mzero) );
	/* return signed fractional part with sign of input */
	if ((retval = (value - *iptr)) == 0.0)
		VALH(retval) = (VALH(retval) & msign) | (VALH(value) & ~msign);
	return (retval);
}
