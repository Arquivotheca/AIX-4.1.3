static char sccsid[] = "@(#)93	1.3  src/bos/usr/ccs/lib/libc/POWER/f64toi64rz.c, libccnv, bos411, 9428A410j 9/10/93 13:41:31";
/*
 *   COMPONENT_NAME: LIBCCNV
 *
 *   FUNCTIONS: RETURN
 *		__f64toi64rz
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
#include <fpxcp.h>		/* fp_raise_xcp and it's arguments */
#include <limits.h>		/* LONGLONG_MIN, LONGLONG_MAX */
#include <math.h>		/* __f64toi64rz prototype */

/*
 * If COMPILER_BUG is defined some extra code is include to work
 * around a bug in the development level compiler.  Hopefully we
 * can undefine this before ship.
 */
#define COMPILER_BUG

/*
 * If SET_XX is defined the code will set inexact (XX) for arguments 
 * which are not integer but are otherwise in range.
 */
#define SET_XX
     
#define RETURN(XX, YY) {                                         \
  return (((long long)(XX)<<32) | (unsigned)(YY));      \
  }

#define BIG        (*((double *) (big)))       /* largest positive number to convert */
#define MBIG       (*((double *) (mbig)))      /* largest neative number to convert */
#define TWO31      (*((double *) (two31)))     /* 2 ** 31 */
#define TWO32      (*((double *) (two32)))     /* 2 ** 32 */
#define TWO52      (*((double *) (two52)))     /* 2 ** 52 */
#define TWOM32     (*((double *) (twom32)))    /* 2 ** -32 */
#define RND_ZERO   (*((double *) (rnd_zero)))  /* FPSCR value for round to zero mode */
#define UIT_CONST  (*((double *) (uit_const))) /* conversion magic number */

static const long big[]       =  {0x43dfffff, 0xffffffff}; /*  */
static const long mbig[]      =  {0xc3e00000, 0x0}; 
static const long two31[]     =  {0x41e00000, 0x0}; 
static const long two32[]     =  {0x41f00000, 0x0}; 
static const long twom32[]    =  {0x3df00000, 0x0}; 
static const long two52[]     =  {0x43300000, 0x0};
static const long rnd_zero[]  =  {0x0, 0x1};
static const long uit_const[] =  {0x43300800,0x0}; /* 2^52 +  2^43 */

/*
 * NAME: __f64toi64rz
 *                                                                    
 * FUNCTION: convert double precision number to 64-bit
 *           signed integer with round toward zero.
 *                                                                    
 * NOTES:
 *          Following are return values and behavior on errors:
 *        
 *  ARG              RESULT
 *  ---------------  -----------------------
 *  large positive   LONGLONG_MAX
 *  large negative   LONGLONG_MIN
 *  +INF             LONGLONG_MAX
 *  -INF             LONGLONG_MIN
 *  NaN              LONGLONG_MIN
 *
 *  If argument is a signalling NaN, raise VXSNAN.
 *  If argument is outside ranges of representable results,
 *     raise VXCVI.
 *
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
 *
 * RETURNS: a signed 64-bit integer
 */

long long
__f64toi64rz(double x)
  {
  double r;
  unsigned int rh, rl;		/* hi and low part of result */
  int srh, srl;			/* signed result */
  double wh, wl;		/* work variables */
  union {
      double x;
      struct { unsigned long hi, lo; } i;
         } user_fpscr;
  double temp;			/* work variable */
  double x_int;			/* argument rounded to integer */
  union {
      double x;
      struct { unsigned long hi, lo; } i;
         } work;
  double abs_x;			/* absolute value of argument */
  int sign;			/* store sign of argument */
#ifdef SET_XX
  int inexact = 0;
#endif

  /* note that if x is a signalling NaN, the compare is sufficient
   * to raise VXSNAN
   */

  (x != x);
  (x > BIG);
  (x < MBIG);
  abs_x = __fabs(x);
  (abs_x < TWO31);
  (x > 0.0);
  
  if (x != x)
    goto NAN;

  if (x > BIG)
    goto BIG_POSITIVE;
  
  if (x < MBIG)
    goto BIG_NEGATIVE;
  
  if (abs_x < TWO31)
      {
      /* if the number is small enough to fit in one 32-bit integer,
       * use the simple algorithm as in itrunc()
       */
      if (!(x < 0.0))		/* easier than (x >= 0) because of NaNs */
	  {
	  /* set round to minus inf -- note that __setrnd() uses
	   * hardware encoding, not the ANSI encoding 
	   */
	  user_fpscr.x =__setrnd(0x3); 
	  work.x = x + UIT_CONST;	     /* may set inexact */
	  srl = work.i.lo;		     /* extract integer */
#ifdef COMPILER_BUG
	  __setrnd(0x0);		     /* req'd for bug in compiler */
#endif
	  __setrnd(user_fpscr.i.lo);         /* restore user round mode */
	  return (long long) srl;
	  }
      else
	  {
	  user_fpscr.x =__setrnd(0x2);       /* set round to plus inf */
	  work.x = x + UIT_CONST;	     /* may set inexact */
	  srl = work.i.lo;		     /* extract integer */
#if 0
	  __setrnd(0x0);		     /* req'd for bug in compiler */
#endif
	  __setrnd(user_fpscr.i.lo); /* restore user round mode */
	  if (srl != 0)		             /* cater to case of small negative which */
	                                     /* rounds to zero. */
	      { /* These brackets are necessary -- RETURN() is a macro  */
	      RETURN(~0x0, srl);	
	      }
	  else
	      {
	      return 0x0LL;
	      }
	  }
      }

  /* main path for arguments which are in range to convert 
   * and will require both 32-bit parts of result
   */
  user_fpscr.x = __setflm(RND_ZERO);

  sign = (x < 0.0) ? 1 : 0;	/* save sign of argument */

  x = abs_x;
  /*  wh = rint(x); */
  if (x < TWO52)
      {
      temp = x + TWO52;
      x_int = temp - TWO52;
#ifdef SET_XX
      if (x != x_int)
	inexact = 1;
#endif
      }
  else
      {
      x_int = x;
      }
  
  wh = x_int;
  
  wh *= TWOM32;

  /*   rh = (unsigned int) wh; */
  work.x = wh + UIT_CONST;
  rh = work.i.lo;
  
  /*   wl = rint(wh); */
  temp = wh + TWO52;
  wl = temp - TWO52;
  
  wl *= TWO32;
  wl = x_int - wl;

  /*   rl = (unsigned int) wl; */
  work.x = wl + UIT_CONST;
  rl = work.i.lo;

#ifdef SET_XX
  if (inexact != 0)
    user_fpscr.i.lo |= (FP_INEXACT | FP_ANY_XCP);
#endif

  (void) __setflm(user_fpscr.x);

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

 NAN:
  fp_raise_xcp(FP_INV_CVI);
  return LONGLONG_MIN;
  
 BIG_POSITIVE:
  fp_raise_xcp(FP_INV_CVI);
  return LONGLONG_MAX;
  
 BIG_NEGATIVE:
  fp_raise_xcp(FP_INV_CVI);
  return LONGLONG_MIN;
  }

