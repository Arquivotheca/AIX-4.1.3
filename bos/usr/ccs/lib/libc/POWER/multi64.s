# @(#)98	1.5  src/bos/usr/ccs/lib/libc/POWER/multi64.s, libccnv, bos411, 9428A410j 2/21/94 14:00:31
#
#   COMPONENT_NAME: LIBCCNV
#
#   FUNCTIONS: __multi64
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#******************************************************************************
#
# long long	__multi64( long long, long long )
#
# Signed Multiply
#   (r3:r4) = (r3:r4) * (r5:r6)    (64b) = (64b * 64b) discard high 64b
#
# Because the multiply actually produces a 128-bit result and the high part
# is ignored, there is no code here to adjust the sign of the result.
# It simply isn't necessary.
#
# This implementation calls processor-dependant milicode for its only
# 32x32=64 bit multiply.  The PowerPC has no native instruction for this.
# Per the milicode spec, r0 is volatile.
# Notice: Special convolutions are required to compute Result 1 because the
# milicode only implements a Signed Multiply operation.
#
#                        A : B                   (r3:r4) -> (r7:r8)
#                  x     C : D                   (r5:r6)
#      <32b>       -----------
#     |     |     |  B  x  D  |	 Result 1        (r3:r4)  Full 64bit product
#  +  |     |  C  x  B  |     |  Result 2     (  :r5)	  Only LSW matters
#  +  |     |  A  x  D  |     |  Result 3     (  :r7)	  Only LSW matters
#  +  |  A  x  C  |     |     |            (  :  )	  All truncated
#     -------------------------            -------------
#                 |     |     |                  (r3:r4)  Truncated Result
	.file "multi64.s"

# AIX Version 3 release(s) won't use millicode; the bla to _mull is replaced
# by the POWER only (+601) 'mul'/'mfspr' instruction pair.

#	.machine "pwr"
#	S_PROLOG(__multi64)
#
#	oril	 r7,  r3,   0			# Save A, B
#	oril	 r8,  r4,   0			# (r7:r8) = (A:B)
#	oril	 r3,  r6,   0			# Move D
#	mul	 r3,  r3,  r4			# r3 = r3 * r4
#	mfspr	 r4,  mq			# get low order part
#						# (r3:r4) = B * D (signed)
#	srai	 r9,  r6,  31			# Propogate Sign Bit
#	and	 r9,  r8,  r9			# Zero or B
#	a	 r3,  r3,  r9			#
#	srai	 r9,  r8,  31			# Propogate Sign Bit
#	and	 r9,  r6,  r9			# Zero or D
#	a	 r3,  r3,  r9			# (r3:r4) = B x D (unsigned)
#						#
#	muls	 r7,  r6,  r7			# Result 3 LSW  D x A
#	muls	 r5,  r8,  r5			# Result 2 LSW  B x C
#	a	 r3,  r3,  r5			# Result 3 LSW + Result 2 LSW
#	a	 r3,  r3,  r7			# + Result 1 MSW
#
#	S_EPILOG(__multi64)

# The code below is a complete replacement for AIX 4

	.machine "com"
	S_PROLOG(__multi64)
	.extern	__mull
						#
	mflr	 r9				# Our return address
	oril	 r7,  r3,   0			# Save A, B
	oril	 r8,  r4,   0			# (r7:r8) = (A:B)
	oril	 r3,  r6,   0			# Move D
	bla	__mull				# Call milicode
	mtlr	 r9				# (r3:r4) = B * D (signed)
						#
	srai	 r9,  r6,  31			# Propogate Sign Bit
	and	 r9,  r8,  r9			# Zero or B
	a	 r3,  r3,  r9			#
	srai	 r9,  r8,  31			# Propogate Sign Bit
	and	 r9,  r6,  r9			# Zero or D
	a	 r3,  r3,  r9			# (r3:r4) = B x D (unsigned)
						#
	muls	 r7,  r6,  r7			# Result 3 LSW  D x A
	muls	 r5,  r8,  r5			# Result 2 LSW  B x C
	a	 r3,  r3,  r5			# Result 3 LSW + Result 2 LSW
	a	 r3,  r3,  r7			# + Result 1 MSW
	S_EPILOG(__multi64)
