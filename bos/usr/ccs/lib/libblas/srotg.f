* @(#)45	1.1  src/bos/usr/ccs/lib/libblas/srotg.f, libblas, bos411, 9428A410j 12/21/89 12:34:25
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: SROTG
*
* ORIGINS: 51
*
*                  SOURCE MATERIALS
*
************************************************************************
*
      SUBROUTINE SROTG( SA, SB, C, S )
*
*     construct givens plane rotation.
*     jack dongarra, linpack, 3/11/78.
*                    modified 9/27/86.
*
*     .. Scalar Arguments ..
      REAL              C, S, SA, SB
*     ..
*     .. Local Scalars ..
      REAL              R, ROE, SCALE, Z
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC         ABS, SIGN, SQRT
*     ..
*     .. Executable Statements ..
*
      ROE = SB
      IF( ABS( SA ).GT.ABS( SB ) )
     $   ROE = SA
      SCALE = ABS( SA ) + ABS( SB )
      IF( SCALE.NE.0.0 )
     $   GO TO 10
      C = 1.0
      S = 0.0
      R = 0.0
      GO TO 20
   10 R = SCALE*SQRT( ( SA / SCALE )**2+( SB / SCALE )**2 )
      R = SIGN( 1.0, ROE )*R
      C = SA / R
      S = SB / R
   20 Z = S
      IF( ABS( C ).GT.0.0 .AND. ABS( C ).LE.S )
     $   Z = 1.0 / C
      SA = R
      SB = Z
      RETURN
      END
