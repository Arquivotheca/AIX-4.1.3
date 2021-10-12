static char sccsid[] = "@(#)04	1.2  src/bos/usr/ccs/lib/libm/POWER/fmodl.c, libm, bos411, 9428A410j 10/1/93 17:09:05";
/*
 *   COMPONENT_NAME: LIBM
 *
 *   FUNCTIONS: fmodl
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

#include <errno.h>		/* errno, EDOM */

#define TWOTO54 (*((double *) twoto54))
#define LD_TWOTO54 (*((long double *) twoto54))
static unsigned twoto54[]={ 0x43500000, 0x0, 0x0, 0x0 };  /* 2 ^ 54 */

#define TWOTOM54 (*((double *) twotom54))
static unsigned twotom54[]={ 0x3c900000, 0x0 }; /* 2 ^ -54 */

/*
 * NAME:  fmodl
 *
 * FUNCTION:  calculate long double "remainder" of x/y, defined as
 *
 *       result = x - i * y
 *
 * for some value of i such that, if y is nonzero, the result has the
 * sign as x and magnitude less that the magnitude of y.
 *
 * A domain error occurs if y is zero.
 *
 */

long double
fmodl(long double x, long double y)
  {
  union {
      long double l;
      struct { double hi, lo; } d;
      struct { unsigned int a, b, c, d; } u;
      } ldx, ldy, tmp, ldxa, ldya;
  double t1;
  double save;
  unsigned int ui;
  union {
      double d;
      struct { unsigned int a,b; } u;
      } d;
  double zero = 0.0;
  
  /* scheduling help */
  ldx.l = x;
  ldy.l = y;
  ldx.d.hi != ldx.d.hi;
  ldy.d.hi != ldy.d.hi;
  ldy.d.hi == 0.0;
  ldy.d.lo == 0.0;
  0x7ff00000 == (ldx.u.a & 0x7ff00000);
  0x7ff00000 == (ldy.u.a & 0x7ff00000);
  
  /* first take care of boundary cases */

  /* if x is a NaN, return it (perhaps quieted) */
  if (ldx.d.hi != ldx.d.hi)
      return x + y;

  /* if y is a NaN, return it (perhaps quieted) */
  if (ldy.d.hi != ldy.d.hi)  
      return x + y;

  /* if y = 0.0, return NaN and set EDOM */
  if ((ldy.d.hi == 0.0) && (ldy.d.lo == 0.0))
      {
      errno = EDOM;
      tmp.d.hi = zero / zero;
      tmp.d.lo = 0.0;
      return tmp.l;
      }
  
  (ldx.d.hi == ldy.d.hi);
  (ldx.d.lo == ldy.d.lo);
  
  /* if x == +/- INF, return NaN */
  if (0x7ff00000 == (ldx.u.a & 0x7ff00000))
      {
      tmp.d.hi = zero / zero;
      tmp.d.lo = 0.0;
      return tmp.l;
      }

  /* if y == +/- INF, return x */
  if (0x7ff00000 == (ldy.u.a & 0x7ff00000))
    return x;

  /* if y == x then result is 0.0L */
  if ((ldx.d.hi == ldy.d.hi) &&
      (ldx.d.lo == ldy.d.lo))
    return 0.0L;
  
  /* take absolute value of arguments */
  ldxa.l = x;
  ldya.l = y;
  if (ldxa.d.hi < 0.0)
      {
      ldxa.d.hi = - ldxa.d.hi;
      ldxa.d.lo = - ldxa.d.lo;
      }
  if (ldya.d.hi < 0.0)
      {
      ldya.d.hi = - ldya.d.hi;
      ldya.d.lo = - ldya.d.lo;
      }

  /* if abs(x) < abs(y) the result is x */
  if (ldxa.l < ldya.l)
    return x;
  
  /* If y is denormal scale.  The main loop plays with exponents
   * directly, so we must have a number with an explicit exponent.
   * This is done with appropriate scaling.  The test below works
   * because y == 0.0 has been filtered out above.
   */
  if (0x0 == (ldya.u.a & 0x7ff00000))
      {
      /* calculate y * 2^54 */
      ldy.d.hi *= TWOTO54;
      ldy.d.lo *= TWOTO54;

      tmp.l = fmodl(x, ldy.l);

      /* tmp = tmp * 2^54 */
      tmp.d.hi *= TWOTO54;
      tmp.d.lo *= TWOTO54;

      tmp.l = fmodl(tmp.l, ldy.l);

      tmp.d.hi *= TWOTOM54;
      tmp.d.lo *= TWOTOM54;

      return tmp.l;
      }

  /* main reduction loop */
  d.d = 0.0;
  while (ldxa.d.hi >= ldya.d.hi)
      {
      ui = (ldxa.u.a - ldya.u.a) & 0x7ff00000;
      ui += 0x3ff00000;		/* offset */
      tmp.l = ldya.l;
      d.u.a = ui;
      tmp.d.hi *= d.d;
      tmp.d.lo *= d.d;

      /* we may have overestimated; if so divide by 2 */
      if (ldxa.l < tmp.l)
	  {
	  tmp.d.hi *= 0.5;
	  tmp.d.lo *= 0.5;
	  }
      
      ldxa.l -= tmp.l;
      }

  if (ldx.d.hi < 0.0)
    ldxa.l = -ldxa.l;
  
  return ldxa.l;
  }

