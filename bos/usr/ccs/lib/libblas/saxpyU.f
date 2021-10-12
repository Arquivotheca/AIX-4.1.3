* @(#)55	1.1  src/bos/usr/ccs/lib/libblas/saxpyU.f, libblas, bos411, 9428A410j 9/20/90 17:48:15
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: _SAXPY
*
* ORIGINS: 51, 27
*
* This module contains IBM CONFIDENTIAL code. -- (IBM
* Confidential Restricted when combined with the aggregated
* modules for this product)
*                  SOURCE MATERIALS
* (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1990
* All Rights Reserved
*
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
* Note:  This same code also exists in another file with the
*        same routine name except without the underscore prepended.
*        This is to provide an entry point name for VAST.  Be
*       certain that the code in the two files is kept in sync!
*
************************************************************************
*
      SUBROUTINE _SAXPY( N, SA, SX, INCX, SY, INCY )
*
*     constant times a vector plus a vector.
*     uses unrolled loop for increments equal to one.
*     jack dongarra, linpack, 3/11/78.
*
      implicit none

*     .. Scalar Arguments ..
      INTEGER           INCX, INCY, N
      REAL              SA
*     ..
*     .. Array Arguments ..
      REAL              SX( * ), SY( * )
*     ..
*     .. Local Scalars ..
      INTEGER           I, IX, IY
      INTEGER           K, J
      INTEGER           ND4, NDLOOP
      REAL*8            S1, S2, S3, S4                                         
*     ..
*     ..
*     .. Executable Statements ..
*
      IF( N.LE.0 )
     $   RETURN
      IF( SA.EQ.0.0 )
     $   RETURN
      IF( INCX.EQ.1 .AND. INCY.EQ.1 )
     $   GO TO 7
*
*        code for unequal increments or equal increments
*          not equal to 1
*     
      IX = 1
      IY = 1
      IF( INCX.LT.0 )
     $     IX = ( -N+1 )*INCX + 1
      IF( INCY.LT.0 )
     $     IY = ( -N+1 )*INCY + 1
      DO 5 I = 1, N
         SY( IY ) = SY( IY ) + SA*SX( IX )
         IX = IX + INCX
         IY = IY + INCY
 5    CONTINUE
      RETURN
*
*        code for both increments equal to 1
*
*
C                                                                               
C----  IF THE VECTOR IS SHORT, USE SPECIAL CODE                                 
C                                                                               

 7    CONTINUE

      IF ( N.LE.30 ) GOTO 30                                                   
C     
C----  CODE FOR INCX = INCY = 1                                                 
C     
      ND4 = ISHFT(N,-2)                                                        
      NDLOOP = ISHFT(ND4-1,2)                                                  
      S1 = SY(1) + DBLE(SA) * SX(1)                                            
      S2 = SY(2) + DBLE(SA) * SX(2)                                            
      S3 = SY(3) + DBLE(SA) * SX(3)                                            
      S4 = SY(4) + DBLE(SA) * SX(4)                                            
      DO 10 I = 1,NDLOOP,4                                                     
         SY(I)   = S1                                                          
         S1 = SY(I+4)   + DBLE(SA) * SX(I+4)                                   
         SY(I+1) = S2                                                          
         S2 = SY(I+5)   + DBLE(SA) * SX(I+5)                                   
         SY(I+2) = S3                                                          
         S3 = SY(I+6)   + DBLE(SA) * SX(I+6)                                   
         SY(I+3) = S4                                                          
         S4 = SY(I+7)   + DBLE(SA) * SX(I+7)                                   
 10   CONTINUE                                                                 
      ND4 = ISHFT(ND4,2)                                                       
      SY(ND4-3) = S1                                                           
      SY(ND4-2) = S2                                                           
      SY(ND4-1) = S3                                                           
      SY(ND4)   = S4                                                           
      DO 20 J = ND4+1,N                                                        
         SY(J) = SY(J) + DBLE(SA) * SX(J)                                      
 20   CONTINUE                                                                 
      RETURN                                                                   
C     
C---- CODE FOR SHORT VECTORS                                                   
C     
 30   CONTINUE                                                                 
      DO 40 K = 1,N                                                            
         SY(K) = SY(K) + DBLE(SA) * SX(K)                                      
 40   CONTINUE                                                                 
      RETURN                                                                   
c      
      END
      
