#if (!( _FRTINT ))
static char sccsid[] = "@(#)79	1.2  src/bos/usr/ccs/lib/libm/POWER/floor.c, libm, bos411, 9428A410j 9/7/93 09:18:21";
#endif
/*
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS: floor, ceil, nearest, trunc, fabs,
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <math.h>
#include <float.h>
#include <fp.h>

static	double	zero = 0.0;

/*
 *
 *   This module contains the following routines:
 *
 *      double    floor(double x);
 *
 *      double    ceil(double x);
 *
 *      double    nearest(double x);
 *
 *      double    trunc(double x);
 *
 *      double    fabs(double x);
 *
 *
 *      The code in this module replaces floor.c in the Berkeley 
 *      math library.  The routines assume that the arithmetic conforms
 *      to the IEEE standard, and that the compiler supports the following
 *      built-in functions:
 *        __setrnd(x)  This function returns the current floating point status
 *           and control information, including the current rounding mode.
 *           The new rounding mode is determined by the integer x as follows:
 *             00 round-to-nearest
 *             01 round-toward-zero (a.k.a truncate)
 *             10 round-toward-plus-infinity (a.k.a. ceil)
 *             11 round-toward-minus-infinity (a.k.a. floor)
 *
 *        __setflm(x)  This function returns the current floating point status
 *           and control information, including the current rounding mode, and
 *           sets the new f.p. status and control information to x.  x is in
 *           the same format as the results returned by __setrnd and __setflm
 *               
 *      The routines assume that double IEEE floating point numbers are
 *      encoded with the sign in the ms bit followed by the exponent in
 *      the next 11 ms bits. (This is the common representation found in
 *      in almost all current implementations of the IEEE standard.)
 *
 */

#define closest 00
#define truncate 01
#define ceiling 10
#define bottom 11

/*
 * NAME: floor
 *
 * FUNCTION: RETURNS THE GREATEST DOUBLE INTEGER <= X
 *
 * NOTES:
 *
 * RETURNS: the largest fp integer not greate than x
 *
 */

double
#ifdef _FRTINT
_floor(double x)
#else
floor(double x)
#endif

{
	double u, v, q;
	double big = 8192.0 * 8192.0 * 8192.0 * 8192.0; /* 2**52 */
	
	(fabs(x) < big) & (x > 0.0);  /* "compiler directive"-do all compares 
				       * before branches*/
	q = __setrnd(bottom);       /* round down mode*/
	u = x + big;       /* throws away fraction bits for positive x*/
	v = x - big;       /* throws away fraction bits for negative x*/
	__setflm(q);       /* restore user fl. pt. status and rounding mode*/
	if (fabs(x) < big) { /* is there rounding down to be done? */
		if (x == 0.0)		  /* XPG4 requirement */
			return x;
		if (x > 0.0) 
			return(u - big);  /* yes - finish the job, depending 
					   * on sign*/
		else 
			return(v + big);
		}
	else 
		return(x + zero);  /* adding zero converts SNaNs to QNaNs, 
				    * and sets fp status*/
}




#ifndef _FRTINT

/*
 * NAME: ceil  
 *
 * FUNCTION: RETURNS THE SMALLEST DOUBLE INTEGER >= Xdouble u, v, q;
 
 *
 * NOTES:
 *
 * RETURNS: the smallest fp integer not less than x
 *
 */


double
ceil(double x)
{ 
	double u, v, q;
	double big = 8192.0 * 8192.0 * 8192.0 * 8192.0; /* 2**52 */

	(fabs(x) < big) & (x > 0.0);  /* "compiler directive"-do all compares 
				     * before branches*/
	q = __setrnd(ceiling);      /* round up mode*/
	u = x + big;       /* throws away fraction bits for positive x*/
	v = x - big;       /* throws away fraction bits for negative x*/
	__setflm(q);       /* restore user floating point status and rounding 
			    * mode*/
	if (fabs(x) < big) { /* is there rounding up to be done? */
		if (x == 0.0)		  /* XPG4 requirement */
			return x;
		if (x > 0.0) 
			return(u - big);  /* yes - finish the job, depending 
					   * on sign*/
		else 
			return(v + big);
		}
	else 
		return(x + zero);  /* adding zero converts SNaNs to QNaNs, 
				    * and sets fp status*/
}

/*
 * NAME: nearest
 *
 * FUNCTION: RETURNS THE DOUBLE INTEGER NEAREST TO X.
 *
 * NOTES:
 *   If x lies half way between two double integers then the even
 *   integer is returned.
 *
 * RETURNS: nearest fp integer to x
 *
 */


double
nearest(double x)
{
	double u, v, q;
	double big = 8192.0 * 8192.0 * 8192.0 * 8192.0; /* 2**52 */

	(fabs(x) < big) & (x > 0);  /* "compiler directive"-do all compares 
				     * before branches*/
	q = __setrnd(closest);      /* round-to-nearest mode*/
	u = x + big;       /* throws away fraction bits for positive x*/
	v = x - big;       /* throws away fraction bits for negative x*/
	__setflm(q);       /* restore user floating point status and rounding 
			    * mode*/
	if (fabs(x) < big) /* is there rounding to be done? */
		if (x > 0) 
			return(u - big);  /* yes - finish the job, depending 
					   * on sign*/
		else 
			return(v + big);
	else 
		return(x + zero);  /* adding zero converts SNaNs to QNaNs, 
				    * and sets fp status*/
}



/*
 * NAME: trunc
 *
 * FUNCTION: RETURNS THE NEAREST DOUBLE INTEGER TO X IN THE DIRECTION OF 0.0.
 *
 * NOTES:
 *
 * RETURNS: nearest fp integer to x in the direction of 0
 *
 */


double
trunc(double x)
{
	double u, v, q;
	double big = 8192.0 * 8192.0 * 8192.0 * 8192.0; /* 2**52 */

	(fabs(x) < big) & (x > 0);  /*"compiler directive"-do all compares 
				     * before branches*/
	q = __setrnd(truncate);     /* truncate mode*/
	u = x + big;       /* throws away fraction bits for positive x*/
	v = x - big;       /* throws away fraction bits for negative x*/
	__setflm(q);       /* restore user floating point status and 
			    * rounding mode*/
	if (fabs(x) < big) /* is there truncation to be done? */
		if (x > 0) 
			return(u - big);  /* yes - finish the job, depending 
					   * on sign*/
		else 
			return(v + big);
	else 
		return(x + zero);  /* adding zero converts SNaNs to QNaNs, 
				    * and sets fp status*/
}

#ifdef fabs
#undef fabs
#endif

/*
 * NAME: fabs
 *
 * FUNCTION: RETURNS THE ABSOLUTE VALUE OF X.
 *
 * NOTES:
 *
 * No exceptions are signalled.
 *
 * RETURNS: fp absolute value of x
 *
 */

double
fabs(double x)
{   
	union {
		double y;
		unsigned long i;
	} arg;

	arg.y = x;
	arg.i &= 0x7fffffff;
	return(arg.y); 		/* return the absolute value */
}

#endif
