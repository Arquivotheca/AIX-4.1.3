* @(#)24	1.3  src/bos/usr/ccs/lib/libblas/dzasum.f, libblas, bos411, 9428A410j 6/15/90 17:51:42
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: DZASUM
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
      DOUBLE PRECISION FUNCTION DZASUM( N, ZX, INCX )
*
*     takes the sum of the absolute values.
*     jack dongarra, 3/11/78.
*
*     .. Scalar Arguments ..
      INTEGER                           INCX, N
*     ..
*     .. Array Arguments ..
      COMPLEX*16                        ZX( 1 )
*     ..
*     .. Local Scalars ..
      INTEGER                           I, IX, NINCX
      DOUBLE PRECISION                  STEMP
*     ..
*     .. External Functions ..
      DOUBLE PRECISION                  DCABS1
      EXTERNAL                          DCABS1
*     ..
*     .. Executable Statements ..
*
      DZASUM = 0.0D0
      STEMP = 0.0D0
      IF( N.LE.0 )
     $   RETURN
*
*** modified to use xerbla error handling if
*   incx == 0.
*
      if (incx .lt. 1) then
         call xerbla('DZASUM', 3)
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
         STEMP = STEMP + DCABS1( ZX( I ) )
   10 CONTINUE
      DZASUM = STEMP
      RETURN
*
*        code for increment equal to 1
*
   20 DO 30 I = 1, N
         STEMP = STEMP + DCABS1( ZX( I ) )
   30 CONTINUE
      DZASUM = STEMP
      RETURN
      END
