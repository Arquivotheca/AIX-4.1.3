* @(#)76	1.2  src/bos/usr/ccs/lib/libblas/zdscal.f, libblas, bos411, 9428A410j 6/15/90 17:52:57
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: ZDSCAL
*
* ORIGINS: 51, 27
*
* This module contains IBM CONFIDENTIAL code. -- (IBM
* Confidential Restricted when combined with the aggregated
* modules for this product)
*                  SOURCE MATERIALS
* (C) COPYRIGHT International Business Machines Corp. 1985, 1989
* All Rights Reserved
*
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
************************************************************************
*
      SUBROUTINE ZDSCAL( N, DA, ZX, INCX )
*
*     scales a vector by a constant.
*     jack dongarra, 3/11/78.
*
*     .. Scalar Arguments ..
      INTEGER            INCX, N
      DOUBLE PRECISION   DA
*     ..
*     .. Array Arguments ..
      COMPLEX*16         ZX( 1 )
*     ..
*     .. Local Scalars ..
      INTEGER            I, IX, NINCX
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC          DCMPLX
*     ..
*     .. Executable Statements ..
*
      IF( N.LE.0 )
     $   RETURN
*
*** modified to use xerbla error handling if
*   incx == 0.
*
      if (incx .lt. 1) then
         call xerbla('ZDSCAL', 4)
         return
         end if
*
      IF( INCX.EQ.1 )
     $   GO TO 20
*
*        code for increment not equal to 1
*
      IX = 1
      IF( INCX.LT.0 )
     $   IX = 1 - ( N-1 )*INCX
      NINCX = IX + ( N-1 )*INCX
      DO 10 I = IX, NINCX, INCX
         ZX( I ) = DCMPLX( DA, 0.0D0 )*ZX( I )
   10 CONTINUE
      RETURN
*
*        code for increment equal to 1
*
   20 DO 30 I = 1, N
         ZX( I ) = DCMPLX( DA, 0.0D0 )*ZX( I )
   30 CONTINUE
      RETURN
      END
