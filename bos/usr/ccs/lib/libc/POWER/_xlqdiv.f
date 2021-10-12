* @(#)71	1.1  src/bos/usr/ccs/lib/libc/POWER/_xlqdiv.f, libccnv, bos411, 9428A410j 12/13/90 19:56:07
*
* COMPONENT_NAME: LIBCCNV
*
* FUNCTIONS: _xlqdiv
*
* ORIGINS: 27
*
* IBM CONFIDENTIAL -- (IBM Confidential Restricted when
* combined with the aggregated modules for this product)
*                  SOURCE MATERIALS
* (C) COPYRIGHT International Business Machines Corp. 1990
* All Rights Reserved
*
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

@process debug(callby)

* NAME: _xlqdiv
*                                                                    
* FUNCTION: divide two quad precision numbers
*
* RETURNS: a quad precision number containing the quotient
*
* NOTE:  Variable names are case-sensitive, must have
*        the @process mixed to work right.

      double complex function _xlqdiv(%val(X),%val(xx),%val(Y),%val(yy))

C       Computes (Z,zz) = (X,xx) / (Y,yy), where
C       the notation (p,q) denotes a quad precision number, in which
C       p is the high order word (as an IEEE double precision word),
C       and q is the low order word (as an IEEE double precision word.)
C
C       Division is carried out by the elementary school method:
C
C                   t     tau
C                  -----------------
C          Y,yy   ) X     xx
C                  -t*(Y+yy)
C                   ---------
C                        top
C
C      If the reciprocal of Y can be represented in floating point, then
C      a RS/6000 scheme is used to compute the correctly rounded quotients,
C      Y/X and top/X, as follows:  On RS/6000, if r = 1/X correctly rounded,
C      then for any A, A/X correctly rounded is given by:
C
C               A/X = (A*r) + (A - X * (A*r))*r
C
C      where the above computation uses one multiplication, one MSF, and
C      one MAF.
C
C      It is economical to use the above form twice after computing the
C      reciprocal, rather than to perform two divisions.  However, a test
C      must be included to assure that r = 1/X does not cause overflow.

       implicit real*8 (a-h,o-z)
       parameter       (alpha = z'7ff0000000000000')  !Infinity
       logical i,j
       real one/1.0/
       equivalence(den,id)
       r = one/y                     !Speculative computation of 1/y
       den = y
       i = iand(id, z'7ff00000') .eq. 0            !While the division is
       j = iand(id, z'7fe00000') .eq. z'7fe00000'  !going on, use the fixed
       if (iand(id, z'7ff00000') .eq. 0) go to 100 !point engine to determine
       if (iand(id, z'7fe00000') .eq. z'7fe00000') go to 100   !if the above
                                                   !quotient overflows or
                                                   !underflows, in which case
                                                   !the divisions must be done
       t = x * r
       t = t + (x - y*t)*r      !correctly rounded h.o. quotient (see comments)
       top = (x - t*y) + xx - yy*t  !remainder
       tau = top * r                !second superdigit of quotient
       tau = tau + (top - tau*y)*r  !refined as per the above comments
       z = t + tau                  !combine high and low order parts
       zz = t - z + tau             !(we know that |t| > |tau|)
       _xlqdiv = dcmplx(z,zz)       !Combine into 128-bit object to be returned.
       return

 100   t = x / y                !Same algorithm as above, but the divisions
       if (abs(t) .eq. alpha) then  !are explicit.  If the h.o. quotient is
          _xlqdiv = dcmplx(t,t)     !Infinity, return infinity as the quotient.
          return
          end if
       top = (x  - t*y) + xx - yy*t !remainder
       tau = top / y                !Second superdigit of quotient.
       z = t + tau                  !combine high and low order parts
       zz = t - z + tau
       _xlqdiv = dcmplx(z,zz)       !Combine into 128-bit object to be returned.
       return
       end
