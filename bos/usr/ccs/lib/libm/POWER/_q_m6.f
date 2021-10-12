* @(#)63	1.1  src/bos/usr/ccs/lib/libm/POWER/_q_m6.f, libm, bos411, 9428A410j 1/9/91 07:53:26
* COMPONENT_NAME: LIBCCNV
*
* FUNCTIONS: _q_m6
*
* ORIGINS: 55
*
*                  SOURCE MATERIAL
*
* Copyright (c) ISQUARE, Inc. 1990

@process debug(callby)

* NAME: _q_m6
*                                                                    
* FUNCTION: sextuple precision multiply
*
* RETURNS: high order parts are returned explicity as
*          a quad.  the low order parts are returned in
*          argument 'extra'.

*******************************************************************
*            PROGRAM: Sextuple-Precision Multiplication           *
*            AUTHOR:  ISQUARE, Inc (V. Markstein)                 *
*            DATE:    5/4/90                                      *
*            MOD:     6/18/90 C --> FORTRAN                       *
*                     11/27/90 port --> RS/6000                   *
*            NOTICE:  Copyright (c) ISQUARE, Inc. 1990            *
*                                                                 *
*                     Results are returned by VALUE, and the      *
*                     last parameter                              *
*                                                                 *
*******************************************************************
       double complex function  _q_m6(%val(f1),%val(f2),%val(f3),
     +                         %val(a),%val(b),%val(c),extra)
       implicit real*8          (a-h,o-z)
       implicit double complex  (_)
       logical                  l1,l2,l3,l4
       u=a*f3+b*f2+c*f1                 !sextuple precision multiplication
       v=a*f2                           !derived from _q_mp10.c
       w=a*f2-v
       x=b*f1
       y=b*f1-x
       l1=(abs(x) > abs(v))
       e=w+y+u
       z=a*f1
       w=a*f1-z
       u=v+x
       l2=(abs(w) > abs(u))
       if (abs(x) > abs(v)) then
          e=x-u+v+e
       else
          e=v-u+x+e
       end if
       f=u+w
       if (abs(w) > abs(u)) then
          extra=w-f+u+e
       else
          extra=u-f+w+e
       end if
       resbump=f+extra
       extra=f-resbump+extra
       reshi=z+resbump
       resmid=(z-reshi+resbump)+extra
       extra=(z-reshi+resbump)-resmid+extra
       _q_m6=dcmplx(reshi,resmid)
       return
       end

