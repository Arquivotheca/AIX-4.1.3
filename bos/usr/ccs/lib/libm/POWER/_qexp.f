* @(#)81	1.3  src/bos/usr/ccs/lib/libm/POWER/_qexp.f, libm, bos411, 9428A410j 9/21/93 11:57:04
* COMPONENT_NAME: LIBCCNV
*
* FUNCTIONS: _qexp, _qexp_x, _qhexp, expl
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

* NAME: _qexp
*                                                                    
* FUNCTION: quad precision exponential
*
* RETURNS: exponential of argument

@process xflag(callby)
*******************************************************************
*            PROGRAM: Quad-Precision Exponential                  *
*            AUTHOR:  ISQUARE, Inc., (V. Markstein)               *
*            DATE:    4/3/90                                      *
*            MODE:    9/8/90  customized for RS/6000 performance  *
*            NOTICE:  Copyright (c) ISQUARE, Inc. 1990            *
*                     The argument (alpha,beta) is DOUBLE COMPLEX *
*                         or REAL*16 (when available)             *
*                                                                 *
*                     Results are returned by VALUE               *
*                                                                 *
*******************************************************************
*                                                                 *
*            CALLING: _setflm                                     *
*            NOTE:    real&complex arguments not combined due to  *
*                     FORTRAN bug.
*******************************************************************

       DOUBLE COMPLEX FUNCTION _QEXP(%val(alpha),%val(beta))
       implicit real*8          (a-h,o-y)
       implicit double complex  (Z,_)
       real*8                   alpha,beta,extraword
       real*8                   acctbl(3,-24:24),coeff(13)
       real*8                   test,big,reciplog2,log2,one,
     +                          log2b,log2c,log2d
       real*8                   LXIV,accndx
       integer*4                hiorder(2),em(2),ndx(2),nscale(2)
       integer*4                dscale(2)
       equivalence              (test,hiorder(1)),(en,em(1)),
     +                          (accndx,ndx(1)),(rscale,dscale(1)),
     +                          (scale,nscale(1)), (power,newpower)
*
       double complex		expl
       integer			c_function
       real*8			c_head, c_tail
       external			__set_errno128
       integer			ERANGE
       parameter		(ERANGE = 34)

       data     log2            /z'3FE62E42FEFA39EF'/
       data     log2b           /z'3C7ABC9E3B39803F'/
       data     log2c           /z'3907B57A079A1934'/
       data     log2d           /z'B5AACE93A4EBE5D1'/
       data     reciplog2       /z'3FF71547652B82FE'/
       data     big             /z'4338000000000000'/
       data     one             /1.0d0/
       data     LXIV            /64.0d0/
       data     scale           /0.0d0/
       data     coeff           /6227020800.,3113510400.,1037836800.,
     +                           259459200.,51891840.,8648640.,
     +                           1235520.,154440.,17160.,1716.,
     +                           156.,13.,1.  /
       data     r13             /z'3DE6124613A86D09'/
       data     tresbig         /z'7FE0000000000000'/
       data     thou            /1026.0/
       data     DCCC            /-800.0/
       data     half            /0.5/
       data     ((acctbl(j,i),j=1,3),i=-24,24)   /
     +  z'BFD7FFFFFFF78346', z'3FE5FE4615EC7934', z'BBA5D0943F817397',
     +  z'BFD6FFFFFFFFAA8D', z'3FE656F00BF5973E', z'3B9DDE0BAFD470C5',
     +  z'BFD5FFFFFFFFC4B4', z'3FE6B0FF72DECDA3', z'3BA482D0D1C3FD44',
     +  z'BFD4FFFFFFFFD89A', z'3FE70C79EBA34A37', z'3B713FA4162E00F9',
     +  z'BFD3FFFFFFFFEB0A', z'3FE769652DF23729', z'BB870FEEEEBD3A08',
     +  z'BFD2FFFFFFFFC587', z'3FE7C7C708878BF6', z'3B96439C82816F81',
     +  z'BFD1FFFFFFFE834A', z'3FE827A5618926C6', z'BBA80AD514F789EA',
     +  z'BFD0FFFFFFFFDACD', z'3FE8890636E32D97', z'3BA02C0F1AAB3177',
     +  z'BFCFFFFFFFFE8A16', z'3FE8EBEF9EACCAD8', z'BBA6337C56F8CB1D',
     +  z'BFCDFFFFFFFE54CC', z'3FE95067C783CE6F', z'BBA5CD29758448F6',
     +  z'BFCBFFFFFFFE8C9A', z'3FE9B674F8F33E73', z'BBA2FEFB001F06AC',
     +  z'BFC9FFFFFFFFF380', z'3FEA1E1D93D68A5D', z'3B94BC1B34F72879',
     +  z'BFC7FFFFFFFF88CE', z'3FEA876812C0A030', z'BB95BE2964926F2A',
     +  z'BFC5FFFFFFFFCB72', z'3FEAF25B0A61B2C5', z'BBA5D75ACA0F1016',
     +  z'BFC3FFFFFFFF951A', z'3FEB5EFD29F26302', z'3BA9F0ADD4A5C367',
     +  z'BFC1FFFFFFFFF6D4', z'3FEBCD553B9D7D60', z'BBAADABCB264015F',
     +  z'BFBFFFFFFFFFE014', z'3FEC3D6A24ED85A7', z'3B74796110476F00',
     +  z'BFBBFFFFFFFFD9AC', z'3FECAF42E73A50C9', z'3B915C7B4A361F1E',
     +  z'BFB7FFFFFFFFD862', z'3FED22E6A0198085', z'3BA1BBB3BFFB06E1',
     +  z'BFB3FFFFFFFFF168', z'3FED985C89D04353', z'3B5234BB7C215B4D',
     +  z'BFAFFFFFFFFFB37A', z'3FEE0FABFBC70722', z'BB8AE48A8045BF54',
     +  z'BFA7FFFFFFFC4C34', z'3FEE88DC6AFF0846', z'BBACF2F570E3E267',
     +  z'BF9FFFFFFFFC6F5E', z'3FEF03F56A88D17C', z'BBACA2B205D8EB45',
     +  z'BF8FFFFFFFFF580A', z'3FEF80FEABFEF23A', z'BB61AF65BD7BA074',
     +  z'0000000000000000', z'3FF0000000000000', z'0000000000000000',
     +  z'3F900000000039D1', z'3FF04080AB55DF24', z'BBA5A69AB0AA4DB6',
     +  z'3FA0000000040E38', z'3FF0820560114966', z'BBAD31807B207EEE',
     +  z'3FA80000000000A0', z'3FF0C49236829E91', z'3BA01DE1DE261F9E',
     +  z'3FB0000000008504', z'3FF1082B577D3DC7', z'3B8DB72F752AE77F',
     +  z'3FB400000000727E', z'3FF14CD4FC98A493', z'BBA8B3713285C6A1',
     +  z'3FB8000000014EB4', z'3FF192937074F7C7', z'3B8A95AD4E3A0016',
     +  z'3FBC000000007514', z'3FF1D96B0EFF16A3', z'BB8C7B78AC0BEE62',
     +  z'3FC000000000387B', z'3FF2216045B6FDCD', z'BB833654FC2C852C',
     +  z'3FC2000000009DEE', z'3FF26A7793F6181D', z'BBA7415B71161E35',
     +  z'3FC40000000018F5', z'3FF2B4B58B37301F', z'3BB0D4C593F1E693',
     +  z'3FC6000000002679', z'3FF3001ECF6020AD', z'3B6A3633B13F9963',
     +  z'3FC800000000D54A', z'3FF34CB8170B785E', z'BBB2D6911C45DA93',
     +  z'3FCA00000001BC66', z'3FF39A862BD40516', z'BBA4860BD2E696BD',
     +  z'3FCC00000002F156', z'3FF3E98DEAA192FD', z'3BB1EAA69AC1EC1A',
     +  z'3FCE000000006E22', z'3FF439D443F602C0', z'3BB1687C5C76A673',
     +  z'3FD000000000344F', z'3FF48B5E3C3E9251', z'3BB0168AB7BA044E',
     +  z'3FD1000000005518', z'3FF4DE30EC213A1F', z'3BB16D9FF11FD1E1',
     +  z'3FD20000000F1F6D', z'3FF5325180D4AF2B', z'3BB5182E8AA41550',
     +  z'3FD3000000004CC9', z'3FF587C53C5A94C5', z'3BADF0B525890C47',

     +  z'3FD4000000003C35', z'3FF5DE9176047488', z'3B93096571BF40CF',
     +  z'3FD5000000001F9F', z'3FF636BB9A983D52', z'3B834A8943379035',
     +  z'3FD6000000001C8E', z'3FF690492CBF9E44', z'3BAD13A5639824C3',
     +  z'3FD7000000002B17', z'3FF6EB3FC55B2DE4', z'3BAD1F08B4D7D659',
     +  z'3FD80000000018C8', z'3FF747A513DBF86E', z'BB8B5F69BD7540CF'/

       c_function = 0			! Fortran name and semantics
 1018  continue

        normflag=0                    !switch->really doing exp
        oldfpscr=_setflm(%val(big))   !enter round-to-nearest mode and
                                      !disable exception trapping
        go to 50

* NAME: _qhexp
*                                                                    
* FUNCTION: entry point used by other quad precision hyperbolic 
*           functions

        ENTRY _qhexp(%val(alpha),%val(beta),extraword)
        normflag=1                    !switch->call by hyperbolic func
        extraword=0.0d0               !init in case of exceptional
                                      !result.
        oldfpscr=_setflm(%val(big))   !enter round-to-nearest mode and
        go to 50                      !disable exception trapping

* NAME: _qhex_x
*                                                                    
* FUNCTION: extra precision entry point used by _qpow

        ENTRY _qexp_x(%val(alpha),%val(beta),
     +                %val(gamma),extraword,%val(delta))

********************************
*  EXTRA PRECISION ENTRY POINT *
*  COMPUTES:                   *
*  E^(ALPHA+BETA+GAMMA)-DELTA  *
********************************
        normflag=2                   !switch->extra precise computation
        oldfpscr=_setflm(%val(big))   !enter round-to-nearest mode and
                                     !disable trapping

********************************
*  START OF COMMON CODE        *
********************************
50      continue
        head=alpha                   !isolate low and high order parts
        tail=beta                    !of the argument
        test=head                    !copy of argument for testing and
                                      !extraction of exponent.
        en=head*reciplog2+big        !arg/log2 rounded-to-nearest is
                                      !in low order 32 bits of n, in
                                      !2's complement.
        factor=en-big                 !factor=rint(arg/log2)
        redarg=head-log2*factor      !Reduce the argument by factor *
                                      !log2. NOTE:redarg is exact
        notnormal=0                  !switch->NOT IN DANGER of
                                      !under/overflow
        if (iand(hiorder(1),z'7fffffff') > z'40862000')
     +    goto 1000                  !test for all special cases
 100    continue
        b=log2b*factor               !continue argument reduction
        blow=log2b*factor-b
        redarg1=tail-b               !second word of reduced argument
        accndx=redarg*LXIV+big       !index into acctbl
        ca=log2c*factor
        if (abs(b) .gt. abs(tail)) then      !tail - b may be inexact
          carry=tail-(b+redarg1)-blow
        else
          carry=(tail-redarg1)-b-blow
        end if
        redarg=redarg-acctbl(1,ndx(2)) !arg-factor ln2-tbl(ndx(2)) ->
                                        !mucho trailing zeroes in redarg
        arg=redarg+redarg1           !get significant bits from the right
        arg2a=(redarg - arg + redarg1)
        c=carry-ca                   !3rd word of reduced argument
        d=(ca-log2c*factor) - (ca-(carry-c))-log2d*factor     !4th word
        nscale(1)=lshift(em(2)+1023,20)  !pow of 2, i.e. (2^factor)
        arg2=arg2a+c+d
        if (normflag .eq. 2) then
           arg2=arg2+gamma  !extra argument component
           rscale=zero
           dscale(1)=2046*2**20-nscale(1)
        end if
**********************************************************************
*  The result has four components, to be multiplied together.         *
*  One component, 2^factor, is trivial.  A 2nd component ->          *
*  acctbl(2,ndx(2)), acctbl(3,ndx(2)) is by table lookup.  A 3rd     *
*  component is form the reduced argument arg+arg2+d.  Its exponent- *
*  ial is computed as exp(arg)*exp(arg2+d).  arg2+d is so small that *
*  exp(arg2+d)=1+arg2+d is correct to approximately 118 bits.        *
*  The 4th or remaining component is computed by ->                  *
*                                        exp(arg), |arg| <= 1/128    *
**********************************************************************
        sum=coeff(13)
        sum=coeff(12)+sum*arg        !Vanilla working precision will
        sum=coeff(11)+sum*arg        !do for a short while
        sum=coeff(10)+sum*arg
        sum=coeff( 9)+sum*arg
        temp=coeff(8)+sum*arg        !From this point on double length
        suml=(coeff(8)-temp)+sum*arg   !sums
        sum=temp
        do i=7, 2, -1

          hi=coeff(i)+sum*arg
          suml=coeff(i)-hi+sum*arg+suml*arg
          sum=hi
          end do
        prod = sum*r13                 !like dividing by 13!
        residual=(sum-coeff(1)*prod)+suml
        suml=residual*r13
        sum2=one+prod*arg               !add in linear coefficient
        suml2=one-sum2+prod*arg+suml*arg
        sum=sum2*arg+(suml2*arg)      !last multiplication by argument
        suml=(sum2*arg-sum)+(suml2*arg)
        t1=acctbl(2,ndx(2))          !need value of of exp(ndx/64)
        t2=acctbl(3,ndx(2))
        small=t2+sum*(t2+arg2*t1)    ! error <  ulp/1024
        prod=t1*sum                  !main acctbl value*series ->
        high=prod+t1                  !sum high order term
        prodlow=t1*sum-prod          !low order part of product
        residual=(t1-high)+prod      !residual from h.o part
        if (normflag .eq. 2) then    !extraprecise adjustment needed?
           temp = high-delta*rscale  !remove delta from result
           if (temp > 0) then        !e.g. to get expm1, set delta = 1
              bump = high-temp-delta*rscale
           else                      !be careful to catch bits on the right
              bump = high-(temp+delta*rscale)
           end if
           resnew = residual+bump    !propagate bits that fell off h.o. part
           if (abs(residual) > abs(bump)) then   !and even propagate another
              small = ((residual-resnew)+bump)+small  !level of precision
           else
              small = ((bump-resnew)+residual)+small
           end if
           high=temp
           residual=resnew
           end if
        tail=small+arg2*t1+prodlow+suml*t1        !total error < ulp/32
        close=tail+residual          !almost the low order term
        top=high+close               !high order term
        residual2=(residual-close)+tail  ! 5 low order bits left ?
        residual3=(high-top)+close   !dropped bits from h.o. term
        bottom=residual3+residual2   !low order term
        if (normflag .ne. 0) then
          go to 3000                 !is special handling required?
        end if
75      continue
        waste=_setflm(%val(oldfpscr))
        if (notnormal .ne. 0) then
          go to 2000                 !fixup for results close to
        end if                         !over/underflow
 500    continue                     !exit sequence
        top=top*scale
        bottom=bottom*scale
        _qexp=dcmplx(top,bottom)
        return

*********************************************************************
*       All the special cases are handled here.  These include:     *
*       Nans, Infinities, and large magnitudes                      *
*********************************************************************
 1000   scale = one                  !make sure scale factor is defined
        if (head .ne. head) then     !argument is a NaN.
           top=head
           bottom=zero
           waste=_setflm(%val(oldfpscr))
           go to 500                 !return the NaN
        end if
        if (iand(hiorder(1), z'7ff00000') .eq. z'7ff00000') then
                                     !INFINITY, NaNs were scraped above
          if (hiorder(1) > 0) then
              top=head               ! + infinity
              bottom=zero            !return -> +infinity
              waste=_setflm(%val(oldfpscr))
              go to 500
           else                      ! - infinity
              top=zero               !return -> zero
              bottom=zero
              waste=_setflm(%val(oldfpscr))
              go to 500
           end if
         end if
                                     !arg. with very large magnitude
        if (hiorder(1) > 0) then     !large positive arg.
           if ((head .gt. thou*log2)) then ! For sure result overflows
              top=big                !final products form infinity
              bottom=zero            !or DBL_MAX.
              waste=_setflm(%val(oldfpscr))
              scale=tresbig
              if (c_function .eq. 1) then
                 call __set_errno128(%val(ERANGE))
              endif
              go to 500              !allow overflow to happen
           endif
           em(2)=em(2)-4             !for now decrease result exponent
           notnormal=(1023+4)*2**20     !and compensate multiplier
           go to 100                 !continue processing
        else
           if (head < DCCC) then     !For sure result will underflow
              top=one/tresbig          !any small number
              bottom=0
              scale=one/tresbig      !miniscule scaling factor
              waste=_setflm(%val(oldfpscr))
              go to 500
           end if
           em(2)=em(2)+256           !scale up to work in full precision
           notnormal=(1023-256)*2**20   !and compensate multiplier
           go to 100                 !continue processing
        end if

*******************************************************************
* Control reaches this point at the end of a computation that may *
* produce overflow, gradual underflow, or underflow.              *
*******************************************************************
 2000   power=zero
        newpower=notnormal           !fix multiplier to undo scaling
        temp = top
        if ((top + bottom .ge. 1.0d0) .and.
     x      (newpower/2+nscale(1)/2 > 2045*2**2045)) then
                                           !Near overflow threshhold,
           waste=_setflm(%val(big+1.0))    !use round-towards-zero to
           sum=temp+bottom*one       !force both 1/2 of the number to
           suml=(top-sum)+bottom     !the same sign to avoid rounding.
           if (normflag .eq. 2) then !Special handling
              extraword=extraword/scale                 !unscale
              extraword=(top-sum)-suml+bottom+extraword !adjust
              extraword=extraword*scale                 !rescale
           end if
           top=sum                   !causing h.o. part to overflow
           bottom=suml
        end if
        waste=_setflm(%val(oldfpscr)) !restore original rounding mode
        top=top*power                !compensation for earlier scaling
        bottom=bottom*power
        if (normflag .ne. 0) then
          extraword=extraword*power
        endif
        go to 500                  !Complete the job

3000    if (abs(residual2) .gt. abs(residual3)) then
           extraword=residual2-bottom+residual3
        else
           extraword=residual3-bottom+residual2
        end if
        if (normflag .eq. 2) then
           extraword = extraword*scale
           go to 75
        end if
        scale=scale*half             !_qhexp returns .5*e^x
        extraword=extraword*scale    !this value is not scaled on main path
        go to 75

* Entry point for C-language name and semantics
       entry expl(%val(c_head), %val(c_tail))
       alpha = c_head
       beta = c_tail
       c_function = 1
       goto 1018

        END
