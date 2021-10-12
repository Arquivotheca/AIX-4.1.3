* @(#)91	1.3  src/bos/usr/ccs/lib/libblas/ddot.f, libblas, bos411, 9428A410j 6/15/90 17:51:17
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: DDOT
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
      DOUBLE PRECISION FUNCTION DDOT( N, DX, INCX, DY, INCY )
*
*     forms the dot product of two vectors.
*     uses unrolled loops for increments equal to one.
*     jack dongarra, linpack, 3/11/78.
*
*     .. Scalar Arguments ..
      INTEGER                         INCX, INCY, N
*     ..
*     .. Array Arguments ..
      DOUBLE PRECISION                DX( 1 ), DY( 1 )
*     ..
*     .. Local Scalars ..
      INTEGER                         I, IX, IY, M, MP1
      DOUBLE PRECISION                DTEMP
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC                       MOD
*     ..
*     .. Executable Statements ..
*
      DDOT = 0.0D0
      DTEMP = 0.0D0
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
         DTEMP = DTEMP + DX( IX )*DY( IY )
         IX = IX + INCX
         IY = IY + INCY
   10 CONTINUE
      DDOT = DTEMP
      RETURN
*
*        code for both increments equal to 1
*
**************************************************************
* IBM  12/1/89                                               *
* Improvd R2 performanced                                    *
**************************************************************
*
*        clean-up loop
*
*** start old code:
*
c  20 M = MOD( N, 5 )
c     IF( M.EQ.0 )
c    $   GO TO 40
c     DO 30 I = 1, M
c        DTEMP = DTEMP + DX( I )*DY( I )
c  30 CONTINUE
c     IF( N.LT.5 )
c    $   GO TO 60
c  40 MP1 = M + 1
c     DO 50 I = MP1, N, 5
c        DTEMP = DTEMP + DX( I )*DY( I ) + DX( I+1 )*DY( I+1 ) +
c    $           DX( I+2 )*DY( I+2 ) + DX( I+3 )*DY( I+3 ) +
c    $           DX( I+4 )*DY( I+4 )
c  50 CONTINUE
c  60 DDOT = DTEMP
*
*** end old code
*
*** start new IBM code:
*
 20   continue
      do 30 i = 1, n, 1
         dtemp = dtemp + dx(i) * dy(i)
 30      continue
      ddot = dtemp
*
*** end of new IBM code
*
      RETURN
      END
