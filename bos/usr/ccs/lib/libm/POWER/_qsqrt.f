* @(#)65	1.3  src/bos/usr/ccs/lib/libm/POWER/_qsqrt.f, libm, bos411, 9428A410j 10/1/93 17:09:50
* COMPONENT_NAME: LIBCCNV
*
* FUNCTIONS: _qsqrt, _qsqrtsp, sqrtl
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
*******************************************************************
*            PROGRAM: QUAD-PRECISION SQUARE ROOT                  *
*            AUTHOR:  ISQUARE, Inc., (V. Markstein)               *
*            DATE:    5/6/90                                      *
*            MODIFIED:9/23/90 customized for RS/6000 performance  *
*            NOTICE:  Copyright (c) ISQUARE, Inc. 1990            *
*                                                                 *
*            NOTES:   The arguments head and tail are doubles.    *
*                     The pair (head,tail) represents the  QUAD   *
*                     input.                                      *
*                                                                 *
*                     Results are returned by VALUE               *
*                                                                 *
*                     An additional entry point, _q_sqrtsp has    *
*                     been added to return a few extra bits       *
*                     of precision for use by _qacos              *
*******************************************************************
*                                                                 *
*            CALLING: _setflm                                     *
*                                                                 *
*******************************************************************

* NAME: _qsqrt
*                                                                    
* FUNCTION: quad precision square root
*
* RETURNS: square root of argument

        DOUBLE COMPLEX FUNCTION _QSQRT(%val(head), %val(tail))
        implicit double complex (z,_)
        implicit real*8         (a-h, o-y)
        common                  /_q_sqrt/sqrtextra
        real*8                  infinity,zero

        double complex		sqrtl           ! really REAL*16
        external		__set_errno128
        integer			c_function
        integer			EDOM
        parameter		(EDOM = 33)
        real*8			c_head, c_tail

        equivalence             (ix, x), (iguess, guess),
     +                          (irecipguess, recip)
        data                    half /0.5d0/
        data                    scaleup     /z'5ff0000000000000'/
        data                    scaledown   /z'1ff0000000000000'/
        data                    adjustdown  /z'2ff0000000000000'/
        data                    adjustup    /z'4ff0000000000000'/
        data                    infinity    /z'7ff0000000000000'/
        data                    zero        /0.0d0/
        integer*2               sqrttable(0:255)  /
     +  z'6B69', z'6C68', z'6E67', z'6F65',
     +  z'7064', z'7263', z'7361', z'7460',
     +  z'765F', z'775D', z'795C', z'7A5B',
     +  z'7B5A', z'7D58', z'7E57', z'7F56',
     +  z'8155', z'8254', z'8352', z'8551',
     +  z'8650', z'874F', z'894E', z'8A4D',
     +  z'8B4C', z'8C4B', z'8E4A', z'8F48',
     +  z'9047', z'9246', z'9345', z'9444',
     +  z'9543', z'9742', z'9841', z'9940',
     +  z'9A3F', z'9C3E', z'9D3D', z'9E3C',
     +  z'9F3C', z'A13B', z'A23A', z'A339',
     +  z'A438', z'A637', z'A736', z'A835',
     +  z'A934', z'AA33', z'AC33', z'AD32',
     +  z'AE31', z'AF30', z'B02F', z'B12E',
     +  z'B32E', z'B42D', z'B52C', z'B62B',
     +  z'B72A', z'B92A', z'BA29', z'BB28',
     +  z'BC27', z'BD26', z'BE26', z'BF25',
     +  z'C124', z'C223', z'C323', z'C422',
     +  z'C521', z'C621', z'C720', z'C81F',
     +  z'CA1E', z'CB1E', z'CC1D', z'CD1C',
     +  z'CE1C', z'CF1B', z'D01A', z'D11A',
     +  z'D219', z'D418', z'D518', z'D617',
     +  z'D716', z'D816', z'D915', z'DA14',
     +  z'DB14', z'DC13', z'DD13', z'DE12',
     +  z'DF11', z'E111', z'E210', z'E310',
     +  z'E40F', z'E50E', z'E60E', z'E70D',
     +  z'E80D', z'E90C', z'EA0B', z'EB0B',
     +  z'EC0A', z'ED0A', z'EE09', z'EF09',
     +  z'F008', z'F108', z'F207', z'F306',
     +  z'F406', z'F505', z'F605', z'F704',
     +  z'F804', z'F903', z'FA03', z'FB02',
     +  z'FC02', z'FD01', z'FE01', z'FF00',
     +  z'00FF', z'01FD', z'02FB', z'03F9',
     +  z'04F7', z'05F5', z'06F3', z'07F2',
     +  z'08F0', z'09EE', z'0AEC', z'0BEA',
     +  z'0CE9', z'0DE7', z'0EE5', z'0FE4',
     +  z'10E2', z'11E0', z'12DF', z'13DD',
     +  z'14DB', z'15DA', z'16D8', z'17D7',
     +  z'17D5', z'18D4', z'19D2', z'1AD1',
     +  z'1BCF', z'1CCE', z'1DCC', z'1ECB',
     +  z'1FC9', z'20C8', z'20C6', z'21C5',
     +  z'22C4', z'23C2', z'24C1', z'25C0',
     +  z'26BE', z'27BD', z'27BC', z'28BA',
     +  z'29B9', z'2AB8', z'2BB7', z'2CB5',
     +  z'2DB4', z'2DB3', z'2EB2', z'2FB0',
     +  z'30AF', z'31AE', z'32AD', z'33AC',
     +  z'33AA', z'34A9', z'35A8', z'36A7',
     +  z'37A6', z'37A5', z'38A4', z'39A3',
     +  z'3AA2', z'3BA0', z'3C9F', z'3C9E',
     +  z'3D9D', z'3E9C', z'3F9B', z'409A',
     +  z'4099', z'4198', z'4297', z'4396',
     +  z'4495', z'4494', z'4593', z'4692',
     +  z'4791', z'4890', z'488F', z'498E',
     +  z'4A8D', z'4B8C', z'4B8C', z'4C8B',
     +  z'4D8A', z'4E89', z'4E88', z'4F87',
     +  z'5086', z'5185', z'5284', z'5283',
     +  z'5383', z'5482', z'5581', z'5580',
     +  z'567F', z'577E', z'587E', z'587D',
     +  z'597C', z'5A7B', z'5B7A', z'5B79',
     +  z'5C79', z'5D78', z'5D77', z'5E76',
     +  z'5F76', z'6075', z'6074', z'6173',
     +  z'6272', z'6372', z'6371', z'6470',
     +  z'656F', z'656F', z'666E', z'676D',
     +  z'686D', z'686C', z'696B', z'6A6A'
     +  /

       c_function = 0			! Fortran name and semantics

       x=head                                !hi order argument
       y=tail
 1018  continue
       normflag = 0
       iguess=iand((ishft(ix,-1)+z'1ff80000'), z'7ff00000')
       irecipguess=z'7fc00000'-iguess
       if (ix < z'07000000') go to 200       !small values, negative values
       scale=1.0d0
100    continue
       index=iand(ishft(ix,-13),z'000000ff')
       ival=sqrttable(index)                 !8 bit guesses to sqrt and sqrt/2
       iguess=iguess+ishft(iand(ival, z'0000ff00'),4) !guess=8-bit approx
                                             !to sqrt(head,tail)
       irecipguess=irecipguess+ishft(iand(ival, z'000000ff'),12) !recip =
                                             !8-bit approx. to to 1/guess
       if (ix .ge. z'7fe00000') go to 300    !+ infinity, NaN, or arguments
                                             !near the overflow threshhold
                                             !Main algorithm-see P. Markstein,
       diff=x-guess*guess                    !IBM J. R&D, Jan. 1990, p. 117
       recip2=recip+recip                    !recip2 <- 1/guess (approx)
       guess=guess+diff*recip                !16 bit square root
       epsilon=half-recip*guess              !Start N-R to improve reciprocal
       diff=x-guess*guess
       recip=recip+epsilon*recip2            !16 bit reciprocal
       guess=guess+recip*diff                !32 bit square root
       recip2=recip+recip
       epsilon=half-recip*guess
       diff=x-guess*guess+y
       recip=recip+epsilon*recip2            !32 bit reciprocal
       guessnew=guess+recip*diff             !53 bit square root
       recip2=recip+recip
       gsq=guessnew*guessnew                 !NOTE: Departure from P. Markstein
                                             !for greater accuracy.  That is,
       guesslow=guess-guessnew+recip*diff    !at least 64 bits instead of 53
       gsqlow=guessnew*guessnew-gsq
       gg=guessnew+guessnew                  !(guessnew,guesslow)^2 must be
                                             !computed to sextuple precision
       gmid=gg*guesslow                      !in order to control errors to
       gmidlow=gg*guesslow-gmid              !less than 1/2 ulp
       epsilon=half-recip*guessnew
       recip2=recip+recip
       recipnew=recip+epsilon*recip2
       diff1=x-gsq                           !Exact
       diff3=diff1-gmid                      !Not necessarily exact
       diff3x=diff1-diff3-gmid               !Exact

       diff3a=diff3-gsqlow                   !Not necessarily exact
       diff3ax=diff3-(diff3a+gsqlow)         !Exact
       diff4=diff3a+y                        !Exact

       diff5=diff4-gmidlow                   !Error < ulp/1024
       glow2=guesslow*guesslow               !Error < ulp/(2^50)
       diff6=diff5-glow2                     !Error < ulp/1024
       diff7=diff6+(diff3x+diff3ax)          !Error < ulp/1024
       firstfix=recipnew*diff7               !Total error < ulp/256
       fixup=guesslow+firstfix
       _qsqrt=dcmplx(guessnew*scale, fixup*scale)
       if (normflag .eq. 0) return
       sqrtextra = (guesslow-fixup+firstfix)*scale
       return

 200   if (x+y .eq. zero) then               !for zero arguments, return input
          _qsqrt=z                           !as result
          return
       else if (x .lt. zero) then            !for negative arguments

* For C-language semantics, set errno = EDOM for negative argument
          if (c_function .eq. 1) then
             call __set_errno128(%VAL(EDOM))
          end if

          _qsqrt=dcmplx(infinity-infinity, zero)
          return
       else                                  !small positive argument--rescale
          x=x*scaleup                        !and restart computation of
          y=y*scaleup                        !exponent of result
          iguess=iand((ishft(ix,-1)+z'1ff80000'), z'7ff00000')
          irecipguess=z'7fc00000'-iguess
          scale=adjustdown
          go to 100                          !back to mainline
       end if

300    continue
       if (ix < x'7ff00000') then            !For finite but huge args
          x=x*scaledown                      !rescale and move away from
          y=y*scaledown                      !overflow threshhold
          iguess=iand((ishft(ix,-1)+z'1ff80000'), z'7ff00000')
          irecipguess=z'7fc00000'-iguess
          scale=adjustup
          go to 100
       end if

*      Wind up here for NaN
       _qsqrt=dcmplx(x+zero,y)
       return

* NAME: _qsqrtsp
*                                                                    
* FUNCTION: quad precision square root -- extra precision
*           entry point used by other quad functions
*
* RETURNS: square root of argument

*******************************************************************************
*                                                                             *
*              Entry Point _q_sqrtsp                                          *
*                                                                             *
*      This entry point has been created to return a few extra bits of        *
*      precision for other library routines, such as _qacos.  The             *
*      first double word in the common area _q_sqrt contains a                *
*      doube precision floating point word, which contains the next few       *
*      bits of the argument                                                   *
*                                                                             *
*******************************************************************************
       entry _q_sqrtsp (%val(dhead), %val(dtail))
       x=dhead                                !hi order argument
       y=dtail
       normflag = 1
       iguess=iand((ishft(ix,-1)+z'1ff80000'), z'7ff00000')
       irecipguess=z'7fc00000'-iguess
       if (ix < z'07000000') go to 200       !small values, negative values
       scale=1.0d0
       go to 100

* Entry point for C-language name and semantics
       entry sqrtl(%val(c_head), %val(c_tail))
       x = c_head
       y = c_tail
       c_function = 1
       goto 1018

       END
