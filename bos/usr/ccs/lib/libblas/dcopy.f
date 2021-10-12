* @(#)89	1.3  src/bos/usr/ccs/lib/libblas/dcopy.f, libblas, bos411, 9428A410j 6/15/90 17:51:14
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: DCOPY
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
      SUBROUTINE DCOPY( N, DX, INCX, DY, INCY )
*
*     copies a vector, x, to a vector, y.
*     uses unrolled loops for increments equal to one.
*     jack dongarra, linpack, 3/11/78.
*
*     .. Scalar Arguments ..
      INTEGER           INCX, INCY, N
*     ..
*     .. Array Arguments ..
      DOUBLE PRECISION  DX( 1 ), DY( 1 )
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
         DY( IY ) = DX( IX )
         IX = IX + INCX
         IY = IY + INCY
   10 CONTINUE
      RETURN
*
*        code for both increments equal to 1
*
********************************************************
* IBM 12/1/89                                          *
* Rolled loops better R2 perfromance                   *
********************************************************
*
* old code:
*
*        clean-up loop
*
c  20 M = MOD( N, 7 )
c     IF( M.EQ.0 )
c    $   GO TO 40
c     DO 30 I = 1, M
c        DY( I ) = DX( I )
c  30 CONTINUE
c     IF( N.LT.7 )
c    $   RETURN
c  40 MP1 = M + 1
c     DO 50 I = MP1, N, 7
c        DY( I ) = DX( I )
c        DY( I+1 ) = DX( I+1 )
c        DY( I+2 ) = DX( I+2 )
c        DY( I+3 ) = DX( I+3 )
c        DY( I+4 ) = DX( I+4 )
c        DY( I+5 ) = DX( I+5 )
c        DY( I+6 ) = DX( I+6 )
c  50 CONTINUE
*
* end old code
*
* start new IBM code:
*
 20   continue
      do 30 i = 1, n, 1
         dy(i) = dx(i)
 30      continue
*
* end of new IBM code
*
      RETURN
      END
