static char sccsid[] = "@(#)83	1.1  src/bos/usr/ccs/lib/libc/POWER/_qnint.c, libccnv, bos411, 9428A410j 12/13/90 20:32:05";
/*
 * COMPONENT_NAME: LIBCCNV
 *
 * FUNCTIONS: _qnint, _qnintexit
 *
 * ORIGINS: 55
 *
 *                  SOURCE MATERIAL
 *
 * Copyright (c) ISQUARE, Inc. 1990
 */

/*******************************************************************/
/*	      PROGRAM:	Quad-Precision Round-to-Nearest (NINT)	   */
/*			   (floating point)			   */
/*	      AUTHOR:	ISQUARE, Inc., (V. Markstein)		   */
/*	      DATE:	8/23/90 				   */
/*	      NOTICE:	Copyright (c) ISQUARE, Inc. 1990	   */
/*	      MODIFIED: 10/9/90 customized for RS/6000 performance */
/*								   */
/*	      NOTES:	The input argument is a conventional	   */
/*			quad precision number.	In particular,	   */
/*			that means that if the argument is a	   */
/*			non-zero number, the most significant	   */
/*			part is larger in magnitude than the	   */
/*			least significant part.  If the number	   */
/*			is a NaN or Infinity, then the most	   */
/*			significant part is a NaN or Infinity	   */
/*								   */
/*			The argument is passed as two doubles.	   */
/*			To pass a structure instead would impose   */
/*			a hugh overhead 			   */
/*								   */
/*******************************************************************/
typedef struct {double msp,lsp;} quad;
extern _qnintexit(double a,double b);

/*
 * NAME: _qnint
 *                                                                    
 * FUNCTION: round to nearest integer
 *
 * RETURNS: closest integer (as a quad number)
 *
 * NOTES:    the fortran compiler sees this routine
 *           as returning a REAL*16 (quad precision)
 *           number.  This requires serious trickery
 *           since the C compiler at this time does
 *           not support long double.   We need to return
 *           the quad precision number in two floating
 *           registers; the high part in FP1 and the
 *           low part in FP2.  This is done using
 *           the _qnintexit routine.  Calling _qnintexit
 *           just before this routine returns forces
 *           the numbers into the two registers. 
 *           This could break if the compiler actually
 *           schedules the call _qnintexit somewhere
 *           other than the last real operation.  Note
 *           that _qnintexit must be declared as extern
 *           to force the call, and that it must be scoped
 *           locally in this file so that the compiler
 *           knows that it won't buy a stack frame (and
 *           therefore save & restore fp register).
 */

_qnint(double argmsp,double arglsp)
{
  static double 	     big=8192.0*8192.0*8192.0*8192.0 + 1.0;
  static double 	     magic=8192.0*8192.0*8192.0*8192.0;
  static double 	     zero=0.0;
  double		     a,plus,minus,pluslow,minuslow;
  double		     result,oldfpscr;

  oldfpscr=__setflm(big );	    /*force truncate mode		  */
  (argmsp > zero);		    /*induce the compiler to do all	  */
  (__fabs(argmsp) < magic);	    /*compares early			  */
  (arglsp > zero);
  (__fabs(arglsp) < magic);
  plus=argmsp+(arglsp+0.5);	    /*integer right adj in dbleword	  */
  minus= argmsp+(arglsp-0.5);
  if (__fabs(argmsp) < magic)
    {
     result=(argmsp>zero)?(plus+magic)-magic:(minus-magic)+magic;
     __setflm(oldfpscr);
     _qnintexit(result,0.0);
     return;
    }

  else if ((__fabs(arglsp) < magic ) && (__fabs(argmsp) >= magic))
    {
     plus=arglsp+.5;
     minus=arglsp-.5;
     pluslow=(arglsp > zero)?(plus+magic)-magic:(minus-magic)+magic;
     __setflm(oldfpscr);
     _qnintexit(argmsp, pluslow);
     return;
    }
  else
    {
     __setflm(oldfpscr);
     _qnintexit(argmsp+zero,arglsp+zero);
     return;
    }

   }

/*
 * NAME: _qnintexit
 *                                                                    
 * FUNCTION: called by _qnint to force return values
 *           into floating point registers
 *
 * RETURNS:  nothing
 *
 * NOTES:    see notes for _qnint, above
 */

_qnintexit(double a, double b)
{return;}
