* @(#)25	1.2  src/bos/usr/ccs/lib/libblas/dznrm2.f, libblas, bos411, 9428A410j 6/15/90 17:51:45
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: DZNRM2
*
* ORIGINS: 51, 27
*
* This module contains IBM CONFIDENTIAL code. -- (IBM
* Confidential Restricted when combined with the aggregated
* modules for this product)
*                  SOURCE MATERIALS
* (C) COPYRIGHT International Business Machines Corp. 1985, 1989
* All Rights Reserved
*
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
************************************************************************
*
      DOUBLE PRECISION FUNCTION DZNRM2( N, ZX, INCX )
*
*     unitary norm of the complex n-vector stored in zx() with storage
*     increment incx .
*     if    n .le. 0 return with result = 0.
*     if n .ge. 1 then incx must be .ge. 1
*
*           c.l.lawson , 1978 jan 08
*
*     four phase method     using two built-in constants that are
*     hopefully applicable to all machines.
*         cutlo = maximum of  sqrt(u/eps)  over all known machines.
*         cuthi = minimum of  sqrt(v)      over all known machines.
*     where
*         eps = smallest no. such that eps + 1. .gt. 1.
*         u   = smallest positive no.   (underflow limit)
*         v   = largest  no.            (overflow  limit)
*
*     brief outline of algorithm..
*
*     phase 1    scans zero components.
*     move to phase 2 when a component is nonzero and .le. cutlo
*     move to phase 3 when a component is .gt. cutlo
*     move to phase 4 when a component is .ge. cuthi/m
*     where m = n for x() real and m = 2*n for complex.
*
*     values for cutlo and cuthi..
*     from the environmental parameters listed in the imsl converter
*     document the limiting values are as follows..
*     cutlo, s.p.   u/eps = 2**(-102) for  honeywell.  close seconds are
*                   univac and dec at 2**(-103)
*                   thus cutlo = 2**(-51) = 4.44089e-16
*     cuthi, s.p.   v = 2**127 for univac, honeywell, and dec.
*                   thus cuthi = 2**(63.5) = 1.30438e19
*     cutlo, d.p.   u/eps = 2**(-67) for honeywell and dec.
*                   thus cutlo = 2**(-33.5) = 8.23181d-11
*     cuthi, d.p.   same as s.p.  cuthi = 1.30438d19
*     data cutlo, cuthi / 8.232d-11,  1.304d19 /
*     data cutlo, cuthi / 4.441e-16,  1.304e19 /
*
*     .. Scalar Arguments ..
      INTEGER                           INCX, N
*     ..
*     .. Array Arguments ..
      COMPLEX*16                        ZX( 1 )
*     ..
*     .. Local Scalars ..
      LOGICAL                           IMAG, SCALE
      INTEGER                           I, IX, NEXT, NN
      DOUBLE PRECISION                  ABSX, CUTHI, CUTLO, HITEST, ONE,
     $                                  SUM, XMAX, ZERO
      COMPLEX*16                        ZDUMI, ZDUMR
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC                         DABS, DSQRT, FLOAT
*     ..
*     .. Statement Functions ..
      DOUBLE PRECISION                  DIMAG, DREAL
*     ..
*     .. Statement Function definitions ..
      DREAL( ZDUMR ) = ZDUMR
      DIMAG( ZDUMI ) = ( 0.0D0, -1.0D0 )*ZDUMI
*     ..
*     .. Data statements ..
      DATA                              ZERO, ONE / 0.0D0, 1.0D0 /
      DATA                              CUTLO, CUTHI / 8.232D-11,
     $                                  1.304D19 /
*     ..
*     .. Executable Statements ..
*** modified to use xerbla error handling if
*   incx == 0.
*
      if (incx .lt. 1) then
         call xerbla('DZNRM2', 3)
         dznrm2 = zero
         go to 140
         end if
*
      IF( N.GT.0 )
     $   GO TO 10
      DZNRM2 = ZERO
      GO TO 140
*
*
   10 ASSIGN 20 TO NEXT
      SUM = ZERO
      IX = 1
      IF( INCX.LT.0 )
     $   IX = 1 - ( N-1 )*INCX
      NN = IX + ( N-1 )*INCX
*
*        begin main loop
*
      DO 130 I = IX, NN, INCX
         ABSX = DABS( DREAL( ZX( I ) ) )
         IMAG = .FALSE.
         GO TO NEXT( 20, 30, 60, 110, 70 )
   20    IF( ABSX.GT.CUTLO )
     $      GO TO 100
         ASSIGN 30 TO NEXT
         SCALE = .FALSE.
*
*           phase 1.  sum is zero
*
   30    IF( ABSX.EQ.ZERO )
     $      GO TO 120
         IF( ABSX.GT.CUTLO )
     $      GO TO 100
*
*           prepare for phase 2.
*
         ASSIGN 60 TO NEXT
         GO TO 50
*
*           prepare for phase 4.
*
   40    ASSIGN 70 TO NEXT
         SUM = ( SUM / ABSX ) / ABSX
   50    SCALE = .TRUE.
         XMAX = ABSX
         GO TO 80
*
*           phase 2.  sum is small.
*                     scale to avoid destructive underflow.
*
   60    IF( ABSX.GT.CUTLO )
     $      GO TO 90
*
*           common code for phases 2 and 4.
*           in phase 4 sum is large.  scale to avoid overflow.
*
   70    IF( ABSX.LE.XMAX )
     $      GO TO 80
         SUM = ONE + SUM*( XMAX / ABSX )**2
         XMAX = ABSX
         GO TO 120
*
   80    SUM = SUM + ( ABSX / XMAX )**2
         GO TO 120
*
*           prepare for phase 3.
*
   90    SUM = ( SUM*XMAX )*XMAX
*
  100    ASSIGN 110 TO NEXT
         SCALE = .FALSE.
*
*           for real or d.p. set hitest = cuthi/n
*           for complex      set hitest = cuthi/(2*n)
*
         HITEST = CUTHI / FLOAT( N )
*
*           phase 3.  sum is mid-range.  no scaling.
*
  110    IF( ABSX.GE.HITEST )
     $      GO TO 40
         SUM = SUM + ABSX**2
  120    CONTINUE
*
*           control selection of real and imaginary parts.
*
         IF( IMAG )
     $      GO TO 130
         ABSX = DABS( DIMAG( ZX( I ) ) )
         IMAG = .TRUE.
         GO TO NEXT( 30, 60, 110, 70 )
*
  130 CONTINUE
*
*           end of main loop.
*           compute square root and adjust for scaling.
*
      DZNRM2 = DSQRT( SUM )
      IF( SCALE )
     $   DZNRM2 = DZNRM2*XMAX
  140 CONTINUE
      RETURN
      END
