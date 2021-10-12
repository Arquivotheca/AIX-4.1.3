#if ( !(_FRTINT ) )
static char sccsid[] = "@(#)16	1.13  src/bos/usr/ccs/lib/libm/sinh.c, libm, bos411, 9428A410j 6/16/90 02:14:54";
#endif
/*
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS: sinh
 *
 * ORIGINS: 11, 26
 *
 *                  SOURCE MATERIALS
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

#include <sys/errno.h>
#include <math.h>
#include <values.h>

/*
 * NAME: sinh
 *                                                                    
 * SINH(X)
 * FUNCTION: RETURN THE HYPERBOLIC SINE OF X
 * DOUBLE PRECISION (VAX D format 56 bits, IEEE DOUBLE 53 BITS)
 * CODED IN C BY K.C. NG, 1/8/85; 
 * REVISED BY K.C. NG on 2/8/85, 3/7/85, 3/24/85, 4/16/85.
 *                                                                    
 * NOTES:
 *
 * Required system supported functions :
 *	copysign(x,y)
 *	scalb(x,N)
 *
 * Required kernel functions:
 *	expm1(x)	...return exp(x)-1
 *
 * Method :
 *	1. reduce x to non-negative by sinh(-x) = - sinh(x).
 *	2. 
 *
 *	                                      expm1(x) + expm1(x)/(expm1(x)+1)
 *	    0 <= x <= lnovfl     : sinh(x) := --------------------------------
 *			       		                      2
 *     lnovfl <= x <= lnovfl+ln2 : sinh(x) := expm1(x)/2 (avoid overflow)
 * lnovfl+ln2 <  x <  INF        :  overflow to INF
 *	
 *
 * Special cases:
 *	sinh(x) is x if x is +INF, -INF, or NaN.
 *	only sinh(0)=0 is exact for finite argument.
 *
 * Accuracy:
 *	sinh(x) returns the exact hyperbolic sine of x nearly rounded. In
 *	a test run with 1,024,000 random arguments on a VAX, the maximum
 *	observed error was 1.93 ulps (units in the last place).
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following constants.
 * The decimal values may be used, provided that the compiler will convert
 * from decimal to binary accurately enough to produce the hexadecimal values
 * shown.
 *
 * RETURNS: the hyperbolic sine of x
 *	    If the correct value overflows, a signed HUGE_VAL is returned
 *	    and errno is set to ERANGE.
 *
 */

#ifdef VAX
/* double static */
/* mln2hi =  8.8029691931113054792E1     , Hex  2^  7   *  .B00F33C7E22BDB */
/* mln2lo = -4.9650192275318476525E-16   , Hex  2^-50   * -.8F1B60279E582A */
/* lnovfl =  8.8029691931113053016E1     ; Hex  2^  7   *  .B00F33C7E22BDA */
static long    mln2hix[] = {0x0f3343b0, 0x2bdbc7e2};
static long    mln2lox[] = {0x1b60a70f, 0x582a279e};
static long    lnovflx[] = {0x0f3343b0, 0x2bdac7e2};
#define   mln2hi    (*(double*)mln2hix)
#define   mln2lo    (*(double*)mln2lox)
#define   lnovfl    (*(double*)lnovflx)
#else	/* IEEE double */
static double
mln2hi =  7.0978271289338397310E2     , /*Hex  2^ 10   *  1.62E42FEFA39EF */
mln2lo =  2.3747039373786107478E-14   , /*Hex  2^-45   *  1.ABC9E3B39803F */
lnovfl =  7.0978271289338397310E2     ; /*Hex  2^  9   *  1.62E42FEFA39EF */
#endif

#ifdef VAX
static int max = 126;
#else	/* IEEE double */
static int max = 1023;
#endif

#ifdef _SVID
#define LOGHUGE	((DMAXEXP + 1) / LOG2E)
#define	LOG2E	1.4426950408889634074
#endif

#ifdef _FRTINT
double _sinh(double x)
#else
double sinh(double x)
#endif
{
	static double  half=1.0/2.0 ;
	double t, sign;
#ifdef _SVID
	struct exception exc;

	exc.arg1 = x;
#endif

#ifndef VAX
	if(x != x) 			/* x is NaN */
		return(x + 1.0);	
#endif
	sign = copysign(1.0,x);
	x = copysign(x,1.0);
	if(x < lnovfl)
	{
		t = expm1(x); 
		return(copysign((t + t/(1.0 + t))*half,sign));
	}

	else if(x <= lnovfl + 0.7)
		/*
		 * subtract x by ln(2^(max+1)) and return 2^max*exp(x) 
	    	 * to avoid unnecessary overflow
		 */
		return(copysign(scalb(1.0+expm1((x-mln2hi)-mln2lo),max),sign));

#ifdef _SVID
	else if (x > LOGHUGE)
	{       
		exc.type = OVERFLOW;
		exc.name = "sinh";
		exc.retval = copysign(HUGE_VAL,exc.arg1);
		if (!matherr(&exc))
			errno = ERANGE;
		return (exc.retval);
	}
#endif
	else /* sinh(+-INF) = +-INF, sinh(+-big no.) overflow to +-INF */
	{ 
		t = expm1(x);
		if (!finite(t)) 
			errno = ERANGE;
		return(t*sign);
	}
}
