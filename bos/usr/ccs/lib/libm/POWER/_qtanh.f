* @(#)68	1.3  src/bos/usr/ccs/lib/libm/POWER/_qtanh.f, libm, bos411, 9428A410j 9/24/93 10:54:40
* COMPONENT_NAME: LIBCCNV
*
* FUNCTIONS: _qtanh, tanhl
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

* NAME: _tanh
*                                                                    
* FUNCTION: quad precision hyperbolic tanget
*
* RETURNS: hyperbolic tangent of argument

@process xflag(callby)
*******************************************************************
*            PROGRAM: Quad-Precision Hyberbolic TANGENT           *
*            AUTHOR:  ISQUARE, Inc., (V. Markstein)               *
*            DATE:    6/02/90                                     *
*            MOD:     9/13/90  customized for RS/6000 performance *
*            NOTICE:  Copyright (c) ISQUARE, Inc. 1990            *
*                                                                 *
*                     Results are returned by VALUE               *
*                                                                 *
*******************************************************************
*                                                                 *
*            CALLING: _setflm                                     *
*******************************************************************

       DOUBLE COMPLEX FUNCTION _QTANH(%val(head), %val(tail))
       implicit double complex  (z, _)
       implicit real*8          (a-h,o-y)
       parameter                (botcut=2.0)
       parameter                (topcut=40.0)
       parameter                (petitcut=0.0625)
       real*8                   tanhtbl(3,0:32)
       real*8                   coeff(11,2)
       integer*4                idum(2)
       equivalence              (fltndx,idum), (idum(2), ndx)
       parameter                (big=2.0**52 + 2.0**40)

       real*8                   head,tail
       double complex		tanhl
       integer			c_function

       logical                  l1,l2,l3,l4
       data                     ((tanhtbl(j,i),j=1,3),i=0,32)    /
     +   z'0000000000000000',z'0000000000000000',z'0000000000000000',
     +   z'3FB0000000030807',z'3FAFF55997E63AD9',z'BB6CF9C02437818A',
     +   z'3FC000000000284A',z'3FBFD5992BC5078A',z'BB77F9D4704D912A',
     +   z'3FC8000000005880',z'3FC7B8FF903C4CEC',z'3B47716FB54B60B5',
     +   z'3FD0000000000CDC',z'3FCF597EA69A34B3',z'BB794C01C101435A',
     +   z'3FD400000000313C',z'3FD35F98A0EA91C7',z'BB904F6326BD63B6',
     +   z'3FD80000000021EA',z'3FD6EF53DE8CAD3F',z'BB91172E8FAFB7A2',
     +   z'3FDC000000005214',z'3FDA5729EE48C464',z'3B9543D8C4943292',
     +   z'3FE0000000001E86',z'3FDD9353D756BAF6',z'BB8F03A034F7B469',
     +   z'3FE2000000007115',z'3FE05086F2F72867',z'BB95D9183E3269EE',
     +   z'3FE4000000005720',z'3FE1BF47EABBCBE9',z'3B918CCEA6F1FA3A',
     +   z'3FE60000000021CF',z'3FE3157DFE9F8724',z'BBA1AC2C2B395168',
     +   z'3FE8000000016908',z'3FE45323E553C98B',z'BBA14AF9B571AFA7',
     +   z'3FEA000000005202',z'3FE5788FF10D56AF',z'BBA3C229C063AD80',
     +   z'3FEC000000005742',z'3FE686650B8C4C1B',z'3B8BE9FCF1B4428B',
     +   z'3FEE000000008582',z'3FE77D838E34430D',z'3B829FA7F1623176',
     +   z'3FF0000000005F76',z'3FE85EFAB51543C3',z'BB4B35922498C785',
     +   z'3FF1000000001F8D',z'3FE92BFB370DB380',z'3BA29188377121C3',
     +   z'3FF200000000130F',z'3FE9E5CB5BA45A90',z'3B7DFB4EF67C44E2',
     +   z'3FF3000000004F18',z'3FEA8DBCBC31BABE',z'3B99C45DAE5D7827',
     +   z'3FF4000000005131',z'3FEB2523BB6B5B77',z'3B85A025161129AB',
     +   z'3FF50000000029D4',z'3FEBAD50A4A6A0D5',z'BB98300772D2D609',
     +   z'3FF60000000007E1',z'3FEC278A52A4E807',z'3BA451F731DE764B',
     +   z'3FF700000000185B',z'3FEC950A3340D299',z'3BAABF14E874463C',
     +   z'3FF80000000024C4',z'3FECF6F9786E02C1',z'3B6B3F8E2446EA6B',
     +   z'3FF900000000BF0C',z'3FED4E6F4642C44F',z'3BA657E4DE96E741',
     +   z'3FFA000000006986',z'3FED9C6FAFE63ACE',z'3BA71F1BC2E3ED65',
     +   z'3FFB000000003690',z'3FEDE1EB59375F86',z'3B94D79808425B54',
     +   z'3FFC0000000068CA',z'3FEE1FBF97E34D01',z'3BAD07C7A6BF9B47',
     +   z'3FFD000000008A24',z'3FEE56B6F3EFC7EE',z'BBA26658A2EB55F0',
     +   z'3FFE00000003C04A',z'3FEE8789ECECBA51',z'BBA8B27F9026BBD4',
     +   z'3FFF000000000D68',z'3FEEB2DFEDD5EEB6',z'3B9EE74B12149464',
     +   z'4000000000004DFD',z'3FEED9505E1BD9DE',z'3B971596795B2F55'
     +   /
       data                     ((coeff(i,j),j=1,2),i=1,11) /
     +     z'3FF0000000000000',   z'0000000000000000',
     +     z'BFD5555555555555',   z'BC75555555555554',
     +     z'3FC1111111111111',   z'3C41111110FF0F6D',
     +     z'BFABA1BA1BA1BA1C',   z'3C47917AA287E6B6',
     +     z'3F9664F4882C10FA',   z'BC0A8F5F684BD9FF',
     +     z'BF8226E355E6C238',   z'3C01097B425ED098',
     +     z'3F6D6D3D0E154787',   z'BC0BA781948B0FCF',
     +     z'BF57DA36446C8BDA',   z'BBD2F684BDA12F6B',
     +     z'3F435580BCDA12F7',   z'BBEED097B425ED0A',
     +     z'BF2F545A781948B1',   z'3B7948B0FCD6E991',
     +     z'3F17B29161F9ADD4',   z'BBAF9ADD3C0CA458'
     +     /

       c_function = 0			! Fortran name and semantics
 1018  continue

       l1=head .lt. 0.0
       l2=abs(head) .gt. topcut
       l3=abs(head) .lt. botcut
       l4=abs(head) .lt. petitcut
       a= head
       b= tail
       if (head < 0.0) then       !abs val of 2arg
          a=-a
          b=-b
       endif
       if (abs(head) < botcut) go to 1000 !small arg-special case
*****************************************************************************
*                                                                           *
*                         1.0                                               *
*      tanh(x)= 1 -  -------------           |x| > 2.0                      *
*                    .5 +.5*e^(2x)                                          *
*                                                                           *
*****************************************************************************
       if (abs(head) .gt. topcut) then    !results = 1, for big args.
          if (head < 0.0) then            !sign of result= sign of arg
             _qtanh=dcmplx(-1.0d0, 0.0d0)
          else
             _qtanh=dcmplx(1.0d0, 0.0d0)
          end if
          return
       end if
       a=2.0*a                                  !double arg
       b=2.0*b
       zexp=_qhexp(%val(a),%val(b),extra)       !compute e^(2*arg)
       az=dble(zexp)
       bz=dimag(zexp)
       temp=az+0.5                              !az >= 0.5
       carry=az-temp+0.5                        !exact
       temp1=carry+bz
       carry1=carry-temp1+bz
       den=temp+temp1
       den1=temp-den+temp1+carry1
       r=1.0/den                                !guess recip. of denominator
       rem=1.0-den*r                            !first remainder
       rem1=rem-r*den1                          !(rem1,rem2) is better rem.
       if (abs(rem) .gt. abs(r*den1)) then
          rem2=rem-rem1-r*den1
       else
          rem1=rem-(r*den1)
          rem2=-(r*den1)-rem1+rem -(r*den1-(r*den1))
       end if
       qq=r+r*rem1                              !going for the full quotient
       qa=r-qq+r*rem1
       qb=qa+r*rem2
       q=qq+qb
       qc=qa-qb+r*rem2-(extra*r)*r
       q1=qq-q+qb+qc
       res=1.0-q                                !high order result
       res2=1.0-res-q                           !exact low order
       res3=res2-q1
       resfin=res+res3
       res4=res2-res3-q1
       res5=res-resfin+res3
       reslow=res4+res5
       if (head < 0) then
          resfin=-resfin
          reslow=-reslow
       end if
       _qtanh=dcmplx(resfin, reslow)
       return
****************************************************************************
*                                                                          *
*     Accurate table is used to reduce small arguments such that the       *
*     range of the |reduced argument| < 1/32. The tanh addition formula    *
*     is used to piece together three tanhs                                *
*                                                                          *
****************************************************************************

 1000  fltndx=big+a*16.0                !compute table lookup index
       if (abs(head) < petitcut) ndx=0
       arg1=a-tanhtbl(1,ndx)            !reduce argument by table value
                                        !which is close to ndx/16 (exact)
       arg=arg1+b                       !full 53 bit argument
       arg2=arg1-arg+b                  !low order argument
*******************************************************************************
*     The argument has been broken up as follows:                             *
*                   Zarg=tanhtbl(1,ndx)+arg+arg2                              *
*                                                                             *
*     tanh(tanhtbl(1,ndx)) is read from tantbl(2,ndx),tantbl(3,ndx)           *
*         with at least 122 bits of precision                                 *
*                   tanh(arg2)=arg2+o(2^-172)                                 *
*                                                                             *
*     Compute tanh(arg) by economized power series.                           *
*     Then the task is to piece together the three parts!                     *
*                                                                             *
*                              (tanh(b)+tanh(c))(1-tanh^2(a))                 *
* tanh(a+b+c)=tanh(a) +   ------------------------------------------          *
*                         1+tanh(a) tanh(b)+tanh(c)(tanh(a)+tanh(b))          *
*                                                                             *
*******************************************************************************
      sum=coeff(11,1)
      suml=coeff(11,2)
      argsq=arg*arg
      argsq2=arg*arg-argsq              !arg^2 for series
      do i=10,2,-1
         pl=suml*argsq+sum*argsq2
         prod=sum*argsq+pl
         prodlow=sum*argsq-prod+pl
         sum=coeff(i,1)+prod            !add in the next coefficient
         sum2=coeff(i,2)+prodlow
         suml=coeff(i,1)-sum+prod+sum2
      end do

      pl=suml*argsq+sum*argsq2
      temp=sum*argsq+pl
      templow=sum*argsq-temp+pl
      pl=templow*arg                    !last multiplication by arg
      prodx=temp*arg+pl
      prodlowx=temp*arg-prodx+pl        !tanh(arg)-1.0  is done.
      prod=arg+prodx
      prodlow=arg-prod+prodx+prodlowx   !tanh(arg) is done.
                                        !Let's do it! the outrageous formula
      if (abs(head) < petitcut) then      !trivial: tanh(a)=0 in formula
         proderr=(arg-prod+prodx)-prodlow+prodlowx
         bump=arg2-prod*prod*arg2
         reslow=prodlow+bump
         residual=prodlow-reslow+bump+proderr
         res=reslow+prod
         reslow=prod-res+reslow+residual
         if (head < 0) then
            res=-res
            reslow=-reslow
         end if
         _qtanh=dcmplx(res,reslow)
         return

      end if
      tiny=arg2*(tanhtbl(2,ndx)+prod)   !The last addend in denominator
      pl=tanhtbl(2,ndx)*prodlow+tanhtbl(3,ndx)*prod  !tanh(a)*tanh(b)
      den=tanhtbl(2,ndx)*prod+pl        !denominator completed except
      den1=tanhtbl(2,ndx)*prod-den+pl+tiny           !for the +1 term.
      suml2=prodlow+arg2                !starting on numerator
      t2=2.0*tanhtbl(2,ndx)*tanhtbl(3,ndx)           !table look up value
      tsq=tanhtbl(2,ndx)*tanhtbl(2,ndx)+t2           !squared.
      tsql=tanhtbl(2,ndx)*tanhtbl(2,ndx)-tsq+t2
      t3=2.0*tanhtbl(2,ndx)*tanhtbl(3,ndx)-t2 +
     +                           tanhtbl(3,ndx)*tanhtbl(3,ndx)
      denf=1.0+den                      !denominator is really done now
      denf2=1.0-denf+den+den1
      fac=1.0-tsq                       !computing 1-tanhtble(ndx)^2 becomes
      residual=1.0-fac-tsq              !(fac, fac2)
      fac2=residual-tsql-t3
      pl=fac*suml2+fac2*prod            !(b+c)(1-a*a) ... above formula
      top=fac*prod+pl
      top2=fac*prod-top+pl              !doing division
      recip=1.0/denf
      quot=top*recip
      ra=top-quot*denf                  !3 part remainder:
      rb=-(quot*denf2)                  !rem=ra+rb+top2
      rc=-(quot*denf2+rb)               !3rd order remainder
      rd=top2+rb                        !summing 2nd order rems. into
      if (abs(top2) > abs(rb)) then     ! rf
         re=top2-rd+rb
      else
         re=rb-rd+top2
      end if
      rf=ra+rd
      if (abs(ra) > abs(rd)) then
         rg=ra-rf+rd                    !more 3rd order rems.
      else                              !rc+re+rg
         rg=rd-rf+ra
      end if
      quot2=rf*recip
      quot3=rf*recip-quot2
      frac=quot+quot2
      frac2=quot-frac+quot2+(quot3+recip *(rc+re+rg))


      ansx=tanhtbl(2,ndx)+frac          !pasting together the result
      ansl=tanhtbl(3,ndx)+frac2
      ansmid=tanhtbl(2,ndx)-ansx+frac
      anstiny=tanhtbl(3,ndx)-ansl+frac2
      almost=ansmid+ansl
      ans=ansx+almost
      anslow=(ansx-ans+almost)+((ansmid-almost+ansl) +anstiny)
      if (head < 0) then
         ans=-ans
         anslow=-anslow
      end if
      _qtanh=dcmplx(ans, anslow)
      return

* Entry point for C-language name and semantics
      entry tanhl(%val(head), %val(tail))
      c_function = 1
      goto 1018

      end

