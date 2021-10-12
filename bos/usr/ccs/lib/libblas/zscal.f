* @(#)96	1.2  src/bos/usr/ccs/lib/libblas/zscal.f, libblas, bos411, 9428A410j 6/15/90 17:53:02
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: ZSCAL
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
      SUBROUTINE ZSCAL( N, ZA, ZX, INCX )
*
*     scales a vector by a constant.
*     jack dongarra, 3/11/78.
*
*     .. Scalar Arguments ..
      INTEGER           INCX, N
      COMPLEX*16        ZA
*     ..
*     .. Array Arguments ..
      COMPLEX*16        ZX( 1 )
*     ..
*     .. Local Scalars ..
      INTEGER           I, IX, NINCX
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
         call xerbla('ZSCAL ', 4)
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
         ZX( I ) = ZA*ZX( I )
   10 CONTINUE
      RETURN
*
*        code for increment equal to 1
*
   20 DO 30 I = 1, N
         ZX( I ) = ZA*ZX( I )
   30 CONTINUE
      RETURN
      END
