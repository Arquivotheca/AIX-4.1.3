* @(#)59	1.5  src/bos/usr/ccs/lib/libm/POWER/_qcosh.f, libm, bos411, 9428A410j 10/1/93 17:09:31
* COMPONENT_NAME: LIBCCNV
*
* FUNCTIONS: _qcosh, coshl
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

* NAME: _qcosh
*                                                                    
* FUNCTION: quad precision hyperbolic cosine
*
* RETURNS: quad precision hyperbolic coseine

*******************************************************************
*            PROGRAM: Quad-Precision Hyberbolic COSINE            *
*            AUTHOR:  ISQUARE, Inc., (V. Markstein)               *
*            DATE:    5/6/90                                      *
*            MOD:     9/8/90  customized for RS/6000 performance  *
*            NOTICE:  Copyright (c) ISQUARE, Inc. 1990            *
*                                                                 *
*                     Results are returned by VALUE               *
*                                                                 *
*******************************************************************
*                                                                 *
*            CALLING: _setflm                                     *
*                                                                 *
*******************************************************************

       DOUBLE COMPLEX FUNCTION _QCOSH(%val(head), %val(tail))
       implicit real*8          (a-h,o-y)
       implicit double complex  (z,_)
*
       real*8                   coeff(9) / 306.0,
     +                                73440.0d0, 13366080.0d0,
     +                           1764322560.0d0, 158789030400.0d0,
     +                        8892185702400.0d0, 266765571072000.0d0,
     +                     3201186852864000.0d0, 6402373705728000.0d0/
       real*8   recip         /1.561920696858622646221636435005d-16 /
*
       parameter                (botcut = 0.125)
       parameter                (topcut = 40.0)
       logical                  l1,l2,l3,l4

       real*8                   head, tail
       double complex		coshl            ! really REAL*16
       external			__set_errno128
       integer			c_function
       integer			ERANGE
       parameter		(ERANGE = 34)
       real*8                   infinity
       data 		        infinity  /z'7ff0000000000000'/
*
       c_function = 0			! Fortran name and semantics
 1018  continue

       arg = head
       arg2 = tail
       l4=(arg < 0)
       l1=abs(arg) > botcut
       l2=abs(arg) > topcut
       l3=(arg .eq. arg)
       if (abs(arg) > botcut) then      !Good use of exp
          if (arg < 0) then
             argx=- arg
             arg2x=- arg2
          else
             argx=arg
             arg2x=arg2
          end if
          l1=(abs(arg) > topcut)
          zres=_qhexp(%val(argx),%val(arg2x), extraword)  !get .5*e^x
          if (abs(arg) > topcut) then   !only one evaluation of exp.

*     For C-language semantics, we must set errno to ERANGE
*     in case of overflow
             if (c_function .eq. 1) then
                if ((abs(dble(zres)) .eq. infinity) .and.
     x               (abs(arg) .ne. infinity)) then
                   call __set_errno128(%VAL(ERANGE))
                end if
             end if

             _qcosh=zres
          else
             t1=dble(zres)              !zres=.5*e^x
             t2=dimag(zres)             !def. of cosh: (e^x + e^-x)/2
             r1=0.25/t1
             residual=(0.25d0-t1*r1)-t2*r1
             r2=residual*(4.0d0*r1)     !(r1,r2)=.5*e^-x + errors< 4 ulps
             residual1=(0.25d0-t1*r1)-residual-t2*r1
                                        !rest of rem. from HO divisor only
             r3=(t1*r2-residual)        !(reversed sign)...LO quotient
             r4=(extraword*r1+(t2*r2))  !(reversed sign)...LO quotient
             reshi=t1+r1
             reslow=t2+r2
             residual=(t1-reshi)+r1     !exact
             if (abs(t2) .gt. abs(r2)) then
               residual2=(t2-reslow)+r2 !exact
             else
               residual2=r2-reslow+t2   !exact
             end if
             r5=(r3+r4-residual1)*(4.0d0*r1)    !reversed sign
             resnew=reshi+reslow
             residual3=reshi-resnew+reslow      !exact
             residual4=residual+residual3
             reshi=resnew+residual4
             residual5=residual-residual4+residual3
             residual6=resnew-reshi+residual4
             reslow=(residual2+(extraword-r5))+residual5+residual6
             _qcosh=dcmplx(reshi,reslow)
          end if
       else
          if (arg .ne. arg) then
             _qcosh=dcmplx(head+botcut,0.0d0)
          else
             temp=2.0d0*arg*arg2        !small argument
             argsq=arg*arg+temp         !use power series
             argsq2=arg*arg-argsq+temp+arg2*arg2
             sum=1.0d0
             suml=0.0d0
             sum=coeff(1)+argsq*sum
             sum=coeff(2)+argsq*sum
             sum=coeff(3)+argsq*sum
             do i=4,8
                templow=suml*argsq+sum*argsq2
                temp=sum*argsq+templow
                bottom=sum*argsq-temp+templow
                sum=coeff(i)+temp
                residual=coeff(i)-sum+temp
                suml=bottom+residual
             end do
             prodlow=suml*argsq+sum*argsq2      !last mult. by arg^2
             prod=sum*argsq+prodlow
             prodlow=sum*argsq-prod+prodlow
             sum=prod*recip                     !sum of series ---
             remainder=(prod-sum*coeff(9))
             partial=remainder+prodlow
             residual=remainder-partial+prodlow
             suml=partial*recip+(residual*recip)
             res=1.0d0+sum                      !except for 1.0 term.
             reslow=1.0d0-res+sum+suml
             _qcosh=dcmplx(res,reslow)
          end if
       end if
       return

* Entry point for C-language name and semantics
       entry coshl(%val(head), %val(tail))
       c_function = 1
       goto 1018

       end
