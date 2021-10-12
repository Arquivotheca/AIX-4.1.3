static char sccsid[] = "@(#)94	1.3  src/bos/usr/ccs/lib/libc/POWER/f64tou64rz.c, libccnv, bos411, 9428A410j 9/10/93 13:41:44";
/*
 *   COMPONENT_NAME: LIBCCNV
 *
 *   FUNCTIONS: RETURN
 *		__f64tou64rz
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
#include <math.h>		/* __f64tou64rz prototype */

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
  return (((unsigned long long)(XX)<<32) | (unsigned)(YY));      \
  }

#define BIG        (*((double *) (big)))       /* largest positive number to convert */
#define TWO32      (*((double *) (two32)))     /* 2 ** 32 */
#define TWO52      (*((double *) (two52)))     /* 2 ** 52 */
#define TWOM32     (*((double *) (twom32)))    /* 2 ** -32 */ 
#define RND_ZERO   (*((double *) (rnd_zero)))  /* FPSCR value for round to zero mode */
#define UIT_CONST  (*((double *) (uit_const))) /* conversion magic number */

static const long big[]       =  {0x43efffff, 0xffffffff}; 
static const long two32[]     =  {0x41f00000, 0x0}; 
static const long twom32[]    =  {0x3df00000, 0x0}; 
static const long two52[]     =  {0x43300000, 0x0};
static const long rnd_zero[]  =  {0x0, 0x1};
static const long uit_const[] =  {0x43300800,0x0}; /* 2^52 +  2^43 */

/*
 * NAME: __f64tou64rz
 *                                                                    
 * FUNCTION: convert double precision number to 64-bit
 *           unsigned integer with round toward zero.
 *                                                                    
 * NOTES:
 *          Following are return values and behavior on errors:
 *        
 *  ARG              RESULT
 *  ---------------  -----------------------
 *  large positive   ULONGLONG_MAX
 *  negative         0LL
 *  +INF             LONGLONG_MAX
 *  -INF             0LL
 *  NaN              0LL
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
 * RETURNS: an unsigned 64-bit integer
 */

unsigned long long
__f64tou64rz(double x)
  {
  double r;
  unsigned int rh, rl;
  double wh, wl;
  union {
      double x;
      struct { unsigned long hi, lo; } i;
         } user_fpscr;
  double temp;
  double x_int;
  union {
      double x;
      struct { unsigned long hi, lo; } i;
         } work;
#ifdef SET_XX
  int inexact = 0;
#endif
  
/* note: the compare operation, with user's FPSCR,
 * will cause VXSNAN to be raised if it is a signalling NaN.
 *
 * The following tries to induce the compiler to do the compares
 * as early as possible.  Also, we "goto" exception code because
 * we want to fall thru on the main path.
 */

  (x != x);
  (x > 0.0);
  (x < BIG);
  (x < TWO32);

  if (x != x)
    goto NAN;
  
  if (!(x > 0.0))
    goto NEGATIVE_OR_ZERO;
  
  if (x > BIG)
    goto OVERFLOW2;

  if (x < TWO32)
      {
      /* the following code assumes that __setrnd() is
       * fully atomic, which it is not in the compiler
       * currently used in the build environment
       */
      user_fpscr.x =__setrnd(0x1); /* set round to zero -- machine encoding, not ANSI */
      work.x = x + UIT_CONST;	   /* may set inexact */
      rh = work.i.lo;		   /* extract integer */
#ifdef COMPILER_BUG
      __setrnd(0x0);		   /* req'd for bug in compiler */
#endif
      __setrnd(user_fpscr.i.lo);   /* restore user round mode */
      return (unsigned long long) rh;
      }
  user_fpscr.x = __setflm(RND_ZERO);
  /*  wh = rint(x); */
  /* 
   * any number greater than 2^52
   * is guaranteed to be an integer w/o extra work
   */
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


  RETURN(rh, rl);

  /* process NaN's */

 NAN:
  fp_raise_xcp(FP_INV_CVI);
  return 0x0LL;


 NEGATIVE_OR_ZERO:  
  if (x == 0.0)
    return 0x0LL;
  fp_raise_xcp(FP_INV_CVI);
  return 0x0LL;

 OVERFLOW2:
  fp_raise_xcp(FP_INV_CVI);
  return 0xffffffffffffffffLL;
  }

