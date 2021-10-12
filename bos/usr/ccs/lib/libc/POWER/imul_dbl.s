# @(#)27	1.7  src/bos/usr/ccs/lib/libc/POWER/imul_dbl.s, libccnv, bos411, 9428A410j 2/21/94 13:59:56
#
# COMPONENT_NAME: LIBCCNV imul_dbl
#
# FUNCTIONS: imul_dbl
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# NAME: imul_dbl
#                                                                    
# FUNCTION: multiplies two integers and stores the 64 bit result
#                                                                    
# NOTES:
#
#       void  imul_dbl(int i; int j; int *r );
#
#       32 bit x 32 bit --> 64 bit  signed multiplication
#
#       Multiplies i * j and stores the 64 bit result in the
#       double word pointed to by r. i and j are signed integers.
#
#       This function lets the user get access to underlying
#       signed multiply hardware.
#
#       These functions must work on all platforms.  To accomplish
#	this, millicode is used for the multiplication.  Per the
#	the millicode specification, register r0 is not preserved.
#
# RETURNS: 
#	A 64 bit double in r
#
#
#       Code
#
#
	.file "imul_dbl.s"
	.machine "com"
	.extern __mull
	.align  2
	S_PROLOG(imul_dbl)
	mflr	r6		# save our return address
	bla	__mull		# call millicode, args already in r3, r4
				# (r3:r4) := r3 * r4 (signed, 64-bit)
	mtlr	r6		# restore our return address
	st      r3,0(r5)        # Store the MS word of the result
	st      r4,4(r5)        # Store the LS word of the result
	S_EPILOG

	FCNDES(imul_dbl)
