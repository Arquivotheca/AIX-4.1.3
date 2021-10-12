static char sccsid[] = "@(#)15	1.18  src/bos/usr/ccs/lib/libm/cabs.c, libm, bos411, 9428A410j 12/7/93 08:06:48";
/*
 *   COMPONENT_NAME: libm
 *
 *   FUNCTIONS: cabs, hypot
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
 */

#include <math.h>
#include <errno.h>
#include <fp.h>

/*
 * NAME: cabs
 *                                                                    
 * CABS(Z)
 * FUNCTION: RETURN THE ABSOLUTE VALUE OF THE COMPLEX NUMBER  Z = X + iY
 * DOUBLE PRECISION (VAX D format 56 bits, IEEE DOUBLE 53 BITS)
 * CODED IN C BY K.C. NG, 11/28/84.
 * REVISED BY K.C. NG, 7/12/85.
 *                                                                    
 * NOTES:
 *
 * Required kernel function :
 *	hypot(x,y)
 *
 * Method :
 *	cabs(z) = hypot(x,y) .
 *
 * RETURNS: complex absolute value of z
 *
 */

double cabs(struct dbl_hypot z)
{
	return(hypot(z.x,z.y));
}


/*
 * NAME: hypot
 *                                                                    
 * HYPOT(X,Y)
 * FUNCTION: RETURN THE SQUARE ROOT OF X^2 + Y^2  WHERE Z=X+iY
 * DOUBLE PRECISION (VAX D format 56 bits, IEEE DOUBLE 53 BITS)
 * CODED IN C BY K.C. NG, 11/28/84; 
 * REVISED BY K.C. NG, 7/12/85.
 *                                                                    
 * NOTES:
 *
 * Required system supported functions :
 *	copysign(x,y)
 *	finite(x)
 *	scalb(x,N)
 *	sqrt(x)
 *
 * Method :
 *	1. replace x by |x| and y by |y|, and swap x and
 *	   y if y > x (hence x is never smaller than y).
 *	2. Hypot(x,y) is computed by:
 *	   Case I, x/y > 2
 *		
 *				       y
 *		hypot = x + -----------------------------
 *			 		    2
 *			    sqrt ( 1 + [x/y]  )  +  x/y
 *
 *	   Case II, x/y <= 2 
 *				                   y
 *		hypot = x + --------------------------------------------------
 *				          		     2 
 *				     			[x/y]   -  2
 *			   (sqrt(2)+1) + (x-y)/y + -----------------------------
 *			 		    			  2
 *			    			  sqrt ( 1 + [x/y]  )  + sqrt(2)
 *
 *
 *
 * Special cases:
 *	hypot(x,y) is INF if x or y is +INF or -INF; else
 *	hypot(x,y) is NAN if x or y is NAN.
 *
 * Accuracy:
 * 	hypot(x,y) returns the sqrt(x^2+y^2) with error less than 1 ulps (units
 *	in the last place). See Kahan's "Interval Arithmetic Options in the
 *	Proposed IEEE Floating Point Arithmetic Standard", Interval Mathematics
 *      1980, Edited by Karl L.E. Nickel, pp 99-128. (A faster but less accurate
 *	code follows in	comments.) In a test run with 500,000 random arguments
 *	on a VAX, the maximum observed error was .959 ulps.
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following constants.
 * The decimal values may be used, provided that the compiler will convert
 * from decimal to binary accurately enough to produce the hexadecimal values
 * shown.
 *
 * RETURNS: euclidean distance for x and y
 *
 */

#ifdef VAX	/* VAX D format */
/* static double */
/* r2p1hi =  2.4142135623730950345E0     , Hex  2^  2   *  .9A827999FCEF32 */
/* r2p1lo =  1.4349369327986523769E-17   , Hex  2^-55   *  .84597D89B3754B */
/* sqrt2  =  1.4142135623730950622E0     ; Hex  2^  1   *  .B504F333F9DE65 */
static long    r2p1hix[] = { 
	0x8279411a, 0xef3299fc};
static long    r2p1lox[] = { 
	0x597d2484, 0x754b89b3};
static long     sqrt2x[] = { 
	0x04f340b5, 0xde6533f9};
#define   r2p1hi    (*(double*)r2p1hix)
#define   r2p1lo    (*(double*)r2p1lox)
#define    sqrt2    (*(double*)sqrt2x)
#else		/* IEEE double format */
static double
r2p1hi =  2.4142135623730949234E0     , /*Hex  2^1     *  1.3504F333F9DE6 */
r2p1lo =  1.2537167179050217666E-16   , /*Hex  2^-53   *  1.21165F626CDD5 */
sqrt2  =  1.4142135623730951455E0     ; /*Hex  2^  0   *  1.6A09E667F3BCD */
#endif

double hypot(double x,double y)
{
	static double zero=0.0, one=1.0;
	static double ibig=30.0;		/* fl(1+2**(2*ibig))==1 */
	double t,r;
	double tmpexp;
#ifdef _SVID
	struct exception exc;

	exc.arg1 = x;
	exc.arg2 = y;
	exc.name = "hypot";
#endif

	if(finite(x))
		if(finite(y))
		{
			x = copysign(x,one);
			y = copysign(y,one);
			if(y > x)
			{ 
				t = x;
				x = y;
				y = t;
			}
			if(x == zero) 
				return(y);
			if(y == zero) 
				return(x);
			tmpexp = logb(x);
			if(tmpexp - logb(y) > ibig)
			/* raise inexact flag and return |x| */
			{ 
				fp_set_flag((fpflag_t) FP_INEXACT);
				return(x); 
			}

			/* start computing sqrt(x^2 + y^2) */
			r = x - y;
			if(r > y) { 	/* x/y > 2 */
				r = x/y;
				r = r + sqrt(one + r*r); 
			}
			else {		/* 1 <= x/y <= 2 */
				r /= y; 
				t = r*(r + 2.0);
				r += t/(sqrt2 + sqrt(2.0 + t));
				r += r2p1lo; 
				r += r2p1hi; 
			}

			r = y/r;
#ifndef _SVID
			return(x + r);
#else
			exc.retval = x + r;
			if (! finite (exc.retval)) {
				exc.type = OVERFLOW;
				exc.retval = HUGE_VAL;
				if (!matherr(&exc)) {
					errno = ERANGE;
				}
			}
			return(exc.retval);
#endif

		}

		else if(y == y) {  	   /* y is +-INF */
#ifndef _SVID
			return(copysign(y,one));
#else
			exc.type = OVERFLOW;
			exc.retval = HUGE_VAL;
			if (!matherr(&exc)) {
				errno = ERANGE;
			}
			return(exc.retval);
#endif
		}
		else 
			return(y + 1.0);	/* y is NaN and x is finite */

	else if(x == x) {		   /* x is +-INF */
#ifndef _SVID
		return (copysign(x,one));
#else
		exc.type = OVERFLOW;
		exc.retval = HUGE_VAL;
		if (!matherr(&exc)) {
			errno = ERANGE;
		}
		return(exc.retval);
#endif
	}
	else if(finite(y)) 		/* x is NaN, y is finite */
		return(x + 1.0);	/* Quiet any SNaN's */
	else if( isnan(y) ) 			/* x and y is NaN */
		return(y + 1.0);	/* Quiet any SNaN's */
	else {				/* y is INF */
#ifndef _SVID
		return(copysign(y,one));
#else
		exc.type = OVERFLOW;
		exc.retval = HUGE_VAL;
		if (!matherr(&exc)) {
			errno = ERANGE;
		}
		return(exc.retval);
#endif
	}
}

/* A faster but less accurate version of cabs(x,y) */
#if 0
double hypot(x,y)
double x, y;
{
	static double zero=0, one=1;
	static double ibig=30.0;		/* fl(1+2**(2*ibig))==1 */
	double temp;
	double exp;

	if(finite(x))
		if(finite(y))
		{
			x = copysign(x,one);
			y = copysign(y,one);
			if(y > x)
			{ 
				temp = x; 
				x = y; 
				y = temp; 
			}
			if(x == zero) 
				return(zero);
			if(y == zero) 
				return(x);
			exp = logb(x);
			x = scalb(x,-exp);
			if(exp - logb(y) > ibig)
			/* raise inexact flag and return |x| */
			{ 
				fp_set_flag((fpflag_t) FP_INEXACT);
				return(scalb(x,exp)); 
			}
			else 
				y = scalb(y,-exp);
			return(scalb(sqrt(x*x+y*y),exp));
		}

		else if(y == y)   	   /* y is +-INF */
			return(copysign(y,one));
		else 
			return(y);	   /* y is NaN and x is finite */

	else if(x == x) 		   	/* x is +-INF */
		return (copysign(x,one));
	else if(finite(y)) 		/* x is NaN, y is finite */
		return(x + 1.0);
	else if( isnan(y) ) 			/* x and y is NaN */
		return(y + 1.0);  	
	else 				/* y is INF */
		return(copysign(y,one));
}
#endif
