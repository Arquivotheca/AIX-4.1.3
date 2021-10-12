#if (!( _FRTINT ))
static char sccsid[] = "@(#)08	1.2  src/bos/usr/ccs/lib/libm/POWER/rsqrt.c, libm, bos411, 9428A410j 10/22/93 10:25:49";
#endif
/*
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS: rsqrt
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifdef _SVID
#include <math.h>		/* for struct exception */
#endif /* _SVID */
#include <errno.h>		/* for errno declaration, EDOM */
#include <fptrap.h>		/* for fp_raise_xcp */
#include <fpxcp.h>		/* for FP_INV_SQRT, FP_DIV_BY_ZERO */
#include <fp.h>			/* for VALH() macro */
#include <float.h>		/* for DBL_QNAN */


/* This is actually a compiler built-in function;
 * the prototype reduces noise from lint.
 */
double __setflm(double);	/* compiler built-in to manipulate fpscr */

/*
 * NAME: rsqrt
 *                                                                    
 * FUNCTION: calculate 1/sqrt(x) directly without division
 *                                                                    
 * NOTES:
 *
 * method -- uses formula
 *
 *     g' = g - ((((g * (x/2)) * g) - 1/2) * g)
 *
 * which results from Newton's method.
 * The initial guess is obtained from the same
 * table used by sqrt(), and three refinements are
 * necessary to get to good accuracy. 
 *
 * If you #define ONE_MORE_CYCLE you can trim
 * a cycle from the main-line path.  The cost, however,
 * is that the results for round to +/- INF will be
 * wrong by 1 ulp, due to the odd rounding of the
 * fnms instruction.
 *
 * RETURNS: reciprocal of sqrare root of argument
 */


/* The following can be used so that rsqrt() can
 * be used as-is with AIX version 3.1.x should that
 * be necessary.
 */
#ifdef USE_AIX31
int fp_raise_xcp(fpflag_t mask)
  {
  fp_set_flag(FP_INVALID | mask);
  return 0;
  }
#endif     

#define INF   (*((double *) (dinf)))

double 
#ifdef _FRTINT
_rsqrt(double x)
#else    /* _FRTINT */
rsqrt(double x)
#endif   /* _FRTINT */
{
  extern short guesses[];	/* this is found in libm.a */
  double rm;			/* user's fpscr */
  union { double x;
          int i[2]; } arg;	/* used to 'twiddle bits' in argument */
  double t1, t2;		/* temps, used to get ops in right order */
#ifndef ONE_MORE_CYCLE
  double t3;			/* temp. to get op in right order */
#endif /* ONE_MORE_CYCLE */
  long int gmx, ymx, xm;	/* temps, used in constructing guess */
  union { double x;
          int i[2]; } y;        /* used in constructing guess */
  double g;			/* current guess of result */
  double xhalf;			/* one half of argument */
  int index;			/* index into guesses */
  static const long dinf[] =  {0x7ff00000, 0x0}; 
  const double tiny =  3.6455610097781989e-304;             /* 0x00f00000 00000000 */
  const double one_half = 0.5;
  const double zero = 0.0;
  const double up_factor =  1.3407807929942597e+154; /* 0x5ff00000 00000000 */
  const double down_factor =  1.157920892373162e+77; /* 0x4ff00000 00000000 */
#ifdef _SVID
    struct exception exc;	/* defined in math.h */
#endif

  /* save user's FPSCR, and clear FPSCR, disabling traps and
   * causing round to nearest mode.  Note that 'tiny' is carefully chosen
   * so that the low-order 32 bits are all zero.  This allows the constant
   * to be used twice, thus avoiding loading another constant, saving a
   * cycle.  WARNING:  If some future architecture uses a 64-bit FPSCR,
   * this might be a problem; check it.
   */
  rm = __setflm(tiny); 

  /* pre-compute condition registers */
  (void) (x != x);		/* is arg a NaN? */
  (void) (x > tiny);		/* is arg zero, negative or very small? */
  (void) (x == INF);		/* is arg INF? */

  /* algorithm needs x/2; compute it now; the
   * computation will be covered by fixed-point
   * calcuations.
   */
  xhalf = x * one_half;

  arg.x = x;			/* move arg to structure */
  xm = arg.i[0];		/* get hi part of arg as long */

  /* get index of guess.  This is taken from sqrt() code */
  gmx = ( (xm + 0x3ff00000) >> 1) & 0x7ff00000;
  y.i[1] = 0x0;
  ymx = 0x7fc00000 - gmx;
  index = ((xm & 0x001fe000) >> 13);

  /* condition registers are available now, so these
   * branches are "free" on fall-thru.  Branch for 
   * boundary conditions if necessary.  We use the
   * dreaded 'goto' because it represents exactly what
   * we want -- a branch for the exceptional case, and
   * fall-thru processing for the normal case.
   */

  if (x != x)
    goto NAN;
  if (!(x > tiny))
    goto SMALL_OR_NEGATIVE;
  if (x == INF)
    goto INFINITY;

  /* recover guess */
  y.i[0] = ymx + ((0x000000ff & guesses[index]) << 12);

  /* table actually has value of guess / 2.0; correct.
   * Use addition instead of multiplication to avoid
   * loading constant 2.0
   */
  g = y.x + y.x ;

  /* guess is good to 8 bits; it takes 3 refinements to
   * arrive at enough bits for a double.
   */
  t1 = xhalf * g;		/* iteration 1 */
  t2 = t1 * g - one_half;
  g = g - t2 * g;
  t1 = xhalf * g;		/* iteration 2 */
  t2 = t1 * g - one_half;
  g = g - t2 * g;
  t1 = xhalf * g;		/* iteration 3 */
  t2 = t1 * g - one_half;

  /* return to user's floating point environment.
   * Last operation, in user's rounding mode,
   * characterizes result.  The 'logical' next operation
   * is g = g - t2 * g; however, this generates an fnms
   * instruction, which rounds the result before the 
   * negation, which causes a 1-ulp error in the directed
   * rounding modes.  The solution, at the cost of 1 extra
   * cycle, is to negate one of the terms and generate
   * an fma instruction instead.  However, it's left
   * #ifdef'd to build either way, depending on which
   * version you want.
   */
#ifdef ONE_MORE_CYCLE
  (void) __setflm(rm);
  g = g - t2 * g;
#else /* ONE_MORE_CYCLE */
  t3 = -g;
  (void) __setflm(rm);
  g = t3 * t2 + g;
#endif /* ONE_MORE_CYCLE */

  return g;

  /* For a quiet NaN, return the argument.  For a signalling
   * NaN, return a quiet NaN, and do the operation to quiet
   * the NaN is the user's floating point environment, so
   * an invalid op trap will occur if enabled.
   */

 NAN:
  (void) __setflm(rm);
  g = x + one_half;		/* re-use existing constant */
  return g;

  /* Here for small, zero, or negative  arguments.  We delay 
   * determination of which case to keep branches out of main
   * line code.  
   *
   * For small numbers, we multiply the argument by a large number
   * compute the result, and then correct the result.
   * 
   * For +/- zero, return +/- INF; this involves
   * some bit manipulation as most easy way to get sign right.
   *
   * For negative numbers return NaN.  In both cases explicity
   * raise appropriate IEEE floating point error after restoring
   * user's floating point state.
   */

 SMALL_OR_NEGATIVE:  
  if (x > zero)
      {
      /* denorm or very small normal argument */
      x *= up_factor;
      arg.x = x;			/* move arg to structure */
      xhalf = x * one_half;
      xm = arg.i[0];		/* get hi part of arg as long */
      gmx = ( (xm + 0x3ff00000) >> 1) & 0x7ff00000;
      ymx = 0x7fc00000 - gmx;
      index = ((xm & 0x001fe000) >> 13);
      y.i[0] = ymx + ((0x000000ff & guesses[index]) << 12);
      y.i[1] = 0x0;
      g = y.x + y.x;
      
      t1 = xhalf * g;		/* iteration 1 */
      t2 = t1 * g - one_half;
      g = g - t2 * g;
      t1 = xhalf * g;		/* iteration 2 */
      t2 = t1 * g - one_half;
      g = g - t2 * g;
      t1 = xhalf * g;		/* iteration 3 */
      t2 = t1 * g - one_half;

      t1 = g * down_factor;
      t2 *= down_factor;

      /* return to user's floating point environment.
       * Last operation, in user's rounding mode,
       * characterizes result.
       */
      (void) __setflm(rm);
      g = t1 - t2 * g;
      return g;
      }
  else
      {
      if (x == zero)
	  {
	  (void) __setflm(rm);	/* restore user's FP state */
#ifndef _SVID
	  g = tiny / x;		/* divide by zero gets fpscr bits set */
	  return g;		/* return (appropriately signed) INF */
#else /* ifndef _SVID */
		exc.arg1 = x;
		exc.type = SING;
		exc.name = "rsqrt";
		exc.retval = tiny / x;
		if (!matherr(&exc))
		{       
		write (2, exc.name, 5);
		write (2, ": SING error\n", 13);
		errno = EDOM;
		}
	  return exc.retval;
#endif /* ifndef _SVID */
	  } 
      else
	  {  
	  /* negative argument */
	  g = INF - INF;	/* produce a quiet NaN */
	  (void) __setflm(rm);
#ifndef _SVID
	  errno = EDOM;
	  (void) fp_raise_xcp(FP_INV_SQRT);
	  return g;
#else /* ifndef _SVID */
	exc.arg1 = x;
	exc.type = DOMAIN;
	exc.name = "rsqrt";
	exc.retval = 0.0;
	if (!matherr(&exc))
	    {
	    write (2, exc.name, 5);
	    write (2, ": DOMAIN error\n", 15);
	    errno = EDOM;	/* don't set errno if user matherr */
	    }
	return exc.retval;
#endif /* ifndef _SVID */
	  }
      }
  
  /* Here for positive infinity.  The answer is zero, exactly,
   * without overflow set.
   */

 INFINITY:
  (void) __setflm(rm);
  return zero;
}    
