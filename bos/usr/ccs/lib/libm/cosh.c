#if ( !(_FRTINT ) )
static char sccsid[] = "@(#)59	1.15  src/bos/usr/ccs/lib/libm/cosh.c, libm, bos411, 9428A410j 2/6/91 07:28:47";
#endif
/*
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS: cosh
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
 * NAME: cosh
 *                                                                    
 * COSH(X)
 * FUNCTION: RETURN THE HYPERBOLIC COSINE OF X
 * DOUBLE PRECISION (VAX D format 56 bits, IEEE DOUBLE 53 BITS)
 * CODED IN C BY K.C. NG, 1/8/85; 
 * REVISED BY K.C. NG on 2/8/85, 2/23/85, 3/7/85, 3/29/85, 4/16/85.
 *                                                                    
 * NOTES:
 *
 * Required system supported functions :
 *	copysign(x,y)
 *	scalb(x,N)
 *
 * Required kernel function:
 *	exp(x) 
 *	exp__E(x,c)	...return exp(x+c)-1-x for |x|<0.3465
 *
 * Method :
 *	1. Replace x by |x|. 
 *	2. 
 *		                                        [ exp(x) - 1 ]^2 
 *	    0        <= x <= 0.3465  :  cosh(x) := 1 + -------------------
 *			       			           2*exp(x)
 *
 *		                                   exp(x) +  1/exp(x)
 *	    0.3465   <= x <= 22      :  cosh(x) := -------------------
 *			       			           2
 *	    22       <= x <= lnovfl  :  cosh(x) := exp(x)/2 
 *	    lnovfl   <= x <= lnovfl+log(2)
 *				     :  cosh(x) := exp(x)/2 (avoid overflow)
 *	    log(2)+lnovfl <  x <  INF:  overflow to INF
 *
 *	Note: .3465 is a number near one half of ln2.
 *
 * Special cases:
 *	cosh(x) is x if x is +INF, -INF, or NaN.
 *	only cosh(0)=1 is exact for finite x.
 *
 * Accuracy:
 *	cosh(x) returns the exact hyperbolic cosine of x nearly rounded.
 *	In a test run with 768,000 random arguments on a VAX, the maximum
 *	observed error was 1.23 ulps (units in the last place).
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following constants.
 * The decimal values may be used, provided that the compiler will convert
 * from decimal to binary accurately enough to produce the hexadecimal values
 * shown.
 *
 * RETURNS: hyperbolic cosine of x
 *	    If the correct value overflows, a signed HUGE_VAL is
 *	    returned and errno is set to ERANGE.
 *
 */


#ifdef VAX
/* double static  */
/* mln2hi =  8.8029691931113054792E1     , Hex  2^  7   *  .B00F33C7E22BDB */
/* mln2lo = -4.9650192275318476525E-16   , Hex  2^-50   * -.8F1B60279E582A */
/* lnovfl =  8.8029691931113053016E1     ; Hex  2^  7   *  .B00F33C7E22BDA */
static long    mln2hix[] = { 0x0f3343b0, 0x2bdbc7e2};
static long    mln2lox[] = { 0x1b60a70f, 0x582a279e};
static long    lnovflx[] = { 0x0f3343b0, 0x2bdac7e2};
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
double _cosh(double x)
#else
double cosh(double x)
#endif
{	
	static double half=1.0/2.0, small=1.0E-18; /* fl(1+small)==1 */
	double t;
#ifdef _SVID
	struct exception exc;
#endif

#ifndef VAX
	if(x != x) 				/* x is NaN */
		return(x + 1.0);
#endif
	if((x = copysign(x,1.0)) <= 22)
	{
	    	if(x < 0.3465)
		{
			if(x < small) 
				return(1.0 + x);
		
			else 
			{
				t = x + exp__E(x,0.0);
				x = t + t; 
				return(1.0 + t*t/(2.0 + x));
			}
		}

	    	else /* for x lies in [0.3465,22] */
	        { 
			t = exp(x); 
			return((t + 1.0/t)*half);
		}
	}

        /* for x lies in [lnovfl, lnovfl+ln2], decrease x by ln(2^(max+1)) 
         * and return 2^max*exp(x) to avoid unnecessary overflow 
         */
	if( lnovfl <= x && x <= (lnovfl+0.7)) 
	    	return(scalb((double)exp((x - mln2hi) - mln2lo),max));
#ifdef _SVID
	else if (x > LOGHUGE)
	{       
		exc.type = OVERFLOW;
		exc.name = "cosh";
		exc.arg1 = x;
		exc.retval = HUGE_VAL;
		if (!matherr(&exc))
			errno = ERANGE;
		return (exc.retval);
	}
#endif
	else 
	{  
		t = exp(x)*half;
	    	if(!finite(t)) 
			errno = ERANGE;
	    	return(t);        /* for large x,  cosh(x)=exp(x)/2  */
	}
}
