* @(#)03	1.5  src/bos/usr/ccs/lib/libblas/dscal.f, libblas, bos411, 9428A410j 6/15/90 17:51:34
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: DSCAL
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
      SUBROUTINE DSCAL( N, DA, DX, INCX )
*
*     scales a vector by a constant.
*     uses unrolled loops for increment equal to one.
*     jack dongarra, linpack, 3/11/78.
*
*     .. Scalar Arguments ..
      INTEGER           INCX, N
      DOUBLE PRECISION  DA
*     ..
*     .. Array Arguments ..
      DOUBLE PRECISION  DX( 1 )
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
         call xerbla('DSCAL ', 4)
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
         DX( I ) = DA*DX( I )
   10 CONTINUE
      RETURN
*
*        code for increment equal to 1
*
****************************************************************
* R2 runs faster w/o this sort of unrolling.                   *
****************************************************************
*
*** start of old code:
*
*        clean-up loop
*
c  20 M = MOD( N, 5 )
c     IF( M.EQ.0 )
c    $   GO TO 40
c     DO 30 I = 1, M
c        DX( I ) = DA*DX( I )
c  30 CONTINUE
c     IF( N.LT.5 )
c    $   RETURN
c  40 MP1 = M + 1
c     DO 50 I = MP1, N, 5
c        DX( I ) = DA*DX( I )
c        DX( I+1 ) = DA*DX( I+1 )
c        DX( I+2 ) = DA*DX( I+2 )
c        DX( I+3 ) = DA*DX( I+3 )
c        DX( I+4 ) = DA*DX( I+4 )
c  50 CONTINUE
*
*** end of old code
*
*** start of new IBM code:
*
 20   continue
      do 30 i = 1, n, 1
         dx(i) = da * dx(i)
 30      continue
*
*** end of new IBM code
*
      RETURN
      END
