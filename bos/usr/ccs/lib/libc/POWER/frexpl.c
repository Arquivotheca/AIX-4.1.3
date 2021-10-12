static char sccsid[] = "@(#)96	1.3  src/bos/usr/ccs/lib/libc/POWER/frexpl.c, libccnv, bos411, 9428A410j 12/7/93 08:13:02";
/*
 *   COMPONENT_NAME: LIBCCNV
 *
 *   FUNCTIONS: frexpl
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <limits.h>		/* INT_MAX, INT_MIN */
#include <math.h>		/* scalb, logb */
#include <stdlib.h>		/* NULL */

/*
 * NAME: frexpl
 *                                                                    
 * FUNCTION: Break a long double value into a normalized fraction
 *           and an integer power of 2.  The fraction is returned
 *           as the explicit result, and the power of 2 is stored
 *           into *exp.  The normalized fraction is in the range
 *           [1/2, 1] or is zero.
 *
 *           Boundary conditions are as follows:
 *
 *           Argument    Result     Exponent
 *           -------     ------     --------
 *           +INF        +INF       INT_MAX
 *           -INF        -INF       INT_MIN
 *           NaN         NaN        INT_MIN
 *           +0.0        +0.0       0
 *           -0.0        -0.0       0
 */

long double
frexpl(long double x, int *exp)
  {
  double x_hi, x_lo;
  double exp_work;
  union {
  	long double l;
  	struct { double hi, lo;  } d;
  	} ld;
  ld.l = x;
  x_hi = ld.d.hi;
  x_lo = ld.d.lo;

  /* make sure pointer is valid */
  if (exp == NULL )
    exp = &exp_work;

  /* Take care of boundary conditions first.
   * If number is not finite, return appropriate
   * values (depending on what it is).  We assume
   * a "good" quad number in that if the number is
   * NaN, the high part must be NaN.
   */

  if ( (!finite(x_hi)) || (!finite(x_lo)))
      {
      if (x_hi != x_hi)		/* NaN case */
	  {
	  *exp = INT_MIN;
	  return x + 1.0L;	/* make NaN quiet */
	  }
      
      if (x_hi > 0.0)		/* +INF */
	  {
	  *exp = INT_MAX;
	  return x;
	  }
      else			/* -INF */
	  {
	  *exp = INT_MIN;
	  return x;
	  }
      }
  
  if (x == 0.0L)		/* +/- 0.0 */
      {
      *exp = 0;
      return x;
      }

  /*
   * All boundary conditions are taken care of, so
   * proceed with normal case.  Get exponent by converting
   * long double to double and calling logb.  The exponent
   * from logb must be incremented by one to get the
   * resulting fraction into the right range.  scalb is
   * used to generate the fraction part; two calls to double 
   * precision scalb are OK since this is the same thing
   * as multiplying by a power of 2, which will not
   * generate a carry between the two parts of the long
   * double.
   */
  
  exp_work = logb((double) x);
  exp_work++;
  *exp = (int) exp_work;
  ld.d.hi  = scalb(x_hi, -exp_work);
  ld.d.lo  = scalb(x_lo, -exp_work);
  return ld.l; 
  }  
