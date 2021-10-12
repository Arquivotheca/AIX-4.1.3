#if (!( _FRTINT ))
static char sccsid[] = "@(#)41	1.15  src/bos/usr/ccs/lib/libm/POWER/tan.c, libm, bos411, 9428A410j 1/13/94 14:35:59";
#endif
/*
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS: tan
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <float.h>
#include <math.h>
#include <sys/errno.h>
#include <values.h>

extern long tantab[];

#ifdef tan
#undef tan
#endif

#ifdef _SVID
#define X_PLOSS_2      (X_PLOSS / 2)
#define X_TLOSS_2      (X_TLOSS / 2)
#endif

/*
 * NAME: tan
 *                                                                    
 * FUNCTION: TANGENT OF X
 *                                                                    
 * NOTES:
 *
 * Tan loses accuracy when passed large values.
 *
 * Input argument needs to be in fixed point and floating point registers
 *
 * RETURNS: the tangent of x
 *
 */

#ifdef _FRTINT
double _tan (double xx)
#else
double tan (double xx)
#endif
{

	static int pi2_1[] =  { 0x3ff921fb, 0x54442d18 }; /* high */
	static int pi2_2[] =  { 0x3c91a626, 0x33145c07 }; /* medium */
        static int pi2_3[] =  { 0xb91f1976, 0xb7ed8fbc }; /* low */
	double pi2_h, pi2_m, pi2_l;
	double temp1, temp2;
	double arg, argl, sq, a3, a4, t0, t1, y;
	double u, v, w, s, fmode;
	long i, iz, flag;
	static double pihbig = 13816870609430.547; /* pi*2**42 */ 
	static long recipi[]={ 0x3fe45f30,0x6dc9c883	};
	static double tn7 = 0.05396875452473400572649,
		        tn5 = 0.1333333333304691192925,
			tn3 = 0.3333333333333333357614,
			xn11 = 0.008898406739539066157565,
			xn9  = 0.02186936821731655951177,
			xn7  = 0.05396825413618260185395,
			xn5  = 0.1333333333332513155016,
			xn3  = 0.3333333333333333333333;
	static long very_tiny[]= { 0x00080000, 0 };
	static double sixteen_pihbig = 2.829695100811376e16;
	static long bigger2[] = { 0x43580000, 0x0 };

#include "atan.h"

#ifdef _SVID
	struct exception exc;
#endif

	union { 
		double flt;
		long i[2];
	} x;
	union { 
		double flt;
		long i[2];
	} a;

	union { 
		double flt;
		long i[2];
	} z;

	static long ndx[] =
	{  
		16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
		27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
		38, 39, 40, 41, 42, 43, 44, 45, 47, 48, 49,
		50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
		61, 62, 63, 64, 65, 66, 68, 69, 70, 71, 72,
		73, 74, 75, 76, 77, 78, 79, 81, 82, 83, 84,
		85, 86, 87, 88, 89, 91, 92, 93, 94, 95, 96,
		97, 98, 100, 101, 102, 103, 104, 105, 107, 108, 109,
		110, 111, 113, 114, 115, 116, 117, 119, 120, 121, 122,
		123, 125, 126, 127, 128, 130, 131, 132, 133, 135, 136,
		137, 139, 140, 141, 142, 144, 145, 146, 148, 149, 150,
		152, 153, 154, 156, 157, 159, 160, 161, 163, 164, 166,
		167, 168, 170, 171, 173, 174, 176, 177, 179, 180, 182,
		183, 185, 186, 188, 189, 191, 192, 194, 196, 197, 199,
		200, 202, 204, 205, 207, 209, 210, 212, 214, 215, 217,
		219, 220, 222, 224, 226, 228, 229, 231, 233, 235, 237,
		238, 240, 242, 244, 246, 248, 250, 252, 254, 256, 256
	};
	x.flt = xx;
        flag = (fabs(x.flt) < pihbig); /* less than pi*2**42 */
        CCLVI + big2;     /* force loading of CCLVI and big2 into registers */
	flag = (fabs(x.flt) <= 0.785398006439208984375); /* less than pi/4 */
	t = (struct tdef *)  (tantab - (16 *14)); 
	pi2_h = *(double *) pi2_1; /* pi/2 high */
	pi2_m = *(double *) pi2_2; /* pi/2 medium */
	pi2_l = *(double *) pi2_3; /* pi/2 low */
	fmode = __setflm(0.0); /* must use RIOS encoding - not ANSI */
	if (fabs(x.flt) < pihbig)
entry:
	/* less than pi/4 */
	{ 
		if ( fabs(x.flt) <= 0.785398006439208984375)
		{ 
			arg = x.flt;
			iz = 0;
                        y = fabs(arg);
		        a.flt = CCLVI * y +  big2 ;
		        i = ndx[a.i[1]-16];
		        flag = (arg > 0.0);
		      /*  flag = (a.i[1] < 16); */
			if ( a.i[1] < 16 )  /* fabs(arg) less than 0.0625 */
			{ 
                       	    u =  y;
			    sq = u * u;
			    a3 = sq * u;
			    xn11*sq+xn9;   
			    flag = (xn7 == xn7);
	  	  	    t1 = (((xn11*sq+xn9)*sq+xn7)*sq+xn5)*sq+xn3;
			    __setflm(fmode);
			    t1*a3+u;
                            if (arg > 0.0)
		                return (t1*a3+u);
                            else if(arg < 0.0)
			        return (-(t1*a3+u));
			    return(arg);	
			}
		        else  /* pi/4 >= fabs(arg) > 0.0625 */
		        {    
		            w = y -t[i].f0;
		            sq = w * w;
		            /* tan(delta) */
		            t0 = (( tn7 * sq + tn5) * sq + tn3) * (sq*w)+w;
	       	            v = t[i].p*t0;
		            sq = v * v;
		            a3 = 1.0 + v;
		            a4 =sq * sq;
		            w = a3 + a3 * sq;
	                    a3 = w + w * a4;
	                    __setflm(fmode);
		            w = t[i].p + (t0+v*t[i].p)*a3;
	                    if ( arg > 0.0) 
			        return(w);
		            else 
			        return(- w);
		         }
                }
		/* extract multiple of pi/2 */
		z.flt = x.flt * *((double*)recipi) + big2 ;
		iz = z.i[1] % 2;	/* modulo 2 */
		z.flt -= big2;  	/* normalize multiple of pi/2 */
		/* in rounding mode, */
		/* arg lies between plus & minus pi/4 */

		/* argument reduction uses a sextuple precision value of
		 * pi/2 (pi2_h, pi2_m, pi2_l) and yields a quad precision
		 * value for the reduced argument (arg, argl).  The
		 * high part is calculated exactly with 3 accumulate
		 * instructions.  The low part is calculated with only
		 * one term, but care must be taken to be sure it
		 * doesn't overlap the high part.
		 */

		temp1 = x.flt - z.flt * pi2_h;
		temp2 = temp1 - z.flt * pi2_m;
		arg = temp2 - z.flt * pi2_l;
		argl = (temp1 - arg) - z.flt * pi2_m;

		/* extract on the part of "arg1" which doesn't
		 * overlap the high part
		 */
		temp1 = arg + argl;
		argl = (arg - temp1) + argl;

                y = fabs(arg);
	        a.flt = CCLVI * y +  big2 ;

	        i = ndx[a.i[1]-16];		
	        flag = (arg > 0.0);
	        flag = (a.i[1] < 16);
		if ( a.i[1] < 16 )    /* fabs(reduced x) less than 0.0625 */
		{ 
                	u =  y;
			sq = u * u;
			v = argl;
			t1 = (((xn11*sq+xn9)*sq+xn7)*sq+xn5)*sq+xn3;
			a3 = sq * u;
			if ( iz == 0 )  /* tan section */ 
			{
#ifndef _SVID
                            if (arg > 0.0)
			       { w = t1*a3+v;
			        __setflm(fmode);
			        w = w + u;
		                return (w);}
                            else 
			       { w = v - t1*a3;
			        __setflm(fmode);
			        w = w - u;
			        return (w);}
#else
                            if (arg > 0.0) {
			        w = t1*a3+v;
			        __setflm(fmode);
			        w = w + u;
		                exc.retval = w;
			        goto ploss;}
                            else {
			        w = v - t1*a3;
			        __setflm(fmode);
			        w = w - u;
			        exc.retval = w;
			        goto ploss;}
#endif
			}
		        else   /* cot section */
			{ 
			    /* hi order part of cot */
                            if (arg > 0.0) 
				{s = t1*a3+v;
			         t0 = s+u;}
			    else
				{s = t1*a3-v;
			         t0 = s+u;}
		            flag = fabs(t0) < *(double*)very_tiny;
			    /* lo order part of cot */
			    a4 = (u-t0) + s; 
			    u = 1.0/t0;
	 	            if ( fabs (t0) < *(double*)very_tiny)
			    {
	               		     __setflm(fmode);
# ifndef _SVID
                                     if (arg > 0.0) 
					return (u); 
                                     else
					return (-u);
#else
                                     if (arg > 0.0) {
					exc.retval = u;
	           	        	goto ploss;}
                                     else {
					exc.retval = -u;
					goto ploss;}
#endif
		       	    }
			    v = 1.0 - u * t0;
			    __setflm(fmode);
# ifndef _SVID
                            if (arg > 0.0) 
			        return ((a4*u-v) *u -u);
                            else 
			        return (u - (a4*u-v)*u);
#else
			    if (arg > 0.0) {
			        exc.retval = (a4*u-v) *u -u;
			        goto ploss;}
                            else {
			        exc.retval = u - (a4*u-v);
			        goto ploss;}
#endif
			}
		}
		if (arg > 0.0) { 
		    w = y -t[i].f0 + argl;
		    sq = w * w;
		    /* 2nd order delta */
		    v =  y - t[i].f0 - w + argl;
		    t[i].p;
		    t0 = (( tn7 * sq + tn5) * sq + tn3) * (sq*w)+v+w;}
		else {
		    w = y -t[i].f0 - argl;
		    sq = w * w;
		    /* 2nd order delta */
		    v =  y - t[i].f0 - w - argl;
		    t[i].p;
		    t0 = (( tn7 * sq + tn5) * sq + tn3) * (sq*w)+v+w;}

		/* tan(delta) */
	        v = t[i].p*t0;
		/* polynomial approximation of 1/(1-v) */
		sq = v * v;
		a3 = 1.0 + v;
		a4 =sq * sq;
		w = a3 + a3 * sq;
		a3 = w + w * a4;   /* a3=1/(1-v) */
		s = (1.0+t[i].p*t[i].p)*a3; 
		if ( iz == 0 )   /* tan section */
		{     
	            __setflm(fmode);
	            w = t[i].p+t0*s;
	            if ( arg > 0.0) {
#ifndef _SVID
			return(w);
#else
	    	        exc.retval = w;
		        goto ploss;
#endif
	 	    }
		    else {
#ifndef _SVID
			return(-w);
#else
                        exc.retval = -w;
			goto ploss;
#endif
			}
		}
		else     /* cot section */
		{  
	            w = t[i].p+t0*s;
		    a4 = t[i].p - w + t0 * s;
		    u = 1.0 / w;
		    v = 1.0 - u * w;
                    u*a4-v;
	            __setflm(fmode);
	            if ( arg > 0.0 ) {
#ifndef _SVID
			return (u*(u*a4-v)-u);
#else
			exc.retval = u*(u*a4-v) - u;
			goto ploss;
#endif
			}
	 	   else {   
#ifndef _SVID
			return (u-u*(u*a4-v));
#else
			exc.retval = u-u*(u*a4-v);
			goto ploss;
#endif
			}
		}
	}

#ifdef _SVID
	if (fabs(xx) > X_TLOSS_2)           /* total loss of accuracy */
	{       
		exc.arg1 = xx;
		exc.type = TLOSS;
		exc.name = "tan";
		exc.retval = 0.0;
		if (!matherr(&exc))
		{       
			write (2, exc.name, 3);
			write (2, ": TLOSS error\n", 14);
			errno = ERANGE;
		}
		return (exc.retval);
	}
#endif

	if ( (x.i[0] & 0x7ff00000) >= 0x7ff00000)	/* Inf or NaN, */
	{  
		__setflm(fmode);
		return (x.flt-x.flt);			/* sin(x) = NaN */
	}

	/* Big reduction job!  */
	while ( fabs (x.flt) >= sixteen_pihbig )
	{ 
		w = x.flt * *(double*)recipi;
		x.flt = ( x.flt - w * pihh ) - w * pihl ;
	}
        do
	{ 
		w = x.flt * *(double*)recipi + *(double*)bigger2 -
		    *(double*)bigger2;
		x.flt = ( x.flt - w * pihh ) - w * pihl ;
	} while ( fabs(x.flt) >= pihbig);

        flag = (fabs(x.flt) < pihbig);
	flag = (fabs(x.flt) <= 0.785398006439208984375);

	goto entry;

#ifdef _SVID
ploss:
	if (fabs(xx) > X_PLOSS_2) /* partial loss of accuracy */
	{       
		exc.arg1 = xx;
		exc.type = PLOSS;
		exc.name = "tan";
		if (!matherr(&exc)) 
			errno = ERANGE;
	}
	return (exc.retval);
#endif
}
