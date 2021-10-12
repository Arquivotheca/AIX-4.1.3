* @(#)60	1.2  src/bos/usr/ccs/lib/libm/POWER/_qlog.f, libm, bos411, 9428A410j 9/21/93 11:57:10
* COMPONENT_NAME: LIBCCNV
*
* FUNCTIONS: _qlog, _qlog10, _qlogxtra, logl, log10l
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

* NAME: _qlog
*                                                                    
* FUNCTION: quad precision natural logarithm
*
* RETURNS: natural logarithm of argument

@process xflag(callby)
*******************************************************************
*            PROGRAM: QUAD-PRECISION LOGARITHM                    *
*            AUTHOR:  ISQUARE, Inc., (V. Markstein)               *
*            DATE:    4/3/90                                      *
*            MODIFIED:9/24/90 customized for RS/6000 performance  *
*            NOTICE:  Copyright (c) ISQUARE, Inc. 1990            *
*                     The argument (alpha,beta) is DOUBLE COMPLEX *
*                         or REAL*16 (when available)             *
*                                                                 *
*                     Results are returned by VALUE               *
*                                                                 *
*******************************************************************
*                                                                 *
*            CALLING: _setflm                                     *
*            NOTE:    real&complex arguments not combined due to  *
*                     FORTRAN bug.                                *
*******************************************************************

       DOUBLE COMPLEX FUNCTION _QLOG(%val(alpha),%val(beta))
       implicit real*8          (a-h,o-y)
       implicit double complex  (Z,_)
       real*8                   _setflm
       real*8                   alpha,beta,extraword
       real*8                   logtbl(4,0:63)
       real*8                   test,big,reciplog2,log2,one,
     +                          log2b,log2c,log2d
       real*8                   LXIV,zero
       real*4                   coeff(16)
       integer*4                highorder(2),em(2),nscale(2)
       integer*4                dscale(2)
       logical*4                l1,l2,l3
       equivalence              (test,highorder(1)),(en,em(1)),
     +                          (rscale,dscale(1)),
     +                          (scale,nscale(1)),(power,newpower)
       real*8                   ln10hi,ln10mid,ln10low
*
       data     log2            /z'3FE62E42FEFA39EF'/
       data     log2b           /z'3C7ABC9E3B39803F'/
       data     log2c           /z'3907B57A079A1934'/
       data     log2d           /z'B5AACE93A4EBE5D1'/
       data     reciplog2       /z'3FF71547652B82FE'/
       data     big             /z'4338000000000000'/
       data     one             /1.0d0/
       data     two             /2.0d0/
       data     bias            /1023.0d0/
       data     half            /0.5/
       data     issmall         /z'07000000'/
       data     maxexp          /z'7ff00000'/
       data     leadfracbit     /z'00080000'/
       data     left6frac       /z'000fc000'/
       data     recip15fac      /0.138750138750138750138750d-05/ !1/15!
       data     scaleup         /z'5ff0000000000000'/            !2^512
       data     xscale          /512.0d0/
       data     zero            /0.0d0/
       data     ln10hi          /z'3FDBCB7B1526E50E'/
       data     ln10mid         /z'3C695355BAAAFAD3'/
       data     ln10low         /z'38FEE191F71A3015'/

       double complex		logl
       double complex		log10l
       external			__set_errno128
       integer			c_function
       integer			EDOM
       parameter		(EDOM = 33)
       real*8			c_head, c_tail

       INCLUDE "_qlogdata.f"
       
       c_function = 0			! Fortran name and semantics
 1018  continue

       logtype=0                             !tag main entry point
1      CONTINUE
       test=alpha
       tail=beta
       oldfpscr=_setflm(%val(one))           !round to nearest, disable traps
       rscale=0.0d0
       scale=big
       if (highorder(1) < issmall) go to 100
       iexp=and(highorder(1),maxexp)         !isolate exponent
       dscale(1)=2046*2**20-iexp             !to rescale number
       nscale(2)=ishft(iexp,-20)
50     ndx=ishft(and(highorder(1),left6frac),-14)
       l1=(ndx .eq. 0)
       l2=(and(highorder(1),leadfracbit) .ne. 0)
       if (dscale(1) .gt. 0) then
          head=test*rscale                   !This forces 1<= arg <= 2
          tail=tail*rscale                   !Usual scale arguments
       else if (dscale(1) .eq. 0) then       !maximum exp. for args
          dscale(1)=1*2**19                  !construct reciprocal of
          head=test*rscale                   !2^1023 and scale args
          tail=tail*rscale
       else                                  !out of range positive args
          discard=_setflm(%val(oldfpscr))    !restore original fp status
          _qlog=dcmplx(test*one, zero)       !do arith. to make NaN quiet
          return
       end if
       scale=scale-big-bias                  !force .75 <= arg <= 1.5
       if (and(highorder(1),leadfracbit).ne. 0) then
          scale=scale+one
          head=head*half
          tail=tail*half
       end if
       if (ndx .eq. 0) then
          if (and(highorder(1),z'000fffff').lt.z'00002000') ndx=63
       end if
       redarg=head-logtbl(1,ndx)             !reduce argument
       diff=redarg+tail                      !(arg,arglow) =
       diff2=redarg-diff+tail                !((alpha,beta)-logtbl(1,ndx))/
       quot=diff*logtbl(2,ndx)               !logtbl(1,ndx)
****************************************************
*    NOTE: logtbl(2,ndx)=1/logtbl(1,ndx) approx.   *
****************************************************
       arg=quot+(diff-quot*logtbl(1,ndx))*logtbl(2,ndx)            !h.o arg.
       rem=((diff-arg*logtbl(1,ndx))+diff2)
       reciparg=one-arg                      !start computation of 1/(1+arg)
       l1=(abs(diff-arg*logtbl(1,ndx)) .gt. abs(diff2)) !<compiler directive>
       e=arg*arg
       sum=coeff(15)+arg*coeff(16)
       arglow=rem*logtbl(2,ndx)
       u=scale*log2c                         !begin computing
       x=scale*log2b                         !required multiple
       y=scale*log2b-x                       !of ln 2
       if (abs(diff-arg*logtbl(1,ndx)) .gt. abs(diff2)) then
          remlost=(diff-arg*logtbl(1,ndx))-rem+diff2
       else
          remlost=diff2-rem+(diff-arg*logtbl(1,ndx))
       endif
       arglost=(remlost+(rem-arglow*logtbl(1,ndx)))*logtbl(2,ndx)
*
       sum=coeff(14)+arg*sum                 !begin series evaluation
       reciparg=reciparg+ reciparg*e         !16 bit approx.
       sum=coeff(13)+arg*sum                 !use first 8 terms
       e=e*e
       sum=coeff(12)+arg*sum                 !in double precision
       reciparg=reciparg+reciparg*e          !32 bit approx.
       sum=coeff(11)+arg*sum
       e=e*e
       sum=coeff(10)+arg*sum
       reciparg=reciparg+reciparg*e          !good enough approxiamation
       temp=coeff(9)+arg*sum
       suml=coeff(9)-temp+arg*sum
       sum=temp
       argl=arglow*reciparg                  !tail of argument.
       argl2=((arglow-argl-argl*arg)+arglost)*reciparg
       do i=8, 3, -1
          hi=coeff(i)+sum*arg
          suml=coeff(i)-hi+sum*arg+suml*arg
          sum=hi
       end do
       prod=sum*recip15fac                          !divide by 15! to
       c=u+y                                 !produce scale*ln 2
       enlog2hi=scale*log2
       w=scale*log2-enlog2hi
       enlog2mid=x+w
       l3=(abs(x) .gt. abs(w))               !<compiler directive>
       prodl=(sum-prod*coeff(1)+suml)*recip15fac
C       if (logtype .eq. 2) go to 3000        !special call?
       sum2=prod*arg-half                    !quadratic term is used
       suml=(-one)*half-sum2+prod*arg+prodl*arg
       sum3=sum2*arg+(suml*arg)              !(sum,suml)=ln(arg)-arg
       suml=sum2*arg-sum3+(suml*arg)
       sum=sum3*arg+(suml*arg)               !(sum,suml)=ln(arg)-arg
       suml=sum3*arg-sum+(suml*arg)
       if (scale .eq. zero)  go to 200       !special case:Nln2=0
       if (abs(x) .gt. abs(w)) then
          enlog2low=x-enlog2mid+w+c
       else
          enlog2low=w-enlog2mid+x+c
       end if
       small=argl+suml
       petit=logtbl(4,ndx)+enlog2low +
     +       argl2+small                     !sum of least significant terms
       smallres=argl-small+suml
       summid=sum+enlog2mid                  !middle terms
       if (abs(sum) .gt. abs(enlog2mid)) then
          resmid=sum-summid+enlog2mid        !catch the low order bits
       else                                  !else clause: rare event
          resmid=enlog2mid-summid+sum
       end if
       summid1=arg+summid
       resmid1=arg-summid1+summid
       summid2=summid1+logtbl(3,ndx)
       resmid2=logtbl(3,ndx)-summid2+summid1+resmid1
       petit=petit+resmid+smallres
       top=enlog2hi+summid2
       resmid3=enlog2hi-top+summid2
       resmid4=resmid3+resmid2
       bottom=petit+resmid4
       resmid5=resmid3-resmid4+resmid2
       residual=resmid4-bottom+petit+resmid5
       result=top+bottom
       if (logtype .ne. 0) go to 1000
       discard=_setflm(%val(oldfpscr))
       resultlow=top-result+bottom+residual
       _qlog=dcmplx(result, resultlow)
       return
100    if (test+tail .eq. zero) then         !argument=0
         discard=_setflm(%val(oldfpscr))     !return -> -Infinity
         _qlog=dcmplx (-one/zero, zero)
         return
       else if (test+tail .gt. zero) then    !petit argument
           test=test*scaleup
           tail=tail*scaleup
           head=test+tail
           tail=test-head+tail
           iexp=and(highorder(1), maxexp)    !isolate exponent
           dscale(1)=2046*2**20-iexp         !to rescale number
           nscale(2)=ishft(iexp,-20)         !true power of two
           scale=scale-xscale
           go to 50
       else if (test .lt. zero) then         !argument<0 -> invalid

* For C-language semantics, set errno = EDOM for negative argument
          if (c_function .eq. 1) then
             call __set_errno128(%VAL(EDOM))
          end if
          
           discard=_setflm(%val(oldfpscr))
           _qlog=dcmplx(zero/zero, zero)
           return
       else                                  !Must be a NaN
         discard=_setflm(%val(oldfpscr))
         _qlog=dcmplx(test*zero, zero)       !Make NaN quiet
         return
       end if
200    top=arg+sum
       resmid=arg-top+sum
       small=suml+argl
       smallres=argl-small+suml
       if (logtbl(1,ndx) .eq. one) then      !accurate table yields zero
          bottom=resmid+small
          residual2=smallres+argl2
          residual=resmid-bottom+small+residual2
          result=top+bottom
          if (logtype .ne. 0) go to 1000
          discard=_setflm(%val(oldfpscr))
          resultlow=top-result+bottom+residual
          _qlog=dcmplx(result, resultlow)
          return
       end if
       hi=logtbl(3,ndx)+top
       resmid2=logtbl(3,ndx)-hi+top+resmid
       petit=small
       bottom=resmid2+petit
       if (abs(resmid2) .gt. abs(petit)) then
         residual=resmid2-bottom+petit+smallres+argl2+logtbl(4,ndx)
       else
         residual=petit-bottom+resmid2+smallres+argl2+logtbl(4,ndx)
       end if
       top=hi
       result=top+bottom
       if (logtype .ne. 0) go to 1000
       discard=_setflm(%val(oldfpscr))
       resultlow=top-result+bottom+residual
       _qlog=dcmplx(result, resultlow)
       return

* NAME: _qlog10
*                                                                    
* FUNCTION: quad precision base 10 logarithm
*
* RETURNS: base 10 logarithm of argument

****************************
* ENTRY POINT for _QLOG10  *
****************************
       entry _qlog10(%val(alpha),%val(beta))

       c_function = 0
 2018  continue

       logtype=1                             !tag log10 entry point
       go to 1

* NAME: _qlogxtra
*                                                                    
* FUNCTION: entry point for use by _qpow

*****************************
* ENTRY POINT for _QLOGXTRA *
*****************************

*      Special entry point for use by POW
*      This routine does not piece together the various addends
*      that constitute log.  Instead, the components are packed
*      into the array RESULTS and shipped back to the caller
*      for further operations

       entry _qlogextra(%val(alpha),%val(beta),extra)
       logtype=2                             !tag logextra entry point
       go to 1


1000   CONTINUE                              !transfer pt. to finish up
                                             !log for non-standard calls
       resmidtemp=top-result+bottom
       resmid=resmidtemp+residual            !middle term of result
       resbot=resmidtemp-resmid+residual     !low order part of result

       if (logtype .eq. 2) then              !call by power?
          _qlog=dcmplx(result,resmid)
          extra=resbot
          discard=_setflm(%val(oldfpscr))
          return
       end if

********************************************************************
* A very careful multiplication of the above result is required -> *
*      (result, resmid, resbot) * log10(e) as follows:             *
********************************************************************
       u=result*ln10low+resmid*ln10mid+resbot*ln10hi
       v=result*ln10mid                      !reference ISQUARE's _q_mp10.c
       w=result*ln10mid-v                    !A less fanatic approach of
       x=resmid*ln10hi                       !getting the last bits right,
       y=resmid*ln10hi-x                     !permits the use of the usual
       c=w+y+u                               !10 cycle multiplication idiom,
       top=result*ln10hi                     !BUT the cost would be 2 ulps
       w=result*ln10hi-top                   !of precision. We have chosen
       u=v+x                                 !to be fanatic.
       l1 = (abs(w) .gt. abs(u))             !compiler scheduling directive
       if (abs(x) .gt. abs(v)) then
          c=x-u+v+c
       else
          c=v-u+x+c
       end if
       bottom=u+w
       if (abs(w) .gt. abs(u)) then
          c=w-bottom+u+c
       else
          c=u-bottom+w+c
       end if
       result=top+bottom
       discard=_setflm(%val(oldfpscr))
       resultlow=top-result+bottom+c
       _qlog=dcmplx(result, resultlow)
       return

* Entry point for C-language name and semantics
       entry logl(%val(c_head), %val(c_tail))
       alpha = c_head
       beta = c_tail
       c_function = 1
       goto 1018

* Entry point for C-language name and semantics
       entry log10l(%val(c_head), %val(c_tail))
       alpha = c_head
       beta = c_tail
       c_function = 1
       goto 2018

       END
