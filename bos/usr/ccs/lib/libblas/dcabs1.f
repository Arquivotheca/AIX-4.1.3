* @(#)88	1.1  src/bos/usr/ccs/lib/libblas/dcabs1.f, libblas, bos411, 9428A410j 12/21/89 12:15:44
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: DCABS1
*
* ORIGINS: 51
*
*                  SOURCE MATERIALS
*
************************************************************************
*
      DOUBLE PRECISION FUNCTION DCABS1( Z )
*
*     .. Scalar Arguments ..
      COMPLEX*16                        Z
*     ..
*     .. Local Scalars ..
      COMPLEX*16                        ZZ
*     ..
*     .. Local Arrays ..
      DOUBLE PRECISION                  T( 2 )
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC                         DABS
*     ..
*     .. Equivalences ..
      EQUIVALENCE                       ( ZZ, T( 1 ) )
*     ..
*     .. Executable Statements ..
*
      ZZ = Z
      DCABS1 = DABS( T( 1 ) ) + DABS( T( 2 ) )
      RETURN
      END
