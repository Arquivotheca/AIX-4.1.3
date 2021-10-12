#if (!( _FRTINT ))
static char sccsid[] = "@(#)37	1.17  src/bos/usr/ccs/lib/libm/POWER/exp.c, libm, bos411, 9428A410j 5/3/91 11:21:31";
#endif
/*
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS: exp
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <float.h>
#include <sys/errno.h>
#include <math.h>

#ifdef exp
#undef exp
#endif

/*
 * NAME: exp
 *                                                                    
 * FUNCTION: EXPONENT OF X - E(X)
 *                                                                    
 * NOTES:
 *
 * Exp receives one double floating point argument by value,
 * and returns a double IEEE floating point result.
 *
 * Exp uses the accurate table method of Gal et. al.  The
 * routine gets the correctly rounded result over 99.97% of the time,
 * (rounded as required by the setting of the rounding mode), and will
 * never be in error by as much as one ulp.
 *
 * CAUTION:  This routine uses the IEEE representation of floating point
 * numbers.  Furthermore, it assumes that a+b*c is computed with only
 * one rounding error.  These routines are not portable to architectures
 * that do not meet these assumptions.
 *
 *
 * RETURNS: exponential of x
 *
 * If the correct value overflows, exp returns HUGE_VAL and sets
 * errno to ERANGE
 *
 */

typedef long int32;

static double rbig2= 6.7553994410557440000e+15 +1023.0; /* 0x43380000, 0x000003ff */
static double rbig= 6.7553994410557440000e+15; /* 0x43380000, 0x00000000 */
static double rilog2=1.4426950408889633000e+00; /* 0x3ff71547,0x652b82fe */
static double ilog2h=6.9314718055994530000e-01; /* 0x3fe62e42,0xfefa39ef */
static double ilog2l=2.3190468138462996000e-17; /* 0x3c7abc9e,0x3b39803f */
static double cm1= 8.3333336309523691e-03; /* 0x3f811111,0x1b4af38e */
static double c0= 4.1666668402777808000e-02; /* 0x3fa55555,0x643f1505 */
static double c1= 1.6666666666666655000e-01; /* 0x3fc55555,0x55555551 */
static double c2= 4.9999999999999955000e-01; /* 0x3fdfffff,0xfffffff8 */

#ifndef _FRTINT
static double cc4 = 0.5000000000000000000;
static double cc3 = 0.16666666666663809522820;
static double cc2 = 0.04166666666666111110939;
static double cc1 = 0.008333338095239329810170;
static double cc0 = 0.001388889583333492938381;
#endif

static double xmaxexp=7.0978271289338397000e+02; /* 0x40862e42,0xfefa39ef */
static double xmin2exp= -7.4513321910194122000e+02; /*0xc0874910,0xd52d3052*/
static double denorm=2.9387358770557188000e-39; /* 0x37f00000,0x00000000 */
static double rdenorm=3.402823669209384635e+38; /* 0x47f00000,0x00000000 */
static double half = 0.5;

extern int32 __itab[];

static struct A { 
	double t;
	double expt;
}*a; /*(struct A*)(__itab + 2832) init pntr to middle of array */


#ifdef _FRTINT
double _exp (double xx)
#else
double exp (double xx)
#endif
{
	union { 
		int32 i[2];
		double flt;
	} x;
	union  { 
		int32 i[2];
		double  flt;
	} scale;

	union  { 
		int32 i[2];
		double flt;
	} uexp ;
	union  { 
		int32 i[2];
		double flt;
	} z;
	register long in;
#ifdef _SVID
	struct exception exc;
#endif

	double y, yl, diff, arg, res, p, result, power, fake;
	double sx,uexp2,argsq, addend, fmode; /* , rbig, rilog2; */
	int flag;

	x.flt = xx;
	flag = fabs(x.flt) < 512.0;
	fake = rbig + rilog2;
	scale.flt = 0.0; 
	fmode = __setflm(0.0);		/* Must use RIOS encoding - not ANSI */
					/* remember current fl. pt. status   */
					/* and force into round-to-nearest   */
	fake = ilog2h + ilog2l + c0;  
	if ( fabs(x.flt) < 512.0)
	{
	        a = (struct A *) __itab + 177;
		uexp.flt = x.flt * rilog2 + rbig2;
		uexp2 = uexp.flt - rbig2;
		/* uexp.flt = uexp.flt + 1023.0;  */
		y = x.flt - ilog2h * uexp2;
		z.flt = 512.0 * y +  rbig;
		in = (long)z.i[1] ;
		diff = y - a[in].t;		/* reduced arg */
		arg = diff - ilog2l * uexp2;	/* fully reduced arg */
		argsq = arg * arg;
		res = diff - arg -  ilog2l * uexp2;	/* residual of fully 
						 * reduced arg */
		p = ( ( cm1*argsq+c1)*arg+(c0*argsq+c2) ) * argsq+arg; 
		/* p = (((cm1*arg+c0)*arg+c1)*arg+c2)*arg+arg; */
		scale.i[0] = uexp.i[1] << 20;
	        result = res + res * arg;		
		sx = scale.flt * a[in].expt;
		addend = sx*p;
		__setflm(fmode);
		result = addend + sx * result;
		result = result + sx;
exit:
		return (result);
	}
	a = (struct A *) __itab + 177;
	flag = (x.flt==DBL_INFINITY);
	flag = (x.flt == -DBL_INFINITY);
	flag = (x.flt != x.flt);
	flag = (x.flt > xmaxexp);

	__setflm (fmode);

	if (x.flt == DBL_INFINITY)
	{ 
#ifndef _SVID
		errno = ERANGE;           /* Is exp(inf) an overflow? */
		return (DBL_INFINITY);    /* Set errno for overflow.  */
#else
		exc.arg1 = xx;
		exc.type = OVERFLOW;
		exc.name = "exp";

		/* exp(INF) is INF */
		exc.retval = HUGE_VAL;

		if (!matherr(&exc))
			errno = ERANGE;
		return (exc.retval);
#endif
	}
	flag =  (x.flt < xmin2exp);
	if (x.flt == -DBL_INFINITY) {

#ifndef _SVID
		return (0.0);
#else
		exc.arg1 = xx;
		exc.type = UNDERFLOW;
		exc.name = "exp";

		/* exp(-INF) is zero */
		exc.retval = 0.0;

		if (!matherr(&exc))
			errno = ERANGE;
		return (exc.retval);
#endif
	}
	flag = (x.flt >= 0.0);
	if ( x.flt != x.flt) {
		return (x.flt + 1.0);
	}
	else
	{ 
		if (x.flt > xmaxexp)		/* Overflow to inf, set 
						 * errno=ERANGE */
		{ 
#ifndef _SVID
			errno = ERANGE;
			return (DBL_INFINITY);
#else
			exc.arg1 = xx;
			exc.type = OVERFLOW;
			exc.name = "exp";

			/* exp(+big#) overflows to INF */
			exc.retval = scalb(1.0,5000);

			if (!matherr(&exc))
				errno = ERANGE;
			return (exc.retval);
#endif
		}
		else if (x.flt >= 0)
		{   
			__setflm(0.0);	/* back to round nearest */
			uexp.flt = x.flt * rilog2 + rbig;
			scale.i[0] = (uexp.i[1] + 1023-1) << 20;
			power = 2.0;
ex_range:
			uexp.flt -=  rbig;
			y = x.flt -  ilog2h * uexp.flt;
			yl =  ilog2l * uexp.flt;
			z.flt = 512.0 * y + rbig;
			in = z.i[1] ;
			diff = y - a[in].t;		/* reduced arg */
			arg = diff - yl;		/* fully reduced arg */
			argsq = arg * arg;
			res = diff - arg - yl;		/* residual of fully 
							 * reduced arg */
			p = ((cm1*argsq+ c1) * arg +(c0*argsq+c2))*argsq+arg;
			addend = p + (res + res * p);
			__setflm(fmode);
			result =  ((a[in].expt+a[in].expt * addend)* scale.flt) 
				* power;
			return (result);
		}
		else
		if ( x.flt > xmin2exp)
		{  
			uexp.flt = x.flt * rilog2 + rbig;
			scale.i[0] = (uexp.i[1] + 1023+128) << 20;
			power = denorm;
			goto ex_range;
		}
		else {
#ifndef _SVID
                        errno = ERANGE;
			return(0.0);
#else
			exc.arg1 = xx;
			exc.type = UNDERFLOW;
			exc.name = "exp";

			/* exp(-big#) underflows to zero */
			exc.retval = scalb(1.0,-5000);

			if (!matherr(&exc))
				errno = ERANGE;
			return (exc.retval);
#endif
		}
	}
}



