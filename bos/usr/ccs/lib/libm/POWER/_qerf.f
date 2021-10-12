* @(#)57	1.2  src/bos/usr/ccs/lib/libm/POWER/_qerf.f, libm, bos411, 9428A410j 9/21/93 11:56:57
* COMPONENT_NAME: LIBCCNV
*
* FUNCTIONS: _qerf, _qerfc, erfl, erfcl
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

@process xflag(callby)
*******************************************************************
*	     PROGRAM: Quad-Precision ERF FUNCTION (QERF)	  *
*				     ERFC FUNCTION (QERFC)	  *
*	     AUTHOR:  ISQUARE, Inc (V. Markstein)		  *
*	     DATE:    6/8/90					  *
*	     MOD:     6/27/90 C --> FORTRAN			  *
*		      12/16/90 port --> RS/6000 		  *
*                     1/22/91  Fix bug in parameters to _q_a6;    *
*                              Improve precision                  *
*	     NOTICE:  Copyright (c) ISQUARE, Inc. 1990		  *
*								  *
*		      The argument (dhead, dtail) is		  *
*			  real*16				  *
*								  *
*		      Results are returned by VALUE		  *
*								  *
*******************************************************************
*								  *
*	     CALLING: _setflm, _qexp_x, 			  *
*		      _q_a6, _q_m6, _q_d6			  *
*								  *
*******************************************************************

* NAME: _qerf
*                                                                    
* FUNCTION: quad precision error function
*
* RETURNS: error function of argument

       DOUBLE COMPLEX FUNCTION _QERF (%val(dHead), %val(dTail))
       implicit real*8		(a-h, o-z)
       implicit double complex	(_)
       implicit logical*4	(l)
       real*8			infinity

       data	    unit	/1.0/
       data	    pi		/z'400921FB54442D18'/     !sextuple PI
       data	    pib 	/z'3CA1A62633145C07'/
       data	    pic 	/z'B92F1976B7ED8FBC'/
       data	    toosmall	/z'3c80000000000000'/
       data	    zero	/0.0d0/
       data	    infinity	/z'7ff0000000000000'/
       data	    ca		/z'3FF20DD750429B6D'/     !sextuple of
       data	    cb		/z'3C71AE3A914FED80'/     !2/sqrt(pi)
       data	    cc		/z'B903CBBEBF65F145'/

       double complex		erfl
       double complex		erfcl
       integer			c_function
       real*8			c_head, c_tail

       include "_qerfdata.f"

       c_function = 0			! Fortran name and semantics
 1018  continue

       oldfpscr=_setflm(%val(zero))
       ierf=1					!_qerf entry pt switch
       go to 100

* NAME: _qerfc
*                                                                    
* FUNCTION: quad precision compilmentary error function
*
* RETURNS: compilmentary error function of argument

       entry _QERFC(%val(dHead), %val(dTail))
       c_function = 0			! Fortran name and semantics
 2018  continue
       oldfpscr=_setflm(%val(zero))
       ierf=0					!_qerfc entry pt switch

100    continue 				!common code
       l1=(abs(dhead) <= 2.0)
       if (abs(dhead) <= 0.75) then		!power series expansion
	  temp=(2.0*dhead)*dtail		!series is odd series-
	  argsq=dhead*dhead+temp		!most of expansion is
	  argsq2=dhead*dhead-argsq+temp+	!is in terms of arg^2
     +	       ((2.0*dhead)*dtail-temp)
	  sum=erfpow(1,21)			!sum of series init
	  suml=erfpow(2,21)
	  do i=20,3,-1				!all but the 3 h.o.
	     temp=erfpow(1,i)+sum*argsq 	!terms executed in
	     suml=erfpow(1,i)-temp+sum*argsq+	!q&d quad precision
     +		  (erfpow(2,i)+sum*argsq2+
     +		   suml*argsq)
	     sum=temp
	  end do
	  suminf=0.0d0
	  do i=2,0,-1
	     prodlow=(suml*argsq)+(sum*argsq2)	!mult. by arg^2
	     addend=(suml*argsq-(suml*argsq))+
     +		     (sum*argsq2-(sum*argsq2))+
     +		     ((suml*argsq)-prodlow+(sum*argsq2))+
     +		     suminf*argsq
	     temp=sum*argsq+prodlow		!last 3 terms computed
	     temp2=sum*argsq-temp+prodlow	!with greater care
	     temp3=(sum*argsq-temp)-temp2+prodlow+addend
	     res=erfpow(1,i)+temp
	     reslow=(erfpow(1,i)-res+temp)
	     resmid=erfpow(2,i)+temp2
	     restiny=erfpow(2,i)-resmid+temp2+erfxtr(i)+temp3
	     p=reslow+resmid			!sum of middle terms
	     q=reslow-p+resmid			!possible residual
	     sum=res+p
	     suml=(res-sum+p)+(q+restiny)
	     suminf=(res-sum+p)-suml +(q+restiny)
	  end do

**********************************************************************
*    To complete the job, the series (so far summed in even powers), *
*    must be multiplied by the argument and by 2/PI^.5. 	     *
*    These products are executed in sextuple precision to avoid      *
*    excessive rounding errors. 				     *
**********************************************************************

	  _prod=_q_m6(%val(sum),%val(suml),%val(suminf),
     +		      %val(dhead),%val(dtail),%val(0.0d0),px)
	  _qerf=_q_m6(%val(_prod),%val(px),%val(ca),%val(cb),
     +		      %val(cc),qx)
	  if (ierf .eq. 0) then 		!computing erfc with->
						!erfc(x)=1-erf(x)
	     _qerf=_q_a6(%val(1.0d0),%val(0.0d0),%val(0.0d0),
     +			 %val(-_qerf),%val(-qx),qqx)
	  end if
	  discard=_setflm(%val(oldfpscr))
	  return
       else if (abs(dhead) <= 2.0) then

**********************************************************************
*    For arguments 0.75 < |x| <= 2.0, erfc is computed as:           *
*								     *
*			    erfc(x)= e^(-x^2+series)		     *
*								     *
*	WHERE: "series" is a polynomial approximation devised to     *
*	       make the above formula correct to within 105 bits of  *
*	       precision.  The actual series is in powers of	     *
*	       |x|-center.					     *
**********************************************************************

	  if (abs(dhead) < 1.00) then		!3 subintervals used:
	     center=0.625			!    .75 < x <1.0
	     adjust=0.28125			!   1.0 <= x <1.25
	     times=2.0				!   1.25 <= x <= 2.0
	     ir=8
	     is=18
	     ie=15
	  else if (abs(dhead) < 1.25) then
	     center=0.8125
	     adjust=0.28125
	     times=2.0
	     ir=6
             is=18
	     ie=16
	  else
	     center=1.5
	     adjust=1.375
	     times=3.0
	     ir=7
	     is=23
	     ie=14
	  end if

	  if (dhead < 0.0) then
	     oarg=dhead
	     oarg2=dtail
	     temp=-center-dhead 		!Compute |x|-center
	     arg2=-dtail
	  else
	     temp=dhead-center
	     arg2=dtail
	     oarg=-dhead
	     oarg2=-dtail
	  end if
	  arg=temp+arg2
	  arg2=temp-arg+arg2
	  sum=coeff(1,is,ir)
	  suml=coeff(2,is,ir)
	  do i=is-1,ie,-1			!computed using q&d
	     temp=coeff(1,i,ir)+sum*arg 	!quad precision
	     suml=coeff(1,i,ir)-temp+sum*arg+
     +		 (coeff(2,i,ir)+sum*arg2+
     +		  suml*arg)
	     sum=temp
	  end do
	  suminf=0.0d0				!remaining terms
	  _sum=dcmplx(sum,suml) 		!require full sextuple
	  do i=ie-1,0,-1			!precision arithmetic
	     _prod=_q_m6(%val(_sum),%val(suminf),
     +			 %val(arg),%val(arg2),%val(0.0d0),px)
	     _sum=_q_a6(%val(_prod),%val(px),%val(coeff(1,i,ir)),
     +                  %val(coeff(2,i,ir)),%val(0.0d0),suminf)

	  end do
	  a2=times*oarg2
	  a3=times*oarg2-a2			!series is adjusted by
	  a1=oarg*times 			!adding times*x-adjust
	  ab=oarg*times-a1+a2
	  ac=(oarg*times-a1)-ab+a2+a3
	  _sum=_q_a6(%val(_sum),%val(suminf),
     +		     %val(adjust),%val(0.0d0),%val(0.0d0),as)
	  _sum=_q_a6(%val(_sum),%val(as),
     +		     %val(a1),%val(ab),%val(ac),asx)
	  _res=_qexp_x(%val(_sum),%val(asx),resx,%val(0.0d0))
						!Erfc(|x|) done!
	  if (ierf .eq. 0) then
	     if (dhead > 0.0) then		!For pos args, done!
		_qerf=_res
	     else				!For neg args ->
						!Erfc(-x)=2-Erfc(x)
		_qerf=_q_a6(%val(2.0d0),%val(0.0d0),%val(0.0d0),
     +			    %val(-_res),%val(-resx),qx)
	     end if
	  else					!Caller wanted Erf.
						!Use Erf(x)=1-Erfc(x)
	     if (dhead > 0.0) then
		_qerf=_q_a6(%val(1.0d0),%val(0.0d0),%val(0.0d0),
     +			    %val(-_res),%val(-resx),qx)
	     else
		_qerf=_q_a6(%val(-1.0d0),%val(0.0d0),%val(0.0d0),
     +			    %val(_res),%val(resx),qx)
	     end if
	  end if
	  discard=_setflm(%val(oldfpscr))
	  return
       else

**********************************************************************
*   For all remaining arguments, ERFC(x) is calculated as follows:   *
*								     *
*                      erfc(x)=2/sqrt(pi)*e^(-x^2) / x * cf,         *
*								     *
*      WHERE: cf is a continued fraction that converges in the left  *
*	      half plane.					     *
*								     *
*	       VARIABLE NAME		 MEANING		     *
*		     cf 	   over various intervals, is	     *
*				   approximated by polynomials	     *
*								     *
*		     ir 	   identifies the polynomial	     *
*								     *
*		     is 	   the degree of the polynomial      *
*								     *
*		     ie 	   the lower bound after which	     *
*				   sextuple precision must be used   *
*								     *
*	The approximating polynomial is in powers of (1/x)^2	     *
**********************************************************************

	 _asq=_q_m6(%val(dhead),%val(dtail),%val(0.0d0),       !arg^2
     +		    %val(dhead),%val(dtail),%val(0.0d0),asq)
	 _arg=_q_d6(%val(1.0d0),%val(0.0d0),%val(0.0d0),      !1/arg^2
     +		    %val(_asq),%val(asq),argx)
	 if(dble(_arg) .ge. 0.18d0) then	! .18 <= _arg < .25
	     ir=1				!which polynomial
	     is=22				!How many terms
	     ie=10				!Last term done q&d
	 else if (dble(_arg) .ge. .11d0) then
	     ir=2				! .11 <= _arg < .18
	     is=24
	     ie=10
	 else if (dble(_arg) .ge. .06d0) then
	     ir=3				! .06 <= _arg < .11
	     is=23
	     ie=6
	     if (ierf .ne. 1) ie=11
	 else if (dble(_arg) .ge. .01d0) then
	     ir=4				! .01 <= _arg < .06
	     is=28
	     ie=10
	 else if (dble(_arg) .ge. .00138d0) then
	     ir=5				! .00138 <= _arg < .01
	     is=16
	     ie=4
	     if (ierf .ne. 1) ie=8
	 else
*************************************************************
*    For larger arguments, erfc is so small that it cannot  *
*    be represented, so the value of zero is used for erfc  *
*************************************************************
	     _res=dcmplx(zero, zero)
	     rx=zero
	     if (dhead .eq. dhead) go to 1000
						!For NaN inputs ->NaNQ
	     discard=_setflm(%val(oldfpscr))
	     _qerf=dcmplx(dhead+unit, zero)
	     return
	 end if
	 arg=dble(_arg)
	 arg2=dimag(_arg)
	 if ((ierf.eq.1) .and. (arg < .01)) then  !For erf(x), x>10,
	    _res=dcmplx(zero, zero)		!the result is 1.
	    rx=zero				!(to save time)
	    go to 1000
	 end if
	 sum=coeff(1,is,ir)
	 suml=coeff(2,is,ir)			!q&d quad prec.
	 do i=is-1,ie,-1			!part of polynomial
	    temp=coeff(1,i,ir)+sum*arg		!evaluation
	    suml=coeff(1,i,ir)-temp+sum*arg+
     +		 (coeff(2,i,ir)+sum*arg2+
     +		  suml*arg)
	    sum=temp
	 end do
	 suminf=0.0d0				!sextuple prec eval
	 _sum=dcmplx(sum,suml)			!of remaining terms of
	 do i=ie-1,0,-1 			!the approximating
	    _prod=_q_m6(%val(_sum),%val(suminf),!polynomial
     +			%val(arg),%val(arg2),%val(argx),px)
	    _sum=_q_a6(%val(_prod),%val(px),%val(coeff(1,i,ir)),
     +		       %val(coeff(2,i,ir)),%val(0.0d0),suminf)

	 end do
	 _prod=_q_m6(%val(_sum),%val(suminf),%val(ca),
     +		     %val(cb),%val(cc),px)	!multiply by 2/sqrt(pi)
	 _exp=_qexp_x(%val(-_asq),%val(-asq),exp,%val(0.0d0))
	 _p2=_q_m6(%val(_prod),%val(px),%val(_exp),%val(exp),p2x)
	 _res=_q_d6(%val(_p2),%val(p2x),%val(dhead),%val(dtail),
     +		    %val(0.0d0),rx)
	 _res=0.5*_res
	 rx=0.5*rx
1000	 continue				!Same as before...
	 if (dhead > 0.0) then			!to return ERF or ERFC
	    if (ierf .eq. 1) then		!for pos and neg args
	       _qerf=_q_a6(%val(1.0d0),%val(0.0d0),%val(0.0d0),
     +			   %val(-_res),%val(-rx),qx)
	    else
	       _qerf=_res
	    end if
	 else
	    if (ierf .eq. 1) then
		_qerf=_q_a6(%val(-1.0d0),%val(0.0d0),%val(0.0d0),
     +			    %val(-_res),%val(-rx),qx)
	    else
		_qerf=_q_a6(%val(2.0d0),%val(0.0d0),%val(0.0d0),
     +			    %val(_res),%val(rx),qx)
	    end if
	 end if
	 discard=_setflm(%val(oldfpscr))
	 return
      end if

* Entry point for C-language name and semantics - erf
       entry erfl(%val(c_head), %val(c_tail))
       dhead = c_head
       dtail = c_tail
       c_function = 1
       goto 1018

* Entry point for C-language name and semantics - erfc
       entry erfcl(%val(c_head), %val(c_tail))
       dhead = c_head
       dtail = c_tail
       c_function = 1
       goto 2018

      END
