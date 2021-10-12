* @(#)60	1.2  src/bos/usr/ccs/lib/libm/POWER/_qgamma.f, libm, bos411, 9428A410j 9/21/93 11:57:06
* COMPONENT_NAME: LIBCCNV
*
* FUNCTIONS: _qgamma, _qlgamma, lgammal
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

* NAME: _qgamma
*                                                                    
* FUNCTION: compute quad precision gamma function
*
* RETURNS: gamma of argument as a quad precision number
*
* NOTES:  contains an entry point for _qlgamma (log of 
*         gamma) as well.

*******************************************************************
*	     PROGRAM: Quad-Precision GAMMA FUNCTION (QGAMMA)	  *
*				 LOG GAMMA FUNCTION (QLGAMA)	  *
*	     AUTHOR:  ISQUARE, Inc (V. Markstein)		  *
*	     DATE:    6/4/90					  *
*	     MOD:     6/18/90 C --> FORTRAN			  *
*		      11/21/90 port --> RS/6000 		  *
*	     NOTICE:  Copyright (c) ISQUARE, Inc. 1990		  *
*								  *
*		      The argument (dhead, dtail) is		  *
*			  real*16				  *
*								  *
*		      Results are returned by VALUE		  *
*								  *
*******************************************************************
*								  *
*            CALLING: _setflm, _qlogextra, _qexp_x, _qsin,        *
*                     _q_a6, _q_m6, _q_d6, _qnint                 *
*								  *
*******************************************************************

       DOUBLE COMPLEX FUNCTION _QGAMMA (%val(dHead), %val(dTail))
       common			/signgam/signgam
       implicit real*8		(a-h, o-z)
       implicit double complex	(_)
       implicit logical*4	(l)
       integer			signgam
       real*8			infinity

       double complex		lgammal
       real*8			c_head, c_tail

       data	    unit	/1.0/
       data	    pi		/z'400921FB54442D18'/
       data	    pib 	/z'3CA1A62633145C07'/
       data	    pic 	/z'B92F1976B7ED8FBC'/
       data	    toosmall	/z'3c80000000000000'/
       data	    zero	/0.0d0/
       data	    infinity	/z'7ff0000000000000'/
       data         twopwr52    /z'4330000000000000'/
       data         scaleup     /z'4ff0000000000000'/
       data         scaledown   /z'2ff0000000000000'/
       data         tiny        /z'0E00000000000000'/
       data         sclog       /z'40662E42FEFA39EF'/
       data         sclog2      /z'3CFABC9E3B39803F'/
       data         sclog3      /z'3987B57A079A1934'/
       parameter		(NSLOW=38)

       include "_qgammadata.f"

       igamma=1 			!indicates _qgamma entry point
       go to 100

       entry _QLGAMA(%val(dHead), %val(dTail))
       igamma=0 			!indicates _qlgama entry point

100    continue 			!common code
       oldfpscr=_setflm(%val(zero))
       signgam=1
       l1 = (dHead+dTail .ne. dHead+dTail)
       if (dhead <= 0) then		!special for neg arguments
          _int=_qnint(%val(dHead),%val(dTail))  !get integer part of argument
	  if (dHead-dble(_int)+dTail-dimag(_int) .eq. 0) then

***********************************************************************
*    Argument is a non-positive integer.  Gamma is not defined.       *
*    Return a NaN, and signal invalid operation 		      *
***********************************************************************

	     discard=_setflm(%val(oldfpscr))
	     _qgamma=dcmplx(zero/zero,zero)
	     return
	  end if
	  if (dhead > -20.0) then
	     ireflect=0 		!reflection formula not used for
					!modest sized neg numbers
	  else
	     ireflect=1 		!signal use of reflection formula
	     dhead=-dhead		!change sign of argument
	     dtail=-dtail
	  end if
       else				!else, positive arguments --
	  ireflect=0			!signal reflection formula not used
       end if
       if (dhead+dtail .ne. dhead+dtail) then
          _qgamma=dcmplx(dhead+dtail,0.0d0)
          discard=_setflm(%val(oldfpscr))
          return
       end if
       arg=dhead			!working copy of argument
       arg2=dtail
       arg3=0.0
       factor=1.0
       factor2=0.0
       _factor=dcmplx(1.0d0,0.0d0)
       factor3=0.0
       carry=0
       if (arg > 18.0) then		!use asymptotic formula
	  do while (arg < 31.0)
	     _factor=_q_m6(%val(_factor),%val(factor3),
     +			  %val(arg),%val(arg2),%val(arg3),factor3)
	     arg=arg+1.0
	  end do			!argument in range for asympt. formula
	  do while  (arg < 35.0)
	     _factor=_q_m6(%val(_factor),%val(factor3),
     +			  %val(arg),%val(arg2),%val(arg3),factor3)
	     argt=arg+1.0
	     arg2t=(arg-argt+1.0)+arg2
	     arg3=((arg-argt+1.0)-arg2t+ arg2)+arg3
	     arg=argt
	     arg2=arg2t
	  end do
	  _lnarg=_qlogextra(%val(arg),%val(arg2),f3)   !ln x
	  f3=f3+arg3/arg
	  f1=dble(_lnarg)
	  f2=dimag(_lnarg)
	  _prod=_q_m6(%val(f1),%val(f2),%val(f3),%val(arg-0.5),
     +		      %val(arg2),%val(arg3),c)		!(x-.5)ln x
	  _sum=_q_a6(%val(_prod),%val(c),%val(-arg),%val(-arg2),
     +		      %val(-arg3),sextra)		!(x-.5)ln x-x
	  _sum2=_q_a6(%val(_sum),%val(sextra),%val(bump),
     +		      %val(bump2),%val(bump3),sextra2)

********************************************************
*   (_sum2,extra2) represent (x-.5)ln x-x+.5 ln(2 Pi)  *
********************************************************

	  _recip=_q_d6(%val(1.0d0),%val(0.0d0),%val(0.0d0),
     +			%val(arg),%val(arg2),%val(arg3),extra)
					!argument for asymptotic formula
	  _recipsq=_q_m6(%val(_recip),%val(extra),
     +			  %val(_recip),%val(extra),extra2)
	  sum=bern(1,11)
	  suml=bern(2,11)
	  do i=10,1,-1
	     temp=bern(1,i)+sum*dble(_recipsq)
	     suml=bern(1,i)-temp+sum*dble(_recipsq) +
     +		     (bern(2,i)+sum*dimag(_recipsq) +
     +		     suml*dble(_recipsq))
	     sum=temp
	  end do			!finish summation of asymptotic series
	  _prod =_q_m6(%val(sum),%val(suml),%val(0.0d0),
     +		      %val(_recip),%val(extra),extra3)
	  _sum3=_q_a6(%val(_sum2),%val(sextra2),%val(_prod),
     +		      %val(extra3),sextra3)

**********************************************************************
*   At this point, log(gamma*factor) is computed as (sum3,sextra3).  *
**********************************************************************

          if ((igamma .eq. 1).and.(ireflect .ne. 1))  then
                !Gamma entry point (without use of reflection formula)?
	     _usual=_qexp_x(%val(_sum3),%val(sextra3),
     +			   eexp,%val(0.0d0))
	     if (dble(_factor) .eq. 1.0) then
		_qgamma=_usual
		gmextra=eexp
	     else
		_qgamma=_q_d6(%val(_usual),%val(eexp),%val(_factor),
     +			   %val(factor3),gmextra)
	     end if
	     l9=(dble(_qgamma) .eq. dble(_qgamma))
          else                          !Computing log(gamma(x))
	     factor=dble(_factor)
	     if (factor .eq. 1.0) then
		_qgamma=_sum3
		gmextra=sextra3
	     else
		_factorx=_qlogextra(%val(_factor),extrax)
					!computing log gamma.
		extray=-(extrax+factor3/factor)
		_factory=-_factorx
		_qgamma =_q_a6(%val(_sum3),%val(sextra3),%val(_factory),
     +			    %val(extray),gmextra)
	     end if
	  end if
       else				!use formula for interval [0.5,1.5]
	  arg=dhead
	  arg2=dtail			!working copy of argument
	  arg3=0.0
	  increase=0			!signal-> no scaling for result
	  if (arg < 1.5) then		!scale up argument by recursion formula
	     increase=-1		!signal-> result to be divided by factor
	     _factor=dcmplx(arg,arg2)
	     factor3=0.0d0		!factor is the old argument
	     do while (arg < 1.5)
		_arg=_q_a6(%val(arg),%val(arg2),%val(arg3),
     +		      %val(1.0d0),%val(0.0d0),%val(0.0d0),arg3)
		arg=dble(_arg)
		arg2=dimag(_arg)
		if (arg < 1.5) then
		   _factor=_q_m6(%val(_factor),%val(factor3),
     +			%val(arg),%val(arg2),%val(arg3),factor3)
		end if
	     end do
	     if (dble(_factor) .lt. 0.0) then
		signgam=-1		!special case of negative arguments
	     end if
	  else if (arg > 2.5) then
	     increase=+1		!signal-> result must be mult by factor
	     _factor=dcmplx(1.0d0, 0.0d0)
	     factor3=0.0
	     do while (arg > 2.5)
		arg=arg-1.0		!reduce argument by 1
		anew=arg+arg2		!there may be room for bits to
		arg2=arg-anew+arg2	!shift from low order word to
		arg=anew		!high order word
		_factor=_q_m6(%val(_factor),%val(factor3),
     +			       %val(arg),%val(arg2),%val(0.0d0),
     +			       factor3)
	     end do
	     arg3=0.0
	  end if
	  diff=arg-2.0			!series in terms of
	  z=(arg-(diff+2.0))+arg2	!(diff,z,x2)=arg-2
	  z2=(arg-(diff+2.0))-z+arg2+arg3
	  y=diff+z
	  y2=(diff-y+z)+z2
	  y3=(diff-y+z)-y2+z2
	  sum=zeta(1,53)
	  suml=zeta(2,53)
	  do i=52,NSLOW+2,-1
	     sum=zeta(1,i)+sum*y
	  end do
	  sumt=zeta(1,NSLOW+1)+sum*y
	  suml=zeta(1,NSLOW+1)-sumt+sum*y +
     +		   (zeta(2,NSLOW+1)+sum*y2 )
	  sum=sumt
	  do i =NSLOW,3,-1
	     temp=zeta(1,i)+sum*y
	     suml=(zeta(1,i)-temp+sum*y) +
     +		   zeta(2,i)+(sum*y2 +
     +		   suml*y)
	     sum=temp
	  end do
	  _prod=_q_m6(%val(sum),%val(suml),%val(0.0d0),
     +		     %val(y),%val(y2),%val(y3), pextra2)
	  _sum=_q_a6(%val(_prod),%val(pextra2),%val(zeta(1,2)),
     +		     %val(zeta(2,2)),%val(zeta32),sextra)
	  _prod= _q_m6(%val(_sum),%val(sextra),
     +		    %val(y),%val(y2),%val(y3), pextra1)  !multiply sum by z
	  _sum= _q_a6(%val(_prod),%val(pextra1),%val(ec),
     +		     %val(ec2),%val(ec3),sextra)	 !add linear term
	  _prod=_q_m6(%val(_sum), %val(sextra),
     +		    %val(y),%val(y2),%val(y3), pextra)	 !final mult. by z.
	  if (igamma .eq. 1) then			 !a Gamma entry
	     _value=_qexp_x(%val(_prod),%val(pextra),
     +			   eexp,%val(0.0d0))
	     if (increase .eq. 1) then
		_qgamma=_q_m6(%val(_value),%val(eexp),
     +			   %val(_factor),%val(factor3),gmextra)
	     else if (increase .eq. -1) then
		_qgamma=_q_d6(%val(_value),%val(eexp),%val(_factor),
     +			   %val(factor3),gmextra)
	     else
		_qgamma=_value
		gmextra=eexp
	     end if
	  else				!entry was for log gamma
	     if (increase .eq. 0) then
		_qgamma=_prod
		gmextra=pextra
	     else
		if (signgam .lt. 0) then
		   _factor=-_factor
		   factor3=-factor3
		end if
		factor=dble(_factor)
		_factor=_qlogextra(%val(_factor),extra) !computing log gamma.
		extra=extra+factor3/factor
		if (increase .eq. -1) then		!change sign of log
		   _factor=dcmplx(-dble(_factor),-dimag(_factor))
		   extra=-extra
		end if
		_qgamma=_q_a6(%val(_prod),%val(pextra),
     +		       %val(_factor),%val(extra),gmextra)
	     end if
	     l9=(dble(_qgamma) .ne. dble(_qgamma))
	  end if
       end if
       if (dble(_qgamma) .ne. dble(_qgamma)) then
	  _qgamma=(infinity, zero)
	  gmextra=0.0
	  end if
       if (ireflect .eq. 1) then	!original argument less than 0.0
	  arg=-dHead			!recover original argument
	  arg2=-dTail
                                        !reduce argument
          _diff=_q_a6(%val(arg),%val(arg2),%val(0.0d0),%val(-_int),
     +                 %val(0.0d0),dextra)  !for computation
          asize = abs(dble(_diff))
          if (asize .lt. tiny) then   !For arguments very close
             _diff=dcmplx(scaleup*dble(_diff),   !to an integer, rescale,
     +                    scaleup*dimag(_diff))
             dextra=dextra * scaleup            !so that sin can be computed
                                                !to a full 107+ bits
         end if
	  _sarg=_q_m6(%val(_diff),%val(dextra),%val(pi),	!of sin (Pi x)
     +		       %val(pib),%val(pic),saextra)
          _hint = 0.5*_int                  !test for evenness of integer by
          _xhint = 2.0*_qnint(%val(_hint))  !which argument was reduced
	  _sinarg=_qsin(%val(_sarg))				!sin of argument
          if (dble(_xhint) - dble(_int) +dimag(_xhint) - dimag(_int)
     +           .ne. 0.0d0) _sinarg = -_sinarg
	     if (dble(_sinarg) .lt. 0.0) then
		_sinarg=-_sinarg	!force sin(pi x) to have plus sign
		signgam=-signgam	!show gamma(x) has negative sign
	     end if
          if (abs(dble(_qgamma)).eq.infinity)then   !result will underflow
             if(igamma .eq. 1) then                 
                  _qgamma = dcmplx(zero,zero)
             else
                  _qgamma = dcmplx(-infinity, zero) ! gamma underflows, so
                                                    ! lgamma overflows to -INF
             end if
             discard=_setflm(%val(oldfpscr))
             return
          end if
	     _prod=_q_m6(%val(dHead),%val(dTail),%val(0.0d0),
     +		      %val(_sinarg),%val(0.0d0),pextra) ! x sin(pi x)
	     _log=_qlogextra(%val(_prod),extralg)	!log (x sin(pi x))
	     extralg=extralg+pextra/dble(_prod)
	     _denom= _q_a6(%val(_log),%val(extralg),
     +			%val(_qgamma),%val(gmextra),dextra)
	     _denom=-_denom
	     dextra=-dextra
	     _qgamma=_q_a6(%val(piln),%val(pilnb),%val(pilnc),
     +			 %val(_denom),%val(dextra),gmextra)
          if (asize .lt. tiny) then
             !Compensate for scaling of argument to sin(pi x)
             _qgamma=_q_a6(%val(_qgamma),%val(gmextra),%val(sclog),
     +                     %val(sclog2),%val(sclog3),gmextra)
          end if
          if (igamma .eq. 1) then    !we really want gamma itself ?
             _value=_qexp_x(%val(_qgamma),%val(gmextra),
     +                  eexp,%val(0.0d0))
             if (signgam .eq. 1) then
                _qgamma = _value
             else
                _qgamma = -_value
             end if
          end if
       end if
       discard=_setflm (%val(oldfpscr))
       return

* Entry point for C-language name and semantics
       entry lgammal(%val(dHead), %val(dTail))
       igamma=0 			!indicates _qlgama entry point
       goto 100

       end
