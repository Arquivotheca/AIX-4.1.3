static char sccsid[] = "@(#)69	1.20  src/bos/usr/ccs/lib/libm/drem.c, libm, bos411, 9428A410j 12/7/93 08:06:17";
/*
 *   COMPONENT_NAME: LIBM
 *
 *   FUNCTIONS: drem
 *		drem_fmod
 *		fmod
 *		remainder
 *
 *   ORIGINS: 11,26,27
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
 *
 */

#include <sys/errno.h>
#include <math.h>
#include <fp.h>

/*
 * NAME: drem
 *                                                                    
 * DREM(X,Y)
 * FUNCTION: RETURN X REM Y =X-N*Y, N=[X/Y] ROUNDED (ROUNDED TO EVEN IN THE
 *           HALF WAY CASE)
 * DOUBLE PRECISION (IEEE DOUBLE 53 BITS)
 *                                                                    
 * NOTES:
 *
 * IEEE p754 required operations:
 *     drem(x,p)
 *              returns  x REM y  =  x - [x/y]*y , where [x/y] is the integer
 *              nearest x/y; in half way case, choose the even one.
 *
 * This is the portable version of this function. It may set the
 * inexact exception bit even tho the remainder is always exact.
 * This occurs because the inexact bit is set by some of the
 * intermediate calculations. The other version of drem below is
 * faster and will set inexact correctly. However it requires some
 * hardware specific functions that are not portable. It is suggested
 * that an implementor either create these functions for his hardware
 * or, better yet, that he implements drem in assembly language.
 *
 * This function is portable to the extent that it runs on IEEE
 * Standard hardware that uses the common encoding of double values.
 *
 * Since both drem and fmod share most of their code, a common
 * routine "drem_fmod()" is used to calculate either the drem or fmod.
 *
 * RETURNS: double result from drem_fmod
 *
 */

#define DREM 1
#define FMOD 2

double drem(double x,double p)
{
	double drem_fmod();

	return(drem_fmod(x,p,DREM));
}

/* 
 * NAME: remainder
 * 
 * FUNCTION:  Calculates IEEE remainder (just like drem) but
 *            but the name and semantics regarding errno come
 *            from the COSE (UU) API.
 *
 * NOTES:
 *            If y is zero and x is not NaN, return NaN and errno = EDOM
 *            If x is +/-INF and y is not NaN, return NaN and errno = EDOM
 *            If either x or y is NaN, return x+y and errno is unchanged.
 *
 * RETURNS:   IEEE remainder.
 */

double
remainder(double x, double y)
  {
  double drem_fmod(double, double, int);
  if ((fabs(x) == HUGE_VAL) && (y == y))
    errno = EDOM;
  if ((y == 0.0) && (x == x))
    errno = EDOM;
  return drem_fmod(x, y, DREM);
  }


/*
 * NAME: fmod
 *                                                                    
 * FUNCTION: CALCULATES AN ANSI C STANDARD MODULO REMAINDER OF X/P.
 *                                                                    
 * NOTES:
 *
 * if (p == 0) then errno is set to EDOM and a QNaN is returned.
 *
 * This function is portable to the extent that it runs on IEEE
 * Standard hardware that uses the common encoding of double values.
 *
 * Since both drem and fmod share most of their code, a common
 * routine "drem_fmod()" is used to calculate either the drem or fmod.
 *
 * RETURNS: double result from drem_fmod
 *
 */

double fmod(double x,double p)
{
	double drem_fmod();
#ifdef _SVID
	double retval;
#endif

#ifndef _SVID
	if ( p == 0.0 )
		errno = EDOM;
	return(drem_fmod(x,p,FMOD));
#else
	if ( p == 0.0 ) {
#ifdef _SAA
		return (0.0);
#else
		return (x);
#endif
	}
	retval = drem_fmod(x,p,FMOD);
	if (!finite(retval)) {
#ifdef _SAA
		return (0.0);
#else
		return (x);
#endif
	}
	return (retval);
#endif
}


/*
 * NAME: drem_fmod
 *                                                                    
 * FUNCTION: DO EITHER AN IEEE REMAINDER (DREM) OR AN ANSI C FMOD OF X/P.
 *                                                                    
 * NOTES:
 *
 *  drem_fmod(x,p,which)
 *
 *  "which" determines whether it does the drem or fmod.
 *
 */

static unsigned long msign = 0x7fffffff, mexp = 0x7ff00000;
static short  gap = 20;
static double novf = 1.7E308, zero = 0.0;
static double prep1 = 54.0;

double drem_fmod(x,p,which)
double x,p;
int    which;

{
	/* if x or y is NaN return a QNaN */
	if ( unordered(x,p) )
		return (x * p);

	VALH(p) &= msign ;     /* p = |p| */

	/* if x == infinity or y == 0, return QNaN */
	if((VALH(x) & mexp) == mexp || p == zero)
	      return( zero/zero );

	/* if p == QNaN or SNaN, return QNaN    */
	if ( isnan(p) )
		return(p + 1.0);

	/* if p is subnormal or almost subnormal */
	else  if ( ((VALH(p) & mexp) >> gap) <= 1 )
	{ 
		double b;
		b = scalb(1.0, prep1);
		p *= b;
		x = drem_fmod(x,p,which);
		x *= b;
		return(drem_fmod(x,p,which)/b);
	}

	/* if p is large do: (drem_fmod(x/2,p/2)*2) */
	else  if(p >= (novf * 0.5))
	{
		if(p == HUGE_VAL)
			return (x);
		p *= 0.5;
		x *= 0.5;
		return(drem_fmod(x,p,which)*2);
	}

	/* normal algorithm */
	else
	{
		double hp,dp;
		long sign;

		dp = p + p; 
		hp = p * 0.5;
		sign = VALH(x) & ~msign;	/* sign x   */
		VALH(x) &= msign;		/* x = |x|  */

		/* main loop: reduce x to <= 2*p */
		while (x > dp)
		{
			unsigned long k;
			double tmp;

			k = (VALH(x) - VALH(dp)) & mexp;
			tmp = dp;
			VALH(tmp) += k ;
			if(x < tmp)
				VALH(tmp) -= 0x00100000;
			x -= tmp;
		}
		if (which == DREM) 
		{
			/* final iteration to get the remainder */
			if (x > hp)
			{
				x -= p;
				if (x >= hp)
					x -= p ;
			}
		}
		else
		{
			/* final iteration to get fmod */
			if(x >= p)
				x -= p;
			if(x >= p)
				x -= p;
		}
		if (x == 0.0)
			VALH(x) = (VALH(x) & msign) | sign;
		else
			VALH(x) = VALH(x) ^ sign;
		return(x);

	}
}
