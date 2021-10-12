* @(#)49	1.4  src/bos/usr/ccs/lib/libblas/sscal.f, libblas, bos411, 9428A410j 6/15/90 17:52:46
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: SSCAL
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
      SUBROUTINE SSCAL( N, SA, SX, INCX )
*
*     scales a vector by a constant.
*     uses unrolled loops for increment equal to 1.
*     jack dongarra, linpack, 3/11/78.
*
*     .. Scalar Arguments ..
      INTEGER           INCX, N
      REAL              SA
*     ..
*     .. Array Arguments ..
      REAL              SX( 1 )
*     ..
*     .. Local Scalars ..
      INTEGER           I, IX, M, MP1, NINCX
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC         MOD
*     ..
*     .. Executable Statements ..
*
      IF( N.LE.0 )
     $   RETURN
*
*** modified to use xerbla error handling if
*   incx == 0.
*
      if (incx .lt. 1) then
         call xerbla('SSCAL ', 4)
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
         SX( I ) = SA*SX( I )
   10 CONTINUE
      RETURN
*
*        code for increment equal to 1
*
******************************************************
* IBM 12/4/89 Removed use of unrolled loops          *
******************************************************
*
*** start removed code:
*
*        clean-up loop
*
c  20 M = MOD( N, 5 )
c     IF( M.EQ.0 )
c    $   GO TO 40
c     DO 30 I = 1, M
c        SX( I ) = SA*SX( I )
c  30 CONTINUE
c     IF( N.LT.5 )
c    $   RETURN
c  40 MP1 = M + 1
c     DO 50 I = MP1, N, 5
c        SX( I ) = SA*SX( I )
c        SX( I+1 ) = SA*SX( I+1 )
c        SX( I+2 ) = SA*SX( I+2 )
c        SX( I+3 ) = SA*SX( I+3 )
c        SX( I+4 ) = SA*SX( I+4 )
c  50 CONTINUE
*
*** end removed code.
*** start new code:
*
 20   continue
      do 30 i = 1, n, 1
         sx(i) = sa * sx(i)
 30      continue
*
*** end new code.
*
      RETURN
      END
