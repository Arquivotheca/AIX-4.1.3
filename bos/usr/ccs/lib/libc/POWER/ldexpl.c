static char sccsid[] = "@(#)97	1.4  src/bos/usr/ccs/lib/libc/POWER/ldexpl.c, libccnv, bos411, 9428A410j 12/7/93 08:13:05";
/*
 *   COMPONENT_NAME: LIBCCNV
 *
 *   FUNCTIONS: 
 *		ldexpl
 *		
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

#include <math.h>		/* scalb, copysign */
#include <errno.h>		/* errno, ERANGE */

#define LDEXIT(HI, LO) { \
  ld.d.hi = (HI);        \
  ld.d.lo = (LO);        \
  return ld.l;  }

/*
 * NAME: ldexpl
 *                                                                    
 * FUNCTION: Multiply the long double value 'x' by 2 raised to
 *           the 'exp' power.  The result is returned explicity.
 *           
 *           A range error occurs if the value overflows.
 *
 *           If value is NaN, return it (perhaps made quiet).
 *           If value is +/- INF, return it.
 *           IF value is +/- zero, return it.
 *
 *           scalb() is used to scale each part independently.
 */

long double
ldexpl(long double x, int exp)
  {
  double x_hi, x_lo;
  double result_hi, result_lo;
  union {
  	long double l;
  	struct { double hi, lo;  } d;
  	} ld;
  ld.l = x;
  x_hi = ld.d.hi;
  x_lo = ld.d.lo;

  if (x_hi != x_hi)		/* NaN case */
    return x + 1.0L;

  if ((!finite(x_hi) ||         /* +/- INF, zero */
       (x_hi == 0.0)))
    return x;
  
  result_hi = scalb(x_hi, (double) exp);	/* scale each part */
  result_lo = scalb(x_lo, (double) exp);
  
  if (finite(result_hi))
      {
      LDEXIT(result_hi, result_lo);
      }
  else
      {
      errno = ERANGE;
      LDEXIT(result_hi, copysign(0.0, result_hi));
      }
  }
