* @(#)07	1.4  src/bos/usr/ccs/lib/libblas/dswap.f, libblas, bos411, 9428A410j 6/15/90 17:51:38
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: DSWAP
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
      SUBROUTINE DSWAP( N, DX, INCX, DY, INCY )
*
*     interchanges two vectors.
*     uses unrolled loops for increments equal one.
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
      DOUBLE PRECISION  DTEMP
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
         DTEMP = DX( IX )
         DX( IX ) = DY( IY )
         DY( IY ) = DTEMP
         IX = IX + INCX
         IY = IY + INCY
   10 CONTINUE
      RETURN
*
*        code for both increments equal to 1
*
***********************************************************
* R2 goes faster w/o this sort of unrolling               *
***********************************************************
*
*** beginning of old code:
*
*        clean-up loop
*
c  20 M = MOD( N, 3 )
c     IF( M.EQ.0 )
c    $   GO TO 40
c     DO 30 I = 1, M
c        DTEMP = DX( I )
c        DX( I ) = DY( I )
c        DY( I ) = DTEMP
c  30 CONTINUE
c     IF( N.LT.3 )
c    $   RETURN
c  40 MP1 = M + 1
c     DO 50 I = MP1, N, 3
c        DTEMP = DX( I )
c        DX( I ) = DY( I )
c        DY( I ) = DTEMP
c        DTEMP = DX( I+1 )
c        DX( I+1 ) = DY( I+1 )
c        DY( I+1 ) = DTEMP
c        DTEMP = DX( I+2 )
c        DX( I+2 ) = DY( I+2 )
c        DY( I+2 ) = DTEMP
c  50 CONTINUE
*
* end of old code
*
*** start of new IBM code:
*
 20   continue
      do 30 i = 1, n, 1
         dtemp = dx(i)
         dx(i) = dy(i)
         dy(i) = dtemp
 30      continue
*
*** end of new IBM code
*
      RETURN
      END
