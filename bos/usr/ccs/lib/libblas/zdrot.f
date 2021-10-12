* @(#)75	1.1  src/bos/usr/ccs/lib/libblas/zdrot.f, libblas, bos411, 9428A410j 12/21/89 12:42:40
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: ZDROT
*
* ORIGINS: 51
*
*                  SOURCE MATERIALS
*
************************************************************************
*
      SUBROUTINE ZDROT( N, ZX, INCX, ZY, INCY, C, S )
*
*     applies a plane rotation, where the cos and sin (c and s) are
*     double precision and the vectors zx and zy are double complex.
*     jack dongarra, linpack, 3/11/78.
*
*     .. Scalar Arguments ..
      INTEGER           INCX, INCY, N
      DOUBLE PRECISION  C, S
*     ..
*     .. Array Arguments ..
      COMPLEX*16        ZX( 1 ), ZY( 1 )
*     ..
*     .. Local Scalars ..
      INTEGER           I, IX, IY
      COMPLEX*16        ZTEMP
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
         ZTEMP = C*ZX( IX ) + S*ZY( IY )
         ZY( IY ) = C*ZY( IY ) - S*ZX( IX )
         ZX( IX ) = ZTEMP
         IX = IX + INCX
         IY = IY + INCY
   10 CONTINUE
      RETURN
*
*        code for both increments equal to 1
*
   20 DO 30 I = 1, N
         ZTEMP = C*ZX( I ) + S*ZY( I )
         ZY( I ) = C*ZY( I ) - S*ZX( I )
         ZX( I ) = ZTEMP
   30 CONTINUE
      RETURN
      END
