# @(#)12	1.2  src/bos/usr/ccs/lib/librs2/POWER/itrunc.s, librs2, bos411, 9428A410j 8/5/93 11:06:40
#
# COMPONENT_NAME: librs2
#
# FUNCTIONS: itrunc (Rios II version)
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1992
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#
# NAME: itrunc  Rios-II version
#                                                                    
# FUNCTION: converts a double to a signed 32-bit integer
#                                                                    
# EXECUTION ENVIRONMENT:
#
#       Problem-state library routine.
#                                                                    
# NOTES:
#
#       int itrunc( double x );
#
#       Double to Integer Conversion -- Truncate (Round toward zero)
#
#       itrunc is the function performed when a FORTRAN or C double
#       is converted to a signed integer.
#
#       Converts a double precision value in fr0 to a signed
#       integer in r3 as if the rounding mode were round toward zero.
#
#       itrunc expects its argument to be an FPR not a GPR; Therefore
#       a C prototype should be used so that the argument is only
#       passed in the FPR's.
#
#       Note that this routine needs a little bit of storage
#       to move a value from an FPR to a GRP.  Rather than buying
#       a stack frame, it assumes that the storage at a small
#       negative offset from the caller's stack pointer (r1) is
#       available.  This is exactly what the compiler does for
#       short leaf routines.
#
#       Uses Rios-II convert to integer instruction, and assumes
#       that the instruction correctly sets all required IEEE
#       status flags.
#
# RETURNS: 
#	A signed integer
#
# ***********************************************************************
#

	.machine "pwrx"
	S_PROLOG(itrunc)
	fcirz	fr1,fr1		# convert to integer
	stfd	fr1,-8(r1)	# result to memory
	l	r3,-4(r1)	# result to gpr
	S_EPILOG
	FCNDES(itrunc)

include(sys/comlink.m4)
