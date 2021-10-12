* @(#)54	1.4  src/bos/usr/ccs/lib/libm/POWER/_qasin.f, libm, bos411, 9428A410j 10/1/93 17:09:23
* COMPONENT_NAME: LIBCCNV
*
* FUNCTIONS: _qasin, asinl
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

* NAME: _qasin
*                                                                    
* FUNCTION: quad precision inverse sine function
*
* RETURNS: inverse sine of argumene

@process xflag(callby)
*******************************************************************
*            PROGRAM: Quad-Precision INVERSE SINE (QASIN)         *
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
       DOUBLE COMPLEX FUNCTION _QASIN (%val(dHead), %val(dTail))
       implicit real*8          (a-h, o-z)
       implicit double complex  (_)
       logical*4                l1,l2,l3,l4,l5,l6

       double complex		asinl                 ! really REAL*16
       external			__set_errno128
       integer			c_function
       integer			EDOM
       parameter		(EDOM = 33)
       real*8			c_head, c_tail

       data         unit        /1.0/
       data         pih         /z'3FF921FB54442D18'/
       data         pihb        /z'3C91A62633145C07'/
       data         pihbm       /z'3C91A62633145C06'/
       data         pihc        /z'B91F1976B7ED8FBC'/
       data         piq         /z'3FE921FB54442D18'/
       data         piqb        /z'3C81A62633145C07'/
       data         piqc        /z'B90F1976B7ED8FBC'/
       data         toosmall    /z'3c80000000000000'/
       data         zero        /0.0d0/
       INCLUDE "_qasindata.f"
       PARAMETER                (NTERMS=23)      !numb of full precision terms
                                                 !in power series for ASIN

       c_function = 0			! Fortran name and semantics
 1018  continue

       oldfpscr=_setflm(%val(zero))
       l2=(abs(dhead) < toosmall)
       l1=(dhead > 0.0)                         !Tests to be done later
       l3=(dhead-1.0+dtail < 0.0)
       l4=(dhead+1.0+dtail > 0.0)
       if (abs(dhead) < toosmall) go to 300     !miniscule argument--
       l5=(abs(dhead) > 0.5)
       if (((dhead > 0.0) .and. (dhead-1.0+dtail <= 0.0)) .or.
     +   ((dhead < 0.0) .and. (dhead+1.0+dtail >= 0.0))) then
         !Argument is valid -- range reduce
         if (abs(dhead) > 0.5) go to 200        !large arguments are special

         temp=(2.0*dhead)*dtail
         argsq=dhead*dhead+temp
         argsq2=dhead*dhead-argsq+temp+
     +           ((2.0*dhead)*dtail-temp)
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
     +            (asincoeff(2,NTERMS-i+1)+sum*argsq2+
     +             suml*argsq)
            sum=temp
         end do
         prodlow=suml*argsq+sum*argsq2      !mult. by arg^2
         prod=sum*argsq+prodlow
         prodlow=sum*argsq-prod+prodlow
         temp2=prodlow*dhead+prod*dtail        !mult. by arg
         temp=prod*dhead+temp2
         temp2=prod*dhead-temp+temp2

         res=dhead+temp                        !except for argument
         reslow=(dhead-res+temp)               !exact
         resmid=dtail+temp2
         restiny=dtail-resmid+temp2
         p=reslow+resmid                    !sum of middle terms
         q=reslow-p+resmid                  !possible residual
         reshi=res+p
         resbot=(res-reshi+p)+(q+restiny)
         _qasin=dcmplx(reshi,resbot)
         discard=_setflm (%val(oldfpscr))
         return
       end if

       !If control reaches here, argument
       !is not in the domain of this function
       discard=_setflm(%val(oldfpscr))

       if (dhead .ne. dhead) then
          _qasin=dcmplx(dhead+unit, zero)
       else
*     For C-language semantics, set errno = EDOM
          if (c_function .eq. 1) then
             call __set_errno128(%VAL(EDOM))
          end if
          _qasin=dcmplx(zero/zero, zero)   !return a nan, and cause invalid
       end if
       return                                !operation to be set


200    if (abs(dhead) < 0.866025403684439d0) then !  Pi/6 < |result| < Pi/3
                                          !Use 1-2.*(dhead,dtail)^2 as arg.
         h=dhead*dhead                !careful computation of
         h2=dhead*dhead-h           !square of original argument
         g2=(2.0*dhead)*dtail
         g3=(2.0*dhead)*dtail-g2+dtail*dtail
         t=h2+g2                      !sum of middle parts
         sq=h+t
         if (abs(h2) > abs(g2)) then      !More than 107 bits are needed,
           g4=h2-t+g2+g3          !because the square will be
         else                             !subtracted from small constants,
           g4=g2-t+h2+g3          !causing leading bit cancellations,
         end if                           !which must be filled in from the
         sq2=(h-sq+t)+g4          !right
         sq3=(h-sq+t)-sq2+g4    !This captures extra low order bits
         temp=1.0-2.0*sq
         temp2=1.0-temp-2.0*sq-2.0*sq2
         arg=temp+temp2
         argl=temp-arg+temp2-2.0*sq3
         temp=2.0*arg*argl
         argsq=arg*arg+temp             !Square of new argument
         argsq2=arg*arg-argsq+temp
                                            !Compute result as
                                            !Pi/4-0.5*asin(arg,argl)
                                            !Compute asin with
         sum=asincoeff(1,50)              !Taylor Series
         do i= 49,NTERMS+2,-1             !First half(approx) terms in working
            sum=asincoeff(1,i)+sum*argsq  !precison
         end do
         sumt=asincoeff(1,NTERMS+1)+sum*argsq
         suml=asincoeff(1,NTERMS+1)-sumt+sum*argsq
         sum=sumt
         do i=1, NTERMS                   !remaining terms in quad precision
            temp=asincoeff(1,NTERMS-i+1)+sum*argsq
            suml=asincoeff(1,NTERMS-i+1)-temp+sum*argsq +
     +            (asincoeff(2,NTERMS-i+1)+sum*argsq2 +
     +             suml*argsq)
            sum=temp
         end do
         prodlow=suml*argsq+sum*argsq2      !mult. by arg^2
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
         resbot=(res-reshi+p)+(q+restiny)       !get ready to subtract from pi/4
         resinf=(res-reshi+p)-resbot+(q+restiny)
*
         temp=piq
         temp2 =piqb
         temp3=piqc
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
         if (dhead > 0.0) then
           _qasin=dcmplx(reshi,resbot)
         else
           _qasin=dcmplx(-reshi,-resbot)
         end if
         discard=_setflm(%val(oldfpscr))
         return
       end if
************************************************************
*  The "large arguments",  .866 < abs(dhead,dtail) <= 1.0  *
************************************************************
       if (dhead < 0.0) then                    !Use absolute value of input
         temp=-dhead                            !to work only in the first
         temp2=-dtail                           !quadrant
       else                                     !For arg x > 0.5, use identity
         temp=dhead                             !asin(x)=pi/2-2asin((1-x)/2)^.5)
         temp2=dtail
       end if
       h=0.5-0.5*temp                           !exact
       argsq=h-0.5*temp2
       argsq2=h-argsq-0.5*temp2                 !square of reduced arg, exact
       _zroot=_qsqrt(%val(argsq),%val(argsq2))  !The new argument
       sum=asincoeff(1,25)                      !Taylor Series
       do i= 24,NTERMS/2+2,-1                   !1st half(approx) terms in wp
          sum=asincoeff(1,i)+sum*argsq          !precison
       end do
       sumt=asincoeff(1,NTERMS/2+1)+sum*argsq
       suml=asincoeff(1,NTERMS/2+1)-sumt+sum*argsq
       sum=sumt
       do i=1, NTERMS/2                         !remaining terms in quad
          temp=asincoeff(1,NTERMS/2-i+1)+sum*argsq
          suml=asincoeff(1,NTERMS/2-i+1)-temp+sum*argsq+
     +          (asincoeff(2,NTERMS/2-i+1)+sum*argsq2+
     +           suml*argsq)
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
       restiny=argl-resmid+temp2
       p=reslow+resmid                          !sum of middle terms
       q=reslow-p+resmid                        !possible residual
       reshi=res+p
       resbot=(res-reshi+p)+(q+restiny)         !get ready to subtract from pi/4
       resinf=(res-reshi+p)-resbot+(q+restiny)

       res=pih-(2.0*reshi)
       reslow=(pih-res-(2.0*reshi))
       resmid=pihb-(2.0*resbot)
       if (pihb > abs(2.0*resbot)) then
         restiny=pihb-resmid-(2.0*resbot)+(pihc-2.0*resinf)
       else
         restiny=pihb-(resmid+(2.0*resbot))+(pihc-2.0*resinf)
       end if
       p=reslow+resmid
       q=reslow-p+resmid
       reshi=res+p
       resbot=(res-reshi+p)+(q+restiny)
       if (dhead < 0) then
         _qasin=dcmplx(-reshi,-resbot)
       else
         _qasin=dcmplx(reshi,resbot)
       end if
       discard=_setflm(%val(oldfpscr))
       return

300    _qasin=dcmplx(dhead, dtail)              !arcsin(x) approx x, |x| < 2^-55
       discard=_setflm(%val(zero))
       return

* Entry point for C-language name and semantics
       entry asinl(%val(c_head), %val(c_tail))
       dhead = c_head
       dtail = c_tail
       c_function = 1
       goto 1018

       end
