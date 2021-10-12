* @(#)96	1.2  src/bos/usr/ccs/lib/libblas/dnrm2.f, libblas, bos411, 9428A410j 6/15/90 17:51:30
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: DNRM2
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
      DOUBLE PRECISION FUNCTION DNRM2( N, DX, INCX )
*
*     euclidean norm of the n-vector stored in dx() with storage
*     increment incx .
*     if    n .le. 0 return with result = 0.
*     if n .ge. 1 then incx must be .ge. 1
*
*           c.l.lawson, 1978 jan 08
*
*     four phase method     using two built-in constants that are
*     hopefully applicable to all machines.
*         cutlo = maximum of  dsqrt(u/eps)  over all known machines.
*         cuthi = minimum of  dsqrt(v)      over all known machines.
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
      INTEGER                          INCX, N
*     ..
*     .. Array Arguments ..
      DOUBLE PRECISION                 DX( 1 )
*     ..
*     .. Local Scalars ..
      INTEGER                          I, IX, J, NEXT, NN
      DOUBLE PRECISION                 CUTHI, CUTLO, HITEST, ONE, SUM,
     $                                 XMAX, ZERO
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC                        DABS, DSQRT, FLOAT
*     ..
*     .. Data statements ..
      DATA                             ZERO, ONE / 0.0D0, 1.0D0 /
      DATA                             CUTLO, CUTHI / 8.232D-11,
     $                                 1.304D19 /
*     ..
*     .. Executable Statements ..
*
*** modified to use xerbla error handling if
*   incx == 0.
*
      if (incx .lt. 1) then
         call xerbla('DNRM2 ', 3)
         dnrm2 = 0
         go to 140
         end if
*
      IF( N.GT.0 )
     $   GO TO 10
      DNRM2 = ZERO
      GO TO 140
*
   10 ASSIGN 30 TO NEXT
      SUM = ZERO
*
*        begin main loop
*
      IX = 1
      IF( INCX.LT.0 )
     $   IX = 1 - ( N-1 )*INCX
      I = IX
      NN = IX + ( N-1 )*INCX
   20 GO TO NEXT( 30, 40, 70, 80 )
   30 IF( DABS( DX( I ) ).GT.CUTLO )
     $   GO TO 110
      ASSIGN 40 TO NEXT
      XMAX = ZERO
*
*        phase 1.  sum is zero
*
   40 IF( DX( I ).EQ.ZERO )
     $   GO TO 130
      IF( DABS( DX( I ) ).GT.CUTLO )
     $   GO TO 110
*
*        prepare for phase 2.
*
      ASSIGN 70 TO NEXT
      GO TO 60
*
*        prepare for phase 4.
*
   50 I = J
      ASSIGN 80 TO NEXT
      SUM = ( SUM / DX( I ) ) / DX( I )
   60 XMAX = DABS( DX( I ) )
      GO TO 90
*
*        phase 2.  sum is small.
*                  scale to avoid destructive underflow.
*
   70 IF( DABS( DX( I ) ).GT.CUTLO )
     $   GO TO 100
*
*        common code for phases 2 and 4.
*        in phase 4 sum is large.  scale to avoid overflow.
*
   80 IF( DABS( DX( I ) ).LE.XMAX )
     $   GO TO 90
      SUM = ONE + SUM*( XMAX / DX( I ) )**2
      XMAX = DABS( DX( I ) )
      GO TO 130
*
   90 SUM = SUM + ( DX( I ) / XMAX )**2
      GO TO 130
*
*        prepare for phase 3.
*
  100 SUM = ( SUM*XMAX )*XMAX
*
*        for real or d.p. set hitest = cuthi/n
*        for complex      set hitest = cuthi/(2*n)
*
  110 HITEST = CUTHI / FLOAT( N )
*
*        phase 3.  sum is mid-range.  no scaling.
*
      DO 120 J = I, NN, INCX
         IF( DABS( DX( J ) ).GE.HITEST )
     $      GO TO 50
         SUM = SUM + DX( J )**2
  120 CONTINUE
      DNRM2 = DSQRT( SUM )
      GO TO 140
*
  130 CONTINUE
      IF( I.NE.NN ) THEN
         I = I + INCX
         GO TO 20
      ENDIF
*
*        end of main loop.
*
*        compute square root and adjust for scaling.
*
      DNRM2 = XMAX*DSQRT( SUM )
  140 CONTINUE
      RETURN
      END
