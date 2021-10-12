#if (!( _FRTINT ))
static char sccsid[] = "@(#)19	1.20  src/bos/usr/ccs/lib/libm/POWER/log.c, libm, bos411, 9428A410j 11/10/93 10:48:15";
#endif
/*
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS: log
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
#include <values.h>

#ifdef log
#undef log
#endif

/*
 * NAME: log
 *                                                                    
 * FUNCTION: COMPUTE LOGARITHM OF X
 *                                                                    
 * NOTES:
 *
 * RETURNS: log of x
 *
 * If x is less than 0, then log returns the value QNaN and sets
 * errno to EDOM.
 *
 * If x is equals 0, then log returns the value -HUGE_VAL and does
 * not modify errno.
 *
 */

typedef long int32;

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
extern int32 __tab[];

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

#ifdef _FRTINT
double _log (double tmpx)
#else
double log (double tmpx)
#endif
{

	double unnorm = 4503601774854144.000;   /* 2**53+2**31 */
	int32 adjust = 0x7ffffc01;
	double adj = 4.3934705024835902176e158; /* 0x60e0000000000000 */
#ifdef _SVID
	struct exception exc;
#endif

	union { 
		double flt;
		int32 i[2];
	} x;
	union { 
		double flt;
		int32 i[2];
	} tmp;
	union { 
		double flt;
		struct { 
			int32 h;
			int32 l;
		} hl;
	} fy;
	double n;
	union { 
		double flt;
		struct { 
			int32 h;
			int32 l;
		} hl;
	} fn;

	double high, low, z, t1, t2, t0, t3, result, arg, dely, zz;
	double fmode;
	long i, j, k, flag;
	flag = (tmpx >= SMALL); /* smallest normalized double */
	flag = (tmpx <= MAXDOUBLE);   /* 1.7976931348623157e308 */
	x.flt = tmpx;		     /* init x.flt for union access */
        fn.hl.h = 0x00000000; 
	fy.hl.l = x.i[1];
	t = (struct tdef *)__tab;      /* init table pointer */
	i = (x.i[0] & 0x000fffff);
	fy.hl.h = i | 0x3ff00000;
	fn.hl.l = x.i[0] & 0x7ff00000;
	fmode = __setflm(0.0);	/* Must use RIOS encoding - not ANSI */
	if ((tmpx >= SMALL) && (tmpx <= MAXDOUBLE)) 
	{

	/* flag = (fy.flt >= 1.5);  */
 	flag = (fy.hl.h >= 0x3ff80000);    
	fn.flt = fn.flt * adj;

		if (fy.hl.h >= 0x3ff80000) 
	  	/* if (fy.flt >= 1.5) */  /* upper bit of fraction */
		{
	                n = fn.flt * adj - 1022.0;  /* number to multiply by log 2. */
			/* table lookup index  */
			i = (i >> 13) & 0x0000003f;
			arg = 0.5 * fy.flt; 
		}
		else
		{
	                n = fn.flt * adj - 1023.0; /* number to multiply by log 2. */
			/* table lookup index  */
			i = ( i >> 12) + 64;
			arg = fy.flt;
		}
entry:		/*  at this point i, n and arg must be defined */
		fy.flt = (arg - t[i].table) * t[i].recip ;
		flag = (t[i].fcn.i != 0);
		dely = ((arg - t[i].table) - fy.flt * t[i].table) * t[i].recip;
		z = fy.flt * fy.flt;            /* polynomial evaluation */
		if (n == 0.0)
		{
			t0 = (((d8 * z + d6) * z + d4) * fy.flt +
			    ((d7 * z + d5) * z + d3)) * fy.flt + d2;
			t1 = fy.flt + t[i].fcn.flt;
			low = t[i].fcn.flt - t1 + fy.flt + 
				(dely - dely * fy.flt);
			result = (t0 * z + low);
			result = result + t1;
		}
		else if (t[i].fcn.i != 0)    /* small argument? */
		{
			/* n times the low order bits of log2. */
			low = n * *(double*)log2l + dely;
			high = fy.flt + t[i].fcn.flt;
			dely = t[i].fcn.flt - high + fy.flt;

			/*  high order part of n*log2   */
			t1 = n * *(double*)log2h + low;

			/*low order part of n*log2 */
			low = (n * *(double*)log2h - t1) + low;

			/* No. use 5th degr. appro */
			t0=((c5*z+c3)*fy.flt+(c6*z + c4)*z)+c2;
			/* high order part of fcn(i) + log2*n  */
			t2 = high + t1;
			result = t0*z + low;
			/* low order part of fcn(i) + log2*n  */
			t3 = (t1 - t2) + high;
			
			/* final result */
			result = (( result + t3) +dely);
			result = result + t2;
		}
		else /* Yes, arg. close to 1, use 8th deg. poly. */
		{
			dely = dely - fy.flt * dely;

			/* n times the low order bits of log2. */
			low=n * *(double*)log2l + dely;

			/* high order part of n*log2 */
			t1 = n* *(double*)log2h + low;

			/* low order part of n*log2 */
			low=(n* *(double*)log2h - t1)+low;
			t0 = (((d8*z + d6)*z + d4)*fy.flt +
			    ((d7 * z+ d5) * z + d3)) * fy.flt + d2;

			/* final result */
			result = ((t0*z + low) + fy.flt); 
			result = result + t1;
		}

#ifdef _SVID
		exc.arg1 = tmpx;
		exc.retval = result;
#endif

exit:
		__setflm(fmode);     /* Restore original round mode. */
#ifndef _SVID
		return(result);
#else
		return(exc.retval);
#endif
	}

	flag = (x.flt==0.0);
	flag = (x.flt!=x.flt);
	flag = (x.flt== DBL_INFINITY);
	flag = (x.i[0] < 0);                     /* 1 bit result */

	if (x.flt==0.0) {
#ifndef _SVID
		result = ((-1.0)/fabs(x.flt)); /* log(0) = -inf; */
#else
		exc.arg1 = tmpx;
		exc.type = SING;
		exc.name = "log";
		exc.retval = -HUGE_VAL;
		if (!matherr(&exc))
		{       write (2, exc.name, 3);
			write (2, ": SING error\n", 13);
#ifdef _SAA
			errno = ERANGE;
#else
			errno = EDOM;
#endif
		}
#endif
		goto exit;
	}
	if (x.flt!=x.flt)
	{ 
#ifndef _SVID
		result = (x.flt + 1.0);     /* log(NaN) = NaN */
#else
		exc.arg1 = tmpx;
		exc.retval = x.flt + 1.0;
#endif
		goto exit;
	}

	if (x.flt == DBL_INFINITY)
	{ 
#ifndef _SVID
		result = (x.flt);     /* log(INF) = INF */
#else
		exc.arg1 = tmpx;
		exc.retval = x.flt;
#endif
		goto exit;
	}

	if (x.i[0] < 0)                     /* log(negative number) = NaN  */
	{ 
#ifndef _SVID
		errno = EDOM;
		result = (DBL_INFINITY- DBL_INFINITY);
#else
		exc.arg1 = tmpx;
		exc.type = DOMAIN;
		exc.name = "log";
		exc.retval = -HUGE_VAL;
		if (!matherr(&exc))
		{       write (2, exc.name, 3);
			write (2, ": DOMAIN error\n", 15);
			errno = EDOM;
		}
#endif
		goto exit;
	}
	/* here for very small numbers */
	fy.flt=x.flt* *((double*)big);  /* force xx into the normalized range */
	fn.flt=(double)(((fy.hl.h>>20)&0x07ff)-1151); /* Get exp of number */
	fy.hl.h=(fy.hl.h & 0x000fffff) | 0x3ff00000;  /* force 1.0 <= y < 2.0 */
        if (fy.hl.h & 0x00080000) /* fraction part > 1.5 */
        {
        	n = fn.flt + 1.0;
		i = (fy.hl.h >> 13) & 0x0000003f;
		arg = 0.5 * fy.flt;
	}
	else
	{
		n = fn.flt;
	 	i = ((fy.hl.h >> 12) & 0x07f)+63;
		arg = fy.flt;
	}
	goto entry;
} /* end log */




