* @(#)72	1.1  src/bos/usr/ccs/lib/libc/POWER/_xlqmul.f, libccnv, bos411, 9428A410j 12/13/90 19:56:24
*
* COMPONENT_NAME: LIBCCNV
*
* FUNCTIONS: _xlqmul
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

@process mixed debug(callby)

* NAME: _xlqmul
*                                                                    
* FUNCTION: multiply two quad precision numbers
*
* RETURNS: a quad precision number containing the result
*
* NOTE:  Variable names are case-sensitive, must have
*        the @process mixed to work right.

        double complex function _xlqmul(%val(X),%val(x),%val(Y),%val(y))

C       This file must be compiled with the -U option, becuase
C       the variables are case sensitive.  Upper case variables
C       are the most significant part, and lower case variables
C       are the least significant part.
C       Computes (Z,z) = (X,x) * (Y,y), where
C       the notation (p,q) denotes a quad precision number, in which
C       p is the high order word (as an IEEE double precision word),
C       and q is the low order word (as an IEEE double precision word.)
C
C       The full computation is (Z,z) = X Y + X y + x Y + x y
C       However, the product xy is so small that it is ignored.  The cross-terms
C       xY and Xy are carefully computed to avoid the use of the MAF instruction
C       so that quad precision multiplication will be commutative.

        implicit real*8(a-h,o-z)
        u = x * Y                        !computation of the crossproduct terms
        v = X * y
        w = u + v                        !sum of the cross product terms
        Z = X*Y + w                      !The high order term (exploits MAF)
        z = X*Y - Z + w                  !The low order term (exploits MAF)
        _xlqmul = dcmplx(Z,z)            !Combine results into 128 bit object
        return
        end

