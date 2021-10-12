static char sccsid[] = "@(#)82	1.2  src/bos/usr/ccs/lib/libc/POWER/_qitrunc.c, libccnv, bos411, 9428A410j 12/1/93 12:03:28";
/*
 *   COMPONENT_NAME: libccnv
 *
 *   FUNCTIONS: _qitrunc
 *
 *   ORIGINS: 55,27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) ISQUARE, Inc. 1990
 *
 */

/*******************************************************************/
/*            PROGRAM: Quad-Precision Truncate to Integer          */
/*            AUTHOR:  ISQUARE, Inc., (V. Markstein)               */
/*            DATE:    8/23/90                                     */
/*            NOTICE:  Copyright (c) ISQUARE, Inc. 1990            */
/*                                                                 */
/*            Assuption:  The input argument is a conventional     */
/*                        quad precision number.  In particular,   */
/*                        that means that if the argument is a     */
/*                        non-zero number, the most significant    */
/*                        part is larger in magnitude than the     */
/*                        least significant part.  If the number   */
/*                        is a NaN or Infinity, then the most      */
/*                        significant part is a NaN or Infinity    */
/*                        If the most significant part IS NOT a    */
/*                        NaN or Infinity, then neither is the     */
/*                        least significant part.                  */
/*                                                                 */
/*            The argument is passed as two doubles.  To pass      */
/*                        a structure instead would impose         */
/*                        a hugh overhead                          */
/*                                                                 */
/*******************************************************************/
/*                                                                 */
/*            CALLING: _setflm                                     */
/*                                                                 */
/*******************************************************************/
#undef _ANSI_C_SOURCE
#include <limits.h>

/*
 * NAME: _qitrunc
 *                                                                    
 * FUNCTION: convert a quad precision number to an integers
 *
 * RETURNS: greatest integer less than argument
 *
 */

int _qitrunc(double argmsp, double arglsp)

{
  static double    big = 8192.0 * 8192.0 * 8192.0 * 8192.0 + 1.0;
  static double    magic = 8192.0 * 8192.0 * 8192.0 * 8192.0;
  static double    upper = INT_MAX;
  static double    lower = INT_MIN;
  static double    zero = 0.0;
  double           a, plus, minus;
  double           oldfpscr;
  int              result;

  oldfpscr = __setflm(big );     /*force truncate mode                        */
  a =  argmsp + arglsp;          /*truncate value to working precision*/
  (a > zero);                    /*induce the compiler to do all compares     */
  (a < upper);                   /*early!                                     */
  (a > lower);
  plus = __fabs(a) + magic ;     /*integer right adjusted in doubleword       */
  __setflm(oldfpscr);            /*restore user rounding mode                 */
  if (a >= zero)
     if (a < upper) return *((int *)&plus + 1); else return INT_MAX;
  result = -(*((int *)&plus + 1));  /*fetch result and complement             */
  if (a < zero)
     if (a > lower) return result; else return INT_MIN;
  return INT_MIN;                 /*NaNs wind up here, according to the       */
                                  /*writeup in manual                         */
}
