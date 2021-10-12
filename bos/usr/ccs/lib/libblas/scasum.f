* @(#)33	1.2  src/bos/usr/ccs/lib/libblas/scasum.f, libblas, bos411, 9428A410j 6/15/90 17:52:15
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: SCASUM
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
      REAL             FUNCTION SCASUM( N, CX, INCX )
*
*     takes the sum of the absolute values of a complex vector and
*     returns a single precision result.
*     jack dongarra, linpack, 3/11/78.
*
*     .. Scalar Arguments ..
      INTEGER                           INCX, N
*     ..
*     .. Array Arguments ..
      COMPLEX                           CX( 1 )
*     ..
*     .. Local Scalars ..
      INTEGER                           I, IX, NINCX
      REAL                              STEMP
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC                         ABS, AIMAG, REAL
*     ..
*     .. Executable Statements ..
*
      SCASUM = 0.0E0
      STEMP = 0.0E0
      IF( N.LE.0 )
     $   RETURN
*
*** modified to use xerbla error handling if
*   incx == 0.
*
      if (incx .lt. 1) then
         call xerbla('SCASUM', 3)
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
         STEMP = STEMP + ABS( REAL( CX( I ) ) ) +
     $           ABS( AIMAG( CX( I ) ) )
   10 CONTINUE
      SCASUM = STEMP
      RETURN
*
*        code for increment equal to 1
*
   20 DO 30 I = 1, N
         STEMP = STEMP + ABS( REAL( CX( I ) ) ) +
     $           ABS( AIMAG( CX( I ) ) )
   30 CONTINUE
      SCASUM = STEMP
      RETURN
      END
