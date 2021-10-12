# @(#)90	1.4  src/bos/usr/ccs/lib/libc/POWER/divu64.s, libccnv, bos411, 9428A410j 2/21/94 13:59:40
#
#   COMPONENT_NAME: LIBCCNV
#
#   FUNCTIONS: __divu64
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
# long long	__divu64( long long, long long )
#
#   (r3:r4) = (r3:r4) / (r5:r6)    (64b) = (64b / 64b)
#     quo       dvd       dvs
#
# Final REMAINDER is computed and returned in r5:r6.
#
# Implementation:
#
# This algorithm, described in detail in the design specification, rotates the
# dividend (dvd) left into a temporary (tmp) one bit per iteration, each
# iteration comparing tmp with the divisor (dvs).  The result of the compare
# (0 if # tmp<dvs, else 1) is rotated into the quotient (quo), which is built
# physically in the dvd as it is rotated out.  When the dvd is exhausted
# (logically # after 64 bits), quo occupies dvd, the remainder (rmd) is in tmp.
#
# First, the dvd is preshifted into tmp based on the number of leading zeroes
# in both the dvd and dvs.  This makes the first iteration of the main loop
# always produce a quo '0' bit, but otherwise reduces the number of iterations
# in a highly data-dependent way.  The code is a little convoluted since the
# MSWs and LSWs have to be treated separately in word-sized (32-bit) chunks.
# Thus the count leading zeroes may return 32, i.e. a whole word of zeroes,
# and such situations are handled.  The main loop iteration count is lowered
# to reflect any optimization from preshifting.
#
# When the Count Register reaches zero, the last quo bit is rotated in.
#
# Code comment notation:
#   MSW : Most Significant (High) Word, i.e. bits 0..31
#   LSW : Least Significant (Low) Word, i.e. bits 32..63
#   LZ  : Leading Zeroes
#   SD  : Significant Digits
#
# r3:r4 : dvd  (dividend)     :     quo (quotient)
# r5:r6 : dvs  (divisor)      :     rem (remainder)
#
# r7:r8 : tmp
# r0    : Intermediate subtract LSW
# r9    : Intermediate subtract MSW
# r10   : -1 (always)
# CTR   : cnt

	.file "divu64.s"
	.machine "com"
	S_PROLOG(__divu64)

					# Compute total Dividend Leading 0s
	cntlz	 r0,  r3			# dvd.msw
	cmpi	cr0,  r0,  32			# dvd.msw == 0?
	cntlz	 r9,  r4			# dvd.lsw
	bne	cr0, ud1			# dvd.msw == 0
	a	 r0,  r0,  r9			# all dvd.LZ
ud1:					# Compute total Divisor Leading 0s
	cntlz	 r9,  r5			# dvs.msw
	cmpi	cr7,  r9,  32			# dvd.msw == 0?
	cntlz	r10,  r6			# dvs.lsw
	bne	cr7, ud2			# dvs.msw == 0
	a	 r9,  r9, r10			# all dvs.LZ
ud2:					# Is Divisor > Dividend?
	cmp	cr0,  r0,  r9			# dvd.LZ to dvs.LZ
	sfi	r10,  r0,  64			# dvd.LZ -> dvd.SD
	bgt	cr0, ud9			# dvs > dvd (quo=0)
					# Compute the number of iterations
	ai	 r9,  r9,   1			# ++dvs.LZ (--dvs.SD)
	sfi	 r9,  r9,  64			# dvs.LZ -> dvs.SD
	a	 r0,  r0,  r9			# dvd shift (L) dvd.LZ+dvs.SD
	sf	 r9,  r9, r10			# tmp shift (R) dvd.SD-dvs.SD
	mtctr	 r9				# Iterations
					# Preshift Divisor into Temp
						# r7:r8 = r3:r4 << r9
	cmpi	cr0,  r9,  32			# 32 or more bits?
	si	 r7,  r9,  32			#
	blt	cr0, ud3			# Jumps if not
	sr	 r8,  r3,  r7			# MSW shifted into LSW
	cal	 r7, 0(r0)			# final tmp.msw
	b	ud4				#
ud3:						#
	sr	 r8,  r4,  r9			# Shift LSW
	sfi	 r7,  r9,  32			# Bits to rotate (left)
	sl	 r7,  r3,  r7			# Isolate LSBits of MSW
	or	 r8,  r8,  r7			# Insert MSBits of LSW
	sr	 r7,  r3,  r9			# final tmp.msw
ud4:					# Preshift Divisor, Preclear Quotient
						# r3:r4 = r3:r4 << r0
	cmpi	cr0,  r0,  32			# 32 or more bits?
	si	 r9,  r0,  32			#
	blt	cr0, ud5			# Jumps if not
	sl	 r3,  r4,  r9			# LSW shifted into MSW
	cal	 r4, 0(r0)			# final dvd.lsw
	b	ud6				#
ud5:						#
	sl	 r3,  r3,  r0			# Shift MSW
	sfi	 r9,  r0,  32			#
	sr	 r9,  r4,  r9			#
	or	 r3,  r3,  r9			# final dvd.msw
	sl	 r4,  r4,  r0			# final dvd.lsw
ud6:					# Prepare constants for the main loop
	cal	r10, -1(r0)			#
	ai	 r7,  r7,   0			# tmp.lsw (Clear CY)
ud7:					# Main Loop
	ae	 r4,  r4,  r4			# dvd.lsw (lsb = CY)
	ae	 r3,  r3,  r3			# dvd.msw (lsb from lsw.msb)
	ae	 r8,  r8,  r8			# tmp.lsw (lsb from dvd carry)
	ae	 r7,  r7,  r7			# tmp.msw
						#
	sf	 r0,  r6,  r8			#
	sfe.	 r9,  r5,  r7			#
	blt	ud8				# Jmp !CY
	oril	 r8,  r0,   0			# Move lsw
	oril	 r7,  r9,   0			# Move msw
	ai	 r0, r10,   1			# Set CY
ud8:						#
	bdn	ud7				# Decrement CTR, branch !0
					# Rotate in the final Quotient bit
	ae	 r4,  r4,  r4			# quo.lsw (lsb = CY)
	ae	 r3,  r3,  r3			# quo.msw (lsb from lsw)
	oril	 r6,  r8,   0			# rem.lsw
	oril	 r5,  r7,   0			# rem.msw
	br					# Return
ud9:					# Quotient is 0 (dvs > dvd)
	oril	 r6,  r4,   0			# rmd.lsw = dvd.lsw
	oril	 r5,  r3,   0			# rmd.msw = dvd.msw
	cal	 r4, 0(r0)			# dvd.lsw = 0
	oril	 r3,  r4,   0			# dvd.msw = 0

	S_EPILOG(__divu64)
