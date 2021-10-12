static char sccsid[] = "@(#)86	1.2  src/bos/usr/ccs/lib/libm/POWER/expm1.c, libm, bos411, 9428A410j 11/29/93 10:33:59";
/*
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS: expm1, expinner
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <float.h>
#include <sys/errno.h>
#include <math.h>

typedef long int32;
extern int32 __itab[];

static double rbig2= 6.7553994410557440000e+15 +1023.0; /* 0x43380000, 0x000003ff */
static double rbig= 6.7553994410557440000e+15; /* 0x43380000, 0x00000000 */
static double rilog2=1.4426950408889633000e+00; /* 0x3ff71547,0x652b82fe */
static double ilog2h=6.9314718055994530000e-01; /* 0x3fe62e42,0xfefa39ef */
static double ilog2l=2.3190468138462996000e-17; /* 0x3c7abc9e,0x3b39803f */
static double cm1= 8.3333336309523691e-03; /* 0x3f811111,0x1b4af38e */
static double c0= 4.1666668402777808000e-02; /* 0x3fa55555,0x643f1505 */
static double c1= 1.6666666666666655000e-01; /* 0x3fc55555,0x55555551 */
static double c2= 4.9999999999999955000e-01; /* 0x3fdfffff,0xfffffff8 */

static double cc4 = 0.5000000000000000000;
static double cc3 = 0.16666666666663809522820;
static double cc2 = 0.04166666666666111110939;
static double cc1 = 0.008333338095239329810170;
static double cc0 = 0.001388889583333492938381;

static double xmaxexp=7.0978271289338397000e+02; /* 0x40862e42,0xfefa39ef */
static double xmin2exp= -7.4513321910194122000e+02; /*0xc0874910,0xd52d3052*/
static double denorm=2.9387358770557188000e-39; /* 0x37f00000,0x00000000 */
static double rdenorm=3.402823669209384635e+38; /* 0x47f00000,0x00000000 */
static double half = 0.5;

static struct A { 
	double t;
	double expt;
}*a; /*(struct A*)(__itab + 2832) init pntr to middle of array */

static double expinner(double , double , double );

/*
 * NAME: expm1
 *                                                                    
 * FUNCTION: COMPUTES EXP(X) - 1
 *                                                                    
 * NOTES:
 *
 * RETURNS: exponential of x minus 1
 *
 */

double expm1(double xx)  /*    computes exp(x) - 1      */
{
	return (expinner (xx, 1.0, 0.0));
}


/*
 * NAME: expinner
 *                                                                    
 * FUNCTION: COMPUTES EXP(X) - F
 *                                                                    
 * NOTES:
 *
 * RETURNS: exp(xx + eps) -f; |eps| << |x|;
 *
 */

static double expinner(double xx, double f, double eps)
{
	union { 
		double flt;
		long n;
	} scale;
	union { 
		double flt;
		long n;
	} recip;
	union { 
		double flt;
		long f[2];
	} z;
	union { 
		long f[2];
		double flt;
	} e ;
	double	d, x, y, yl, diff, arg, res, p, result, power;
	double	fake,fmode, addend; /* , rbig, rilog2; */ 
	int	i, flag;

	x = xx;
	a = (struct A *) __itab + 177;
	flag = (fabs(x) < 512.0);
	fmode = __setflm(0.0);	/* Must use RIOS encoding - not ANSI */
	fake = rbig + rilog2;
	scale.flt = 0.0;
	recip.flt = 0.0;
	if (fabs(x) < 512.0)
	{

		e.flt = x * rilog2 +rbig;	/* exponent of result */
		power = 1.0;
		scale.n =(e.f[1]+1023)<<20;  	/* exponent of result in 
						 * integer form */
main_path:
		e.flt -= rbig; 			/* multiple of ln 2 to subtract
						 * from orig arg. */
		y = x - ilog2h * e.flt; 	/* 1st reduced argument( high 
						 * order part)  */
		recip.n = 2046* 0x100000 - scale.n; /* exponent of result's 
						     * recip.  */
		yl = ilog2l * e.flt;   		/* 1st reduced argument( low 
						 * order part) */
		z.flt = 512.0 * y + rbig; 	/* extract table for lookup  */
		i = z.f[1] ;
		diff = y - a[i].t;     		/* reduced argument    */
		arg = diff - yl;     		/* fully reduced argument */
		res = diff - arg - yl + eps;  	/* residual of fully reduced 
						 * argument  */
		p = ((((cc0*arg+cc1)*arg+cc2)*arg+cc3)*arg+cc4)*arg*arg + arg;
		d = a[i].expt - f * recip.flt;


		if (d < 0) {
			/* residual from table lookup */
			yl = -(d + f * recip.flt) + a[i].expt;
		}
		else {
			/* residual from table lookup */
			yl = a[i].expt - d - f * recip.flt;
		}

		if ((scale.n == 0) && (i==0))
		{
			__setflm(fmode);
			result = (0.0 - f); /* return positive zero in case that */
				            /* f is equal to zero                */
		}
		else
		{
			addend = p + (res + res * p);
			__setflm(fmode);
			result = (d + a[i].expt * addend) * scale.flt;
			result *= power;
		}
		return(result);
	}

	flag = (x == DBL_INFINITY);
	flag = (x == -DBL_INFINITY);
	flag = (x != x);
	flag = (x > xmaxexp);

	__setflm(fmode);	/* restore user's rounding mode */

	if (x == DBL_INFINITY) {
		return ( DBL_INFINITY);           /* exp(inf) = inf */
        }
	flag = (x < xmin2exp);
	if (x == -DBL_INFINITY)
	{
small_argument:
		return (0.0 - f );             /* exp(-inf) = 0.0  return positive  */
                                               /* zero in case f is zero            */
	}
	flag = (x >= 0.0);
	if (x != x) 
       		return(x + 1.0);     /* Exp(NaN) = NaN */
	else
	{
		if (x > xmaxexp)
		{ 
			errno = ERANGE;
			return (DBL_INFINITY);	/*  exp(x very big) = inf */
		}
		else if (x >= 0.0)              /*  very large result    */
		/*  be careful to avoid overflow.  Use same algorithm  */
		/*  but keep exponent too small by 1 til the very end. */
		{
			__setflm(0.0);			/* Back to round-to-
							 * nearest */
			e.flt = (x * rilog2 + rbig);   	/* exponent of result */
			scale.n = (e.f[1]+1023-1)<<20; 	/* exp of result in 
							 * int form  */
			power = 2.0;
                        f = f * half;
			goto main_path;
		}
		if (x > xmin2exp)	/* result is subnormal or very small */
		{
                        (fabs(f) > denorm);
			e.flt = (x * rilog2 + rbig);   	/* exponent of result */
			scale.n = (e.f[1]+1023+128)<<20;/* exp of result in 
                                                         * int form  */
                        if ((scale.n < (168<<20)) &&
			    (fabs(f) > denorm)) 
                		    goto small_argument;
                        __setflm(0.0);
			power = denorm;
                        f = f * rdenorm;
			goto main_path;
		}
		goto small_argument;     /*  return -f for large neg args. */
	}
}  /*  end of expinner routine  */
 
