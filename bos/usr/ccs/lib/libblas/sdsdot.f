* @(#)37	1.1  src/bos/usr/ccs/lib/libblas/sdsdot.f, libblas, bos411, 9428A410j 12/21/89 12:31:57
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: SDSDOT
*
* ORIGINS: 51
*
*                  SOURCE MATERIALS
*
************************************************************************
*
      REAL FUNCTION SDSDOT(N,SB,SX,INCX,SY,INCY)
C1    ********************************* SDSDOT *************************
C        RETURNS SUM OF SB AND DOT PRODUCT OF SX AND SY.
C        INPUT IS S.P.,  COMPUTATION IS D.P.,  OUTPUT IS S.P.
C2
      REAL              SX(1),SY(1),SB
      DOUBLE PRECISION DSDOT
C
      DSDOT = DBLE(SB)
      IF(N .LE. 0) GO TO 30
      IF(INCX.EQ.INCY.AND.INCX.GT.0) GO TO 40
      KX = 1
      KY = 1
      IF(INCX.LT.0) KX = 1+(1-N)*INCX
      IF(INCY.LT.0) KY = 1+(1-N)*INCY
          DO 10 I = 1,N
          DSDOT = DSDOT + DBLE(SX(KX))*DBLE(SY(KY))
          KX = KX + INCX
          KY = KY + INCY
   10     CONTINUE
   30 SDSDOT = SNGL(DSDOT)
      RETURN
   40 CONTINUE
      NS = N*INCX
          DO 50 I=1,NS,INCX
          DSDOT = DSDOT + DBLE(SX(I))*DBLE(SY(I))
   50     CONTINUE
      SDSDOT = SNGL(DSDOT)
      RETURN
      END
