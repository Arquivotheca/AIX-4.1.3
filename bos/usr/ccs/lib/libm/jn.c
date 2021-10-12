static char sccsid[] = "@(#)64	1.15  src/bos/usr/ccs/lib/libm/jn.c, libm, bos411, 9428A410j 9/7/93 09:16:06";
/*
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS: jn, yn
 *
 * ORIGINS: 3, 27
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

/*
 *	floating point Bessel's function of
 *	the first and second kinds and of
 *	integer order.
 *
 *	int n;
 *	double x;
 *	jn(n,x);
 *
 *	returns the value of Jn(x) for all
 *	integer values of n and all real values
 *	of x.
 *
 *	There are no error returns.
 *	Calls j0, j1.
 *
 *	For n=0, j0(x) is called,
 *	for n=1, j1(x) is called,
 *	for n<x, forward recursion us used starting
 *	from values of j0(x) and j1(x).
 *	for n>x, a continued fraction approximation to
 *	j(n,x)/j(n-1,x) is evaluated and then backward
 *	recursion is used starting from a supposed value
 *	for j(n,x). The resulting value of j(0,x) is
 *	compared with the actual value to correct the
 *	supposed value of j(n,x).
 *
 *	yn(n,x) is similar in all respects, except
 *	that forward recursion is used for all
 *	values of n>1.
 */

#include <math.h>
#include <values.h>
#include <errno.h>
#include <fp.h>			/* VALH macro */

#define NEG_INF  (*((double *) (neg_inf)))
static const long neg_inf[] =  {0xfff00000, 0x0};

#ifndef _SVID
/* IEEE double */
static double zero = 0.e0;
#endif

double
jn(int n,double x) 
{
	int i;
	double a, b, temp;
	double xsq, t;
#ifdef _SVID
	struct exception exc;

	exc.arg1 = n;
	exc.arg2 = x;
	exc.name = "jn";
#endif

	if(n < 0){
		n = -n;
		x = -x;
	}

#ifdef _SVID
	if (exc.arg2 > X_TLOSS)
	{       
		exc.type = TLOSS;
		exc.retval = 0.0;
		if (!matherr(&exc))
		{       write (2, exc.name, 2);
			write (2, ": TLOSS error\n", 14);
			errno = ERANGE;
		}
		return (exc.retval);
	}
#endif

	if(n == 0)
		return(j0(x));
	if(n == 1)
		return(j1(x));
	if(x == 0.) 
		return(0.);
	if(n > x) 
		goto recurs;

	a = j0(x);
	b = j1(x);
	for(i = 1; i < n; i++) {
		temp = b;
		b = (2.*i/x)*b - a;
		a = temp;
	}
	return(b);

recurs:
	xsq = x*x;
	for(t = 0, i = n+16; i > n; i--) {
		t = xsq/(2.*i - t);
	}
	t = x/(2.*n - t);

	a = t;
	b = 1;
	for(i = n-1; i > 0; i--) {
		temp = b;
		b = (2.*i/x)*b - a;
		a = temp;
	}
	return(t*j0(x)/b);
}

double
yn(int n,double x) 
{
	int i;
	int sign;
	double a, b, temp;
	double minus_inf;
#ifdef _SVID
	struct exception exc;
#endif

#ifndef _SVID /* libm.a code */

	if (x == 0.0) {
	        return -HUGE_VAL;       /* -INF per XPG4 */
	}

	if (x < 0.0) {
		return(zero/zero);	/* IEEE machines: invalid operation */
	}	

#else /* #ifndef _SVID; must be libmsaa.a code */

	if (x <= 0) {
		exc.arg1 = n;
		exc.arg2 = x;
		exc.name = "yn";
		exc.type = DOMAIN;
		exc.retval = -HUGE_VAL;
		if (!matherr(&exc)) {
			write(2,"yn: DOMAIN error\n",17);
			errno = EDOM;
		}
		return(exc.retval);
	}

#endif /* #ifndef _SVID */

	sign = 1;
	if(n < 0){
		n = -n;
		if(n%2 == 1) 
			sign = -1;
	}
	if(n == 0) 
		return(y0(x));
	if(n == 1) 
		return(sign*y1(x));

	minus_inf = NEG_INF;
	a = y0(x);
	b = y1(x);
	for(i = 1; i < n; i++) {
		/* recomputation of b below can result in NAN if
		 * both a and b are INF.  For small arguments the
		 * result should be -INF, so we must test and 
		 * intercept when this happens.
		 */
		if (b == minus_inf)
			return b;
		temp = b;
		b = (2.*i/x)*b - a;
		a = temp;
	}
	return(sign*b);
}
