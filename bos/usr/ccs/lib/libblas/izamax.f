* @(#)29	1.3  src/bos/usr/ccs/lib/libblas/izamax.f, libblas, bos411, 9428A410j 4/5/91 12:51:20
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: IZAMAX
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
      INTEGER          FUNCTION IZAMAX( N, ZX, INCX )
*
*     finds the index of element having max. absolute value.
*     jack dongarra, 3/11/78.
*
*     .. Scalar Arguments ..
      INTEGER                           INCX, N
*     ..
*     .. Array Arguments ..
      COMPLEX*16                        ZX( 1 )
*     ..
*     .. Local Scalars ..
      INTEGER                           I, IX
      DOUBLE PRECISION                  SMAX
*     ..
*     .. External Functions ..
      DOUBLE PRECISION                  DCABS1
      EXTERNAL                          DCABS1
*     ..
*     .. Executable Statements ..
*
      IZAMAX = 0
      IF( N.LT.1 )
     $   RETURN
*
      IZAMAX = 1
      IF( N.EQ.1 )
     $   RETURN
      IF( INCX.EQ.1 )
     $   GO TO 30
*
*        code for increment not equal to 1
*
      IX = 1
      IF( INCX.LT.0 )
     $   IX = 1 - ( N-1 )*INCX
      SMAX = DCABS1( ZX( IX ) )
      IX = IX + INCX
      DO 20 I = 2, N
         IF( DCABS1( ZX( IX ) ).LE.SMAX )
     $      GO TO 10
         IZAMAX = I
         SMAX = DCABS1( ZX( IX ) )
   10    IX = IX + INCX
   20 CONTINUE
      IF( INCX.LT.0 )
     $   IZAMAX = N - IZAMAX + 1
      RETURN
*
*        code for increment equal to 1
*
   30 SMAX = DCABS1( ZX( 1 ) )
      DO 40 I = 2, N
         IF( DCABS1( ZX( I ) ).LE.SMAX )
     $      GO TO 40
         IZAMAX = I
         SMAX = DCABS1( ZX( I ) )
   40 CONTINUE
      RETURN
      END
