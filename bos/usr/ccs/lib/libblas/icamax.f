* @(#)26	1.3  src/bos/usr/ccs/lib/libblas/icamax.f, libblas, bos411, 9428A410j 4/5/91 12:51:11
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: ICAMAX
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
      INTEGER          FUNCTION ICAMAX( N, CX, INCX )
*
*     finds the index of element having max. absolute value.
*     jack dongarra, linpack, 3/11/78.
*
*     .. Scalar Arguments ..
      INTEGER                           INCX, N
*     ..
*     .. Array Arguments ..
      COMPLEX                           CX( 1 )
*     ..
*     .. Local Scalars ..
      INTEGER                           I, IX
      REAL                              SMAX
      COMPLEX                           ZDUM
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC                         ABS, AIMAG, REAL
*     ..
*     .. Statement Functions ..
      REAL                              CABS1
*     ..
*     .. Statement Function definitions ..
      CABS1( ZDUM ) = ABS( REAL( ZDUM ) ) + ABS( AIMAG( ZDUM ) )
*     ..
*     .. Executable Statements ..
*
      ICAMAX = 0
      IF( N.LT.1 )
     $   RETURN
*
      ICAMAX = 1
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
      SMAX = CABS1( CX( IX ) )
      IX = IX + INCX
      DO 20 I = 2, N
         IF( CABS1( CX( IX ) ).LE.SMAX )
     $      GO TO 10
         ICAMAX = I
         SMAX = CABS1( CX( IX ) )
   10    IX = IX + INCX
   20 CONTINUE
      IF( INCX.LT.0 )
     $   ICAMAX = N - ICAMAX + 1
      RETURN
*
*        code for increment equal to 1
*
   30 SMAX = CABS1( CX( 1 ) )
      DO 40 I = 2, N
         IF( CABS1( CX( I ) ).LE.SMAX )
     $      GO TO 40
         ICAMAX = I
         SMAX = CABS1( CX( I ) )
   40 CONTINUE
      RETURN
      END
