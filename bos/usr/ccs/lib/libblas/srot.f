* @(#)43	1.1  src/bos/usr/ccs/lib/libblas/srot.f, libblas, bos411, 9428A410j 12/21/89 12:32:57
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: SROT
*
* ORIGINS: 51
*
*                  SOURCE MATERIALS
*
************************************************************************
*
      SUBROUTINE SROT( N, SX, INCX, SY, INCY, C, S )
*
*     applies a plane rotation.
*     jack dongarra, linpack, 3/11/78.
*
*     .. Scalar Arguments ..
      INTEGER          INCX, INCY, N
      REAL             C, S
*     ..
*     .. Array Arguments ..
      REAL             SX( 1 ), SY( 1 )
*     ..
*     .. Local Scalars ..
      INTEGER          I, IX, IY
      REAL             STEMP
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
         STEMP = C*SX( IX ) + S*SY( IY )
         SY( IY ) = C*SY( IY ) - S*SX( IX )
         SX( IX ) = STEMP
         IX = IX + INCX
         IY = IY + INCY
   10 CONTINUE
      RETURN
*
*        code for both increments equal to 1
*
   20 DO 30 I = 1, N
         STEMP = C*SX( I ) + S*SY( I )
         SY( I ) = C*SY( I ) - S*SX( I )
         SX( I ) = STEMP
   30 CONTINUE
      RETURN
      END
