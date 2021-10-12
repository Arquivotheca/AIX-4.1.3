* @(#)36	1.3  src/bos/usr/ccs/lib/libblas/sdot.f, libblas, bos411, 9428A410j 6/15/90 17:52:26
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: SDOT
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
      REAL             FUNCTION SDOT( N, SX, INCX, SY, INCY )
*
*     forms the dot product of two vectors.
*     uses unrolled loops for increments equal to one.
*     jack dongarra, linpack, 3/11/78.
*
*     .. Scalar Arguments ..
      INTEGER                         INCX, INCY, N
*     ..
*     .. Array Arguments ..
      REAL                            SX( 1 ), SY( 1 )
*     ..
*     .. Local Scalars ..
      INTEGER                         I, IX, IY, M, MP1
      REAL                            STEMP
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC                       MOD
*     ..
*     .. Executable Statements ..
*
      STEMP = 0.0E0
      SDOT = 0.0E0
      IF( N.LE.0 )
     $   RETURN
      IF( INCX.EQ.1 .AND. INCY.EQ.1 )
     $   GO TO 20
*
*        code for unequal increments or equal increments
*          not equal to 1
*
      IX = 1
      IY = 1
      IF( INCX.LT.0 )
     $   IX = ( -N+1 )*INCX + 1
      IF( INCY.LT.0 )
     $   IY = ( -N+1 )*INCY + 1
      DO 10 I = 1, N
         STEMP = STEMP + SX( IX )*SY( IY )
         IX = IX + INCX
         IY = IY + INCY
   10 CONTINUE
      SDOT = STEMP
      RETURN
*
*        code for both increments equal to 1
*
******************************************************
* IBM 12/4/89 Removed use of unrolled loops          *
******************************************************
*
*** start of removed code:
*
*        clean-up loop
*
c  20 M = MOD( N, 5 )
c     IF( M.EQ.0 )
c    $   GO TO 40
c     DO 30 I = 1, M
c        STEMP = STEMP + SX( I )*SY( I )
c  30 CONTINUE
c     IF( N.LT.5 )
c    $   GO TO 60
c  40 MP1 = M + 1
c     DO 50 I = MP1, N, 5
c        STEMP = STEMP + SX( I )*SY( I ) + SX( I+1 )*SY( I+1 ) +
c    $           SX( I+2 )*SY( I+2 ) + SX( I+3 )*SY( I+3 ) +
c    $           SX( I+4 )*SY( I+4 )
c  50 CONTINUE
c  60 SDOT = STEMP
*
* end of removed code.
* start of new code:
*
 20   continue
      do 30 i = 1, n, 1
         stemp = stemp + sx(i) * sy(i)
 30      continue
      sdot = stemp
*
*** end of new code.
*
      RETURN
      END
