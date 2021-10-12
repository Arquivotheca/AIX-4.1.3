#if (!( _FRTINT ))
static char sccsid[] = "@(#)34	1.10  src/bos/usr/ccs/lib/libm/POWER/asin.c, libm, bos411, 9428A410j 6/15/90 17:56:48";
#endif
/*
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS: asin
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/errno.h>
#include <math.h>

#ifdef asin
#undef asin
#endif

/*
 * NAME: asin
 *                                                                    
 * FUNCTION: RETURN THE PRINCIPAL VALUE OF THE ARC SINE OF X
 *                                                                    
 * NOTES:
 * 
 * Uses economized power series to evaluate arc-sine for arguments
 * whose magnitue is less than PI/2.  For arguments with greater
 * magnitude, it uses an economized power series to evaluate
 *	arcsin(x) = PI/2 - 2*arcsin(sqrt(0.5(1-x))).
 * The sqrt is computed concurrently with the evaluation of the
 * power series.
 *
 *
 * RETURNS: arc sine of x in the range [-PI/2,PI/2]
 *
 * A QNaN is returned and errno is set to EDOM if the input parameter
 * is not in the range [-1,+1]
 *
 */

extern short guesses[]; /* sqrt static ext, used for acos, asin */

#ifdef _FRTINT
double _asin(double x)
#else
double asin(double x)
#endif
{
#ifdef _SVID
	struct	exception exc;
#endif
	union { 
		double flt;
		unsigned long ci;
	} c;
	union { 
		double flt;
		unsigned long gm;
	} g;
	union { 
		double flt;
		unsigned long ym;
	} y;
	double b, s, t1, t0;
	double r, d, h, d1, h1, yy1, g1, d2, h2, gl;
	double e, e1, g2, u, v, yy0;
#ifndef _SVID
	double zero = 0.0;
#endif
	int    q, flag;
	static long pihh[]= { 0x3FF921FB, 0x54442D18 },
		pihl[]= { 0x3C91A626, 0x33145C07 };
	static double
		a14 =  0.03085091303188211304259,
		a13 = -0.02425125216179617744614,
		a12 = 0.02273334623573271023373,
		a11 = 0.0002983797813053653120360,
		a10 = 0.008819738782817955152102,
		a9  = 0.008130738583790024803650,
		a8  = 0.009793486386035222577675,
		a7  = 0.01154901189590083099584,
		a6  = 0.01396501522140471126730,
		a5  = 0.01735275722383019335276,
		a4  = 0.02237215928757573539490,
		a3  = 0.03038194444121688698408,
		a2  = 0.04464285714288482921087,
		a1  = 0.07499999999999990640378,
		a0  = 0.1666666666666666667191,
		aa7 = 0.01154901189590083099584,
		aa6 = 0.01396501522140471126730,
		aa5 = 0.01735275722383019335276,
		aa4 = 0.02237215928757573539490,
		aa3 = 0.03038194444121688698408,
		aa2 = 0.04464285714288482921087,
		aa1 = 0.07499999999999990640378,
		aa0 = 0.1666666666666666667191;


	flag = (fabs(x) < 1.0);
	flag = (fabs(x) <= 0.5);
	s =  a14 + a13 ;

	if (fabs(x) < 1.0)         /*    Good cases   */
	{
		b = fabs(x) * fabs(x);
		c.flt = 0.5 - 0.5 * fabs(x);
		if (fabs(x) <= 0.5 )       /*    Straight power series  */
		{
			t0 = a14 * b + a13;
			s = b * b;
			t0 = (((((t0*s + a11)*s + a9)*s + a7)*s + a5)*s +a3)*s 
				+ a1;
			t1 = (((((a12*s + a10)*s + a8)*s + a6)*s + a4)*s + a2);
			return ( x + (x*b)*(a0 + b*(t0 + b*t1)));
		}
		else
		{
			b = c.flt;
			s = b*b;
			t1 = (((((a14*s + a12)*s + a10)*s + a8)*s + aa6)*s +
			    aa4)*s + aa2;
			t0 = (((((a13*s + a11)*s + a9)*s + aa7)*s + aa5)*s +
			    aa3)*s + aa1;
			g.gm =((c.ci + 0x3ff00000) >> 1) & 0xfff00000;
			y.ym = 0x7fc00000 - g.gm;
			q = (c.ci >>13) & 0xff;
			g.gm = g.gm + ((0xff00 & guesses[q]) << 4);
			y.ym = y.ym + ((0x0ff & guesses[q]) << 12);
			d = c.flt - g.flt *g.flt;	/* refine square root */
			h = y.flt;
			r = g.flt + h*d; 
			yy0 = h + h;     /*  16 bits   */
			e = 0.5 - h*r;
			d1 = c.flt - r*r;
			h1 = h + e*yy0;
			g1 = r + h1*d1;        /*  32  */
			yy1 = h1 + h1; 
			e1 = 0.5 - h1 * g1;
			d2 = c.flt - g1 * g1;
			h2 = h1 + e1 * yy1;
			g2 = g1 + h2 * d2;  /* hi precision sqrt. */
			gl =(c.flt - g2*g2)*h2;
			u = *(double*)pihh - 2.0*g2;
			v = *(double*)pihh - u - 2.0*g2;
			s = (g2*b)*(aa0 + b*(t0 + b*t1)) + ((gl - 0.5* 
				*(double*)pihl) - v*0.5);
			if (x > 0.0)  {
				return (u -2.0*s);
			}
			else {
				return (2.0*s - u);
			}
		}
	}
	if (fabs(x) == 1.0) {
		return (x * *(double *)pihh);
	}

#ifndef _SVID
	errno = EDOM;
	return(zero/zero);       /* Improper arguments-generate a NaN.  */
#else
	exc.arg1 = x;
	exc.type = DOMAIN;
	exc.name = "asin";
	exc.retval = 0.0;
	if (!matherr(&exc))
	{       
		write (2, exc.name, 4);
		write (2, ": DOMAIN error\n", 15);
		errno = EDOM;
	}
	return(exc.retval);
#endif
}
