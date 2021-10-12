static char sccsid[] = "@(#)92	1.4  src/bos/usr/ccs/lib/libc/POWER/f128tou64rz.c, libccnv, bos411, 9428A410j 5/13/94 16:28:27";
/*
 *   COMPONENT_NAME: LIBCCNV
 *
 *   FUNCTIONS: 
 *		__f128tou64rz
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <limits.h>		/* ULONGLONG_MAX */
#include <math.h>		/* __f128tou64rz prototype */

extern long double _qint(long double);

#define RETURN(XX, YY) {                                         \
  return (((unsigned long long)(XX)<<32) | (unsigned)(YY));      \
  }

#define BIG        (*((double *) (big)))
#define TWO32      (*((double *) (two32)))
#define TWO52      (*((double *) (two52)))
#define TWOM32     (*((double *) (twom32)))
#define UIT_CONST  (*((double *) (uit_const)))

static const long big[]       =  {0x43f00000, 0x0}; 
static const long two32[]     =  {0x41f00000, 0x0}; 
static const long twom32[]    =  {0x3df00000, 0x0}; 
static const long two52[]     =  {0x43300000, 0x0};
static const long uit_const[] =  {0x43300800,0x0}; /* 2^52 +  2^43 */

unsigned long long
__f128tou64rz(long double Q_arg)
  {
  double x;			/* argument converted to double */
  unsigned int rh, rl;		/* unsigned result */
  double wh, wl;		/* work variables */
  double user_fpscr;		/* user's FPSCR */
  double temp;			/* work variable */
  double x_int;			/* argument as double and rounded to int */
  union {
      double x;
      struct { unsigned long hi, lo; } i;
         } work;
  double arg_hi, arg_lo;	/* two parts of argument */
  union {
      long double q;
      struct { double hi, lo; } d;
        }
        Q_arg_int,		/* argument rounded to integer */
        hi_super,		/* high superdigit */
        lo_super;		/* low superdigit */
  
  x = (double) Q_arg;		/* can raise SNaN */
  (x != x);
  (x > 0.0);
  (x < BIG);
  (x < TWO32);
  
  if (x != x)
    goto NAN;

  if (!(x > 0.0))
    goto NEGATIVE_OR_ZERO;

  if (!(x < BIG))
    goto MAYBE_OVERFLOW;

  if (x < TWO32)
      {
      user_fpscr =__setrnd(0x1); /* set round to zero */
      work.x = x + UIT_CONST;	 /* may set inexact */
      rh = work.i.lo;		 /* extract integer */
      __setflm(user_fpscr);	 /* restore user FPSCR */
      return (unsigned long long) rh;
      }

/*
 * Algorithm:
 *
 * 1. Take care of boundary conditions:
 *    NaN
 *    Negative or zero
 *    < 2^32 (easy algorithm)
 *    overflow
 *
 * 2.  Set round mode to Round to Zero, and
 *     save user's FPSCR
 *
 * 3.  Round argument to integer.
 *       x = int(x)
 *
 * 4.  Extract high base 2^32 digit
 *       high = x * 2^-32
 *       high_digit = integer(high)
 *
 * 5.  Extract low base 2^32 digit
 *       low = x - (int(high) * 2^32)
 *       low_digit = integer(low)
 */
 CONVERT_LONG:
  user_fpscr = __setflm(0.0);	/* disable traps and set round to nearest */
  
  /* high super digit; extended precision multiply */
  /* high = x * 2^-32 */
  Q_arg_int.q = _qint(Q_arg);
  work.x = Q_arg_int.d.lo * TWOM32;
  hi_super.d.hi = Q_arg_int.d.hi * TWOM32 + work.x;
  hi_super.d.lo = Q_arg_int.d.hi * TWOM32 - hi_super.d.hi + work.x;
  hi_super.q = _qint(hi_super.q);
  work.x = (double) hi_super.q;

  (void) __setrnd(0x1);		/* round to zero */
  work.x = work.x + UIT_CONST;
  rh = work.i.lo;
  (void) __setrnd(0x0);		/* round to nearest */

  /* low superdigit */
  /* hi_super * TWO32 */
  work.x = hi_super.d.lo * TWO32;
  lo_super.d.hi = hi_super.d.hi * TWO32 + work.x;
  lo_super.d.lo = hi_super.d.hi * TWO32 - lo_super.d.hi + work.x;
  lo_super.q = Q_arg_int.q - lo_super.q;
  lo_super.q = _qint(lo_super.q);
  work.x = (double) lo_super.q;

  (void) __setrnd(0x1);		/* round to zero */
  work.x = work.x + UIT_CONST;
  rl = work.i.lo;

  (void) __setflm(user_fpscr);	/* restore user's FPSCR */

  RETURN(rh, rl);

 NAN:
  return 0x0ULL;
  
 NEGATIVE_OR_ZERO:
  return 0x0ULL;

 MAYBE_OVERFLOW:
  Q_arg_int.q = Q_arg;
  if ((Q_arg_int.d.hi == BIG) && (Q_arg_int.d.lo < 0.0))
    goto CONVERT_LONG;
  else
    return ULONGLONG_MAX;
  }
