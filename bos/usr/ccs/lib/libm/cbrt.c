static char sccsid[] = "@(#)26	1.12  src/bos/usr/ccs/lib/libm/cbrt.c, libm, bos411, 9428A410j 6/16/90 02:09:30";
/*
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS: cbrt
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

#include <math.h>
#include <fp.h>

/*
 * NAME: cbrt
 *                                                                    
 * FUNCTION: RETURN THE CUBE ROOT OF X
 * kahan's cube root (53 bits IEEE double precision)
 * for IEEE machines only
 * coded in C by K.C. Ng, 4/30/85
 *                                                                    
 * NOTES:
 *
 * Accuracy:
 *	better than 0.667 ulps according to an error analysis. Maximum
 * error observed was 0.666 ulps in an 1,000,000 random arguments test.
 *
 * Warning: this code is semi machine dependent; the ordering of words in
 * a floating point number must be known in advance. I assume that the
 * long interger at the address of a floating point number will be the
 * leading 32 bits of that floating point number (i.e., sign, exponent,
 * and the 20 most significant bits).
 *
 * On a National machine and any machine with little-endian byte order (such
 * as the PS/2), it has different ordering; therefore, this code uses the
 * VALL and VALH machine dependent macros from the header file fp.h.
 *
 * RETURNS: cube root of x
 *
 */

#ifdef VAX
double cbrt(double x)
{
	return (0.0);
}

#else

static unsigned long B1 = 715094163, /* B1 = (682-0.03306235651)*2**20 */
	             B2 = 696219795; /* B2 = (664-0.03306235651)*2**20 */
static double
	    C= 19./35.,
	    D= -864./1225.,
	    E= 99./70.,
	    F= 45./28.,
	    G= 5./14.;

double cbrt(double x)
{
	double r,s,t=0.0,w;
	unsigned int mexp,sign;

	if (x != x)			/* cbrt of NaN is itself */
		return (x + 1.0);	/* added to turn SNaN to QNaN */
	mexp = (unsigned int) (VALH(x) & 0x7ff00000);
	if(mexp == 0x7ff00000) 		/* cbrt(NaN,INF) is itself */
		return(x); 
	if(x == 0.0) 			/* cbrt(0) is itself */
		return(x);

	sign = (unsigned int) (VALH(x) & 0x80000000); 	/* sign= sign(x) */
	VALH(x) ^= sign;		/* x=|x| */


    /* rough cbrt to 5 bits */
	if(mexp == 0) 		/* subnormal number */
	{
		VALH(t) = 0x43500000; 	/* set t = 2**54 */
		t *= x; 
		VALH(t) = (unsigned int) (VALH(t)/3 + B2);
	  }
	else
		VALH(t) = (unsigned int) (VALH(x)/3 + B1);


    /* new cbrt to 23 bits, may be implemented in single precision */
	r = t*t/x;
	s = C + r*t;
	t *= G + F/(s + E + D/s);	

    /* chopped to 20 bits and make it larger than cbrt(x) */ 
	VALL(t) = 0; 
	VALH(t) += 0x00000001;

    /* one step newton iteration to 53 bits with error less than 0.667 ulps */
	s = t*t;		/* t*t is exact */
	r = x/s;
	w = t + t;
	r = (r - t)/(w + r);	/* r-s is exact */
	t = t + t*r;

    /* retore the sign bit */
	VALH(t) |= sign;
	return(t);
}
#endif
