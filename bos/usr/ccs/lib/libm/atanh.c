static char sccsid[] = "@(#)04	1.10  src/bos/usr/ccs/lib/libm/atanh.c, libm, bos411, 9428A410j 11/29/93 10:28:52";
/*
 * COMPONENT_NAME: libm
 *
 * FUNCTIONS: atanh
 *
 * ORIGINS: 11, 26, 27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1985 Regents of the University of California.
 * 
 * Use and reproduction of this software are granted  in  accordance  with
 * the terms and conditions specified in  the  Berkeley  Software  License
 * Agreement (in particular, this entails acknowledgement of the programs'
 * source, and inclusion of this notice) with the additional understanding
 * that  all  recipients  should regard themselves as participants  in  an
 * ongoing  research  project and hence should  feel  obligated  to report
 * their  experiences (good or bad) with these elementary function  codes,
 * using "sendbug 4bsd-bugs@BERKELEY", to the authors.
 */

#include <math.h>
#include <errno.h>

/*
 * NAME: atanh
 *                                                                    
 * FUNCTION: High level description of what the procedure does
 * ATANH(X)
 * FUNCTION: RETURN THE HYPERBOLIC ARC TANGENT OF X
 * DOUBLE PRECISION (VAX D format 56 bits, IEEE DOUBLE 53 BITS)
 * CODED IN C BY K.C. NG, 1/8/85; 
 * REVISED BY K.C. NG on 2/7/85, 3/7/85, 8/18/85.
 *                                                                    
 * NOTES:
 *
 * Required kernel function:
 *	log1p(x) 	...return log(1+x)
 *
 * Method :
 *	Return 
 *                          1              2x                          x
 *		atanh(x) = --- * log(1 + -------) = 0.5 * log1p(2 * --------)
 *                          2             1 - x                      1 - x
 *
 * Special cases:
 *	atanh(x) is NaN if |x| > 1 with signal;
 *	atanh(NaN) is that NaN with no signal;
 *	atanh(+-1) is +-INF with signal.
 *
 * Accuracy:
 *	atanh(x) returns the exact hyperbolic arc tangent of x nearly rounded.
 *	In a test run with 512,000 random arguments on a VAX, the maximum
 *	observed error was 1.87 ulps (units in the last place) at
 *	x= -3.8962076028810414000e-03.
 *
 * RETURNS: arc hyperbolic tangent of x in the range [-INF,+INF]
 *	    Returns a QNaN if |x| is greater than 1.
 *
 */  

double atanh(double x)
{
	double z;
	int flag;
	
	z = copysign(0.5,x);
	x = copysign(x,1.0);
	flag = (x > 1.0);
	x = x/(1.0 - x);

	/* Per COSE (UU) API, if fabs(x) > 1.0, errno = EDOM
	 * and return NaNQ.  Slightly convoluted code attempts
	 * to do evaluation of (x > 1.0) and division in
	 * parallel.
	 */
	if (flag)
	{
		errno = EDOM;
		return HUGE_VAL - HUGE_VAL;
	}
	
	return( z*log1p(x + x) );
}
