* @(#)62	1.3  src/bos/usr/ccs/lib/libm/POWER/_qsincos.f, libm, bos411, 9428A410j 9/21/93 11:57:16
* COMPONENT_NAME: LIBCCNV
*
* FUNCTIONS: _qsin, _qcos, sinl, cosl
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
*                  SOURCE MATERIAL
*
* Copyright (c) ISQUARE, Inc. 1990

@process xflag(callby)
*******************************************************************
*            PROGRAM: Quad-Precision SINE                         *
*                     Quad-Precision COSINE                       *
*            AUTHOR:  ISQUARE, Inc (V. Markstein)                 *
*            DATE:    4/3/90                                      *
*            MOD:     5/1/90 C --> FORTRAN                        *
*                     10/29/90 port --> RS/6000                    *
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

* NAME: _qsin
*                                                                    
* FUNCTION: quad precision sine
*
* RETURNS: sine of argument

       DOUBLE COMPLEX FUNCTION _QSIN (%val(dHead), %val(dTail))
       implicit real*8          (a-h, o-z)
       implicit double complex  (_)
       real*8                   infinity, LXIV
       integer*4                N(2),ndx(2)
       logical*4                l1,l2,l3,l4,l5,l6,l7

       double complex		cosl
       double complex		sinl
       integer			c_function
       real*8			c_head, c_tail

       data                     unit /1.0d0/
       data                     big  /z'4338000000000000'/
       data                     easylimit /z'42b0000000000000'/
       data                     LXIV /64.0d0/

       data                     pih  /z'3FF921FB54442D18'/
       data                     pihb /z'3C91A62633145C07'/
       data                     pihc /z'B91F1976B7ED8FBC'/
       data                     pihd /z'35B4CF98E804177D'/
       data                     infinity /z'7ff0000000000000'/

       PARAMETER                (RPIH=0.6366197723675814D0)
       equivalence              (eN,N)
       equivalence              (accndx,ndx), (ndx(2),index)
       INCLUDE                  "_qtrigdata.f"

       c_function = 0			! Fortran name and semantics
 1018  continue

       M=0                                   !to distinguish sin entry point
       l2=(abs(dHead) .lt. easylimit)        !Scheduling directive
       oldfpscr=_setflm(%val(big))
       dleft = 0.0
       go to 100

* NAME: _qcos
*                                                                    
* FUNCTION: quad precision cosine
*
* RETURNS: cosine of argument

       ENTRY _QCOS (%val(dHead), %val(dTail))

       c_function = 0			! Fortran name and semantics
 2018  continue

       M=1                                   !to distinguish cos entry point
       l2=(abs(dHead) .lt. easylimit)        !Scheduling directive
       oldfpscr=_setflm(%val(big))
       dleft = 0.0
100    CONTINUE                              !Common code follows
       if ( .not. (abs(dHead) .lt. easylimit)) then
                                             !Troublesome arguments
          if (dHead .ne. dHead) then         !For NaNs, return a NaNQ
             discard=_setflm(%val(oldfpscr)) !and signal invalid operation
             _qsin=dcmplx(dHead+unit, 0.0d0)
             return
          else if (abs(dHead) .eq. infinity) then   !For Infinity,
             discard=_setflm(%val(oldfpscr)) !return a NaNQ and signal
             _qsin=dcmplx(dHead-dHead, 0.0d0)       !Invalid operation
             return
          else                               !A big argument reduction job!
             _reduce=_q_treducex(%val(dHead),%val(dTail),dleft,M)
             dHead=dble(_reduce)
             dTail=dimag(_reduce)
          end if
       end if
       eN=dHead*rpih+big                     !# of multiples of pi/2 to remove
       q=eN-big                              !Integer
       redarg=dHead-q*pih                    !Exact result
       l1=(redarg .gt. 0.0d0)                !scheduling directive
       M=M+N(2)                              !M=quadrant in which to comp. sin
       b=pihb*q                              !continue argument reduction
       blow=pihb*q-b
       redarg1=dTail-b                       !second word of reduced argument
       accndx=abs(redarg)*LXIV+big           !index into acctbl
       ca=pihc*q
       l2=(index < 6)                        !scheduling directive
       if (abs(b) .gt. abs(dTail)) then      !tail-b may be inexact
         carry=dTail-(b+redarg1)
       else
         carry=(dTail-redarg1)-b
       end if
       if (.not. (index < 6)) then
          if (redarg .gt. 0.0d0) then
             redarg=redarg-trigtbl(1,index) !arg-q ln2-tbl(index) ->
             sintab=trigtbl(2,index)         !fetch sine of table value
             sintab2=trigtbl(3,index)
          else                               !mucho trailing zeroes in redarg
             redarg=redarg+trigtbl(1,index)
             sintab=-trigtbl(2,index)        !fetch sine of table value
             sintab2=-trigtbl(3,index)
          end if
       end if
       arg=redarg+redarg1                    !get signif. bits from the right
       l5=(q .eq. 0.0d0)                     !scheduling directive
       arg2a=(redarg-arg+redarg1)
       arg2b=arg2a-blow                      !fill in with bits from 2nd term
       c=carry-ca                            !3rd word of reduced argument
       l4=(abs(arg2b) .gt. abs(c))           !scheduling directive
       argmid=arg2b+c
       argnew=arg+argmid
       argres=arg-argnew+argmid
       arg=argnew
       cerr=(carry-c)-ca
       l6=(abs(arg) .lt. 2.0**(-52))
       l7=(iand(M,1).eq.0)
       if (abs(arg2b) .gt. abs(c)) then
          d=arg2b-argmid+c+dleft
       else
          d=c-argmid+arg2b+dleft
       end if
       d=d+cerr+(ca-pihc*q) -pihd*q          !4th word
       arg2=argres +d                        !reduced arg is (arg, arg2)
       if (.not.(q .eq. 0) .and. (iand(M,1).eq.0)
     x     .and. (abs(arg) .lt. 2.0**(-52)))  then
          M=M-N(2)
          _result=_q_treducex(%val(dHead),%val(dTail),dleft,M)
          part1=dble(_result)
          part2=dimag(_result)
          part3=part1+part2
          part4=part1-part3+part2+dleft
          _result=dcmplx(part3,part4)
          go to 2000
       end if
       temp=2.0*arg*arg2
       argsq=arg*arg+temp
       argsq2=arg*arg-argsq+temp+arg2*arg2
       if (index < 6) then                   !straight power series
          if (iand(M,1) .eq. 0) then         !sin series evaluation
             sin=sincoeff16(1,6)             !need extra terms when
             sin_l=sincoeff16(2,6)            !used without table lookup
             sinargtiny=argres-arg2+d
             do i=5,0,-1
                temps=sin*argsq2+sin_l*argsq
                prods=sin*argsq+temps
                prodsl=sin*argsq-prods+temps
                sin=sincoeff16(1,i)+prods
                sin2=sincoeff16(2,i)+prodsl
                sin_l=sincoeff16(1,i)-sin+prods+sin2
             end do
          else                               !cos series evaluation
             cos=coscoeff16(1,6)
             cos_l=coscoeff16(2,6)
             do i=5,0,-1
                temps=cos*argsq2+cos_l*argsq
                prods=cos*argsq+temps
                prodsl=cos*argsq-prods+temps
                cos=coscoeff16(1,i)+prods
                cos2=coscoeff16(2,i)+prodsl
                cos_l=coscoeff16(1,i)-cos+prods+cos2
             end do
          end if
       else                                  !shorter series in case
          cos=coscoeff(1,4)                  !where exact table is used.
          sin=sincoeff(1,4)
          cos_l=coscoeff(2,4)
          sin_l=sincoeff(2,4)
          do i=3,0,-1                        !add short series
             tempc=cos*argsq2+cos_l*argsq
             prodc=cos*argsq+tempc
             prodcl=cos*argsq-prodc+tempc
             cos=coscoeff(1,i)+prodc
             cos2=coscoeff(2,i)+prodcl
             cos_l=coscoeff(1,i)-cos+prodc+cos2
             temps=sin*argsq2+sin_l*argsq
             prods=sin*argsq+temps
             prodsl=sin*argsq-prods+temps
             sin=sincoeff(1,i)+prods
             sin2=sincoeff(2,i)+prodsl
             sin_l=sincoeff(1,i)-sin+prods+sin2
          end do
       end if
       tempc=cos*argsq2+cos_l*argsq
       cost=cos*argsq+tempc
       cos_l=cos*argsq-cost+tempc
       cos=cost                              !cos(arg)-1.0 complete
       temps=sin*argsq2+sin_l*argsq
       sint=sin*argsq+temps
       sin_l=sin*argsq-sint+temps
       temps=sint*arg2+sin_l*arg
       sin=sint*arg+temps
       sin_l=sint*arg-sin+temps               !sin(arg)-arg complete
       if (index < 6) then
          if (iand(M,1) .eq. 0) then
            res=arg+sin                      !sin of a small angle
            reslow=arg2+sin_l                 !careful distallation
            resmid=arg-res+sin
            resbot=arg2-reslow+sin_l
            restop=res+(reslow+resmid)
            resin=res-restop+(reslow+resmid)
            resextra=resmid-(reslow+resmid)+reslow
            ressup=restop+(resin+resbot)
            restiny=resin-(resin+resbot)+resbot
            resinf=restop-ressup+(resin+resbot) +
     +          ( resextra+restiny+sinargtiny)
            _result=dcmplx(ressup, resinf)
           else
            res=1.0+cos                      !cos of a small angle
            reslow= cos_l
            resmid=1.0-res+cos
            restop=res+(reslow+resmid)
            resin=res-restop+(reslow+resmid)
            resextra=resmid-(reslow+resmid)+reslow
            ressup=restop+resin
            resinf=restop-ressup+resin+resextra
            _result=dcmplx(ressup, resinf)
           end if
       else
          if (and(M,1).eq.0) then            !even quadrant-- eval. sine(x)
             p1=cos*sintab                   !o(-8)  exact
             p2=cos*sintab-p1                !o(-62) exact
             p3=cos_l*sintab+(cos*sintab2)    !o(-67)
             p4=arg*trigtbl(4,index)         !o(-4)  exact
             p5=arg*trigtbl(4,index)-p4      !o(-58) exact
             p6=arg2*trigtbl(4,index)        !o(-58)
             p6a=arg2*trigtbl(4,index)-p6
             p7=sin*trigtbl(4,index)         !o(-4)  exact
             p8=sin*trigtbl(4,index)-p7      !o(-59) exact
             p9=sin_l*trigtbl(4,index)+(sin*trigtbl(5,index))
                                             !o(-66)
             p10=arg*trigtbl(5,index)+p9+p8  !o(-59)
             p11=p2+p5                       !o(-58) exact
             p12=p5-p11+p2+p6a               !o(-112) exact
             l3=(abs(p6) .gt. abs(p11))      !scheduling directive
             p13=p6+p11                      !o(-57) exact
             p15=sintab+p4                   !o(0)   exact
             p16=sintab-p15+p4               !o(-54) exact
             p17=sintab2+p10+p12+p3          !o(-58)
             p18=p15+p1                      !o(0) exact
             p20=p18+p7                      !o(0) exact
             if (abs(p6) .gt. abs(p11)) then
                p14=p6-p13+p11               !o(-111)
             else
                p14=p11-p13+p6
             end if
             p19=p15-p18+p1+p16              !o(-53) exact
             p21=p18-p20+p7                  !o(-54) exact
             p22=p13+p17                     !o(-56)
             p23=p19+p 21                    !o(-52) exact
             p25=p20+p23                     !o(0) exact
             p26=p20-p25+p23                 !o(-54) exact
             if (abs(p19) .gt. abs(p21)) then
                p24=p19-p23+p21              !o(-108)
             else
                p24=p21-p23+p19
             end if
             p27=(p14 +p24)+p22+p26   !o(-54)
             p27a=p26-p27+((p14+p24)+p22)
             p28=p25+p27

             _result=dcmplx(p28, p25-p28+p27+p27a)
          else                               !evaluation of cos series.
             p1=trigtbl(4,index)*cos         !o(-16) exact
             p2=trigtbl(4,index)*cos-p1      !o(-71) exact
             p5=sintab*arg                   !o(-8) exact
             p6=sintab*arg-p5                !o(-62) exact
             p7=sintab*arg2                  !o(-62)
             p8=sintab*sin                   !o(-24) exact
             p9=sintab*sin-p8                !o(-78) exact
             p12=trigtbl(5,index)*cos -
     +           (sintab2*arg)               !o(-80)
             p13=p12-(sintab2*sin)           !o(-72)
             p14=p13+p2                      !o(-70)
             p15=p14+trigtbl(4,index)*cos_l   !o(-69)
             p16=p15-p9-sintab*sin_l          !o(-67)
             p17=trigtbl(4,index)+p1         !o(-1) exact
             p18=trigtbl(4,index)-p17+p1     !o(-55) exact-16 bits
             p19=p17-p5                      !o(-1) exact
             p20=p17-p19-p5                  !o(-55) exact-8 bits
             p21=p18+p20                     !o(-54) exact-17 bits
             p22=p19-p8                      !o(-1) exact
             p23=p19-p22-p8                  !o(-53) exact-25 bits
             p24=trigtbl(5,index)+p16        !o(-65)
             p23a=p23+p21                    !o(-54)
             p25=p23a-p6                     !o(-53) exact
             p26=p23a-p25-p6                 !o(-106)
             p27=p25-p7                      !o(-53) exact
             if (abs(p23) > abs(p21)) then
                p23b=p23-p23a+p21            !o(-106)
             else
                p23b=p21-p23a+p23
             end if
             p28=p25-p27-p7+p26+p23b !o(-105)
             p29=p27+p24                     !o(-53) exact
             p30=p27-p29+p24+p28             !o(-104)
             p31=p22+p29                     !o(-1) exact

             _result=(p31, p22-p31+p29+p30)
          end if
       end if
2000   continue
       if(and(M,2).ne.0) _result=-_result    !change sign for below x axis.
       _qsin=_result
       discard=_setflm(%val(oldfpscr))
       return

* Entry point for C-language name and semantics -- sinl
       entry sinl(%val(c_head), %val(c_tail))
       dhead = c_head
       dtail = c_tail
       c_function = 1
       goto 1018

* Entry point for C-language name and semantics -- cosl
       entry cosl(%val(c_head), %val(c_tail))
       dhead = c_head
       dtail = c_tail
       c_function = 1
       goto 2018

       end
