# @(#)13	1.2  src/bos/usr/ccs/lib/librs2/POWER/sqrt.s, librs2, bos411, 9428A410j 8/5/93 11:06:44
#
# COMPONENT_NAME: librs2
#
# FUNCTIONS: sqrt (Rios-II version)
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
# NAME: sqrt    Rios-II version
#                                                                    
# FUNCTION: COMPUTES THE SQUARE ROOT OF X
#                                                                    
# EXECUTION ENVIRONMENT:
#
#  Problem-state library routine.
#                                                                   
# NOTES:
#
#  Rios-II version, using square root opcode.  Maintains
#  ANSI semantics by setting errno to EDOM in case of
#  a negative arguement.
#
#  This routine depends on the square root instruction to
#  deliver the correctly rounded result and set all required
#  IEEE status flags correctly.  It must do a compare and branch
#  in order to handle errno correctly as required by Standard C.
#  Note that the branch is delayed until after the square root
#  instruction to allow the compare to overlap with the
#  the square root instruction.
#
# RETURNS: The square root of x
#
# If x is negative, then a QNaN is returned and errno is set to EDOM.
#
# ***********************************************************************
#
	.machine "pwrx"
	.set EDOM,33

	.toc
	TOCE(errno,data)                # external error variable
z0x0000.S: .tc  _0x00000000[tc],0     	# put single precision 0.0 in toc

	.align 2
	S_PROLOG(sqrt)
	lfs     fr0,z0x0000.S(r2)      	# load 0.0 into  fr0 
	fcmpo	cr0,fr1,fr0		# compare arg to 0.0
	fsqrt	fr1,fr1			# do square root
	bger	cr0			# return if non-negative number
# The code falls thru here if the argument was
# negative.
	LTOC(r4,errno,data)             # Get pointer to errno
	cal     r3,EDOM(0)		# domain error to errno
	st      r3,0(r4)              	# errno = EDOM
	S_EPILOG		
	FCNDES(sqrt)

include(sys/comlink.m4)
