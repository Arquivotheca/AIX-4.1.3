static char sccsid[] = "@(#)95	1.3  src/bos/usr/ccs/lib/libc/POWER/_quitrunc.c, libccnv, bos411, 9428A410j 10/19/93 14:47:23";
/*
 *   COMPONENT_NAME: LIBCCNV
 *
 *   FUNCTIONS: _quitrunc
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

#include <limits.h>		/* UINT_MAX */

#define BIG        (*((double *) (big)))
#define RND_ZERO   (*((double *) (rnd_zero)))
#define UIT_CONST  (*((double *) (uit_const)))

static const long big[]       =  {0x41f00000, 0x0}; /* 2^32 */
static const long rnd_zero[]  =  {0x0, 0x1};
static const long uit_const[] =  {0x43300800,0x0}; /* 2^52 +  2^44 */

/*
 * NAME: _quitrunc
 *
 * FUNCTION:
 *      Convert a 128 bit long double value to unsigned integer by
 *      truncation.
 *
 * EXECUTION ENVIRONMENT:
 *      Problem state library routine.
 *
 * RETURNS:
 *      The converted unsigned integer.
 *      Positive overflow (including INF) yeilds UINT_MAX
 *      Negative numbers and NaN yeild 0x0
 *
 * NOTES:
 *      Can raise VXSNAN (signalling NaN).
 */

unsigned int
_quitrunc(long double arg)
  {
  union {
      double d;
      struct { unsigned int hi, lo; } u;
      } x;
  union {
      long double l;
      struct { double hi, lo; } d;
      } ld;
  double user_fpscr;

  /* this is done with user's fpscr, so will raise signalling NaNs */
  ld.l = arg;
  x.d = ld.d.hi + ld.d.lo;
  
  (x.d > 0.0);
  (x.d < BIG);

  if (x.d > 0.0)
      {
      if (x.d < BIG)
	  {
	  user_fpscr = __setflm(RND_ZERO);
	  x.d += UIT_CONST;
	  (void) __setflm(user_fpscr);
	  return x.u.lo;
	  }
      else
	  {
	  return UINT_MAX;
	  }
      }      
  /* if here, x is less than/equal to 0.0 or NaN */
  return 0x0;
  }
