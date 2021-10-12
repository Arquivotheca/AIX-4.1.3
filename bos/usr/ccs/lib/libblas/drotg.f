* @(#)98	1.1  src/bos/usr/ccs/lib/libblas/drotg.f, libblas, bos411, 9428A410j 12/21/89 12:17:49
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: DROTG
*
* ORIGINS: 51
*
*                  SOURCE MATERIALS
*
************************************************************************
*
      SUBROUTINE DROTG( DA, DB, C, S )
*
*     construct givens plane rotation.
*     jack dongarra, linpack, 3/11/78.
*                    modified 9/27/86.
*
*     .. Scalar Arguments ..
      DOUBLE PRECISION  C, DA, DB, S
*     ..
*     .. Local Scalars ..
      DOUBLE PRECISION  R, ROE, SCALE, Z
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC         DABS, DSIGN, DSQRT
*     ..
*     .. Executable Statements ..
*
      ROE = DB
      IF( DABS( DA ).GT.DABS( DB ) )
     $   ROE = DA
      SCALE = DABS( DA ) + DABS( DB )
      IF( SCALE.NE.0.0D0 )
     $   GO TO 10
      C = 1.0D0
      S = 0.0D0
      R = 0.0D0
      GO TO 20
   10 R = SCALE*DSQRT( ( DA / SCALE )**2+( DB / SCALE )**2 )
      R = DSIGN( 1.0D0, ROE )*R
      C = DA / R
      S = DB / R
   20 Z = S
      IF( DABS( C ).GT.0.0D0 .AND. DABS( C ).LE.S )
     $   Z = 1.0D0 / C
      DA = R
      DB = Z
      RETURN
      END
