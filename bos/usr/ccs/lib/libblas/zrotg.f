* @(#)95	1.1  src/bos/usr/ccs/lib/libblas/zrotg.f, libblas, bos411, 9428A410j 12/21/89 12:47:04
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: ZROTG
*
* ORIGINS: 51
*
*                  SOURCE MATERIALS
*
************************************************************************
*
      SUBROUTINE ZROTG( CA, CB, C, S )
*     .. Scalar Arguments ..
      DOUBLE PRECISION  C
      COMPLEX*16        CA, CB, S
*     ..
*     .. Local Scalars ..
      DOUBLE PRECISION  NORM, SCALE
      COMPLEX*16        ALPHA
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC         CDABS, DCMPLX, DCONJG, DSQRT
*     ..
*     .. Executable Statements ..
      IF( CDABS( CA ).NE.0.0D0 )
     $   GO TO 10
      C = 0.0D0
      S = ( 1.0D0, 0.0D0 )
      CA = CB
      GO TO 20
   10 CONTINUE
      SCALE = CDABS( CA ) + CDABS( CB )
      NORM = SCALE*DSQRT( ( CDABS( CA / DCMPLX( SCALE, 0.0D0 ) ) )**2+
     $       ( CDABS( CB / DCMPLX( SCALE, 0.0D0 ) ) )**2 )
      ALPHA = CA / CDABS( CA )
      C = CDABS( CA ) / NORM
      S = ALPHA*DCONJG( CB ) / NORM
      CA = ALPHA*NORM
   20 CONTINUE
      RETURN
      END
