C 	@(#)45  1.1  src/bos/usr/ccs/lib/libdbx/qbridge.f, libdbx, bos411, 9428A410j 12/8/90 18:41:24
C
C	 COMPONENT_NAME: LIBDBX 
C	
C	 FUNCTIONS: qadd, qdiv, qmul, qsub, qpow 
C	
C	 ORIGINS: 27
C	
C	 This module contains IBM CONFIDENTIAL code. -- (IBM
C	 Confidential Restricted when combined with the aggregated
C	 modules for this product)
C	                  SOURCE MATERIALS
C	 (C) COPYRIGHT International Business Machines Corp. 1988, 1989
C	 All Rights Reserved
C	
C	 US Government Users Restricted Rights - Use, duplication or
C	 disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
C	

C	This file contains bridge routines for calling FORTRAN
C	quad routines from a C program. These routines are needed
C	because the current XLC makes no provision for receiving 
C	16 bytes (quad) number as result in registers.

@process mixed debug(callby)
        subroutine qadd(%ref(a), %val(X),%val(x),%val(Y),%val(y))
	implicit real*8 (A-H,O-Z)
	double complex a, _xlqadd
	a = _xlqadd(%val(X),%val(x),%val(Y),%val(y))
	return
	end

@process mixed debug(callby)
        subroutine qdiv(%ref(a), %val(X),%val(x),%val(Y),%val(y))
	implicit real*8 (A-H,O-Z)
	double complex a, _xlqdiv
	a = _xlqdiv(%val(X),%val(x),%val(Y),%val(y))
	return
	end

@process mixed debug(callby)
        subroutine qmul(%ref(a), %val(X),%val(x),%val(Y),%val(y))
	implicit real*8 (A-H,O-Z)
	double complex a, _xlqmul
	a = _xlqmul(%val(X),%val(x),%val(Y),%val(y))
	return
	end

@process mixed debug(callby)
        subroutine qsub(%ref(a), %val(X),%val(x),%val(Y),%val(y))
	implicit real*8 (A-H,O-Z)
	double complex a, _xlqsub
	a = _xlqsub(%val(X),%val(x),%val(Y),%val(y))
	return
	end


@process mixed debug(callby)
        subroutine qpow(%ref(a), %val(X),%val(x),%val(Y),%val(y))
	implicit real*8 (A-H,O-Z)
	double complex a, _xlqsub
	a = _qpow(%val(X),%val(x),%val(Y),%val(y))
	return
	end
