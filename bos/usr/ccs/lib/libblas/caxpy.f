* @(#)49	1.1  src/bos/usr/ccs/lib/libblas/caxpy.f, libblas, bos411, 9428A410j 12/21/89 12:06:52
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: CAXPY
*
* ORIGINS: 51 
*
*                  SOURCE MATERIALS
*
      SUBROUTINE CAXPY( N, CA, CX, INCX, CY, INCY )
*
*     constant times a vector plus a vector.
*     jack dongarra, linpack, 3/11/78.
*
*     .. Scalar Arguments ..
      INTEGER           INCX, INCY, N
      COMPLEX           CA
*     ..
*     .. Array Arguments ..
      COMPLEX           CX( 1 ), CY( 1 )
*     ..
*     .. Local Scalars ..
      INTEGER           I, IX, IY
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC         ABS, AIMAG, REAL
*     ..
*     .. Executable Statements ..
*
      IF( N.LE.0 )
     $   RETURN
      IF( ABS( REAL( CA ) )+ABS( AIMAG( CA ) ).EQ.0.0 )
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
         CY( IY ) = CY( IY ) + CA*CX( IX )
         IX = IX + INCX
         IY = IY + INCY
   10 CONTINUE
      RETURN
*
*        code for both increments equal to 1
*
   20 DO 30 I = 1, N
         CY( I ) = CY( I ) + CA*CX( I )
   30 CONTINUE
      RETURN
      END
