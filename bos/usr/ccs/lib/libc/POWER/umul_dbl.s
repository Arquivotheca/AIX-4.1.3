# @(#)60	1.7  src/bos/usr/ccs/lib/libc/POWER/umul_dbl.s, libccnv, bos411, 9428A410j 2/21/94 14:01:04
#
# COMPONENT_NAME: LIBCCNV umul_dbl
#
# FUNCTIONS: umul_dbl
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
# NAME: umul_dbl
#                                                                    
# FUNCTION: Multiplies unsigned i * j and stores the dbl bit unsigned result in
#           the double word pointed to by r.
#                                                                    
# NOTES:
#
#       void  umul_dbl(int i; int j; int *r );
#
#       32 bit x 32 bit --> dbl bit  UNSIGNED multiplication
#
#       Multiplies unsigned i * j and stores the dbl bit unsigned result in
#       the double word pointed to by r.
#
#       This function lets the user get access to RIOS' underlying
#       multiply hardware. However, the RIOS hardware produces a
#       signed result.
#
#       The algorithm is:
#
#               do signed multiply of i * j --> result
#               if (msb of i set) result = result + (j * 2^32)
#               if (msb of j set) result = result + (i * 2^32)
#               return (result)
#
#       The "if's" are actually implemented by anding a word of all
#       signs of i with j and then adding the result to the msb of
#       the signed product then anding a word of all signs of j with
#       i and also adding the result to the msb of the signed product.
#       This is done since the anding and adding is faster on SJ than
#       a compare and branch.
#
#
#       Note: This algorithm was borrowed from Peter Markstein.
#
#       Performance estimate = 12 - 14 clocks (on Rios-1, plus
#	millicode call overhead)
#
#       These functions must work on all platforms.  To accomplish
#	this, millicode is used for the multiplication.  Per the
#	the millicode specification, register r0 is not preserved.
#
# RETURNS: 
#
# ***********************************************************************
#
#
#
#       Code
#
#
#
#       r3 =  A
#       r4 =  B
#       r5 =  pointer to place to store the result
#
#       r6 =  A*B low (also used to save link register around __mull)
#       r7 =  word full of signs of A
#       r8 =  word full of signs of B
#       r9 =  r7 & B
#       r10 = r8 & A
#       r11 = A*B high
#
#
	.file "umul_dbl.s"
	.machine "com"
	.extern __mull
	.align  2

	S_PROLOG(umul_dbl)
	mflr	 r6			# Our return address
	oril	 r7,  r3,   0		# Save A, B
	oril	 r8,  r4,   0		# (r7,r8) = (A,B)
	bla	__mull			# Call milicode (args already in r3, r4)
					# (r3:r4) = A * B (signed 64-bit)
	mtlr	r6			# restore our return address

	srai	 r6,  r7,  31		# Propogate Sign Bit A
	and	 r6,  r8,  r6		# Zero or B
	a	 r3,  r3,  r6		# Add into MS Word
	srai	 r6,  r8,  31		# Propogate Sign Bit B
	and	 r6,  r7,  r6		# Zero or A
	a	 r3,  r3,  r6		# (r3:r4) = A x B (unsigned)

	st	 r4, 4(r5)		# LS Word of result
	st	 r3, 0(r5)		# MS Word of result

	S_EPILOG

	FCNDES(umul_dbl)
