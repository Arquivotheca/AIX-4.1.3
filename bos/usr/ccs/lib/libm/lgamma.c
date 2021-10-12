#if ( !(_FRTINT ) )
static char sccsid[] = "@(#)75	1.20  src/bos/usr/ccs/lib/libm/lgamma.c, libm, bos411, 9428A410j 4/8/91 17:04:52";
#endif
/*
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS: gamma, lgamma, asym, neg, pos
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <math.h>
#include <errno.h>
#include <fpxcp.h>

/*
 * NAME: gamma
 *                                                                    
 * FUNCTION: 
 *                                                                    
 * NOTES:
 *
 * C program for floating point log Gamma function
 *
 * lgamma(x) computes the log of the absolute
 * value of the Gamma function.
 * The sign of the lgamma function is returned in the
 * external quantity signgam.
 *
 * The coefficients for expansion around zero
 * are #5243 from Hart & Cheney; for expansion
 * around infinity they are #5404.
 *
 * Calls log, floor and sin.
 *
 * NOTE: The name for this module was changed from gamma.c to
 *       lgamma.c to make it compatible with Berkeley. There
 *       are entries with both names in this module.
 *
 * RETURNS: log of the gamma function
 *
 */

#ifndef _FRTINT

int	signgam = 0;
static double goobie	= 0.9189385332046727417803297;	/* log(2*pi)/2 */
static double pi	= 3.1415926535897932384626434;

#define M 6
#define N 8
static double p1[] = {
	0.83333333333333101837e-1,
	-.277777777735865004e-2,
	0.793650576493454e-3,
	-.5951896861197e-3,
	0.83645878922e-3,
	-.1633436431e-2,
};
static double p2[] = {
	-.42353689509744089647e5,
	-.20886861789269887364e5,
	-.87627102978521489560e4,
	-.20085274013072791214e4,
	-.43933044406002567613e3,
	-.50108693752970953015e2,
	-.67449507245925289918e1,
	0.0,
};
static double q2[] = {
	-.42353689509744090010e5,
	-.29803853309256649932e4,
	0.99403074150827709015e4,
	-.15286072737795220248e4,
	-.49902852662143904834e3,
	0.18949823415702801641e3,
	-.23081551524580124562e2,
	0.10000000000000000000e1,
};

static double asym(double arg);
static double pos(double arg);
static double neg(double arg);

#endif

double
#ifdef _FRTINT
_gamma(double arg)
#else
gamma(double arg)
#endif
{
	return(lgamma(arg));
}


#ifndef _FRTINT

/*
 * NAME: lgamma
 *                                                                    
 * FUNCTION: 
 *                                                                    
 * NOTES:
 *
 * RETURNS: log of the gamma function
 *	    If x is non-positive, HUGE_VAL is returned and the division
 *	    by zero bit in the FPSCR is set.
 *	    If the correct value overflows, HUGE_VAL is returned.
 *	    If the correct value underflows, 0 is returned.
 *
 */

double
lgamma(double arg)
{
#ifdef _SVID
	struct exception exc;
	double retval;

	signgam = 1.;
	if(arg <= 0.) return(neg(arg));
	if(arg > 8.) {
		retval = asym(arg);
		if (! finite (retval)) {
			exc.arg1 = arg;
	        	exc.type = DOMAIN;
			exc.name = "gamma";
			exc.retval = HUGE_VAL;
			if (!matherr(&exc))
			errno = ERANGE;
			return(exc.retval);
		}
		return(retval);
	}
	retval = pos(arg);
	if (! finite (retval)) {
		exc.arg1 = arg;
	       	exc.type = DOMAIN;
		exc.name = "gamma";
		exc.retval = HUGE_VAL;
		if (!matherr(&exc))
			errno = ERANGE;
	}
	return(retval);
#else
	signgam = 1.;
	if(arg <= 0.) 
		return(neg(arg));
	if(arg > 8.) 
		return(asym(arg));
	return(pos(arg));
#endif
}


/*
 * NAME: asym
 *                                                                    
 * FUNCTION: 
 *                                                                    
 * NOTES:
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 *
 */

static double
asym(double arg)
{
	double n, argsq;
	int i;

        if(arg==HUGE_VAL) return(arg);
	argsq = 1./(arg*arg);
	for(n = 0,i = M-1; i >= 0; i-- ) {
		n = n*argsq + p1[i];
	}
	return((arg-.5) * log(arg) - arg + goobie + n/arg);
}


/*
 * NAME: neg
 *                                                                    
 * FUNCTION: 
 *                                                                    
 * NOTES:
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 *
 */

static double
neg(double arg)
{
#ifdef _SVID
	struct exception exc;
#endif
	double zero = 0.0;
	double t;

	arg = -arg;
         
     /*
      * to see if arg were a true integer, the old code used the
      * mathematically correct observation:
      * sin(n*pi) = 0 <=> n is an integer.
      * but in finite precision arithmetic, sin(n*PI) will NEVER
      * be zero simply because n*PI is a rational number.  hence
      *	it failed to work with our newer, more accurate sin()
      * which uses true pi to do the argument reduction...
      *	temp = sin(pi*arg);
      */
	t = floor(arg);
	if (arg - t  > 0.5e0)
	    t += 1.e0;				/* t := integer nearest arg */
	signgam = (int) (t - 2*floor(t/2));	/* signgam =  1 if t was odd, */
						/*            0 if t was even */
	signgam = signgam - 1 + signgam;	/* signgam =  1 if t was odd, */
						/*           -1 if t was even */
	t = arg - t;				/*  -0.5 <= t <= 0.5 */
	if (t < 0.e0) {
	    t = -t;
	    signgam = -signgam;
	}
	if (t == 0.0) {				/* Return NaNQ for neg. int */
#ifdef _SVID
		exc.arg1 = -arg;
	        exc.type = SING;
		exc.name = "gamma";
		exc.retval = HUGE_VAL;
		errno = EDOM;
		if (!matherr(&exc))
		{       
			write (2, exc.name, 5);
			write (2, ": SING error\n", 13);
		}
		return(exc.retval);
#else
		fp_set_flag(FP_DIV_BY_ZERO);
		errno = EDOM;
		return (zero/zero);
#endif
	}
        if(arg > 8.)
	   return(-log(arg * sin(pi*t)/pi) -asym(arg));
	else
	   return(-log(arg) - log(sin(pi*t)/pi) - pos(arg));
}


/*
 * NAME: pos
 *
 * FUNCTION:
 *
 * NOTES:
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 *
 */

static double
pos(double arg)
{
	double n, d, s, p;
	register i;
	if(arg < 2.) {
             for(s=1.0, n=arg;n<2.0;n=n+1.0) {
                 s = s*n;
             }
	     p = n - 2.;
	     for(n = 0,d = 0,i = N-1; i >= 0; i-- ) {
		   n = n*p + p2[i];
		   d = d*p + q2[i];
	     }
             d = n/d;
             /* if arg < 4.450148e-308 (2*MINDOUBLE), d/s may overflow */
             if(arg<=4.45148e-308) return(log(d)-log(s));
             /* for performance reason, use log(d/s) instead of log(d)-log(s) */
             else return(log(d/s));
        }
	if(arg > 3.)  {
             for(s=1.0, n=arg;n>3.0;n=n-1.0) {
                 s = s*(n-1.0);
             }
	     p = n - 2.;
	     for(n = 0,d = 0,i = N-1; i >= 0; i-- ) {
		   n = n*p + p2[i];
		   d = d*p + q2[i];
	     }
             d = n/d;
             return(log(d*s));
        }
	p = arg - 2.;
	for(n = 0,d = 0,i = N-1; i >= 0; i-- ) {
	      n = n*p + p2[i];
	      d = d*p + q2[i];
	}
        return(log(n/d));
}

#endif
