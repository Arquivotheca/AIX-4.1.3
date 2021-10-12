* @(#)53	1.4  src/bos/usr/ccs/lib/libm/POWER/_qacos.f, libm, bos411, 9428A410j 10/1/93 17:09:15
* COMPONENT_NAME: LIBCCNV
*
* FUNCTIONS: _qacos, acosl
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

* NAME: _qacos
*                                                                    
* FUNCTION: quad precision inverse cosine
*
* RETURNS: inverse cosine of argument

@process xflag(callby)
*******************************************************************
*            PROGRAM: Quad-Precision INVERSE COSINE (QACOS)       *
*            AUTHOR:  ISQUARE, Inc (V. Markstein)                 *
*            DATE:    4/4/90                                      *
*            MOD:     5/1/90 C --> FORTRAN                        *
*                     11/19/90 port --> RS/6000                   *
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
       DOUBLE COMPLEX FUNCTION _QACOS (%val(dHead), %val(dTail))
       implicit real*8          (a-h, o-z)
       implicit double complex  (_)
       common                   /_q_sqrt/sqrtextra
       logical*4                l1,l2,l3,l4,l5,l6

       double complex		acosl                    ! really REAL*16
       external			__set_errno128
       integer			c_function
       integer			EDOM
       parameter		(EDOM = 33)
       real*8			c_head, c_tail

       data         unit        /1.0/
       data         pih         /z'3FF921FB54442D18'/    !Pi/2
       data         pihb        /z'3C91A62633145C07'/    !Pi/2 l.o. word
       data         pihbm       /z'3C91A62633145C06'/    !(l.o.word unrounded)
       data         pihc        /z'B91F1976B7ED8FBC'/    !Pi/2 3rd order word
       data         piq         /z'3FE921FB54442D18'/    !Pi/4
       data         piqb        /z'3C81A62633145C07'/    !Pi/4 l.o word
       data         piqc        /z'B90F1976B7ED8FBC'/    !Pi/4 3rd order word
       data         pi34        /z'4002D97C7F3321D2'/    !3Pi/4 h.o. word
       data         pi34b       /z'3C9A79394C9E8A0A'/    !3Pi/4 l.o. word
       data         pi34c       /z'393456737B06EA1A'/    !3Pi/4 3rd order word
       data         pi          /z'400921FB54442D18'/    !Pi h.o.word
       data         pib         /z'3CA1A62633145C07'/    !Pi l.o.word rounded
       data         pic         /z'B92F1976B7ED8FBC'/    !Pi 3rd order word
       data         pibm        /z'3CA1A62633145C06'/    !Pi l.o.word chopped
       data         tropetit    /z'3c80000000000000'/
       data         zero        /0.0d0/
       INCLUDE "_qasindata.f"
       PARAMETER                (NTERMS=23)     !number of full precision terms
                                                !in power series for ASIN

       c_function = 0			! Fortran name and semantics
 1018  continue

       oldfpscr=_setflm(%val(zero))
       l2=(abs(dhead) < tropetit  )
       l1=(dhead > 0.0)                         !Tests to be done later
       l3=(dhead-1.0+dtail < 0.0)
       l4=(dhead+1.0+dtail > 0.0)
       if (abs(dhead) < tropetit  ) go to 300   !miniscule argument--
       l5=(abs(dhead) > 0.5)
       if (((dhead > 0.0) .and. (dhead-1.0+dtail <= 0.0)) .or.
     +   ((dhead < 0.0) .and. (dhead+1.0+dtail >= 0.0))) then
                                                !Arg is valid -> range reduce
         if (abs(dhead) > 0.5) go to 200        !large arguments are special

         temp=(2.0*dhead)*dtail                 !Compute acos(x)=pi/2-asin(x)
         argsq=dhead*dhead+temp
         argsq2=dhead*dhead-argsq+temp+
     +          (((2.0*dhead)*dtail)-temp)
         sum=asincoeff(1,50)                    !Use Taylor Series
         sum=asincoeff(1,49)+sum*argsq
         sum=asincoeff(1,48)+sum*argsq
         sum=asincoeff(1,47)+sum*argsq
         sum=asincoeff(1,46)+sum*argsq
         do i= 45,NTERMS+2,-1                   !First 25 terms in working
            sum=asincoeff(1,i)+sum*argsq        !precison
         end do
         sumt=asincoeff(1,NTERMS+1)+sum*argsq
         suml=asincoeff(1,NTERMS+1)-sumt+sum*argsq
         sum=sumt
         do i=1, NTERMS
            temp=asincoeff(1,NTERMS-i+1)+sum*argsq
            suml=asincoeff(1,NTERMS-i+1)-temp+sum*argsq+
     +           (asincoeff(2,NTERMS-i+1)+sum*argsq2+
     +            suml*argsq)
            sum=temp
         end do
         prodlow=suml*argsq+sum*argsq2          !multiply by arg^2
         prod=sum*argsq+prodlow
         prodlow=sum*argsq-prod+prodlow
         temp2=prodlow*dhead+prod*dtail         !multiply by arg
         temp=prod*dhead+temp2
         temp2=prod*dhead-temp+temp2

         res=dhead+temp                         !add argument
         reslow=(dhead-res+temp)
         resmid=dtail+temp2
         restiny=dtail-resmid+temp2
         p=reslow+resmid                        !sum of middle terms
         q=reslow-p+resmid                      !possible residual
         reshi=res+p
         resbot=(res-reshi+p)+(q+restiny)
         resinf=(res-reshi+p)-resbot+(q+restiny)

         res=pih-reshi                          !and subtract from Pi/2
         reslow=pih-res-reshi
         resmid=pihb-resbot
         if (pihb > abs(resbot)) then
           restiny=pihb-resmid-resbot+(pihc-resinf)
         else
           restiny=pihb-(resmid+resbot)+(pihc-resinf)
         end if
         p=reslow+resmid
         q=reslow-p+resmid
         reshi=res+p
         resbot=(res-reshi+p)+(q+restiny)
         _qacos=dcmplx(reshi,resbot)
         discard=_setflm (%val(oldfpscr))
         return
       end if

       !If control reaches here, argument
       !is not in the domain of this function
       discard=_setflm(%val(oldfpscr))

       if (dhead .ne. dhead) then
          _qacos=dcmplx(dhead+unit,zero)
       else
*     For C-language semantics, set errno = EDOM
          if (c_function .eq. 1) then
             call __set_errno128(%VAL(EDOM))
          end if
          _qacos=dcmplx(zero/zero, zero)        !return NaN, and cause invalid
       end if
       return                                   !operation to be set


200    if (abs(dhead) < 0.866025403684439d0) then    !Pi/6 < |result| < Pi/3
                                                !Use 2.*(dhead,dtail)^2-1 as arg.
         h=dhead*dhead                          !careful computation of
         h2=dhead*dhead-h                       !square of original argument
         g2=(2.0*dhead)*dtail
         g3=(2.0*dhead)*dtail-g2+dtail*dtail
         t=h2+g2                                !sum of middle parts
         sq=h+t
         if (abs(h2) > abs(g2)) then            !More than 107 bits are needed,
           g4=h2-t+g2+g3                        !because the square will be
         else                                   !subtracted from small constants,
           g4=g2-t+h2+g3                        !causing leading bit cancel-
         end if                                 !lations, which must be filled
         sq2=(h-sq+t)+g4                        !in from the right
         sq3=(h-sq+t)-sq2+g4                    !This captures extra l.o. bits
         temp=2.0*sq-1.0
         temp2=2.0*sq-(1.0+temp)+2.0*sq2
         arg=temp+temp2
         argl=temp-arg+temp2+2.0*sq3
         temp=2.0*arg*argl
         argsq=arg*arg+temp                     !Square of new argument
         argsq2=arg*arg-argsq+temp
                                                !Compute result as
                                                !Pi/4-0.5*asin(arg,argl)
                                                !Compute asin with
         sum=asincoeff(1,50)                    !Taylor Series
         do i=49,NTERMS+2,-1                    !First half(approx) terms in
           sum=asincoeff(1,i)+sum*argsq         !working precison
         end do
         sumt=asincoeff(1,NTERMS+1)+sum*argsq
         suml=asincoeff(1,NTERMS+1)-sumt+sum*argsq
         sum=sumt
         do i=1, NTERMS                         !remaining terms in quad prec.
            temp=asincoeff(1,NTERMS-i+1)+sum*argsq
            suml=asincoeff(1,NTERMS-i+1)-temp+sum*argsq+
     +           (asincoeff(2,NTERMS-i+1)+sum*argsq2+
     +            suml*argsq)
            sum=temp
         end do
         prodlow=suml*argsq+sum*argsq2          !multiply by arg^2
         prod=sum*argsq+prodlow
         prodlow=sum*argsq-prod+prodlow
         temp2=prodlow*arg+prod*argl            !multiply by arg
         temp=prod*arg+temp2
         temp2=prod*arg-temp+temp2

         res=arg+temp                           !add arg for asin(arg)
         reslow=(arg-res+temp)                  !exact
         resmid=argl+temp2
         restiny=argl-resmid+temp2
         p=reslow+resmid                        !sum of middle terms
         q=reslow-p+resmid                      !possible residual
         reshi=res+p
         resbot=(res-reshi+p)+(q+restiny)       !prepare to subtract from pi/4
         resinf=(res-reshi+p)-resbot+(q+restiny)
*
         if (dhead > 0.0) then
            temp=piq                            !positive argument
            temp2 =piqb                         !acos(x)=pi/4-.5 asin(arg,arg1)
            temp3=piqc
         else
            temp=pi34                           !negative argument
            temp2=pi34b                         !acos(x)=3pi/4 +
            temp3=pi34c                         !0.5 asin(arg,argl)
            reshi=-reshi
            resbot=-resbot
            resinf=-resinf
         end if
         res=temp-0.5*reshi
         reslow=(temp-res-0.5*reshi)
          resmid=temp2-0.5*resbot
         if (abs(temp2) .gt. abs(0.5*resbot)) then
           restiny=temp2-resmid-0.5*resbot+temp3-0.5*resinf
         else
           restiny=temp2-(0.5*resbot+resmid)+temp3-0.5*resinf
         end if
         p=reslow+resmid
         q=reslow-p+resmid
         reshi=res+p
         resbot=(res-reshi+p)+(q+restiny)
         _qacos=dcmplx(reshi,resbot)
         discard=_setflm(%val(oldfpscr))
         return
       end if

*****************************************************************
*      The "large arguments",  .866 < abs(dhead,dtail) <= 1.0   *
*****************************************************************

       if (dhead < 0.0) then                    !Use absolute value of input
         temp=-dhead                            !to work only in the first
         temp2=-dtail                           !quadrant
       else                                     !For input x > 0.5, use identity
         temp=dhead                             !asin(x) =
         temp2=dtail                            !pi/2-2asin(sqrt((1-x)/2))
       end if
       h=0.5-0.5*temp                           !exact
       argsq=h-0.5*temp2
       argsq2=h-argsq-0.5*temp2                 !square of reduced arg, exact!
       if (dhead - 1.0 + dtail .eq. 0.0) then   !for input of 1.0, return zero
          _qacos = dcmplx(zero, zero)
          discard = _setflm(%val(oldfpscr))
          return
       end if
       _zroot=_q_sqrtsp(%val(argsq),%val(argsq2))    !The new argument
       sum=asincoeff(1,25)                      !Taylor Series
       do i=24,NTERMS/2+2,-1                    !First half(approx) terms in
          sum=asincoeff(1,i)+sum*argsq          !working precison
       end do
       sumt=asincoeff(1,NTERMS/2+1)+sum*argsq
       suml=asincoeff(1,NTERMS/2+1)-sumt+sum*argsq
       sum=sumt
       do i=1, NTERMS/2                         !remaining terms in quad prec.
          temp=asincoeff(1,NTERMS/2-i+1)+sum*argsq
          suml=asincoeff(1,NTERMS/2-i+1)-temp+sum*argsq+
     +         (asincoeff(2,NTERMS/2-i+1)+sum*argsq2+
     +          suml*argsq)
          sum=temp
       end do
       arg=dble(_zroot)
       argl=dimag(_zroot)
       prodlow=suml*argsq+sum*argsq2            !multiply by arg^2
       prod=sum*argsq+prodlow
       prodlow=sum*argsq-prod+prodlow
       temp2=prodlow*arg+prod*argl              !multiply by arg
       temp=prod*arg+temp2
       temp2=prod*arg-temp+temp2

       res=arg+temp                             !add arg for asin(arg)
       reslow=(arg-res+temp)                    !exact
       resmid=argl+temp2
       restiny=argl-resmid+temp2+sqrtextra
       p=reslow+resmid                          !sum of middle terms
       q=reslow-p+resmid                        !possible residual
       reshi=res+p
       resbot=(res-reshi+p)+(q+restiny)
       if (dhead > 0) then                      !for first quadrant angles,
          _qacos=(2.0*reshi,2.0*resbot)         !we are finished!
          discard=_setflm(%val(oldfpscr))
          return
       end if
       resinf=(res-reshi+p)-resbot+(q+restiny)  !for second quadrant need to
       res=pi-(2.0*reshi)                       !subtract from Pi.
       reslow=(pi-res-(2.0*reshi))
       resmid=pib-(2.0*resbot)
       if (pib > abs(2.0*resbot)) then
         restiny=pib-resmid-(2.0*resbot)+(pic-2.0*resinf)
       else
         restiny=pib-(resmid+(2.0*resbot))+(pic-2.0*resinf)
       end if
       p=reslow+resmid
       q=reslow-p+resmid
       reshi=res+p
       resbot=(res-reshi+p)+(q+restiny)
       _qacos=dcmplx(reshi,resbot)
       discard=_setflm(%val(oldfpscr))
       return

300    continue                                 !Tiny argument -- |x| < 2^-55
       res=pih
       reslow=- dhead
       resmid=pihb-dtail
       restiny=pihb-resmid-dtail+pihc
       p=reslow+resmid
       reshi=res+p
       if (abs(reslow) > abs(resmid)) then
          q=reslow-p+resmid
       else
          q=resmid-p +reslow
       end if
       resbot=(res-reshi+p)+(q+restiny)
       _qacos=dcmplx(reshi,resbot)
       discard=_setflm(%val(zero))
       return

* Entry point for C-language name and semantics
       entry acosl(%val(c_head), %val(c_tail))
       dhead = c_head
       dtail = c_tail
       c_function = 1
       goto 1018

       END
