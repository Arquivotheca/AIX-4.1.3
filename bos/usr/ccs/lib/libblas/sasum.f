* @(#)31	1.4  src/bos/usr/ccs/lib/libblas/sasum.f, libblas, bos411, 9428A410j 6/15/90 17:52:07
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: SASUM
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
      REAL             FUNCTION SASUM( N, SX, INCX )
*
*     takes the sum of the absolute values.
*     uses unrolled loops for increment equal to one.
*     jack dongarra, linpack, 3/11/78.
*
*     .. Scalar Arguments ..
      INTEGER                          INCX, N
*     ..
*     .. Array Arguments ..
      REAL                             SX( 1 )
*     ..
*     .. Local Scalars ..
      INTEGER                          I, IX, M, MP1, NINCX
      REAL                             STEMP
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC                        ABS, MOD
*     ..
*     .. Executable Statements ..
*
      SASUM = 0.0E0
      STEMP = 0.0E0
      IF( N.LE.0 )
     $   RETURN
*
*** modified to use xerbla error handling if
*   incx == 0.
*
      if (incx .lt. 1) then
         call xerbla('SASUM ', 3)
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
         STEMP = STEMP + ABS( SX( I ) )
   10 CONTINUE
      SASUM = STEMP
      RETURN
*
*        code for increment equal to 1
*
**********************************************
* IBM 12/4/89 Revoved unrolled loops code    *
**********************************************
*
*        clean-up loop
*
* start removed code:
*
c  20 M = MOD( N, 6 )
c     IF( M.EQ.0 )
c    $   GO TO 40
c     DO 30 I = 1, M
c        STEMP = STEMP + ABS( SX( I ) )
c  30 CONTINUE
c     IF( N.LT.6 )
c    $   GO TO 60
c  40 MP1 = M + 1
c     DO 50 I = MP1, N, 6
c        STEMP = STEMP + ABS( SX( I ) ) + ABS( SX( I+1 ) ) +
c    $           ABS( SX( I+2 ) ) + ABS( SX( I+3 ) ) +
c    $           ABS( SX( I+4 ) ) + ABS( SX( I+5 ) )
c  50 CONTINUE
c  60 SASUM = STEMP
*
* end removed code.
*
* start new (rolled) code
*
 20   continue
      do 30 i = 1, n, 1
         stemp = stemp + abs(sx(i))
 30      continue
      sasum = stemp
*
* end unrolled code
*
      RETURN
      END
