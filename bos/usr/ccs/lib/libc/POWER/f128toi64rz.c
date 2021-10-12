static char sccsid[] = "@(#)91  1.5  src/bos/usr/ccs/lib/libc/POWER/f128toi64rz.c, libccnv, bos411, 9428A410j 5/13/94 16:28:24";
/*
 *   COMPONENT_NAME: LIBCCNV
 *
 *   FUNCTIONS: 
 *		__f128toi64rz
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

#include <limits.h>		/* LONGLONG_MIN, LONGLONG_MAX */
#include <math.h>		/* __f128toi64rz prototype */

extern long double _qint(long double);

#define RETURN(XX, YY) {                                         \
  return (((long long)(XX)<<32) | (unsigned)(YY));      \
  }
     
#define BIG        (*((double *) (big)))       /* smallest magnitude postive that might overflow to +INF */
#define MBIG       (*((double *) (mbig)))      /* smallest magnitude negative that might overflow to -INF */
#define TWO31      (*((double *) (two31)))     /* 2 ** 31 */
#define TWO32      (*((double *) (two32)))     /* 2 ** 32 */
#define TWO52      (*((double *) (two52)))     /* 2 ** 52 */
#define TWOM32     (*((double *) (twom32)))    /* 2 ** -32 */
#define UIT_CONST  (*((double *) (uit_const))) /* conversion magic number */

static const long big[]       =  {0x43e00000, 0x0};
static const long mbig[]      =  {0xc3e00000, 0x0}; 
static const long two31[]     =  {0x41e00000, 0x0}; 
static const long two32[]     =  {0x41f00000, 0x0}; 
static const long twom32[]    =  {0x3df00000, 0x0}; 
static const long two52[]     =  {0x43300000, 0x0};
static const long uit_const[] =  {0x43300800,0x0}; /* 2^52 +  2^43 */

long long
__f128toi64rz(long double Q_arg)
  {
  int srh, srl;			/* signed result */
  double x;			/* argument converted to double */
  unsigned int rh, rl;		/* unsigned result */
  double wh, wl;		/* work variables */
  double user_fpscr;		/* user's FPSCR */
  double x_int;			/* argument as double and rounded to int */
  union {
      double x;
      struct { unsigned long hi, lo; } i;
        } work;
  union {
      long double q;
      struct { double hi, lo; } d;
        }
        Q_arg_int,		/* argument rounded to integer */
        hi_super,		/* high superdigit */
        lo_super;		/* low superdigit */
  int sign;			/* store sign of argument */
  
  /* this will raise a signalling NaN */
  x = (double) Q_arg;
  (x != x);
  (x < BIG);
  (x > MBIG);
  (__fabs(x) < TWO31);
  
  if (x != x)
    goto NAN;

  if (!(x < BIG))
    goto BIG_MAYBE_OVERFLOW;
  
  if (!(x > MBIG))
    goto NEGATIVE_BIG_MAYBE_OVERFLOW;

  if (__fabs(x) < TWO31)
      {
      if (!(x < 0.0))		/* easier than (x >= 0.0) because of NaNs */
	  {
	  user_fpscr =__setrnd(0x3);        /* set round to minus inf -- machine encoding */
	  work.x = x + UIT_CONST;	    /* may set inexact */
	  srl = work.i.lo;		    /* extract integer */
	  __setflm(user_fpscr);             /* restore user round mode */
	  return (long long) srl;
	  }
      else
	  {
	  user_fpscr =__setrnd(0x2);        /* set round to plus inf */
	  work.x = x + UIT_CONST;           /* may set inexact */
	  srl = work.i.lo;		    /* extract integer */
	  __setflm(user_fpscr);             /* restore user round mode */
	  if (srl != 0)		            /* cater to case of small negative which */
	                                    /* rounds to zero. */
	      { /* These brackets are necessary -- RETURN() is a macro  */
	      RETURN(~0x0, srl);	
	      }
	  else
	      return 0x0LL;
	  }
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
  sign = (x < 0.0) ? 1 : 0;	/* save sign of argument */
  user_fpscr = __setflm(0.0);	/* disable traps and round to nearest */

  if (sign != 0)
    Q_arg = -Q_arg;		/* absolute value of argument */

  /* high super digit; extended precision multiply */
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

  if (sign != 0)
      {
      if (rl == 0)
	  {
	  srh = -((int) rh);
	  srl = 0;
	  }
      else
	  {
	  srl = -((int) rl);
	  srh = ~((int) rh);
	  }
      RETURN(srh, srl);
      }

  RETURN(rh, rl);

  /* If the value of the high part matches the over value extra
   * care is required, because if the low part is of opposite
   * sign and small you can have a number that has integer bits
   * in the low part, but when converted to double is equal to
   * the high part.
   */

 BIG_MAYBE_OVERFLOW:
  Q_arg_int.q = Q_arg;
  if ((Q_arg_int.d.hi == BIG) && (Q_arg_int.d.lo < 0.0))
    goto CONVERT_LONG;
  else
    return LONGLONG_MAX;  

 NEGATIVE_BIG_MAYBE_OVERFLOW:
  Q_arg_int.q = Q_arg;
  if ((Q_arg_int.d.hi == MBIG) && (Q_arg_int.d.lo > 0.0))
    goto CONVERT_LONG;
  else
    return LONGLONG_MIN;  

 NAN:
  return LONGLONG_MIN;
  }
