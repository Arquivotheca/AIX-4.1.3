static char sccsid[] = "@(#)81	1.1  src/bos/usr/ccs/lib/libc/POWER/_qint.c, libccnv, bos411, 9428A410j 12/13/90 20:31:28";
/*
 * COMPONENT_NAME: LIBCCNV
 *
 * FUNCTIONS: _qint, _qintexit
 *
 * ORIGINS: 55
 *
 *                  SOURCE MATERIAL
 *
 * Copyright (c) ISQUARE, Inc. 1990
 */

/*******************************************************************/
/*	      PROGRAM: Quad-Precision Truncate-to-Integer	   */
/*		       (floating point) 			   */
/*	      AUTHOR:  ISQUARE, Inc., (V. Markstein)		   */
/*	      DATE:    8/23/90					   */
/*	      NOTICE:  Copyright (c) ISQUARE, Inc. 1990 	   */
/*	      MODIFIED:10/17/90 customized for RS/6000 performance */
/*								   */
/*	      NOTES:	  The input argument is a conventional	   */
/*			  quad precision number.  In particular,   */
/*			  that means that if the argument is a	   */
/*			  non-zero number, the most significant    */
/*			  part is larger in magnitude than the	   */
/*			  least significant part.  If the number   */
/*			  is a NaN or Infinity, then the most	   */
/*			  significant part is a NaN or Infinity    */
/*			  If the most significant part IS NOT a    */
/*			  NaN or Infinity, then neither is the	   */
/*			  least significant part.		   */
/*								   */
/*			  The argument is passed as two doubles.   */
/*			  To pass a structure instead would impose */
/*			  a hugh overhead.			   */
/*								   */
/*******************************************************************/

extern _qintexit(double a, double b);

/*
 * NAME: _qint
 *                                                                    
 * FUNCTION:  truncate to nearest integer
 *
 * RETURNS:  quad precsion representation of
 *           truncate to integer of argument
 *
 * NOTES:    the fortran compiler sees this routine
 *           as returning a REAL*16 (quad precision)
 *           number.  This requires serious trickery
 *           since the C compiler at this time does
 *           not support long double.   We need to return
 *           the quad precision number in two floating
 *           registers; the high part in FP1 and the
 *           low part in FP2.  This is done using
 *           the _qintexit routine.  Calling _qintexit
 *           just before this routine returns forces
 *           the numbers into the two registers. 
 *           This could break if the compiler actually
 *           schedules the call _qintexit somewhere
 *           other than the last real operation.  Note
 *           that _qintexit must be declared as extern
 *           to force the call, and that it must be scoped
 *           locally in this file so that the compiler
 *           knows that it won't buy a stack frame (and
 *           therefore save & restore fp register).
 */

_qint(double argmsp, double arglsp)
{
  static double 		big=8192.0*8192.0*8192.0*8192.0+1.0;
  static double 		down=8192.0*8192.0*8192.0*8192.0+3.0;
  static double 		up=8192.0*8192.0*8192.0*8192.0+2.0;
  static double 		magic=8192.0*8192.0*8192.0*8192.0;
  static double 		zero=0.0;
  double			a,plus,minus,pluslow,minuslow;
  double			result,oldfpscr;

  oldfpscr=__setflm(big );	  /*force truncate mode 		   */
  plus=argmsp+arglsp;		  /*integer right adjusted in dbleword	   */
  (plus > zero);		  /*induce the compiler to do all compares */
  (__fabs(plus) < magic);	  /*early!				   */
  (__fabs(arglsp) > magic);
  (arglsp > zero);
  if (__fabs(plus) < magic)
    {
     result=(plus>zero)?(plus+magic)-magic:(plus-magic)+magic;
     __setflm(oldfpscr);	  /*canonical path come here: |arg|<2^52   */
     _qintexit(result,0.0);
     return;
    }

  else if ((__fabs(arglsp) < magic ) && (__fabs(plus) >= magic))
    {				  /*|arg| >= 2^52			   */

     if (plus > zero)
       {
       __setflm(down);
	plus=(arglsp > zero)?(arglsp+magic)-magic:(arglsp-magic)+magic ;
	__setflm(oldfpscr);	  /*restore user rounding mode		   */
	_qintexit(argmsp,plus);
	return;
       }
     else
       {
       __setflm(up);
       plus=(arglsp > zero)?(arglsp+magic)-magic:(arglsp-magic)+magic ;
       __setflm(oldfpscr);	  /*restore user rounding mode		  */
	_qintexit(argmsp, plus);
	return;
	}
      }
  else
    {
     __setflm(oldfpscr);
     _qintexit(argmsp+zero,arglsp+zero);
     return;
    }

   }
/*
 * NAME: _qintexit
 *                                                                    
 * FUNCTION: called by _qint to force return values
 *           into floating point registers
 *
 * RETURNS:  nothing
 *
 * NOTES:    see notes for _qint, above
 */

_qintexit(double a, double b)
{return;}
