#if (!( _FRTINT ))
static char sccsid[] = "@(#)36	1.14  src/bos/usr/ccs/lib/libm/POWER/cos.c, libm, bos411, 9428A410j 7/29/93 13:34:56";
#endif
/*
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS: cos
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
#include <math.h>
#include <errno.h>
#include <values.h>

#ifdef cos
#undef cos
#endif

/*
 * NAME: cos
 *                                                                    
 * FUNCTION: COSINE OF X
 *                                                                    
 * NOTES:
 *
 * Cos loses accuracy when passed large values.
 *
 * RETURNS: the cosine of x
 *
 */

typedef unsigned long int32;

#ifdef _FRTINT
double _cos (double x)
#else
double cos (double x)
#endif
{ 
	union { 
		int32 i[2];
		double flt;
	} z;
	/* pi/2 to sextuple precision */
	static int pi2_1[] =  { 0x3ff921fb, 0x54442d18 }; /* high */
	static int pi2_2[] =  { 0x3c91a626, 0x33145c07 }; /* medium */
        static int pi2_3[] =  { 0xb91f1976, 0xb7ed8fbc }; /* low */
	static double pihbig = 13816870609430.547; /* pi*2**42 */ 
	static double piqint = 0.7853981633974483;
	static int32 recip_pi[] = { 0x3fe45f30, 0x6dc9c883 };
	static int32 big2[] = { 0x43380000, 0x00000000 };
	static double s13 =  0.00000000015868926979889205164;
	static double s11 = -0.000000025050225177523807003;
	static double s9 =  0.0000027557309793219876880;
	static double s7 = -0.00019841269816180999116;
	static double s5 =  0.0083333333332992771264;
	static double s3 = -0.16666666666666463126;
	static double c14 = -.00000000001138218794258068723867;
	static double c12 = .000000002087614008917893178252;
	static double c10 = -.0000002755731724204127572108;
	static double c8 =  .00002480158729870839541888;
	static double c6 = -.001388888888888735934799;
	static double c4 =  .04166666666666666534980;
	static double c2 =-.500000000000000000000;
	static double sixteen_pihbig = 2.829695100811376e16;
	static long bigger2[] = { 0x43580000, 0x0 };
	int32 iz, flag;
	double pi2_h, pi2_m, pi2_l;
	double temp1, temp2;
	double arg1, arg, sq, adjust, a4, ztrue;
	double t1, t2, a3, p, res;
	double recipix, big2x;
	double fmode;
#ifdef _SVID
	double y;
	struct exception exc;

	exc.arg1 = x;
#endif

	flag = (fabs(x) < pihbig);
	flag = (fabs(x) <= piqint);
	fmode = __setflm(0.0);	/* Must use RIOS encoding - not ANSI */
        flag = c14 == s13;
	pi2_h = *(double *) pi2_1; /* pi/2 high */
	pi2_m = *(double *) pi2_2; /* pi/2 medium */
	pi2_l = *(double *) pi2_3; /* pi/2 low */
	recipix = * (double *) recip_pi;
	big2x = *(double*) big2 ;

	if ( fabs(x) < pihbig )
	{
cos_entry:
	
		if ( fabs(x) <= piqint )  /*  less than pi/4  */
		{
			/* special case for number small enough that no further
			 * argument reduction is necessary.
			 */
			arg = x;
			sq = x * x;
			a4 = sq * sq;  
		        t1 = c10 + c14 * a4;
		        t2 = c8 + c12 * a4;
		        t1 = c6 + t1 * a4;
		        t2 = c4 + t2 * a4;
		        t1 = c2 + t1 * a4;
		        t1 = t1 + sq * t2;
		        __setflm(fmode);
		        res = 1.0 + sq * t1;
		        return (res);
		}


		/* reduce argument to range [-pi/2, pi/2].  ztrue is the
		 * number of multiples to be extracted.
		 */
		z.flt = x * recipix + big2x ;	/* extract mult of pi/2 */
		iz = (z.i[1] + 1) % 4;		/* get mult of pi/2 (mod 4) */
		ztrue = z.flt -  big2x ;	/* align rt, normalize  */
		flag = iz <= 1;	                /* scheduling clue */

		z.flt = x * recipix + big2x ;	/* extract mult of pi/2 */
		ztrue = z.flt -  big2x ;	/* align rt, normalize  */

		/* argument reduction uses a sextuple precision value of
		 * pi/2 (pi2_h, pi2_m, pi2_l) and yields a quad precision
		 * value for the reduced argument (arg, arg1).  The
		 * high part is calculated exactly with 3 accumulate
		 * instructions.  The low part is calculated with only
		 * one term, but care must be taken to be sure it
		 * doesn't overlap the high part.
		 */
		temp1 = x - ztrue * pi2_h;
		temp2 = temp1 - ztrue * pi2_m;
		arg = temp2 - ztrue * pi2_l;
		arg1 = (temp1 - arg) - ztrue * pi2_m;
		
		/* extract the part of "arg1" which doesn't
		 * overlap the high part
		 */
		temp1 = arg + arg1;
		arg1 = (arg - temp1) + arg1;

		sq = arg * arg;			/* square of reduced arg */
		flag = s11 == s11;              /* force load of constant */
		a4 = sq * sq;	                /* 4th power of reduced arg */

		/* Use reduced argument in polynomial to evaluate function.
		 * The polynomial coeffients are similar to some in
		 * Hart & Cheney, but not identical.  I don't know the source
		 * for these particular coefficients.
		 */

		if ( iz & 0x00000001) {		/* 1st & 3rd quadrants */
		        t1 = c10 + c14 * a4;
			t2 = c8 + c12 * a4;
			t1 = c6 + t1 * a4;
			t2 = c4 + t2 * a4;
			t1 = c2 + t1 * a4;
			t1 = t1 + sq * t2;
			if(iz <= 1){	
				res = arg * t1 - arg1;
				__setflm(fmode);
				res = 1.0 + arg * res;
				return (res);}
			else {
				res = arg1 - arg * t1;
				__setflm(fmode);
				res = arg * res - 1.0;
				return (res);}
                }
                else {
			t1 = s9 + s13 * a4;
			t2 = s7 + s11 * a4;
			t1 = s5 + t1  * a4;
			t2 = s3 + t2  * a4;
			a3 = sq * arg;
			t1 = t2 + sq * t1;
			if(iz <= 1){	
			     	res = t1 * a3 + arg1;
				__setflm(fmode);
			     	res = arg + res; 
				return (res);}
			else {
				res = t1 * a3 + arg1;
				arg = - arg;
				__setflm(fmode);
				res = arg - res;
				return (res);}
                }
finished:
		__setflm(fmode);
#ifdef _SVID
		y = fabs(exc.arg1) + M_PI_2;
		if (y > X_PLOSS)                  /* partial loss of accuracy */
		{       exc.type = PLOSS;
			exc.name = "cos";
			exc.retval = adjust * res;
			if (!matherr(&exc)) 
				errno = ERANGE;
			return (exc.retval);
		}
		return (adjust * res);
#else
		return (adjust * res);
#endif
	}


#ifdef _SVID
	y = fabs(exc.arg1) + M_PI_2;
	if (y > X_TLOSS)                          /* total loss of accuracy */
	{       
		exc.type = TLOSS;
		exc.name = "cos";
		exc.retval = 0.0;
		if (!matherr(&exc))
		{       
			write (2, exc.name, 3);
			write (2, ": TLOSS error\n", 14);
			errno = ERANGE;
		}
		__setflm(fmode);
		return (exc.retval);
	}
#endif

	if ((x != x) || (fabs(x)==DBL_INFINITY)) /* nan or inf */
	{ 
		res = x;
		adjust = 0.0;
		goto finished;
	}

	/* Big reduction job!  */
	/* the first loop reduces the argument by multiples of
	 * pi/2 while x is sufficiently large that 'p' is guaranteed
	 * to be an integer.  The second loop is identical except
	 * in computing 'p' it is necessary to add and subtract
	 * the magic number to make it an integer.
	 */

	while (fabs(x) >= sixteen_pihbig)
	{ 
		p = x * recipix;       /* number of multiples of pi/2 */
		x =  (x -p * pi2_h) - p * pi2_m ;
	}

        do
	{ 
		p = x * recipix + *(double*)bigger2 -*(double*)bigger2;
		x =  (x -p * pi2_h) - p * pi2_m ;
	} while ( fabs (x) >= pihbig );

	flag = (fabs (x) <= piqint);
	goto cos_entry;
}
