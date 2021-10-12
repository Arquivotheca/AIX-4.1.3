* @(#)97	1.1  src/bos/usr/ccs/lib/libblas/drot.f, libblas, bos411, 9428A410j 12/21/89 12:17:35
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: DROT
*
* ORIGINS: 51
*
*                  SOURCE MATERIALS
*
************************************************************************
*
      SUBROUTINE DROT( N, DX, INCX, DY, INCY, C, S )
*
*     applies a plane rotation.
*     jack dongarra, linpack, 3/11/78.
*
*     .. Scalar Arguments ..
      INTEGER          INCX, INCY, N
      DOUBLE PRECISION C, S
*     ..
*     .. Array Arguments ..
      DOUBLE PRECISION DX( 1 ), DY( 1 )
*     ..
*     .. Local Scalars ..
      INTEGER          I, IX, IY
      DOUBLE PRECISION DTEMP
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
         DTEMP = C*DX( IX ) + S*DY( IY )
         DY( IY ) = C*DY( IY ) - S*DX( IX )
         DX( IX ) = DTEMP
         IX = IX + INCX
         IY = IY + INCY
   10 CONTINUE
      RETURN
*
*        code for both increments equal to 1
*
   20 DO 30 I = 1, N
         DTEMP = C*DX( I ) + S*DY( I )
         DY( I ) = C*DY( I ) - S*DX( I )
         DX( I ) = DTEMP
   30 CONTINUE
      RETURN
      END
