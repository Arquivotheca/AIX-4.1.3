* @(#)86	1.4  src/bos/usr/ccs/lib/libblas/dasum.f, libblas, bos411, 9428A410j 6/15/90 17:51:06
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: DASUM
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
      DOUBLE PRECISION FUNCTION DASUM( N, DX, INCX )
*
*     takes the sum of the absolute values.
*     jack dongarra, linpack, 3/11/78.
*
*     .. Scalar Arguments ..
      INTEGER                          INCX, N
*     ..
*     .. Array Arguments ..
      DOUBLE PRECISION                 DX( 1 )
*     ..
*     .. Local Scalars ..
      INTEGER                          I, IX, M, MP1, NINCX
      DOUBLE PRECISION                 DTEMP
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC                        DABS, MOD
*     ..
*     .. Executable Statements ..
*
      DASUM = 0.0D0
      DTEMP = 0.0D0
      IF( N.LE.0 )
     $   RETURN
*
*** modified to use xerbla error handling if
*   incx == 0.
*
      if (incx .lt. 1) then
         call xerbla('DASUM ', 3)
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
         DTEMP = DTEMP + DABS( DX( I ) )
   10 CONTINUE
      DASUM = DTEMP
      RETURN
*
*        code for increment equal to 1
*
***********************************************
* IBM 12/1/89                                 *
* Removed unrolling for better R2 performance *
***********************************************
*
*** old code:
*
*        clean-up loop
*
c  20 M = MOD( N, 6 )
c     IF( M.EQ.0 )
c    $   GO TO 40
c     DO 30 I = 1, M
c        DTEMP = DTEMP + DABS( DX( I ) )
c  30 CONTINUE
c     IF( N.LT.6 )
c    $   GO TO 60
c  40 MP1 = M + 1
c     DO 50 I = MP1, N, 6
c        DTEMP = DTEMP + DABS( DX( I ) ) + DABS( DX( I+1 ) ) +
c    $           DABS( DX( I+2 ) ) + DABS( DX( I+3 ) ) +
c    $           DABS( DX( I+4 ) ) + DABS( DX( I+5 ) )
c  50 CONTINUE
c  60 DASUM = DTEMP
*
* end old code
*
* start new IBM code:
*
 20   continue
      do 30 i = 1, n, 1
         dtemp = dtemp + dabs(dx(i))
 30      continue
      dasum = dtemp   
*
* end new IBM code
*
      RETURN
      END
