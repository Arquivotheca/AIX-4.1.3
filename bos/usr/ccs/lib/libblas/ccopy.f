* @(#)50	1.1  src/bos/usr/ccs/lib/libblas/ccopy.f, libblas, bos411, 9428A410j 12/21/89 12:07:30
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: CCOPY
*
* ORIGINS: 51
*
*                  SOURCE MATERIALS
*
************************************************************************
*
      SUBROUTINE CCOPY( N, CX, INCX, CY, INCY )
*
*     copies a vector, x, to a vector, y.
*     jack dongarra, linpack, 3/11/78.
*
*     .. Scalar Arguments ..
      INTEGER           INCX, INCY, N
*     ..
*     .. Array Arguments ..
      COMPLEX           CX( 1 ), CY( 1 )
*     ..
*     .. Local Scalars ..
      INTEGER           I, IX, IY
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
         CY( IY ) = CX( IX )
         IX = IX + INCX
         IY = IY + INCY
   10 CONTINUE
      RETURN
*
*        code for both increments equal to 1
*
   20 DO 30 I = 1, N
         CY( I ) = CX( I )
   30 CONTINUE
      RETURN
      END
