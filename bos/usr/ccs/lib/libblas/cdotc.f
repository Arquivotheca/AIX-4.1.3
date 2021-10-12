* @(#)51	1.1  src/bos/usr/ccs/lib/libblas/cdotc.f, libblas, bos411, 9428A410j 12/21/89 12:07:41
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: CDOTC
*
* ORIGINS: 51
*
*                  SOURCE MATERIALS
*
************************************************************************
*
      COMPLEX          FUNCTION CDOTC( N, CX, INCX, CY, INCY )
*
*     forms the dot product of two vectors, conjugating the first
*     vector.
*     jack dongarra, linpack,  3/11/78.
*
*     .. Scalar Arguments ..
      INTEGER                          INCX, INCY, N
*     ..
*     .. Array Arguments ..
      COMPLEX                          CX( 1 ), CY( 1 )
*     ..
*     .. Local Scalars ..
      INTEGER                          I, IX, IY
      COMPLEX                          CTEMP
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC                        CONJG
*     ..
*     .. Executable Statements ..
*
      CTEMP = ( 0.0, 0.0 )
      CDOTC = ( 0.0, 0.0 )
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
         CTEMP = CTEMP + CONJG( CX( IX ) )*CY( IY )
         IX = IX + INCX
         IY = IY + INCY
   10 CONTINUE
      CDOTC = CTEMP
      RETURN
*
*        code for both increments equal to 1
*
   20 DO 30 I = 1, N
         CTEMP = CTEMP + CONJG( CX( I ) )*CY( I )
   30 CONTINUE
      CDOTC = CTEMP
      RETURN
      END
