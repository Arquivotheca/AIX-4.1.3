* @(#)63	1.5  src/bos/usr/ccs/lib/libm/POWER/_qsinh.f, libm, bos411, 9428A410j 10/1/93 17:10:00
* COMPONENT_NAME: LIBCCNV
*
* FUNCTIONS: _qsinh, sinhl
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

* NAME: _qsinh
*                                                                    
* FUNCTION: quad precision hyperbolic sine
*
* RETURNS: hyperbolic sine of argument

@process xflag(callby)
*******************************************************************
*	     PROGRAM: Quad-Precision Hyberbolic SINE		  *
*	     AUTHOR:  ISQUARE, Inc., (V. Markstein)		  *
*	     DATE:    6/02/90					  *
*	     MOD:     9/13/90  customized for RS/6000 performance *
*	     NOTICE:  Copyright (c) ISQUARE, Inc. 1990		  *
*								  *
*		      Results are returned by VALUE		  *
*								  *
*******************************************************************
*								  *
*	     CALLING: _setflm					  *
*******************************************************************

       DOUBLE COMPLEX FUNCTION _QSINH(%val(head), %val(tail))
       implicit real*8		(a-h,o-y)
       implicit double complex	(z,_)
       parameter		(botcut=0.693147)
       parameter		(topcut=39.0)

       real*8                   head, tail
       double complex		sinhl            ! really REAL*16
       external			__set_errno128
       integer			c_function
       integer			ERANGE
       parameter		(ERANGE = 34)

       real*8                   infinity
       data 		        infinity  /z'7ff0000000000000'/
       real*8			coeff(16) /
     +			 0.62132974171578525315635255289d-14,
     +			 0.577836659795680285435407874d-11,
     +			 0.469203367754092391773551193841d-8,
     +			 0.329380764163372859025032938d-5,
     +			 0.1976284584980237154150197628458498d-2,
     +			 1.0d0, 420.0d0, 143640.0d0, 39070080.0d0,
     +			 8204716800.0d0, 1279935820800.0d0,
     +			 140792940288000.0d0, 10137091700736000.0d0,
     +			 425757851430912000.0d0,
     +			 8515157028618240000.0d0,
     +			 51090942171709440000.0d0 /
       real*8			recip /
     +			 0.1957294106339126123084757437350d-19 /
       logical			l1,l2,l3,l4

       c_function = 0			! Fortran name and semantics
 1018  continue
       arg=head
       arg2=tail
       l4=(arg < 0)
       l1=abs(arg) > botcut
       l2=abs(arg) > topcut
       l3=(arg .eq. arg)
       if (abs(arg) > botcut) then	!Good use of exp
	  if (arg < 0) then
	     argx=- arg
	     arg2x=- arg2
	  else
	     argx=arg
	     arg2x=arg2
	  end if
	  zres=_qhexp(%val(argx),%val(arg2x), extraword)  !get .5*e^x
	  if (abs(arg) > topcut) then	!only one evaluation of exp.

*     For C-language semantics, we must set errno to ERANGE
*     in case of overflow
             if (c_function .eq. 1) then
                if ((dble(zres) .eq. infinity) .and.
     x               (abs(arg) .ne. infinity)) then
                   call __set_errno128(%VAL(ERANGE))
                end if
             end if

	     if (arg < 0) zres=-zres
	     _qsinh=zres
	  else
	     t1=dble(zres)		!zres=.5*e^x
	     t2=dimag(zres)		!Def. of sinh: (e^x - e^-x)/2
	     r1=0.25/t1
	     residual=(0.25d0-t1*r1)-t2*r1
	     r2=residual*(4.0d0*r1)	!(r1,r2)=.5*e^-x
	     residual1 =(0.25d0-t1*r1)-residual-t2*r1
	     r3=(t1*r2-residual)	!(reversed sign)
	     r4=(extraword*r1+(t2*r2))	!(reversed sign)
	     reshi=t1-r1
	     reslow=t2-r2
	     residual=(t1-reshi)-r1	!exact
	     if (abs(t2) .gt. abs(r2)) then
		residual2=(t2-reslow)-r2	!exact
	     else
		residual2=t2-(reslow+r2)	!exact
	     end if
	     r5=(r3+r4-residual1)*(4.0d0*r1)
	     resnew=reshi+reslow
	     residual3=reshi-resnew+reslow	!exact
	     residual4=residual+residual3
	     reshi=resnew+residual4
	     residual5=residual-residual4+residual3
	     residual6=resnew -reshi+residual4
	     reslow=(residual2+(extraword+r5))+residual5+residual6
	     if (arg <	0.0) then		!Reverse signs for
		reshi=-reshi			!negative arguments
		reslow=-reslow
		endif
	     _qsinh=dcmplx(reshi, reslow)
	  end if
       else
	  if (arg .ne. arg) then
             _qsinh=dcmplx(head+bigcut,0.0d0)  !return quiet NaN
	  else
	     temp=2.0d0*arg*arg2		!small argument
	     argsq=arg*arg+temp 		!use power series
	     argsq2=arg*arg-argsq+temp+arg2*arg2
	     sum=coeff(1)
	     suml=0.0d0
	     sum=coeff(2)+argsq*sum
	     sum=coeff(3)+argsq*sum
	     sum=coeff(4)+argsq*sum
C	      sum=coeff(5)+argsq*sum
	     do i=5, 15
		templow=suml*argsq+sum*argsq2
		temp=sum*argsq+templow
		bottom=sum*argsq-temp+templow
		sum=coeff(i)+temp
		residual=coeff(i)-sum+temp
		suml=bottom+residual
		end do
C	      zsq=_xlqmul(%val(sum), %val(suml),
c     x 		   %val(argsq), %val(argsq2))
c	      zcb=_xlqmul(%val(zsq),%val(arg),%val(arg2))
c	      zscale=_xlqdiv (%val(zcb),%val(coeff(16)),%val(0.0d0))
c	      _qsinh=_xlqadd(%val(zscale),%val(arg),%val(arg2))
c	      return
	     prodlow=suml*argsq+sum*argsq2	!mult. by arg^2
	     prod=sum*argsq+prodlow
	     prodlow=sum*argsq-prod+prodlow
	     temp2=prodlow*arg+prod*arg2	!mult. by arg
	     temp=prod*arg+temp2
	     temp2=prod*arg-temp+temp2
	     sum=temp*recip			!sum of series ---
	     remainder=(temp-sum*coeff(16))
	     partial=remainder+temp2
	     residual=remainder-partial+temp2
	     suml=partial*recip+(residual*recip)
	     res=arg+sum			!except for argument
	     reslow=(arg-res+sum)		!exact
	     resmid=arg2+suml
	     restiny=arg2-resmid+suml
	     p=reslow+resmid			!sum of middle terms
	     q=reslow-p+resmid			!possible residual
	     reshi=res+p
	     resbot=(res-reshi+p)+(q+restiny)
	     _qsinh=dcmplx(reshi, resbot)
	  end if
       end if
       return

* Entry point for C-language name and semantics
       entry sinhl(%val(head), %val(tail))
       c_function = 1
       goto 1018

       end

