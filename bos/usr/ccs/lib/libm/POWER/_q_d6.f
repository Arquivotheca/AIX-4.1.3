* @(#)62	1.1  src/bos/usr/ccs/lib/libm/POWER/_q_d6.f, libm, bos411, 9428A410j 1/9/91 07:53:09
* COMPONENT_NAME: LIBCCNV
*
* FUNCTIONS: _q_d6
*
* ORIGINS: 55
*
*                  SOURCE MATERIAL
*
* Copyright (c) ISQUARE, Inc. 1990

@process xflag(callby)

* NAME: _q_d6
*                                                                    
* FUNCTION: sextuple precision division
*
* RETURNS: high order two parts returned explicitly
*          as a quad number; low order part returned
*          in last argument

*****************************************
      double complex function   _q_d6(%val(a),%val(b),
     +                          %val(c),%val(d),%val(e),%val(f),
     +                          reslow)
      implicit real*8           (a-h,o-z)
      implicit logical*4        (l)
      implicit double complex   (_)
      real*8                    infinity
      parameter                 (tiny = z'0010000000000000')
      parameter                 (infinity=z'7ff0000000000000')
********************************************************
*                                                      *
*            This function computes the quotient:      *
*                                                      *
*                                (a,b,c)               *
*            frac1,frac2,frac3=-------------         *
*                                (d,e,f)               *
*                                                      *
********************************************************
      lx=(abs(d)<tiny)                  !look out for overflow on 1/d
      p=1.0/d                           !reciprocal of h.o. denominator term
      if(abs(d)<tiny) then
        r=a/d                           !if quotient is infinity,
        if (abs(r).eq.infinity) then    !stop process, and return
           reslow = 0.0d0               !properly signed infinity
           _q_d6=dcmplx(r,0.0d0)        !as the quotient
           return
        end if
      else
         q=a*p
         r=q+(a-q*d)*p                  !h.o. quotient term
      end if
      s=a-r*d                           !exact remainder
      t=r*e                             !second order remainder term
      l1=(abs(b) > abs(t))
      u=(r*e-t)+r*f                     !third order remainder term
      w=b-t                             !combine the
      y=s+w                             !second order terms
      l2=(abs(s) > abs(w))              !and catch bits that may fall
      if (abs(b) > abs(t)) then         !off the end to include in the
         x=b-w-t                        !third order term
      else
         x=b-(w+t)
      end if
      if (abs(s) > abs(w)) then
         z=s-y+w
      else
         z=w-y+s
      end if
      g=c-u+x+z                         !third order term
      if(abs(d)<tiny) then              !caution against tiny quotients, again
         o=y/d
         v=(y-o*d+g-(e*o +(f*o)))/d
      else
         h=y*p
         o=h+(y-h*d)*p                  !second order term
         v=(y-o*d+g-(e*o +(f*o)))*p     !third order quotient term
      end if
      rmid=o+v                          !start distillation of quotient
      rlow=o-rmid+v
      reshi=r+rmid
      resmid=(r-reshi+rmid)+rlow
      reslow=(r-reshi+rmid)-resmid+rlow
      _q_d6=dcmplx(reshi,resmid)
      return
      end
