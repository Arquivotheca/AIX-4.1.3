* @(#)97	1.1  src/bos/usr/ccs/lib/libblas/zswap.f, libblas, bos411, 9428A410j 12/21/89 12:47:31
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: ZSWAP
*
* ORIGINS: 51
*
*                  SOURCE MATERIALS
*
************************************************************************
*
      SUBROUTINE ZSWAP( N, ZX, INCX, ZY, INCY )
*
*     interchanges two vectors.
*     jack dongarra, 3/11/78.
*
*     .. Scalar Arguments ..
      INTEGER           INCX, INCY, N
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
         ZTEMP = ZX( IX )
         ZX( IX ) = ZY( IY )
         ZY( IY ) = ZTEMP
         IX = IX + INCX
         IY = IY + INCY
   10 CONTINUE
      RETURN
*
*        code for both increments equal to 1
*
   20 DO 30 I = 1, N
         ZTEMP = ZX( I )
         ZX( I ) = ZY( I )
         ZY( I ) = ZTEMP
   30 CONTINUE
      RETURN
      END
