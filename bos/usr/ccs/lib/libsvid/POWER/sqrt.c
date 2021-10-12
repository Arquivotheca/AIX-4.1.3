#if (!( _FRTINT ))
static char sccsid[] = "@(#)17	1.17.1.1  src/bos/usr/ccs/lib/libsvid/POWER/sqrt.c, libsvid, bos411, 9428A410j 3/3/93 14:48:08";
#endif
/*
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS: sqrt
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <math.h>		/* for struct exception */
#include <errno.h>		/* errno, EDOM */
#include <fpxcp.h>		/* fp_set_flag; fp_raise_xcp */

/* if math.h defines this, we must now undefine it */
#ifdef sqrt
#undef sqrt
#endif

extern double __setflm(double);
 
/*
 * NAME: sqrt
 *                                                                    
 * FUNCTION: square root of x
 *                                                                    
 * NOTES:
 *
 *  Square Root Routine.
 *  Uses fixed point ops to generate an initial guess with the help
 *  of a table.  Then computes successively better guesses to the
 *  square root and the reciprocal, using Newton-Raphson.  The last
 *  iteration is so constructed that the last floating point
 *  operation correctly sets all the floating point status bits
 *  to characterize the results.
 * 
 *  CAUTION:  This routine is dependent on the IEEE representation
 *  used by POWER, as well as the fact that it accomplishes
 *  multiply-and-add with only one rounding operation.
 *
 *  For further description of the algorithm used, see "Computation
 *  of elementary functions on the IBM RICS System/6000 processor",
 *  P. W. Markstein, IBM J. Res. Develop., Vol. 34, No. 1,
 *  Jan. 1990, pp.115-116.
 *
 * RETURNS: the square root of x
 *
 */

extern const short guesses[];		/* this is found in libm.a */
static const long int infinity[] = {0x7ff00000, 0x00000000 };
#define D_INF (*((double *) infinity))    

#ifdef _FRTINT
double _sqrt(double x)
#else
double sqrt(double x)
#endif
{
    double xx, xxx;
    union { double x;
	    int i[2]; } arg;
    int 
      gmx,			/* exponent of first guess */
      ymx,			/* exponent of 0.5/guess   */
      xm;			/* high order 32 bits of argument */
    union { double x;
	    int i[2]; } guess;	/* first guess at answer */
    union { double x;
	    int i[2]; } y;	/* 0.5/guess */
    double fmode, e, diff, h, y2, r1, r2;
    static const double tiny = 5.776622002767455e-275; /* 0x07000000 00000000 */
    static const double almost_half =  0.50000000000000010; /* 0x3fe00000 00000001 */ 
    static const double two = 2.0;
    static const double up_factor =  1.3407807929942597e+154; /* 0x5ff00000 00000000 */
    static const double down_factor =  8.6361685550944446e-78; /* 0x2ff00000 00000000 */
    static const double zero = 0.0;
    int
      q,			/* index into table */
      flag;			/* dummy target used in pre-computing branches */
#ifdef _SVID
    struct exception exc;	/* defined in math.h */
#endif
    arg.x = x;
    xx = x;
    flag = (xx < tiny);
    flag = (xx < D_INF );
    xm = arg.i[0];
    fmode = __setflm(zero);   /* Preserve floating status
			       *  and enter round to nearest mode.
			       */

    /* start calculating q, the index into the guess table.
     * the table contains an intitial guess for the square root,
     * as well as an initial guess for the reciprocal.
     *
     * The table we use has 256 entries, 16 bits per entry.   The 
     * high 8 bits (masked with 0xff00) contain 8 bits of 
     * significand for the guess; the low 8 bits (masked with 0xff)
     * contain 8 bits of significand for .5/guess.
     *
     * The index for the table is 8 bits; it consists of the
     * low order bit of the exponent and the seven highest bits
     * of the significand (of the argument).  The low bit of the
     * exponent is part of the table because this bit is lost
     * in computing the exponent of the guess.
     *
     * The exponent of the guess is computed by isolating the
     * high 32 bits of the argument; adding 0x3ff00000 to normalize
     * the exponent, shifting right by 1 to divide by 2, and
     * then masking with 0x7ff00000 to isolate the exponent.
     * This result is subtracted from 0x7fc00000 to obtain the 
     * exponent of .5/guess.
     */

    /* obtain the exponent part of guess and .5/guess,
     * as above.
     */
    gmx = ( (xm + 0x3ff00000) >> 1) & 0x7ff00000; 
    ymx = 0x7fc00000 - gmx;      /* approx 0.5/guess */

    /* move the branch below the above two fixed-point ops,
     * so that they are not delayed while waiting for computation
     * of CR to complete.
     */
    if(xx < tiny) 
      goto tiny_neg;
    
    /* now determine index into table */
    q = ((xm & 0x001fe000) >> 13);

    /* We've calculated the exponent for guess and 0.5/guess above.
     * retrieve significands from table, and combine form the
     * quantities as they'll be used.
     */
    guess.i[0] = gmx + ((0x0000ff00 & guesses[q]) << 4);
    y.i[0] = ymx + ((0x000000ff & guesses[q]) << 12);
 

    /* We've already taken care of negative numbers.  If arg
     * is less than INF, apply Newton-Raphson to refine result
     * to full precision.  Notice that the conditional value
     * was pre-computed above.  See Markstein's paper for a pretty
     * careful description of this section.
     */

    diff = xx - guess.x * guess.x;
    if (xx < D_INF ) 
	{
	h = y.x;
	guess.x = guess.x + h * diff;    /* 16 bit approx. */
	y2 = h + h;
	e = almost_half - h * guess.x;  /* Newton Raphson iteration for     */
	diff = xx - guess.x * guess.x;  /* reciprocal is interleaved with   */
	h = h + e * y2;                 /* Newton Raphson iteration for the */
	guess.x = guess.x +  h * diff;  /* sqrt.  32 bit approx.            */
	y2 = h + h;
	e = almost_half - h * guess.x;
	diff = xx - guess.x * guess.x;
	h = h + e * y2;
	guess.x = guess.x +  h * diff;   /* 64 bit approx. */
	y2 = h + h;
	e = almost_half - h * guess.x; /* Malheure! The approximation was  */
	diff = xx - guess.x * guess.x; /* good to 64 bits before rounding, but */
	h = h + e * y2;                /* it may have rounded incorrectly. */
	(void) __setflm(fmode);        /*  Restore original fl. pt. status */
	/* One more iteration gets it right by */
	guess.x = guess.x +  h * diff;   /* Markstein's theorem, and avoids a */
	                                 /* Tuckerman round. Furthermore, the */
	                                 /* last computation completely */
	                                 /* characterizes the result and */
	                                 /* correctly sets all the status bits. */
	return guess.x;
	}

    /* pre-compute the following branches */
    flag = (xx == D_INF)  && (xx != xx);
    (void) __setflm(fmode);               /* restore floating status */

    /* if x == INF, result is INF */
    if (xx == D_INF ) 
	{
	return xx; 
	}
    
    /* if x == NaN, result is Nan. */
    if (xx != xx) 
	{
	return xx + two;	/* Force NaN quiet */
	}

  tiny_neg:
    /******************************/
    /* Small or Negative argument */
    /******************************/

    /* pre-compute the following branches */
    flag = (xx == zero) && (xx < zero) ;

    /* sqrt(zero) = zero (keep sign) */
    if ( xx == zero) 
	{
	(void) __setflm(fmode);             /* restore floating status */
	return xx;
	}

    /* sqrt(neg #) = NaN.  For standard libm.a, must
     * raise the fp_inv_sqrt exception.  For SVID, must
     * call matherr.
     */
    if (xx < zero)
	{
	guess.x = D_INF - D_INF;           /* generate a NaN */
	(void) __setflm(fmode);            /* restore floating status */
#ifndef _SVID
	errno = EDOM;

/* starting with version 3.2, we must use the new interface fp_raise_xcp(), 
 * since sqrt() must trap if the invalid op. execption is enabled.
 */
#ifdef VERSION_AIX320
	fp_raise_xcp(FP_INV_SQRT);
#else	/* VERSION_AIX320 */
	fp_set_flag(FP_INVALID | FP_INV_SQRT);
#endif  /* VERSION_AIX320 */
	return guess.x;   /* return NaN */
#else
	exc.arg1 = x;
	exc.type = DOMAIN;
	exc.name = "sqrt";
	exc.retval = 0.0;
	if (!matherr(&exc))
	    {
	    write (2, exc.name, 4);
	    write (2, ": DOMAIN error\n", 15);
	    errno = EDOM;	/* don't set errno if user matherr */
	    }
	return(exc.retval);
#endif
	}

    /* if here, a small positive argument. */
    
    xxx = xx * up_factor;
    arg.x = xxx;
    /* resume standard algoithm (as commented above) at this point */
    xm = arg.i[0];
    guess.i[0] = ((xm + 0x3ff00000) >> 1) & 0x7ff00000;
    y.i[0] = 0x7fc00000 - guess.i[0]; /* approx 0.5/guess */
    q = (xm & 0x001fe000) >> 13;
    guess.i[0] = guess.i[0] + ((0x0000ff00 & guesses[q]) << 4);
    y.i[0] = y.i[0] + ((0x000000ff & guesses[q]) << 12);
    
    diff = xxx - guess.x * guess.x;
    h = y.x ;
    guess.x = guess.x + h * diff;    /* 16 bit approx. */
    y2 = h + h;
    e = almost_half - h * guess.x;   /* Newton Raphson iteration for */
    diff = xxx - guess.x * guess.x;  /* reciprocal is interleaved with */
    h = h + e * y2;                  /* Newton Raphson iteration for the */
    guess.x = guess.x +  h * diff;   /* sqrt.  32 bit approx. */
    y2 = h + h;
    e = almost_half - h * guess.x;
    diff = xxx - guess.x * guess.x;
    h = h + e * y2;
    guess.x = guess.x +  h * diff;   /*  64 bit approx. */
    y2 = two * h;
    e = almost_half - h * guess.x;   /*  Malheure! The approximation was */
    diff = xxx - guess.x * guess.x;  /*  good to 64 bits before rounding, but */
    h = h + e * y2;                  /*  it may have rounded incorrectly. */
    r1 = guess.x * down_factor;
    r2 = diff*down_factor;
    (void) __setflm(fmode);          /*  Restore original fl. pt. status */
                                     /*  One more iteration gets it right by */
                                     /*  Markstein's theorem, and avoids a */
                                     /*  Tuckerman round. */
    guess.x = r1 + r2 * h;
    return(guess.x);
} 
