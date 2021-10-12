* @(#)50	1.1  src/bos/usr/ccs/lib/libblas/daxpyU.f, libblas, bos411, 9428A410j 9/20/90 17:13:28
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: _DAXPY
*
* ORIGINS: 51, 27
*
* This module contains IBM CONFIDENTIAL code. -- (IBM
* Confidential Restricted when combined with the aggregated
* modules for this product)
*                  SOURCE MATERIALS
* (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1990
* All Rights Reserved
*
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
* Note:  This same code also exists in another file with the
*        same routine name except without the underscore prepended.
*        This is to provide an entry point name for VAST.  Be
*       certain that the code in the two files is kept in sync!
*
************************************************************************
*
      SUBROUTINE _DAXPY( N, DA, DX, INCX, DY, INCY )
*
*     constant times a vector plus a vector.
*     uses unrolled loops for increments equal to one.
*     jack dongarra, linpack, 3/11/78.
*
      implicit none

*     .. Scalar Arguments ..
      INTEGER           INCX, INCY, N
      DOUBLE PRECISION  DA
*     ..
*     .. Array Arguments ..
      DOUBLE PRECISION  DX( * ), DY( * )
*     ..
*     .. Local Scalars ..
      INTEGER*4         I, J, K, IX, IY, ND4, NDLOOP
      REAL*8            D1,D2,D3,D4
      
*     ..
*     .. Executable Statements ..
*
      IF( N.LE.0 )
     $   RETURN
      IF( DA.EQ.0.0D0 )
     $   RETURN
      IF( INCX.EQ.1 .AND. INCY.EQ.1 )
     $   GO TO 7
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
      DO 5 I = 1, N
         DY( IY ) = DY( IY ) + DA*DX( IX )
         IX = IX + INCX
         IY = IY + INCY
 5    CONTINUE
      RETURN
*
*        code for both increments equal to 1
*
C
C----  IF THE VECTOR IS SHORT, USE SPECIAL CODE
C
 7    CONTINUE
      IF ( N.LE.30) GOTO 30
C
C---- CODE FOR INCX = INCY = 1
C     
      ND4 = ISHFT(N,-2)
      NDLOOP = ISHFT(ND4-1,2)
      D1 = DY(1) + DA * DX(1)
      D2 = DY(2) + DA * DX(2)
      D3 = DY(3) + DA * DX(3)
      D4 = DY(4) + DA * DX(4)
      DO 10 I = 1,NDLOOP,4
         DY(I)   = D1
         D1 = DY(I+4)   + DA * DX(I+4)
         DY(I+1) = D2
         D2 = DY(I+5)   + DA * DX(I+5)
         DY(I+2) = D3
         D3 = DY(I+6)   + DA * DX(I+6)
         DY(I+3) = D4
         D4 = DY(I+7)   + DA * DX(I+7)
 10   CONTINUE
      ND4 = ISHFT(ND4,2)
      DY(ND4-3) = D1
      DY(ND4-2) = D2
      DY(ND4-1) = D3
      DY(ND4)   = D4
      DO 20 J = ND4+1,N
         DY(J) = DY(J) + DA * DX(J)
 20   CONTINUE
      RETURN
C     
C---- CODE FOR SHORT VECTORS
C     
 30    CONTINUE
       DO 40 K = 1,N
          DY(K) = DY(K) + DA * DX(K)
 40    CONTINUE
       RETURN
C     
       END
