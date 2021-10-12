* @(#)70	1.2  src/bos/usr/ccs/lib/libblas/xerbla.f, libblas, bos411, 9428A410j 6/15/90 17:52:53
*
* COMPONENT_NAME: LIBBLAS Basic Linear Algebra Subroutine Library
*
* FUNCTIONS: XERBLA
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
************************************************************************
*
      SUBROUTINE XERBLA( SRNAME, INFO )
*
*  -- LAPACK auxiliary routine --
*     Argonne National Laboratory
*     November 16, 1988
*
*     .. Scalar Arguments ..
      CHARACTER*6        SRNAME
      INTEGER            INFO
*     ..
*
*  Purpose
*  =======
*
*     XERBLA  is an error handler for the LAPACK routines.
*     It is called by an LAPACK routine if an input parameter has an
*     invalid value.  A message is printed and execution stops.
*
*     Installers may consider modifying the STOP statement in order to
*     call system-specific exception-handling facilities.
*
*  Parameters
*  ==========
*
*  SRNAME - CHARACTER*6.
*           On entry, SRNAME specifies the name of the routine which
*           called XERBLA.
*
*  INFO   - INTEGER.
*           On entry, INFO specifies the position of the invalid
*           parameter in the parameter-list of the calling routine.
*
*
*     WRITE( *, FMT = 9999 )SRNAME, INFO
*
*************************************************
*
* call a c language routine which actually issues the message 
* (pretty much just as this routine did).  However, it will
* use the message facility, and be able to take care of
* NLS and such.
*
      call blas_fatal(%REF(srname), %REF(info))
*
*************************************************
*
      STOP
*
* 9999 FORMAT( ' ** On entry to ', A6, ' parameter number ', I2, ' had ',
*     $      'an illegal value' )
*
*     End of XERBLA
*
      END
