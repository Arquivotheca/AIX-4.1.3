* @(#)82	1.4  src/bos/usr/ccs/lib/libm/POWER/_qpow.f, libm, bos411, 9428A410j 10/1/93 17:09:41
* COMPONENT_NAME: LIBCCNV
*
* FUNCTIONS: _qpow, powl
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

* NAME: _qpow
*                                                                    
* FUNCTION: power
*
* RETURNS: arg 1 raised o arg 2 power

@process xflag(callby)
*******************************************************************
*            PROGRAM: QUAD-PRECISION POWER                        *
*            AUTHOR:  ISQUARE, Inc., (V. Markstein)               *
*            DATE:    4/3/90                                      *
*            MODIFIED:9/30/90 customized for RS/6000 performance  *
*                                                                 *
*            NOTICE:  Copyright (c) ISQUARE, Inc. 1990            *
*                     The arguments (alpha,beta) and (gamma,delta)*
*                         are DOUBLE COMPLEX, or REAL*16          *
*                                                                 *
*                     Results are returned by VALUE               *
*                                                                 *
*******************************************************************
*                                                                 *
*            CALLING: _setflm, _qlog, _qexp                       *
*                                                                 *
*******************************************************************

       DOUBLE COMPLEX FUNCTION _QPOW(%val(alpha),%val(beta),
     +                               %val(gamma),%val(delta))
       implicit real*8          (a-h,o-y)
       implicit double complex  (Z,_)
       real*8                   _setflm,infinity,zero,one
       real*8                   alpha,beta,gamma,delta,extraword
       logical*4                l1,l2,l3,l4,l5
       data                     big /z'4330000000000000'/
       data                     infinity /z'7ff0000000000000'/
       data                     zero /0.0d0/
       data                     one  /1.0d0/

       double complex		powl             ! really REAL*16
       external			__set_errno128
       integer			c_function
       integer			EDOM
       parameter		(EDOM = 33)
       integer			ERANGE
       parameter		(ERANGE = 34)
       real*8                   c_alpha,c_beta,c_gamma,c_delta
       real*8			q_nan
       data                     q_nan /z'7ff8000000000000'/

*
       c_function = 0			! Fortran name and semantics
 1018  continue

       l1=(gamma .eq. zero)
       l2=(alpha+beta .eq. one)
       l3=(alpha .lt. zero)
       l4=(alpha .eq. infinity)
       flag=one                              !Later, multiply result by flag
       if (gamma+delta .eq. zero) then          !anything ** 0 -> 1, execpt a NaN!
D         _qpow=dcmplx(one, zero)            !Compile with -D to return 1
D         return                                !even for NaN ** 0 (Kahan
                                                !recommendation).
          if ((alpha .eq. alpha) .and. (beta .eq. beta)) then
            _qpow=dcmplx(one, zero)          !IBM specification:
          else                                 !NaN**0 -> NaN
            _qpow=(alpha+beta, zero)         !
          end if
          return
       else
          if (alpha+beta .eq. one) then            !1 ** anything -> 1
             if (gamma .eq. gamma) then            !except 1 ** NaN -> NaN
                if (abs(gamma) .ne. infinity) then !and 1 ** INF -> NaN
                   _qpow=dcmplx(one, zero)
                else
                   _qpow=dcmplx(zero/zero, zero)
                endif
             else
                _qpow=dcmplx(gamma+delta,zero)
             end if
             return
          end if
       end if
       l5 = (gamma .eq. two)
       if (alpha .lt. zero) then
          c=abs(gamma)                       !negative bases only work for
          d=abs(delta)                       !integer powers
          if (((c > big) .or. (c+big-big .eq. c)) .and.
     +       ((d > big) .or. (d+big-big .eq. d))) then
                                             !base is neg., exp. is an integer.
            alpha=- alpha                    !force base to be positive
            beta=-beta
            c=0.5*c
            d=0.5*d                          !test for even exponent
            if (((c < big) .and. (c+big-big .ne. c)) .or.
     +         ((d < big) .and. (d+big-big .ne. d))) then
                                             !For odd exp. -> result negative
               flag=-one
            end if
            if (alpha .eq. infinity) then
               if ((gamma .ne. gamma) .or. (delta .ne. delta)) then
                  _qpow=dcmplx(gamma-gamma,zero)   !infinity**NaN->NaN
               elseif (gamma + delta .gt. zero) then !-Infinity** + integer
                  _qpow=dcmplx(flag*infinity,flag*zero)
               else                              !-Infinity* - integer
                  _qpow=dcmplx(flag*zero, flag*zero)
               end if
               return
            else
*            If here, x is negative and y is not an integer
*            set EDOM, return NaN
               if (c_function .eq. 1) then
                  _qpow = dcmplx(zero / zero, zero)
                  call __set_errno128(%val(EDOM))
                  return
               end if
            end if
         end if
      else if (alpha .eq. zero) then
         if (gamma > zero) then              !0^pos# = 0
            _qpow=dcmplx(zero, zero)
            return
         end if
         if (gamma < zero) then              !0^neg# =infinity
            _qpow=dcmplx(big/zero,zero)
            return
         end if
      else if (alpha .eq. infinity) then
         if ((gamma .ne. gamma) .or. (delta .ne. delta)) then
            _qpow=dcmplx(gamma-gamma,zero)   !infinity**NaN->NaN
         else
            if (gamma + delta > zero) then   !infinity**positive power
               _qpow=dcmplx(alpha,zero)      !return infinity
            else
               _qpow=dcmplx(zero,zero)       !infinity**negative power
                                             !return zero
            end if
         end if
         return
      end if
*********************************************************************
*          End of SPECIAL CASES... Next are ORDINARY CASES          *
*    SPECIAL ENTRY POINT to _QLOG which  returns -->                *
*      extra result bits it the last argument, precise to 115+ bits *
*********************************************************************

      if (gamma .eq. two) then
         if (delta .eq. zero) then           !real**1 = real
            temp = (two * alpha) * beta
            powhi = alpha * alpha + temp
            powlow = alpha * alpha - powhi + temp +
     x               ((two*alpha)*beta-temp)
            _qpow = dcmplx(powhi,powlow)
            return
         end if
      end if
      z1=_qlogextra(%val(alpha),%val(beta),extra)
      oldfpscr=_setflm(%val(infinity))       !preserve user fpscr, disable traps
      ahi=dble(z1)*gamma                     !careful product of quad prec.
      l5=(abs(ahi).eq.infinity)              !
      amid=dble(z1)*gamma-ahi                !expo (gamma,delta), by the 114+
      amid2=dimag(z1)*gamma                  !bit value returned by _qlogextra
      l1=(abs(amid) .gt. abs(amid2))         !<compiler scheduling directive>
      alow=dimag(z1)*gamma-amid2+gamma*extra !(z1, extra)
      amid3=dble(z1)*delta
      alow=dble(z1)*delta-amid3+alow+delta*dimag(z1)
      amid4=amid+amid2
      ahi1=ahi+amid4
      amid5=ahi-ahi1+amid4
      l2=(abs(amid3) .gt. abs(amid5))        !<compiler scheduling directive>
      if (abs(amid) .gt. abs(amid2)) then
         alow=amid-amid4+amid2+alow
      else
         alow=amid2-amid4+amid+alow
      end if
      amid6=amid3+amid5
      ahi2=ahi1+amid6
      if (abs(amid3) .gt. abs(amid5)) then
         alow=amid3-amid6+amid5+alow
      else
         alow=amid5-amid6+amid3+alow
      end if
      amid7=ahi1-ahi2+amid6
      amid8=amid7+alow
      alow=amid7-amid8+alow                  !All ready to call _qexp_x
      if (abs(ahi).eq.infinity) then         !but if arg to exp is too large,
         if (ahi .gt. zero) then
            if (c_function .eq. 1) then
               call __set_errno128(%val(ERANGE))
            end if
            zed=dcmplx(infinity,zero)
         else
            zed=dcmplx(zero,zero)
         end if
         powhi = dble(zed)
         powlow= dimag(zed)
      else
         zed=_qexp_x(%val(ahi2),%val(amid8),%val(alow),discard,
     +        %val(zero))
         powhi = dble(zed)
         powlow= dimag(zed) + discard       !In case close to integer, favor
         if (abs(powlow) < powhi*2.0d0**(-108)) then    !integer
            powlow = 0.0d0
         end if

      end if

      discard=_setflm(%val(oldfpscr))
      _qpow=dcmplx(flag*powhi,flag*powlow)
      return

* Entry point for C-language name and semantics
      entry powl(%val(c_alpha),%val(c_beta),
     +     %val(c_gamma),%val(c_delta))
      alpha = c_alpha
      beta = c_beta
      gamma = c_gamma
      delta = c_delta
      c_function = 1
      goto 1018

      END
