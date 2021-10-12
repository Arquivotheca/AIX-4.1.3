#if (!( _FRTINT ))
static char sccsid[] = "@(#)38	1.13  src/bos/usr/ccs/lib/libm/POWER/log10.c, libm, bos411, 9428A410j 9/7/93 09:18:31";
#endif
/*
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS: log10, loginner
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
extern int32 __tab[];

static int32 log2l[] = { 0x3c7abc9e, 0x3b39803f};
static int32 log2h[] = { 0x3fe62e42, 0xfefa39ef};
static double c6= -0.1666681511199968257150;
static double c5 = 0.2000014541571685319141;
static double c4 = -0.2499999999949632195759;
static double c3 = 0.3333333333296328456505;
static double c2 = -0.5000000000000000042725;

static double d8 =-0.1250122079043555957999; /* BFC0006668466517 */
static double d7 = 0.1428690115662276259345; /* 3FC249882224F72F */
static double d6 =-0.1666666662009609743117; /* BFC5555554554F15 */
static double d5 = 0.1999999996377879912068; /* 3FC9999998D278CB */
static double d4 =-0.2500000000000056849516; /* BFD0000000000066 */
static double d3 = 0.3333333333333360968212; /* 3FD5555555555587 */
static double d2 = -0.5000000000000000000000; /* bfe0000000000000 */
static int32 big[] ={ 0x47f00000, 0x00000000};
static int32 inf[] ={ 0x7ff00000, 0x00000000};
static double SMALL = 2.2250738585072015025e-308;

/* T: x(i), 1/x(i), ln(x(i))     */
/*      (0:6*(3*64)+5) int32     */
/*  T: x(i), 1/x(i), ln(x(i))    */
static struct tdef {
	double table;
	double recip;
	union { 
		double flt;
		int32 i;
	} fcn;
} *t;

static double loginner(double , double , double *);


#ifdef log10
#undef log10
#endif

/*
 * NAME: log10
 *                                                                    
 * FUNCTION: COMPUTES THE LOG BASE 10 OF X
 *                                                                    
 * NOTES:
 *
 * RETURNS: log base 10 of x
 *
 * log10(+INF) is +INF with no errno 
 * log10(0) is -INF and no errno.
 * log10(NaN) is that NaN with no errno.
 * If x is less than 0, then log returns the value QNaN and sets
 * errno to EDOM.
 * If x is equal 0, then log returns the value -HUGE_VAL and does
 * not modify errno.
 *
 */


static long ln10[] = { 0x3fdbcb7b, 0x1526e50e},
	    ln101[] = { 0x3c695355, 0xbaaafad4	};
static double bigfix = 128.0;

#ifdef _FRTINT
double _log10(double xx)
#else
double log10(double xx)
#endif
{
	union { 
		double flt;
		unsigned long i;
	} x;
	double y, z, tail, fmode;
	long flag;
#ifdef _SVID
	struct exception exc;
#endif

	x.flt = xx;
	if ((x.i < 0x7ff00000) && (x.i >= 0x00100000))
	{
		fmode = __setflm(0.0);  /* Set round nearest */
					/* Must use RIOS encoding - not ANSI */
		y = loginner(x.flt,0.0,&tail);
normal:
		z = (tail * *(double*)ln10+y * *(double*)ln101)+y * 
			*(double*)ln10;
		__setflm (fmode);        /* restore the rounding mode */
		return(z);
	}
	flag = (x.flt != x.flt);
	flag = (x.flt > 0.0);
	flag = (x.flt == DBL_INFINITY);
	flag = (x.flt == 0.0);

	if (x.flt != x.flt) {
		return(x.flt + 1.0);     /* Nan - Quiet any SNaN's */
	}

	if (x.flt == DBL_INFINITY)  {
		return(x.flt); /* log10(infinity)=infinity */
	}

	if (x.flt > 0.0)  /* log10(denormalized number) */
	{ 
		fmode = __setflm(0.0);  /* Set round nearest */
					/* Must use RIOS encoding - not ANSI */
		z = loginner ( x.flt * *(double*)big, 0.0, &tail);
		y = z - bigfix * *(double*)log2h ;
		tail = z - y - bigfix * *(double*)log2h + tail
		    - bigfix * *(double*)log2l;
		goto normal;
	}
	if (x.flt == 0.0)  {
#ifndef _SVID
		return(-DBL_INFINITY); /* log10(0) = -infinity */
#else
		exc.arg1 = xx;
		exc.type = SING;
		exc.name = "log10";
		exc.retval = -HUGE_VAL;
		if (!matherr(&exc))
		{       write (2, exc.name, 5);
			write (2, ": SING error\n", 13);
#ifdef _SAA
			errno = ERANGE;
#else
			errno = EDOM;
#endif
		}
		return (exc.retval);
#endif
	}

#ifndef _SVID
	errno = EDOM;                        /* set errno = EDOM */
	return(DBL_INFINITY - DBL_INFINITY); /* log10(neg. number) = Nan  */
#else
	exc.arg1 = xx;
	exc.type = DOMAIN;
	exc.name = "log10";
	exc.retval = -HUGE_VAL;
	if (!matherr(&exc))
	{       write (2, exc.name, 5);
		write (2, ": DOMAIN error\n", 15);
		errno = EDOM;
	}
	return (exc.retval);
#endif
}

/*
 * NAME: loginner
 *                                                                    
 * FUNCTION: COMPUTEST NATURAL LOG OF (ZX + ZY)
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * It is assumed that zx and zy have already been prescreened for
 * inappropriate ranges.  This routine does not handle exceptional cases.
 * Furthermore, it assumes that the machine is in round-to-nearest mode.
 *                                                                   
 * NOTES:
 *
 * Computes ln(zx + zy) and returns the high order bits of the
 * result.  The low order bits of the result are returned in the
 * parameter "tail".  It is best to call
 * this routine with zx > zy, although this condition is not necessary.
 *
 * RETURNS: ln(zx + zy)
 *
 */

static double loginner(double zx, double zy, double *tail)
{
	double unnorm = 4503601774854144.000;   /* 2**53+2**31 */
	int32 adjust = 0x7ffffc01;

	union { 
		double flt;
		unsigned long fx[2];
	} x;
	double xlow;
	union { 
		double flt;
		unsigned long i[2];   /* fy, fyl */
	} y;
	double n;
	union { 
		double flt;
		unsigned long i[2];   /* fnh, fnl */
	} fn;
	union { 
		double flt;
		unsigned long h;      /* fcrh */
	} fcr;
	double arg, dely, high, low, z, t1, t2, t0, t3, result;
	long i, flag;

	t = (struct tdef *)__tab;      /* init table pointer */
	fn.flt = unnorm;                     /* Float the exponent  */
	fcr.flt = 0.0;
	x.flt = zx + zy;
	xlow = zx - x.flt + zy;
	if ( fabs(zy) > fabs(zx) )
		xlow = zy - x.flt + zx ;

	fn.i[1] = ((x.fx[0] >> 20) & 0x07ff) + adjust;
	fcr.h = (2047 * 0x100000) - (x.fx[0] & 0x7ff00000);
	fcr.flt = 0.5 * fcr.flt;
	fn.flt = fn.flt - unnorm;
	y.flt = x.flt * fcr.flt;             /* force 1.0 <= y < 2.0 */
	xlow = xlow * fcr.flt;

	if (x.fx[0] & 0x00080000)
	{ 
		n = fn.flt + 1.0;             /* number to multiply by log 2. */
		i = (x.fx[0] >>13) & 0x3f ;   /* table lookup index  */
		arg = 0.5 * y.flt;
		xlow = 0.5 * xlow;
	}
	else
	{ 
		n = fn.flt;
		i = ((x.fx[0] >> 12) & 0x07f) + 64;  /* table lookup index  */
		arg = y.flt;
	}

	y.flt = (arg - t[i].table) * t[i].recip;
	flag = (t[i].fcn.i != 0);
	dely = ((arg-t[i].table) - y.flt * t[i].table+xlow) * t[i].recip;
	z = y.flt * y.flt;                /* polynomial evaluation  */
	if (n == 0.0)
	{
		t0 = (((d8*z + d6)*z + d4)*y.flt+((d7*z+ d5)*z + d3))*y.flt+ d2;
		t1 = y.flt + t[i].fcn.flt;
		low = t[i].fcn.flt - t1 + y.flt + (dely-y.flt*dely);
		result = (t0*z + low) + t1;
		*tail = t1 - result + (t0*z + low);
	}

	else if (t[i].fcn.i != 0)         /* small argument?        */
	{
		low = n * *(double*) log2l + dely;
		high = y.flt + t[i].fcn.flt;
		dely = t[i].fcn.flt - high +y.flt;

		/* high order part of n*log2     */
		t1 = n* *(double*)log2h + low;

		/* low order part of n*log2 */
		low = (n* *(double*)log2h - t1) + low;

		t0 = ((c5*z + c3)*y.flt + (c6*z + c4)*z) + c2;

		/* No. use 6th degr. approx           */
		t2 = high + t1;  /* high order part of fcn(i) + log2*n */
		t3 = (t1 - t2) + high; /* low order part of fcn(i)+log2*n  */

		/* final result  */
		result = ((((t0*z + low) + t3) + dely) + t2);

		*tail = t2 - result + dely + t3+ (t0 * z + low);  /* residual */
	}
	else /* Yes, arg. close to 1, use 8th deg. poly.   */
	{
		low=n* *(double*)log2l+dely; /* n times the low bits of log2 */
		t1 = n* *(double*)log2h + low;  /* high order part of n*log2 */
		low = (n * *(double*)log2h - t1) + low + xlow;
		t0 = (((d8*z + d6)*z + d4)*y.flt +  
			((d7*z+ d5)*z + d3))*y.flt + d2;
		result = ((t0*z + low) + y.flt) + t1 ;  /* final result  */
		*tail = t1 - result + y.flt +(t0*z + low);  /* residual */
	}
	return(result);
} /* end loginner */
