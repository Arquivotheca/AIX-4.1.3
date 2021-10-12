* @(#)66	1.3  src/bos/usr/ccs/lib/libm/POWER/_qtan.f, libm, bos411, 9428A410j 9/21/93 11:57:24
* COMPONENT_NAME: LIBCCNV
*
* FUNCTIONS: _qtan, tanl
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

* NAME: _qtan
*                                                                    
* FUNCTION: quad precision tangent
*
* RETURNS: tanget of argument

@process xflag(callby)
*******************************************************************
*            PROGRAM: Quad-Precision TANGENT                      *
*            AUTHOR:  ISQUARE, Inc (V. Markstein)                 *
*            DATE:    4/4/90                                      *
*            MOD:     5/1/90 C --> FORTRAN                        *
*                     11/01/90 port --> RS/6000                   *
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
       DOUBLE COMPLEX FUNCTION _QTAN (%val(dHead), %val(dTail))
       implicit real*8          (a-h, o-z)
       implicit double complex  (_)
       real*8                   infinity, LXIV
       integer*4                N(2),ndx(2)
       logical*4                l1,l2,l3,l4,l5,l6

       double complex		tanl
       integer			c_function
       real*8			c_head, c_tail

       data         unit        /1.0d0/
       data         big         /z'4338000000000000'/
       data         easylimit   /z'42b0000000000000'/
       data         LXIV        /64.0d0/

       data         pih         /z'3FF921FB54442D18'/
       data         pihb        /z'3C91A62633145C07'/
       data         pihc        /z'B91F1976B7ED8FBC'/
       data         pihd        /z'35B4CF98E804177D'/
       data         infinity    /z'7ff0000000000000'/

       PARAMETER                (RPIH=0.6366197723675814D0)
       equivalence              (eN,N)
       equivalence              (accndx,ndx), (ndx(2),index)
       INCLUDE                  "_qtandata.f"

       c_function = 0			! Fortran name and semantics
 1018  continue

       M=0                              !determine quadrant
       l2=(abs(dHead) .lt. easylimit)   !scheduling directive
       oldfpscr=_setflm(%val(big))
       dleft=0.0
       if ( .not. (abs(dHead) .lt. easylimit)) then
                                        !Painful arguments
          if (dHead .ne. dHead) then    !For NaNs, return a NaNQ
             discard=_setflm(%val(oldfpscr))    !and signal invalid operation
             _qtan=dcmplx(dHead+unit, 0.0d0)
             return
          else if (abs(dHead) .eq. infinity) then       !For Infinity,
             discard=_setflm(%val(oldfpscr))    !return a NaNQ and signal
             _qtan=dcmplx(dHead-dHead, 0.0d0)   !Invalid operation
             return
          else                                  !Big time argument reduction
             _reduce=_q_treducex(%val(dHead),%val(dTail),dleft,M)
             dHead=dble(_reduce)
             dTail=dimag(_reduce)
          end if
       end if
       eN=dHead*rpih+big                !no. of multiples of pi/2
       q=eN-big                         !to remove integer
       redarg=dHead-q*pih               !Exact result
       l1=(redarg .gt. 0.0d0)           !scheduling directive
       M=M+N(2)                         !M=quadrant in which to compute sin
       b=pihb*q                         !more argument reduction
       blow=pihb*q-b
       redarg1=dTail-b                  !second word of reduced argument
       accndx=abs(redarg)*LXIV+big      !index into acctbl
       ca=pihc*q
       l2= (index < 6)                  !scheduling directive
       if (abs(b) .gt. abs(dTail)) then !tail-b may be inexact
         carry=dTail-(b+redarg1)
       else
         carry=(dTail-redarg1)-b
       end if
       redargold=redarg
       if (.not. (index < 6)) then
          if (redargold .gt. 0.0d0) then
             redarg=redarg-tantbl(1,index)      !arg-q ln2-tbl(index) ->
          else                                  !mucho trailing zeroes in redarg
             redarg=redarg+tantbl(1,index)
          end if
       end if
       arg=redarg+redarg1               !get significant bits from the right
       l5=(q .eq. 0.0d0)                !scheduling directive
       arg2a=(redarg-arg+redarg1)
       arg2b=arg2a-blow                 !fill in with bits from 2nd term
       c=carry-ca                       !3rd word of reduced argument
       argmid=arg2b+c
       argnew=arg+argmid
       argres=arg-argnew+argmid
       arg=argnew
       cerr=(carry-c)-ca
       if (abs(arg2b) .gt. abs(c)) then
          d=arg2b-argmid+c+dleft
       else
          d=c-argmid+arg2b+dleft
       end if
       d=d+cerr+(ca-pihc*q) -pihd*q     !4th word
       arg2=argres +d                   !reduced arg is (arg, arg2)
       if (.not.(q .eq. 0.0d0).and.(abs(arg) .lt. 2.0**(-52))) then
          M=M-N(2)
          _result=_q_treducex(%val(dHead),%val(dTail),dleft,M)
          part1=dble(_result)
          part2=dimag(_result)
          part3=part1+part2
          part4=part1-part3+part2+dleft
          _result=dcmplx(part3,part4)
          if (iand(M,1) .eq. 0) then
             _qtan=_result
             discard=_setflm(%val(oldfpscr))
             return
          else
             arg=dble(_result)
             arg2=dimag(_result)
             argres=arg2
             d=part1-part3+part2-part4+dleft
          end if
       end if
       l1=(iand(M,1) .eq. 0)            !scheduling directive
       temp=2.0*arg*arg2
       argsq=arg*arg+temp
       argsq2=arg*arg-argsq+temp+arg2*arg2
       if (index < 6) then              !straight power series
          if (iand(M,1) .eq. 0) then    !2n multiple of pi/2 -> use TAN
             tan=tancoeff16(1,10)       !need extra terms when
             tan_l=tancoeff16(2,10)      !used without table lookup
             tanargtiny=argres-arg2+d
             do i=9,1,-1
                temps=tan*argsq2+tan_l*argsq
                prods=tan*argsq+temps
                prodsl=tan*argsq-prods+temps
                tan=tancoeff16(1,i)+prods
                tan2=tancoeff16(2,i)+prodsl
                tan_l=tancoeff16(1,i)-tan+prods+tan2
             end do
             temps=tan*argsq2+tan_l*argsq
             tant=tan*argsq+temps
             tan_l=tan*argsq-tant+temps
             temps=tant*arg2+tan_l*arg
             tan=tant*arg+temps
             tan_l=tant*arg-tan+temps      !tan(arg)-arg complete
             res=arg+tan                  !tan of a small angle
             reslow=arg2+tan_l             !distill VERY CAREFULLY
             resmid=arg-res+tan
             resbot=arg2-reslow+tan_l
             restop=res+(reslow+resmid)
             resin=res-restop+(reslow+resmid)
             resextra=resmid-(reslow+resmid)+reslow
             ressup=restop+(resin+resbot)
             restiny=resin-(resin+resbot)+resbot
             resinf=restop-ressup+(resin+resbot) +
     +              ( resextra+restiny+tanargtiny)
             _qtan=dcmplx(ressup, resinf)
             discard=_setflm(%val(oldfpscr))
             return
          else                          !near an 2k+1 multiple of pi/2
                                        !tan (x)=-cot(x+(2k+1) pi/2)

             cot=cotcoeff16(1,9)        !need extra terms when
             cotl=cotcoeff16(2,9)       !used without table lookup
             cotargtiny=argres-arg2+d
             rarg0=1.0/arg              !We use the cotangent
             do i=8,1,-1                !series, which is one term shorter
                temps=cot*argsq2+cotl*argsq     !for the desired precision
                prods=cot*argsq+temps           !than the TAN series.
                prodsl=cot*argsq-prods+temps
                cot=cotcoeff16(1,i)+prods
                cot2=cotcoeff16(2,i)+prodsl
                cotl=cotcoeff16(1,i)-cot+prods+cot2
             end do
             cott=cot
             rarg=rarg0
             temps=cott*arg2+cotl*arg
             cot=cott*arg+temps
             cotl=cott*arg-cot+temps    !1/arg-cot(arg) completed
             p1=1.0d0-arg*rarg          !next complete hi precision
             p2=arg2*rarg               !reciprocal of argument, to subtract
             p3=p1-p2                   !from series.  The reciprocal will
             rarg2a=p3*rarg             !be the dominant term.
             rarg2=rarg2a+(p3-arg*rarg2a)*rarg
             if (abs(p1) > abs(p2)) then
                p4=p1-p3-p2
             else
                p4=p1-(p3+p2)
             end if
             p5=p4+(p2-arg2*rarg)
             p6=p5-cotargtiny*rarg
             p7=p3-rarg2*arg
             rarg3=((p6+p7)-arg2*rarg2)*rarg    !low order recip. bits

***********************************************************************
*     BEGIN the subtraction of 1/arg from the series                  *
*              |cot| much greater than  |rarg|                        *
***********************************************************************

             p8=cot-rarg
             p9=cot-(rarg+p8)
             p10=cotl-rarg2
             l2=(abs(p9) > abs(p10))    !Scheduling directive
             p11=cotl-(rarg2+p10)
             p12=p11-rarg3
             p13=p9+p10
             if (abs(p9) > abs(p10)) then
                p14=p9-p13+p10
             else
                p14=p10-p13+p9
             end if
             p15=p12+p14
             p16=p8+p13                 !HO term of result
             p17=p8-p16+p13             !almost LO term
             p18=p17+p15                !assimilate any other LO bits
             discard=_setflm(%val(oldfpscr))    !prepare to return
             _qtan=dcmplx(p16, p18)
             return
          end if
       else                             !Serious cases
                                        !Accurate tables will be used.
          tant=tancoeff(1,7)            !Need fewer terms when
          tant=tancoeff(1,6)+tant*argsq
          tan=tancoeff(1,5)+tant*argsq
          tan_l=tancoeff(1,5)-tan+tant*argsq

          tanargtiny=argres-arg2+d
          do i=4,1,-1
             temps=tan*argsq2+tan_l*argsq
             prods=tan*argsq+temps
             prodsl=tan*argsq-prods+temps
             tan=tancoeff(1,i)+prods
             tan2=tancoeff(2,i)+prodsl
             tan_l=tancoeff(1,i)-tan+prods+tan2
          end do
          tant=tan
          temps=tant*arg2+tan_l*arg
          tan=tant*arg+temps
          tan_l=tant*arg-tan+temps       !tan(arg) complete
          if (redargold > 0) then       !read out table values
             t1=tantbl(2,index)         !(t1,t2) are tan(x)
             t2=tantbl(3,index)
             c1=tantbl(4,index)         !(c1,c2) are cot(x)
             c2=tantbl(5,index)
          else
             t1=-tantbl(2,index)
             t2=-tantbl(3,index)
             c1=-tantbl(4,index)
             c2=-tantbl(5,index)
          end if
          p1=t1+c1                      !Tan(x)+Cot(x) needed
          p2=t2+c2                      !for all quadrants
          p3=c1-p1+t1
          p4=c2-p2+t2
          p5=p3+p2
          p6=p3-p5+p2+p4                !p1+p5+p6=Tan(x)+Cot(x) exactly
                                        !where x = table lookup argument
                                        !Next, this sum needs to be multiplied
                                        !by the tangent of the reduced argument
          p7=p1*tan
          p8=p1*tan-p7
          p9=p5*tan
          l1=(abs(p8) > abs(p9))        !scheduling directive
          p10=p5*tan-p9
          p11=p1*tan_l
          p12=p1*tan_l-p11
          p13=p10+p12+p5*tan_l
          p14=p13+p6*tan
          p15=p8+p9
          p17=p15+p11
          l2=(abs(p11) > abs(p15))      !scheduling directive
          if (abs(p8) > abs(p9)) then
             p16=p8-p15+p9
          else
             p16=p9-p15+p8
          end if
          if (abs(p11) > abs(p15)) then
             p18=p11-p17+p15+(p14+p16)
          else
             p18=p15-p17+p11+(p14+p16)
          end if
          p19=p7+p17
          p20=p7-p19+p17
          p21=p20+p18
          p22=p20-p21+p18

************************************************************************
*         At this point, different paths are taken for arguments       *
*         "neighborhood" of even and odd multiples of pi/2.            *
************************************************************************

          if (iand(M,1) .eq. 0) then

************************************************************************
*         Let the argument A be written as A=x+y, where                *
*         x=tantbl(1,index).  Then if T=tan(x) and C=cot(x)            *
*         as read from tantbl, and t=tan(y),  (computed above as       *
*         (tan+tan_l), then compute tan(A) as:                         *
*                                                                      *
*                                t ( C+T)                              *
*                tan(A)= T   +  -----------                            *
*                                  C-t                                 *
*                                                                      *
*         numerator represented as (p19,p21,p22) in the code below.    *
*                                                                      *
************************************************************************
             p23=c1-tan                 !High order part of C-t
             p24=c2-tan_l
             l3=(abs(c2) > abs(tan_l))
             rec=1.0 / p23
             p25=c1-p23-tan
             frac=p19*rec               !start computation of fraction
             if (abs(c2) > abs(tan_l)) then
                p26=c2-p24-tan_l
             else
                p26=c2-(p24+tan_l)
             end if
             p27=p25+p24
             p28=p25-p27+p24+p26        !C-t=(p23,p27,p28) done
             frac=frac+(p19-p23*frac)*rec       !refine lead fraction wd.
             p29=p19-p23*frac
             p30=frac*p27               !p30 a negative term
             l2=(abs(p29) > abs(p30))   !scheduling directive
             p31=p30-frac*p27+p22
             p32=p31-frac*p28
             p33=p29-p30
             p35=p33+p21
             l1=(abs(p33) > abs(p21))   !scheduling directive
             frac2=p35*rec
             frac2=frac2+(p35-frac2*p23)*rec    !second fraction wd.
             if (abs(p29) > abs(p30)) then
                p34=p29-p33-p30+p32
             else
                p34=p29-(p33+p30)+p32
             end if
             l3=(abs(tan_l) > abs(frac2))        !scheduling directive
             p36=p35-frac2*p23+p34-frac2*p27
             if (abs(p33) > abs(p21)) then
                p37=p33-p35+p21
             else
                p37=p21-p35+p33
             end if
             p38=p37+p36
             frac3=p38*rec              !Fraction of above formula done

************************************************************************
*        Finally, add (tan,tan_l) and (frac,frac2,frac3) for result    *
************************************************************************

             p39=t1+frac
             p40=t1-p39+frac
             p41=t2+frac2
             if (abs(t2) > abs(frac2)) then
                p42=t2-p41+frac2+frac3
             else
                p42=frac2-p41+t2+frac3
             end if
             p43=p40+p41
             p44=p40-p43+p41+p42
             p45=p39+p43
             p46=p39-p45+p43+p44
             _qtan=dcmplx (p45, p46)
             discard=_setflm(%val(oldfpscr))
             return
           else

************************************************************************
*      The remaining case is for argments in the neighborhood of       *
*      even multiples of pi/2.                                         *
*      Use the identity                                                *
*                          tan(A+(2k+1)pi/2)=-cot(A)                   *
*                                                                      *
*      Let the reduced argument A be written as                        *
*                     A=x+y, where  x=tantbl(1,index)                  *
*      Then if T=tan(x) and C=cot(x) as read from tantbl, and t=tan(y),*
*      (computed above as (tan+tan_l), compute cot(A) as:              *
*                                                                      *
*                                  t(C+T)                              *
*                      cot(A)=C - ----------                           *
*                                  T+t                                 *
************************************************************************
             p23=t1+tan                 !High order part of T+t
             p24=t2+tan_l
             l3=(abs(t2) > abs(tan_l))   !scheduling directive
             rec=1.0 / p23
             p25=t1-p23+tan
             frac=p19*rec               !start computation of fraction
             if (abs(t2) > abs(tan_l)) then
                p26=t2-p24+tan_l
             else
                p26=tan_l-p24+t2
             end if
             p27=p25+p24
             p28=p25-p27+p24+p26        !T+t= (p23,p27,p28) done
             frac=frac+(p19-p23*frac)*rec       !refine lead fraction wd.
             p29=p19-p23*frac
             p30=frac*p27               !p30 a negative term
             l2=(abs(p29) > abs(p30))   !scheduling directive
             p31=p30-frac*p27+p22
             p32=p31-frac*p28
             p33=p29-p30
             p35=p33+p21
             l1=(abs(p33) > abs(p21))           !scheduling directive
             frac2=p35*rec
             frac2=frac2+(p35-frac2*p23)*rec    !second fraction wd.
             if (abs(p29) > abs(p30)) then
                p34=p29-p33-p30+p32
             else
                p34=p29-(p33+p30)+p32
             end if
             l3=(abs(tan_l) > abs(frac2))        !scheduling directive
             p36=p35-frac2*p23+p34-frac2*p27
             if (abs(p33) > abs(p21)) then
                p37=p33-p35+p21
             else
                p37=p21-p35+p33
             end if
             p38=p37+p36
             frac3=p38*rec                      !Fraction done

**************************************************************************
*    At last, subtract (cot,cotl) from (frac,frac2,frac3) for the result *
**************************************************************************
             p39=frac-c1
             p40=frac-(c1+p39)
             p41=frac2-c2
             if (abs(c2) > abs(frac2)) then
                p42=frac2-(c2+p41)+frac3
             else
                p42=frac2-p41-c2+frac3
             end if
             p43=p40+p41
             p44=p40-p43+p41+p42
             p45=p39+p43
             p46=p39-p45+p43+p44
             _qtan=dcmplx (p45, p46)
             discard=_setflm(%val(oldfpscr))
             return
          end if
       end if

* Entry point for C-language name and semantics
       entry tanl(%val(c_head), %val(c_tail))
       dhead = c_head
       dtail = c_tail
       c_function = 1
       goto 1018

       end
