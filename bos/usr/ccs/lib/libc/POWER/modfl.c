static char sccsid[] = "@(#)98	1.2  src/bos/usr/ccs/lib/libc/POWER/modfl.c, libccnv, bos411, 9428A410j 10/3/93 12:56:40";
/*
 *   COMPONENT_NAME: LIBCCNV
 *
 *   FUNCTIONS: copysignl
 *		modfl
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

#include <fp.h>			/* VALH() */
#include <math.h>		/* HUGE_VAL */
#include <stdlib.h>		/* NULL */

extern long double _qint(long double);

/*
 * NAME: copysignl
 *                                                                    
 * FUNCTION: Copy the sign of double precision value y
 *           to long double precision value x, and return
 *           that value explicity.
 *  
 * NOTE: This function is static (only visible in this
 *       module) and doesn't represent a general implimentation
 *       of "copysignl", which would probably want to be
 *       designed to take both arguments in long double precision.
 *       
 */

static long double
copysignl(long double x, double y)
  {
  union {
      long double ld;
      struct { double hi, lo; } d;
      struct { unsigned int u1, u2, u3, u4; } ui;
      } long_d;
  long double result = x;
  union {
      double d;
      struct { unsigned int hi, lo; } ui;
      } dbl;
  
  long_d.ld = x;
  dbl.d = y;
  
  if (long_d.ui.u1 & 0x80000000)
      {
      if (!(dbl.ui.hi & 0x80000000))
	result = -result;
      }
  else
      {
      if (dbl.ui.hi & 0x80000000)
	result = -result;
      }
  return result;
  }      
	
  
/*
 * NAME: modfl
 *                                                                    
 * FUNCTION: Break a long double value into an integer and
 *           fraction part, each of which has the same sign
 *           as the argument.  The fraction part is returned
 *           explicity, and the integer part is stored into
 *           *iptr.
 *
 *           Boundary conditions are as follows:
 *
 *           Argument    Result     *iptr
 *           --------    ------     -----
 *           NaN         NaN        NaN
 *           +INF        0.0        +INF
 *           -INF        -0.0       -INF
 */

long double
modfl(long double value, long double *iptr)
  {
  double fpscr;
  double double_value;
  long double result;
  long double value_int;
  long double iptr_value;

  /* make sure we won't store through a null pointer */
  if (iptr == NULL)
    iptr = &iptr_value;
  
  double_value = (double) value; /* will need below */

  /* take care of boundary conditions */

  if (double_value != double_value) /* NaN case */
      {
      *iptr = value + 1.0L;	/* quiet the NaN */
      return value + 1.0L;	/* quiet the NaN */
      }
  
  if (double_value == HUGE_VAL)	/* +INF case */
      {
      *iptr = value;
      return 0.0L;
      }

  if (double_value == -HUGE_VAL) /* -INF case */
      {
      *iptr = value;
      return -0.0L;
      }

  /* main path */
  fpscr = __setflm(0.0);	/* save user's FPSCR and set round to nearest */
  value_int = _qint(value);	/* compute integer part */
  *iptr = value_int;		/* store integer part */
  result = value - value_int;   /* must be done in round to nearest */
  if (result == 0.0L)		/* get sign of 0.0 right */
    result = copysignl(result, double_value);
  (void) __setflm(fpscr);	/* restore user's FPSCR */
  return result;
  }
