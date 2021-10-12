#ifndef CEILL
static char sccsid[] = "@(#)03	1.3  src/bos/usr/ccs/lib/libm/POWER/floorl.c, libm, bos411, 9428A410j 10/4/93 12:43:58";
#endif
/*
 * COMPONENT_NAME: LIBM
 *
 * FUNCTIONS: floorl
 *
 * ORIGINS: 55,27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) ISQUARE, Inc. 1990
 */

/*******************************************************************/
/*	      PROGRAM: Quad-Precision Truncate to Integer	   */
/*		       (floating point) 			   */
/*	      AUTHOR:  ISQUARE, Inc.,            		   */
/*	      DATE:    8/23/90					   */
/*	      MODIFIED:9/30/90 customized for RS/6000 performance  */
/*	      NOTICE:  Copyright (c) ISQUARE, Inc. 1990 	   */
/*								   */
/*	      NOTES:   The input argument is a conventional quad   */
/*		       precision number.  In particular, this	   */
/*		       implies that if the argument is a non-zero  */
/*		       number, the most significant part (head) is */
/*		       larger in magnitude than the least signifi- */
/*		       cant part (tail).  If the number is a NaN or*/
/*		       Infinity, then the most significant part is */
/*		       a NaN or Infinity.  If the most significant */
/*		       part IS NOT a NaN or Infinity, then neither */
/*		       is the least significant part.		   */
/*								   */
/*******************************************************************/

/*
 * NAME: floorl, ceill
 *                                                                    
 * FUNCTION: return quad precision number containing 
 *           largest integer <= argument
 *
 * RETURNS:  quad precision floor of the argument
 *
 * NOTES:  Builds ceill if macro CEILL is defined; else
 *         builds floorl.
 */

#define LDEXIT(HI, LO)  \
  __setflm(oldfpscr);	     /*restore user's rounding mode    */   \
  ld.d.hi = (HI);       \
  ld.d.lo = (LO);       \
  return ld.l; 

#ifdef CEILL
/* this sets FPSCR to round to positive infinity mode */
  static double const	     big=8192.0*8192.0*8192.0*8192.0+2.0;
#else /* must be floor */
/* this sets FPSCR to round to minus infinity mode */
  static double const	     big=8192.0*8192.0*8192.0*8192.0+3.0;
#endif /* CEILL */

long double
#ifdef CEILL
ceill(long double ld_arg)
#else /* must be floor */
floorl(long double ld_arg)
#endif /* CEILL */
  {
  static double const 	     magic=8192.0*8192.0*8192.0*8192.0; /* 2^52 */
  static double const	     zero=0.0;
  double                     argmsp, arglsp;
  double		     a,plus,minus,pluslow,minuslow;
  double		     oldfpscr;
  union {
  	long double l;
  	struct { double hi, lo;  } d;
  	} ld;

  oldfpscr=__setflm(big );	     /*force truncate mode	       */
  ld.l = ld_arg;
  argmsp = ld.d.hi;
  arglsp = ld.d.lo;
  a= argmsp+arglsp;		     /*trunc value to working precision*/
  (__fabs(argmsp) < magic);	     /*induce the compiler to make all */
  (argmsp > zero);		     /*compares early		       */
  (__fabs(arglsp) < magic);
  (arglsp > zero);
  (a == 0.0);
  plus=a+magic-magic;		     /*integer right adj. in dblword   */
  minus= a-magic+magic;
  pluslow=arglsp+magic;
  minuslow=arglsp-magic;
  if (__fabs(argmsp) < magic)
    if (a == 0.0)
	{
	LDEXIT(argmsp, arglsp);
	}
    else if (argmsp > zero)
	{
	LDEXIT(plus, 0.0);
	}
    else
	{
	LDEXIT(minus,0.0);
	}
  else if ((__fabs(argmsp) >= magic ) && (__fabs(arglsp) < magic))
      {
      if (arglsp != 0.0)
	  {
	  plus=(arglsp > zero)? pluslow-magic: minuslow+magic;
	  LDEXIT(argmsp,plus);
	  }
      else
	  {
	  LDEXIT(argmsp,arglsp);
	  }  
      }
  
  LDEXIT(argmsp+zero, arglsp+zero);
  return;
  }

