#if (!( _FRTINT ))
static char sccsid[] = "@(#)92	1.17  src/bos/usr/ccs/lib/libm/atan2.c, libm, bos411, 9428A410j 12/7/93 08:06:45";
#endif
/*
 *   COMPONENT_NAME: libm
 *
 *   FUNCTIONS: _atan2
 *		atan2
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

#include <sys/errno.h>
#include <math.h>
#include <fp.h>

#ifdef atan2
#undef atan2
#endif

/*
 * NAME: atan2
 *                                                                    
 * ATAN2(Y,X)
 * FUNCTION: RETURN ARG (X+iY)
 * DOUBLE PRECISION (VAX D format 56 bits, IEEE DOUBLE 53 BITS)
 * CODED IN C BY K.C. NG, 1/8/85; 
 * REVISED BY K.C. NG on 2/7/85, 2/13/85, 3/7/85, 3/30/85, 6/29/85.
 *                                                                    
 * NOTES:
 *
 * Required system supported functions :
 *	copysign(x,y)
 *	scalb(x,y)
 *	logb(x)
 *	
 * Method :
 *	1. Reduce y to positive by atan2(y,x)=-atan2(-y,x).
 *	2. Reduce x to positive by (if x and y are unexceptional): 
 *		ARG (x+iy) = arctan(y/x)   	   ... if x > 0,
 *		ARG (x+iy) = pi - arctan[y/(-x)]   ... if x < 0,
 *	3. According to the integer k=4t+0.25 truncated , t=y/x, the argument 
 *	   is further reduced to one of the following intervals and the 
 *	   arctangent of y/x is evaluated by the corresponding formula:
 *
 *         [0,7/16]	   atan(y/x) = t - t^3*(a1+t^2*(a2+...(a10+t^2*a11)...)
 *	   [7/16,11/16]    atan(y/x) = atan(1/2) + atan( (y-x/2)/(x+y/2) )
 *	   [11/16.19/16]   atan(y/x) = atan( 1 ) + atan( (y-x)/(x+y) )
 *	   [19/16,39/16]   atan(y/x) = atan(3/2) + atan( (y-1.5x)/(x+1.5y) )
 *	   [39/16,INF]     atan(y/x) = atan(INF) + atan( -x/y )
 *
 * Special cases:
 * Notations: atan2(y,x) == ARG (x+iy) == ARG(x,y).
 *
 *	ARG( NAN , (anything) ) is NaN;
 *	ARG( (anything), NaN ) is NaN;
 *	ARG(+(anything but NaN), +-0) is +-0  ;
 *	ARG(-(anything but NaN), +-0) is +-PI ;
 *	ARG( 0, +-(anything but 0 and NaN) ) is +-PI/2;
 *	ARG( +INF,+-(anything but INF and NaN) ) is +-0 ;
 *	ARG( -INF,+-(anything but INF and NaN) ) is +-PI;
 *	ARG( +INF,+-INF ) is +-PI/4 ;
 *	ARG( -INF,+-INF ) is +-3PI/4;
 *	ARG( (anything but,0,NaN, and INF),+-INF ) is +-PI/2;
 *
 * Accuracy:
 *	atan2(y,x) returns (PI/pi) * the exact ARG (x+iy) nearly rounded, 
 *	where
 *
 *	in decimal:
 *		pi = 3.141592653589793 23846264338327 ..... 
 *    53 bits   PI = 3.141592653589793 115997963 ..... ,
 *    56 bits   PI = 3.141592653589793 227020265 ..... ,  
 *
 *	in hexadecimal:
 *		pi = 3.243F6A8885A308D313198A2E....
 *    53 bits   PI = 3.243F6A8885A30  =  2 * 1.921FB54442D18	error=.276ulps
 *    56 bits   PI = 3.243F6A8885A308 =  4 * .C90FDAA22168C2    error=.206ulps
 *	
 *	In a test run with 356,000 random argument on [-1,1] * [-1,1] on a
 *	VAX, the maximum observed error was 1.41 ulps (units of the last place)
 *	compared with (PI/pi)*(the exact ARG(x+iy)).
 *
 * Note:
 *	We use machine PI (the true pi rounded) in place of the actual
 *	value of pi for all the trig and inverse trig functions. In general, 
 *	if trig is one of sin, cos, tan, then computed trig(y) returns the 
 *	exact trig(y*pi/PI) nearly rounded; correspondingly, computed arctrig 
 *	returns the exact arctrig(y)*PI/pi nearly rounded. These guarantee the 
 *	trig functions have period PI, and trig(arctrig(x)) returns x for
 *	all critical values x.
 *	
 * Constants:
 * The hexadecimal values are the intended ones for the following constants.
 * The decimal values may be used, provided that the compiler will convert
 * from decimal to binary accurately enough to produce the hexadecimal values
 * shown.
 *
 * RETURNS: arc tangent of y/x in the range of [-PI,PI]
 *
 */  


static double 
athfhi =  4.6364760900080609352E-1    , /*Hex  2^ -2   *  1.DAC670561BB4F */
athflo =  4.6249969567426939759E-18   , /*Hex  2^-58   *  1.5543B8F253271 */
PIo4   =  7.8539816339744827900E-1    , /*Hex  2^ -1   *  1.921FB54442D18 */
at1fhi =  9.8279372324732905408E-1    , /*Hex  2^ -1   *  1.F730BD281F69B */
at1flo = -2.4407677060164810007E-17   , /*Hex  2^-56   * -1.C23DFEFEAE6B5 */
PIo2   =  1.5707963267948965580E0     , /*Hex  2^  0   *  1.921FB54442D18 */
PI     =  3.1415926535897931160E0     , /*Hex  2^  1   *  1.921FB54442D18 */
a1     =  3.3333333333333942106E-1    , /*Hex  2^ -2   *  1.55555555555C3 */
a2     = -1.9999999999979536924E-1    , /*Hex  2^ -3   * -1.9999999997CCD */
a3     =  1.4285714278004377209E-1    , /*Hex  2^ -3   *  1.24924921EC1D7 */
a4     = -1.1111110579344973814E-1    , /*Hex  2^ -4   * -1.C71C7059AF280 */
a5     =  9.0908906105474668324E-2    , /*Hex  2^ -4   *  1.745CE5AA35DB2 */
a6     = -7.6919217767468239799E-2    , /*Hex  2^ -4   * -1.3B0FA54BEC400 */
a7     =  6.6614695906082474486E-2    , /*Hex  2^ -4   *  1.10DA924597FFF */
a8     = -5.8358371008508623523E-2    , /*Hex  2^ -5   * -1.DE125FDDBD793 */
a9     =  4.9850617156082015213E-2    , /*Hex  2^ -5   *  1.9860524BDD807 */
a10    = -3.6700606902093604877E-2    , /*Hex  2^ -5   * -1.2CA6C04C6937A */
a11    =  1.6438029044759730479E-2    ; /*Hex  2^ -6   *  1.0D52174A1BB54 */

#ifdef _FRTINT
double _atan2(double y,double x)
#else
double atan2(double y,double x)
#endif
{  
	static double zero=0.0, one=1.0, small=1.0E-9, big=1.0E18;
	double t,z,signy,signx,hi,lo;
	int k;
	double kd, md;
	
#ifdef _SVID
	struct exception exc;

	exc.arg1 = y;
	exc.arg2 = x;
#endif

    /* if x or y is NAN */
	if( isnan(x) ) {
		return(x + 1.0); 
	}
	if( isnan(y) ) {
		return(y + 1.0);
	}

    /* copy down the sign of y and x */
	signy = copysign(one,y) ;  
	signx = copysign(one,x) ;  

    /* if x is 1.0, goto begin */
	if(x == 1.0) 
	{ 
		y = copysign(y,one); 
		t = y; 
		if(finite(t)) 
			goto begin;
	}

    /* when y = 0 */
	if(y == zero)
	{ 
#ifndef _SVID
		if (x == zero)
			errno=EDOM;
#else
		if (x == zero) {
			exc.type = DOMAIN;
			exc.name = "atan2";
			exc.retval = 0.0;
			if (!matherr(&exc))
			{       write (2, exc.name, 5);
				write (2, ": DOMAIN error\n", 15);
				errno = EDOM;
			}
			return(exc.retval);
		}
#endif
	    	return((signx == one)?y:copysign(PI,signy));
	}

    /* when x = 0 */
	if(x == zero) {
		return(copysign(PIo2,signy));
	}
	    
    /* when x is INF */
	if(!finite(x))
	{
		if(!finite(y)) {
			return(copysign((signx==one)?PIo4:3*PIo4,signy));
		}
	    	else {
			return(copysign((signx==one)?zero:PI,signy));
		}
	}

    /* when y is INF */
	if(!finite(y)) {
		return(copysign(PIo2,signy));
	}


    /* compute y/x */
	x = copysign(x,one); 
	y = copysign(y,one); 
	kd = logb(y);
	md = kd - logb(x);
	if (md > 60.0)
		t = big + big; 
	else 
	{
		if(md < -80.0) 
			t = y/x;
	    	else 
		{ 
			t = y/x ; 
			y = scalb(y,-kd); 
			x = scalb(x,-kd); 
		}
	}

    /* begin argument reduction */
begin:
	if (t < 2.4375) {		 

		/* truncate 4(t+1/16) to integer for branching */
	    	k = 4*(t + 0.0625);
	    	switch (k) {

	    	/* t is in [0,7/16] */
	    	case 0:                    
	    	case 1:
			if (t < small) 
		    	{ 
				/* raise inexact flag */
				fp_set_flag((fpflag_t) FP_INEXACT);
		      		return (copysign((signx>zero)?t:PI-t,signy)); 
			}

			hi = zero;  
			lo = zero;  
			break;

	    	/* t is in [7/16,11/16] */
	    	case 2:                    
			hi = athfhi; 
			lo = athflo;
			z = x + x;
			t = ((y + y) - x) / (z + y); 
			break;

	    	/* t is in [11/16,19/16] */
	    	case 3:                    
	    	case 4:
			hi = PIo4; 
			lo = zero;
			t = (y - x)/(x + y); 
			break;

	    	/* t is in [19/16,39/16] */
	    	default:                   
			hi = at1fhi; 
			lo = at1flo;
			z = y - x; 
			y = y + y + y; 
			t = x + x;
			t = ((z + z) - x)/(t + y); 
			break;
	    	}
	}
	/* end of if (t < 2.4375) */

	else                           
	{
	    	hi = PIo2; lo = zero;

	    	if (t <= big)  		/* t is in [2.4375, big] */
			t = -x/y;

	    	else          		/* t is in [big, INF] */
	      	{ 
			/* raise inexact flag */
			fp_set_flag((fpflag_t) FP_INEXACT);
			t = zero; 
		}
	}

    	/* end of argument reduction */

    	/* compute atan(t) for t in [-.4375, .4375] */
	z = t*t;
	z = t*(z*(a1+z*(a2+z*(a3+z*(a4+z*(a5+z*(a6+z*(a7+z*(a8+
			z*(a9+z*(a10+z*a11)))))))))));
	z = lo - z; 
	z += t; 
	z += hi;

	return(copysign((signx>zero)?z:PI-z,signy));
}
