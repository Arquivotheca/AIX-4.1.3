* @(#)73	1.1  src/bos/usr/ccs/lib/libblas/zdotc.f, libblas, bos411, 9428A410j 12/21/89 12:42:20
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: ZDOTC
*
* ORIGINS: 51
*
*                  SOURCE MATERIALS
*
************************************************************************
*
      DOUBLE COMPLEX   FUNCTION ZDOTC( N, ZX, INCX, ZY, INCY )
*
*     forms the dot product of a vector.
*     jack dongarra, 3/11/78.
*
*     .. Scalar Arguments ..
      INTEGER                          INCX, INCY, N
*     ..
*     .. Array Arguments ..
      COMPLEX*16                       ZX( 1 ), ZY( 1 )
*     ..
*     .. Local Scalars ..
      INTEGER                          I, IX, IY
      COMPLEX*16                       ZTEMP
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC                        DCONJG
*     ..
*     .. Executable Statements ..
      ZTEMP = ( 0.0D0, 0.0D0 )
      ZDOTC = ( 0.0D0, 0.0D0 )
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
         ZTEMP = ZTEMP + DCONJG( ZX( IX ) )*ZY( IY )
         IX = IX + INCX
         IY = IY + INCY
   10 CONTINUE
      ZDOTC = ZTEMP
      RETURN
*
*        code for both increments equal to 1
*
   20 DO 30 I = 1, N
         ZTEMP = ZTEMP + DCONJG( ZX( I ) )*ZY( I )
   30 CONTINUE
      ZDOTC = ZTEMP
      RETURN
      END
