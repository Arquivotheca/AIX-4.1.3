* @(#)57	1.3  src/bos/usr/ccs/lib/libm/POWER/_qatan.f, libm, bos411, 9428A410j 9/21/93 11:56:52
* COMPONENT_NAME: LIBCCNV
*
* FUNCTIONS: _qatan, _qatan2, atanl, atan2l
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

* NAME: _qatan
*                                                                    
* FUNCTION: quad precision inverse tanget function
*
* RETURNS: inverse tanget of argument

@process xflag(callby)
*******************************************************************
*            PROGRAM: Quad-Precision INVERSE TANGENT (QATAN)      *
*            AUTHOR:  ISQUARE, Inc (V. Markstein)                 *
*            DATE:    4/4/90                                      *
*            MOD:     5/1/90 C --> FORTRAN                        *
*                     11/12/90 port --> RS/6000                   *
*            NOTICE:  Copyright (c) ISQUARE, Inc. 1990            *
*                                                                 *
*                     The argument (dhead, dtail) is              *
*                         real*16                                 *
*                                                                 *
*                     Results are returned by VALUE               *
*                                                                 *
* *****************************************************************
*                                                                 *
*            CALLING: _setflm                                     *
*                                                                 *
* *****************************************************************
*
       DOUBLE COMPLEX FUNCTION _QATAN (%val(dHead), %val(dTail))
       implicit real*8          (a-h, o-z)
       implicit double complex  (_)
       real*8                   infinity
       real*8       upper_limit /z'4690000000000000'/
       real*8       LXIV        /64.0d0/
       integer*4                N(2),ndx(2)
       logical*4                l1,l2,l3,l4,l5,l6,l7,l8,l9,l10

       double complex		atanl
       double complex		atan2l
       external			__set_errno128
       integer			c_function
       integer			EDOM
       parameter		(EDOM = 33)
       real*8			c_head, c_tail
       real*8			c_denh, c_denl

       data         small       /0.0859375/
       data         unit        /1.0/
       data         big         /z'4338000000000000'/
       data         recip       /6.869637070016756334588131d-12/  !2/Pi
       data         pih         /z'3FF921FB54442D18'/    !Pi/2 h.o. word
       data         pihb        /z'3C91A62633145C07'/    !Pi/2 l.o.word rounded
       data         pihbm       /z'3C91A62633145C06'/    !Pi/2 l.o.word chopped
       data         pihc        /z'B91F1976B7ED8FBC'/    !Pi/2 3rd order word
       data         infinity    /z'7ff0000000000000'/
       data         pi          /z'400921FB54442D18'/    !Pi h.o.word
       data         pib         /z'3CA1A62633145C07'/    !Pi l.o.word rounded
       data         pic         /z'B92F1976B7ED8FBC'/    !Pi/2 3rd order word
       data         pibm        /z'3CA1A62633145C06'/    !Pi l.o.word chopped
       data         pi34        /z'4002D97C7F3321D2'/    !3Pi/4 h.o. word
       data         pi34b       /z'3C9A79394C9E8A0A'/    !3Pi/4 l.o. word
       data         zero        /0.0d0/                  !zero
       data         Tiny        /z'06b0000000000000'/    !Tiny number that
                                                         !still allows a 107
                                                         !bit quad to consist
                                                         !of two normal numbers
       data         Rescale     /z'4A00000000000000'/    !Needed to rescale
                                                         !if the denominator
                                                         !is "tiny"
****************************************************************************
*    Multiplier -> give the number of table values between the consecutive *
*                  integers 1 to 12                                        *
****************************************************************************

       real*4 factor(12) /32,16,8,4,4,2,2,1,1,1,1,1/

****************************************************************************
*    Adjust     -> index into accurate atan table, for integer arguments   *
*                  from 1 to 12.                                           *
****************************************************************************

       real*4 adjust(12) /32,64,88,104,104,116,116,124,124,124,124,124/

****************************************************************************
*         atancoeff: coefficients of atan approximating polynomial,        *
*                    multiplied by 145568097675.0                          *
****************************************************************************

       integer                  rflag
       real*8 atancoeff(15)     /145568097675.0d0, -48522699225.0d0,
     +         29113619535.0d0, -20795442525.0d0,   16174233075.0d0,
     +        -13233463425.0d0,  11197545975.0d0,  -9704539845.0d0,
     +         8562829275.0d0,  -7661478825.0d0,    6931814175.0d0,
     +        -6329047725.0d0,   5822723907.0d0,   -5391411025.0d0,
     +         5019589575.0d0 /
       INCLUDE "_qatandata.f"
       PARAMETER                (RPIH=0.6366197723675814D0)
       equivalence              (eN,ndx)
       equivalence              (ndx(2),index)

****************************************************************************
*      This routine breaks the arguments into five ranges:                 *
*      Range 1:          0 <= |x| <= .0859375                              *
*      Range 2:   .0859375 <= |x| <= 1.015625                              *
*      Range 3:   1.015625 <= |x| < 13                                     *
*      Range 4:         13 <= |x| < 2^107                                  *
*      Range 5:      2^107 <= |x| <= Infinity                              *
*                                                                          *
*         >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>         *
*                                                                          *
* In Range 1, a 15th degree polynomial in x^2 is used to evaluate QATAN.   *
*                                                                          *
* In Range 2  and,                                                         *
* In Range 3  the accurate table method is used to range-reduce the        *
*             argument to a small value to be used with an interpolating   *
*             polynomial for QATAN.  In Ranges 2 and 3, the resultant      *
*             argument is less than 2^-7, so an 8th degree polynomial can  *
*             be used.  Range 3 requires additional work to find the       *
*             correct index into the accurate table.                       *
*                                                                          *
* In Range 4, QATAN(x) = Pi/2 - QATAN(1/x).  After taking the reciprocal   *
*             of the argument, the quotient is less than 1/13, so that the *
*             15th degree polynomial used for Range 1 is used to compute   *
*             QATAN(1/x)                                                   *
*                                                                          *
* In Range 5, the correctly rounded value differs from Pi/2 by less than   *
*             1/2 ulp.  It is, however, incorrect to let the value become  *
*             correctly rounded, as that correctly rounded value exceeds   *
*             PI/2 and is in the wrong quadrant.  Applying the inverse     *
*             function, QTAN, to the correctly rounded value of PI/2 would *
*             produce a result of the opposite sign to the argument        *
*             originally given to ATAN.                                    *
****************************************************************************

       c_function = 0			! Fortran name and semantics
 1018  continue

       oldfpscr=_setflm(%val(big))
       iatan2flag=0
50     CONTINUE                                 !entry from ATAN2 here
       l2=(abs(dHead) .lt. upper_limit)
       l3=(abs(dHead) .lt. small)
       rflag=0                                  !Indicate normal size argument
       l4=(dhead < 0.0)
       l5=(abs(dHead) > 1.015625)
       if (abs(dHead) .lt. upper_limit) then    !Filter out unusual arguments
         if (abs(dHead) .lt. small) go to 100   !small argument -> easy
         eN=LXIV*abs(dHead)+big
         if (abs(dHead) > 1.015625) then        !for large args, use recip.
            l2=(abs(dHead) < 13.0)
            discard=_setflm(%val(big+1.0))      !use trunc mode momentarily
            en=abs(dHead)+big                   !determine range of argument
            discard=_setflm(%val(big))          !return to round-to-nearest
            i=index
            if (abs(dHead) >= 13.0) go to 90
            en=factor(i)*abs(dHead)+big+adjust(i)
         end if
         if (dHead < 0) then                    !use abs. value
            arg=-dHead
            argl=-dTail
         else
            arg=dHead
            argl=dTail
         end if
         l6=(abs(arg*atantbl(1,index)) > 2.0**32)
         top=arg-atantbl(1,index)
         top2=top+argl
         topextra=arg-top-atantbl(1,index)
         top3=top-top2+argl+topextra            !reduced numerator
         bot=1.0+arg*atantbl(1,index)
         bot2=1.0-bot+arg*atantbl(1,index)+
     +        argl*atantbl(1,index)             !denominator
         r=1.0/bot
         quot=r*top2
         quot=quot+(top2-bot*quot)*r

         p29=top2-quot*bot
         p30=quot*bot2                          !negative term
         p33=p29-p30
         p35=p33+top3
         quot2=p35*r
         quot2=quot2+(p35-quot2*bot)*r          !second fraction wd.
         temp=2.0*quot*quot2                    !reduced arg is (quot,quot2)
         argsq=quot*quot+temp                   !compute square of argument
         argsq2=quot*quot-argsq+temp+quot2*quot2
         sumt=atancoeff(7)+argsq*atancoeff(8)
         sum=atancoeff(6)+argsq*sumt
         suml=atancoeff(6)-sum+argsq*sumt
         do i=5, 2,-1
            temps=atancoeff(i)+sum*argsq
            suml=atancoeff(i)-temps+sum*argsq+
     +           (sum*argsq2+suml*argsq)
            sum=temps
         end do
         prodlow=suml*argsq+sum*argsq2          !mult. by arg^2
         prod=sum*argsq+prodlow
         prodlow=sum*argsq-prod+prodlow
         temp2=prodlow*quot+prod*quot2          !mult. by arg
         temp=prod*quot+temp2
         temp2=prod*quot-temp+temp2
         sum=temp*recip                         !sum of series ---
         remainder=(temp-sum*atancoeff(1))
         partial=remainder+temp2
         residual=remainder-partial+temp2
         suml=partial*recip+(residual*recip)
         res=quot+sum                           !except for argument
         reslow=(quot-res+sum)                  !exact
         resmid=quot2+suml
         restiny=quot2-resmid+suml
         p=reslow+resmid                        !sum of middle terms
         q=reslow-p+resmid                      !possible residual
         reshi=res+p
         resbot=(res-reshi+p)+(q+restiny)
         result=atantbl(2,index)+reshi
         reslow=atantbl(3,index)+resbot
         resmid=atantbl(2,index)-result+reshi
         l1=(abs(resmid) > abs(reslow))
         restiny=resbot-reslow+atantbl(3,index)
         restemp=resmid+reslow
         reshi=result+restemp
         l8=(iatan2flag .eq. 1)
         if (abs(resmid) > abs(reslow)) then
            restiny=resmid-restemp+reslow+restiny
         else
            restiny=reslow-restemp+resmid+restiny
         end if
         resbot=result-reshi+restemp+restiny
         if (dHead < 0.0) then
            reshi=-reshi
            resbot=-resbot
            result=-result
            restemp=-restemp
            restiny=-restiny
         end if
         if (iatan2flag .eq. 1) then            !ATAN called from ATAN2
            ressmall=(result-reshi+restemp)-resbot+restiny
                                                !get residual bits
            go to 200                           !let ATAN2 finish job
         end if
         _qatan=dcmplx(reshi,resbot)
         discard=_setflm (%val(oldfpscr))
         return                                 !Normal arguments done
       else if (abs(dHead) .le. infinity) then  !Range 5-very large numbers
          if (iatan2flag .ne. 1) then
             discard=_setflm(%val(oldfpscr))
             _qatan=dcmplx(pih,pihbm)           !Intentional misround to
          else                                  !stay in proper quadrant
             go to 190                          !call from _qatan2
          end if
          if (dHead < 0.0) _qatan=- _qatan      !stay in correct quadrant
          return
       else                                     !process NaNs
          discard=_setflm(%val(oldfpscr))
          _qatan=dcmplx(dHead+zero,zero)        !Make NaN quiet
          return
       end if
90     rarg=1.0d0/dHead                         !Range 4
       rflag=1                                  !indicate large argument
       p1=1.0d0-dHead*rarg                      !time to complete hi precision
       p2=dTail*rarg                            !reciprocal of argument, to
       p3=p1-p2                                 !use as reduced argument.
       rarg2a=p3*rarg
       rarg2=rarg2a+(p3-dHead*rarg2a)*rarg
       dHead=rarg
       dTail=rarg2
       eN=LXIV*abs(dHead)+big
100    CONTINUE                                 !Code for Range 1 and Range 4
       temp=2.0*dHead*dTail                     !direct evaluation of series
       argsq=dHead*dHead+temp                   !compute square of argument
       argsq2=dHead*dHead-argsq+temp+dTail*dTail
       sum=atancoeff(15)
       sum=atancoeff(14)+argsq*sum
       sum=atancoeff(13)+argsq*sum
       sum=atancoeff(12)+argsq*sum
       sumt=atancoeff(11)+argsq*sum
       sum=atancoeff(10)+argsq*sumt
       suml=atancoeff(10)-sum+argsq*sumt
       do i=9, 2,-1
         temps=atancoeff(i)+sum*argsq
         suml=atancoeff(i)-temps+sum*argsq+
     +        (sum*argsq2+suml*argsq)
         sum=temps
       end do
       prodlow=suml*argsq+sum*argsq2            !multiply by arg^2
       prod=sum*argsq+prodlow
       prodlow=sum*argsq-prod+prodlow
       temp2=prodlow*dHead+prod*dTail           !multiply by arg
       temp=prod*dHead+temp2
       temp2=prod*dHead-temp+temp2
       sum=temp*recip                           !sum of series ---
       remainder=(temp-sum*atancoeff(1))
       partial=remainder+temp2
       residual=remainder-partial+temp2
       suml=partial*recip+(residual*recip)
       res=dHead+sum                            !except for argument
       reslow=(dHead-res+sum)                   !exact
       resmid=dTail+suml
       restiny=dTail-resmid+suml
       p=reslow+resmid                          !sum of middle terms
       q=reslow-p+resmid                        !possible residual
       reshi=res+p
       resbot=(res-reshi+p)+(q+restiny)
150    if (rflag .eq. 1) then                   !large magnitude argument
          if (dHead > 0) then                   !fetch pi/2 with proper sign
             p1=pih
             p2=pihb
             p2a=pihc
          else
             p1=-pih
             p2=-pihb
             p2a=-pihc
          end if
          p3=p1-reshi                           !subtract result from pi/2
          p4=p1-p3-reshi
          p5=p2-resbot
          l4=(abs(p4) > abs(p5))
          p6=p2-p5-resbot+p2a
          p7=p4+p5
          reshi=p3+p7
          if (abs(p4) > abs(p5)) then
             p8=p4-p7+p5
          else
             p8=p5-p7+p4
          end if
          resbot=p3-reshi+p7+(p6+p8-restiny)
          if (iatan2flag .eq. 1) then
             ressmall=(p3-reshi+p7)-resbot+(p6+p8-restiny)
             go to 200
          end if
       else if (iatan2flag .eq. 1) then
          ressmall=(res-reshi+p)-resbot+(q+restiny)
          go to 200
       end if
       discard=_setflm(%val(oldfpscr))
       _qatan=dcmplx(reshi, resbot)
       return

* NAME: _qatan2
*                                                                    
* FUNCTION: quad precision inverse tanget function
*
* RETURNS: inverse tanget of argument

       entry _qatan2(%val(dnum),%val(dnuml),%val(dden),%val(ddenl))

********************************************************************************
*     _QATAN2 expects two quad words as arguments.  It returns the arctan      *
*     of the quotient of the arguments, but differs from _QATAN in that        *
*     the result can be in the interval [-Pi,Pi], whereas _QATAN always        *
*     returns a result in the range [-Pi/2, Pi/2].  After argument reduction   *
*     and the testing of numerous special cases, _QATAN is used for the bulk   *
*     of the work.                                                             *
*                                                                              *
*       atan2(NaN, anything)--------------------------------->   NaN           *
*       atan2(anything, NaN)--------------------------------->   NaN           *
*       atan2(+-0, +(anything but NaN))----------------------> +-0             *
*       atan2(+-0, -(anything but NaN))----------------------> +-Pi            *
*       atan2(+-(anything but 0 and NaN), 0 )----------------> +-Pi/2          *
*       atan2(+-(anything but Infinity and NaN), +Infinity)--> +-0             *
*       atan2(+-(anything but Infinity and NaN), -Infinity)--> +-Pi            *
*       atan2(+-Infinity, +Infinity)-------------------------> +-Pi/4          *
*       atan2(+-Infinity, -Infinity)-------------------------> +-3Pi/4         *
*       atan2(+-Infinity, (anything but 0, NaN, Infinity))---> +-Pi/2          *
*                                                                              *
********************************************************************************

       c_function = 0
 2018  continue
                                                !save old fpscr, and
      oldfpscr=_setflm(%val(Infinity))          !set round-to-nearest
      l1=(dden .eq. dden)                       !lots of scheduling directives
      l2=(dden .eq. 0.0)
      l3=(dnum .eq. dnum)
      l4=(dden-1.0+ddenl .ne. 0.0)
      l5=(dnum .eq. dnum)
      l6=(abs(dden) .eq. Infinity)
      l7=(abs(dnum) .eq. Infinity)
      l8=(dnum .eq. 0.0)
      r=1.0/dden                                !start computation of num/den
      If (dden .eq. dden) then                  !denominator is not NaN
        If (dnum .eq. dnum) then                !numerator is not NaN
          l9=(abs(dden) .lt. Tiny)              !scheduling directives
          l10=(abs(dnum) .lt. Tiny)             !
          If (dden-1.0+ddenl .ne. 0.0) then     !denominator is not 1.0
            If (dnum .ne. 0.0) then             !numerator is not 0.0
              If (dden .ne. 0.0) then
                If (abs(dden) .ne. Infinity) then       !numerator not Infinity
                  If (abs(dnum) .ne. Infinity) then     !denominator not INF
                    iatan2flag=1                        !indicate from ATAN2
                    If ((abs(dden) .lt. Tiny).or.
     +                   (abs(dnum) .lt. Tiny)) then
                       dden=dden*Rescale        !For a tiny arguments,
                       ddenl=ddenl*Rescale      !rescale all the input
                       dnum=dnum*Rescale        !so that the full length
                       dnuml=dnuml*Rescale      !division can procede.
                       r=1.0/dden
                       if (abs(r) < Tiny) then  !rescaling has made numerator
                          if (dden > 0) then    !too large.  Result is
                             dhead = zero       !zero for positive denominator
                             dtail = zero
                             frac3 = zero
                             go to 50
                          else                  !for neg. denominator,
                             dhead = dnum       !send numerator to qatan
                             dtail = zero       !result will be subtracted
                             frac3 = zero       !from properly signed Pi later
                             go to 50
                          end if
                       end if
                    end if
                    fract=dnum*r

*********************************************************
* A detailed division (dnum,dnuml)/(dden,ddenl) follows.*
*********************************************************
                    frac=fract+(dnum-dden*fract)*r      !refine h.o. quotient
                    p29=dnum-dden*frac
                    p30=frac*ddenl                      !negative term
                    l2=(abs(p29) > abs(p30))            !scheduling directive
                    p31=p30-frac*ddenl
                    p32=p31
                    p33=p29-p30
                    p35=p33+dnuml
                    l1=(abs(p33) > abs(dnuml))
                    if (abs(fract) .eq. infinity) then
                       dhead = fract
                       dtail = 0.0
                       frac3 = 0.0
                       go to 50
                    end if
                    frac2=p35*r
                    frac2=frac2+(p35-frac2*dden)*r      !mid. quot. word
                    if (abs(p29) > abs(p30)) then
                      p34=p29-p33-p30+p32
                    else
                      p34=p29-(p33+p30)+p32
                    end if
                    p36=p35-frac2*dden+p34-frac2*ddenl
                    if (abs(p33) > abs(dnuml)) then
                      p37=p33-p35+dnuml
                    else
                      p37=dnuml-p35+p33
                    end if
                    p38=p37+p36
                    frac3=p38*r /(1.0+frac*frac)        !Fraction done!
                    dHead=frac                  !Use ATAN routine to do
                    dTail=frac2                 !most of the work
                    go to 50
                  else
                                                !numerator is Infinite and
                    If (dnum > 0.0) then        !denoinator is finite
                      if (dden > 0.0) then      !first quadrant
                         _qatan=dcmplx(pih, pihbm)
                      else                      !second quadrant
                         _qatan=dcmplx(pih, pihb)
                      end if
                    else
                      if (dden > 0.0) then      !fourth quadrant
                         _qatan=dcmplx(-pih, -pihbm)
                      else                      !third quadrant
                         _qatan=dcmplx(-pih, -pihb)
                      end if
                    end if
                    discard=_setflm (%val(oldfpscr))
                    return
                  end if
                else   !denominator is infinite
                  s=1.0
                  if (dnum < 0.0)  s=-s                 !s=sign of numerator
                  if (abs(dnum) .ne. Infinity) then     !finite numerator
                    if (dden > 0.0) then                !positive denominator
                      _qatan=dcmplx(s*zero,s*zero)      !1st or 4th quadrants
                    else
                      _qatan=dcmplx(s*2.0*pih,s*2.0*pihbm) !2nd or 3rd qudrnt
                    end if
                  else                                  !num. and den.infinite
                    if (dden > 0.0) then                !1st or 4th quadrants
                      _qatan=dcmplx(s*0.5*pih,s*0.5*pihb)
                    else
                      _qatan=dcmplx(s*pi34,s*pi34b)
                    end if
                  end if
                  discard=_setflm(%val(oldfpscr))
                  return
                end if
              end if
********************************************************************
*  numerator is not zero, denominator is.  Result on vertical axis *
********************************************************************
              eN=dden
********************************************************************
*  when denominator is +0, give truncated pi/2 to stay in          *
*  quadrants 1 or 4. When denominator is -0, give pi/2 rounded up  *
*  to stay in quadrants 2 or 3.                                    *
********************************************************************
              If (ndx(1) < 0) then                !denominator is -0
                If (dnum > 0) then
                   _qatan=dcmplx(pih,pihb)      !second quadrant
                else
                   _qatan=dcmplx(-pih,-pihb)    !third quadrant
                end if
              else
                If (dnum > 0) then
                  _qatan=dcmplx(pih,pihbm)      !first quadrant
                else
                  _qatan=dcmplx(-pih,-pihbm)    !fourth quadrant
                end if
              end if
              discard=_setflm(%val(oldfpscr))
              return
            end if
******************************************************************
*  When numerator and denomitor are both 0 --> domain error      *
******************************************************************

* For C-language semantics, set errno = EDOM
            if (c_function .eq. 1) then
               call __set_errno128(%VAL(EDOM))
            end if

            eN=dnum
            if (dden .eq. 0.0) then             !domain error case
              discard=_setflm (%val(oldfpscr))  !reset fpscr
              _qatan=dcmplx(zero/zero, 0.0d0)   !cause 0/0 invalid op signal
              return
            end if
            if (dden .gt. 0.0) then
              _qatan=dcmplx(dnum,zero)          !correctly signed zero result
            else
              if (ndx(1) < 0) then
                _qatan=dcmplx(-pi,-pibm)
              else
                _qatan=dcmplx(pi,pibm)
              end if
            end if
            discard=_setflm (%val(oldfpscr))
            return
          end if
***********************************************************
*  When denominator is Unity! Give numerator to ATAN.     *
***********************************************************
          iatan2flag=0                         !ATAN does entire job
          frac3=0
          dHead=dnum
          dTail=dnuml
          go to 50
        end if
      end if
**************************************************************
*  There are NaNs in the argument.  Return NaN as the result *
**************************************************************
      discard=_setflm (%val(oldfpscr))
      _qatan=dcmplx(dnum+dnuml+dden+ddenl, 0.0d0)
      return

190   continue                                  !return after very larg arg.
      reshi = pih                               !if the second argument is
      if (dden > 0.0) then                      !positive, the result is in
         resbot = pihbm                         !first or fourth quadrant
         ressmall = 0                           !set up appropriate value of
         if (dhead < 0.0) then                  !pi/2, truncated
            reshi = -reshi
            resbot = -resbot
         end if
      else                                      !Otherwise, result is in the
         reshi = pih                            !second or third quadrant.
         resbot = pihb                          !set up pi/2, correctly rounded
         ressmall = pihc
         if (dhead < 0.0) then
            reshi = -reshi
            resbot = -resbot
            ressmall = -ressmall
         end if
      end if
200   extra=ressmall+frac3
      if (dden > 0) then                        !For Positive denominator
        _qatan=dcmplx(reshi,resbot+extra)       !return result given by ATAN
      else
        if (dnum > 0) then                      !For Negative denominator
          pin=pi                                !return 2nd|3rd quadrand angle,
          pin2=pib                              !depending on sign of numerator
          pin3=pic
        else
          pin=-pi                               !-->  Pi+atan, for dnum < 0,
          pin2=-pib                             !     atan-Pi, for dnum > 0
          pin3=-pic
        end if
        restop=pin+reshi
        resmid=pin-restop+reshi
        reslow=pin2+resbot
        restiny=pin2-reslow+resbot+extra
        resint=resmid+reslow
        reshi=restop+resint
        resbot=restop-reshi+resint
        if (abs(resmid) > abs(reslow)) then
          resbot=resbot+(resmid-resint+reslow+restiny)
        else
          resbot=resbot+(reslow-resint+resmid+restiny)
        end if
        _qatan=dcmplx(reshi, resbot)
      end if
      discard=_setflm(%val(oldfpscr))
      return

* Entry point for C-language name and semantics -- atan
       entry atanl(%val(c_head), %val(c_tail))
       dhead = c_head
       dtail = c_tail
       c_function = 1
       goto 1018

* Entry point for C-language name and semantics -- atan2
       entry atan2l(%val(c_head), %val(c_tail), 
     x      %val(c_denh), %val(c_denl))

       dnum = c_head
       dnuml = c_tail
       dden = c_denh
       ddenl = c_denl
       c_function = 1
       goto 2018

      end
