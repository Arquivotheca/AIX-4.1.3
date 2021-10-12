static char sccsid[] = "@(#)80	1.1  src/bos/usr/ccs/lib/libc/POWER/_qfloor.c, libccnv, bos411, 9428A410j 12/13/90 20:00:15";
/*
 * COMPONENT_NAME: LIBCCNV
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 55
 *
 *                  SOURCE MATERIAL
 *
 * Copyright (c) ISQUARE, Inc. 1990
 */

/*******************************************************************/
/*	      PROGRAM: Quad-Precision Truncate to Integer	   */
/*		       (floating point) 			   */
/*	      AUTHOR:  ISQUARE, Inc., (V. Markstein)		   */
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
/*		       The argument is passed as two doubles.  To  */
/*		       pass a structure instead would impose a	   */
/*		       hugh overhead.				   */
/*								   */
/*******************************************************************/

typedef struct {double msp, lsp;} quad;

extern void _qfloorexit(double a, double b);

/*
 * NAME: _qfloor
 *                                                                    
 * FUNCTION: return quad precision number containing 
 *           largest integer <= argument
 *
 * RETURNS:  quad precision floor of the argument
 *
 * NOTES:    the fortran compiler sees this routine
 *           as returning a REAL*16 (quad precision)
 *           number.  This requires serious trickery
 *           since the C compiler at this time does
 *           not support long double.   We need to return
 *           the quad precision number in two floating
 *           registers; the high part in FP1 and the
 *           low part in FP2.  This is done using
 *           the _qfloorexit routine.  Calling _qfloorexit
 *           just before this routine returns forces
 *           the numbers into the two registers. 
 *           This could break if the compiler actually
 *           schedules the call _qfloorexit somewhere
 *           other than the last real operation.  Note
 *           that _qfloorexit must be declared as extern
 *           to force the call, and that it must be scoped
 *           locally in this file so that the compiler
 *           knows that it won't buy a stack frame (and
 *           therefore save & restore fp register).
 */

_qfloor(double argmsp, double arglsp)
  {
  static double 	     big=8192.0*8192.0*8192.0*8192.0+3.0;
  static double 	     magic=8192.0*8192.0*8192.0*8192.0;
  static double 	     zero=0.0;
  double		     a,plus,minus,pluslow,minuslow;
  double		     oldfpscr;
  
  oldfpscr=__setflm(big );	     /*force truncate mode	       */
  a= argmsp+arglsp;		     /*trunc value to working precision*/
  (__fabs(argmsp) < magic);	     /*induce the compiler to make all */
  (argmsp > zero);		     /*compares early		       */
  (__fabs(arglsp) < magic);
  (arglsp > zero);
  plus=a+magic-magic;		     /*integer right adj. in dblword   */
  minus= a-magic+magic;
  pluslow=arglsp+magic;
  minuslow=arglsp-magic;
  if (__fabs(argmsp) < magic)
    if (argmsp > zero)
	{
	__setflm(oldfpscr);	     /*restore user's rounding mode    */
	_qfloorexit(plus, 0.0);
	return;
	}
    else
	{
	__setflm(oldfpscr);	     /*restore user's rounding mode    */
	_qfloorexit(minus,0.0);
	return;
	}
  else if ((__fabs(argmsp) >= magic ) && (__fabs(arglsp) < magic))
    
      {
      plus=(arglsp > zero)? pluslow-magic: minuslow+magic;
      __setflm(oldfpscr);	     /*restore user's rounding mode    */
      _qfloorexit(argmsp,plus);
      return;
      }
  
  __setflm(oldfpscr);
  _qfloorexit(argmsp+zero, arglsp+zero);
  return;
  }

/*
 * NAME: _qfloorexit
 *                                                                    
 * FUNCTION: called by _qfloor to force return values
 *           into floating point registers
 *
 * RETURNS:  nothing
 *
 * NOTES:    see notes for _qfloor, above
 */
  
  void _qfloorexit(double a, double b)
  {
  return;
  }

