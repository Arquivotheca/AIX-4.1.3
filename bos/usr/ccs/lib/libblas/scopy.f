* @(#)35	1.3  src/bos/usr/ccs/lib/libblas/scopy.f, libblas, bos411, 9428A410j 6/15/90 17:52:22
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: SCOPY
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
      SUBROUTINE SCOPY( N, SX, INCX, SY, INCY )
*
*     copies a vector, x, to a vector, y.
*     uses unrolled loops for increments equal to 1.
*     jack dongarra, linpack, 3/11/78.
*
*     .. Scalar Arguments ..
      INTEGER           INCX, INCY, N
*     ..
*     .. Array Arguments ..
      REAL              SX( 1 ), SY( 1 )
*     ..
*     .. Local Scalars ..
      INTEGER           I, IX, IY, M, MP1
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC         MOD
*     ..
*     .. Executable Statements ..
*
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
         SY( IY ) = SX( IX )
         IX = IX + INCX
         IY = IY + INCY
   10 CONTINUE
      RETURN
*
*        code for both increments equal to 1
*
*******************************************************
* IBM 12/4/89 Removed use of unrolled loops           *
*******************************************************
*
* start of removed code
*
*        clean-up loop
*
c  20 M = MOD( N, 7 )
c     IF( M.EQ.0 )
c    $   GO TO 40
c     DO 30 I = 1, M
c        SY( I ) = SX( I )
c  30 CONTINUE
c     IF( N.LT.7 )
c    $   RETURN
c  40 MP1 = M + 1
c     DO 50 I = MP1, N, 7
c        SY( I ) = SX( I )
c        SY( I+1 ) = SX( I+1 )
c        SY( I+2 ) = SX( I+2 )
c        SY( I+3 ) = SX( I+3 )
c        SY( I+4 ) = SX( I+4 )
c        SY( I+5 ) = SX( I+5 )
c        SY( I+6 ) = SX( I+6 )
c  50 CONTINUE
*
*** start of new (rolled) code
*
 20   continue
      do 30 i = 1, n, 1
         sy(i) = sx(i)
 30      continue
*
*** end of new code
*
      RETURN
      END
