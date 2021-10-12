* @(#)68	1.1  src/bos/usr/ccs/lib/libblas/crotg.f, libblas, bos411, 9428A410j 12/21/89 12:12:25
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: CROTG
*
* ORIGINS: 51
*
*                  SOURCE MATERIALS
*
************************************************************************
*
      SUBROUTINE CROTG( CA, CB, C, S )
*     .. Scalar Arguments ..
      REAL              C
      COMPLEX           CA, CB, S
*     ..
*     .. Local Scalars ..
      REAL              NORM, SCALE
      COMPLEX           ALPHA
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC         CABS, CONJG, SQRT
*     ..
*     .. Executable Statements ..
      IF( CABS( CA ).NE.0. )
     $   GO TO 10
      C = 0.
      S = ( 1., 0. )
      CA = CB
      GO TO 20
   10 CONTINUE
      SCALE = CABS( CA ) + CABS( CB )
      NORM = SCALE*SQRT( ( CABS( CA / SCALE ) )**2+
     $       ( CABS( CB / SCALE ) )**2 )
      ALPHA = CA / CABS( CA )
      C = CABS( CA ) / NORM
      S = ALPHA*CONJG( CB ) / NORM
      CA = ALPHA*NORM
   20 CONTINUE
      RETURN
      END
