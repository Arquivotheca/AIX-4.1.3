# @(#)89	1.4  src/bos/usr/ccs/lib/libc/POWER/divi64.s, libccnv, bos411, 9428A410j 2/21/94 13:59:32
#
#   COMPONENT_NAME: LIBCCNV
#
#   FUNCTIONS: __divi64
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
# long long	__divi64( long long, long long )
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
# First in the function, the final quo and rmd signs are computed and saved.
# The rmd sign is always that of the dvd, while the quo sign is the XOR of
# dvd and dvs signs.  Then both dvd and dvs are 'made positive'.  For either
# == LONGLONG_MIN, the result is hardware dependant and the only necessity
# is that changing the sign again has the opposite effect (if any).
#
# Next, the dvd is preshifted into tmp based on the number of leading zeroes
# in both the dvd and dvs.  This makes the first iteration of the main loop
# always produce a quo '0' bit, but otherwise reduces the number of iterations
# in a highly data-dependent way.  The code is a little convoluted since the
# MSWs and LSWs have to be treated separately in word-sized (32-bit) chunks.
# Thus the count leading zeroes may return 32, i.e. a whole word of zeroes,
# and such situations are handled.  The main loop iteration count is lowered
# to reflect any optimization from preshifting.
#
# When the Count Register reaches zero, the last quo bit is rotated in and
# sign adjustments are made, if necessary, to both the quo and rmd.
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
#
# cr6   : Final Quotient Sign
# cr7   : Final Remainder Sign
#
# CTR   : cnt

	.file "divi64.s"
	.machine "com"
	S_PROLOG(__divi64)

					# Compute and save final signs
	cmpi	cr5,  r3,   0			# dvd.msw = 0?
	cmpi	cr1,  r5,   0			# Parm 2 positive?
	xor	 r0,  r3,  r5			# Final Quotient Sign
	cmpi	cr7,  r3,   0			# Final Remainder Sign
	cmpi	cr6,  r0,   0			# Final Quotient Sign
					# Make the dividend positive
	bge	cr5,  sd1			# dvd > 0?
	sfi	 r4,  r4,   0			# dvd.lsw
	sfze	 r3,  r3			# dvd.msw
sd1:					# Make the Divisor positive
	bge	cr1,  sd2			# dvs > 0?
	sfi	 r6,  r6,   0			# dvs.lsw
	sfze	 r5,  r5			# dvs.msw
sd2:					# Compute total Dividend Leading 0s
	cntlz	 r0,  r3			# dvd.msw
	cmpi	cr0,  r0,  32			# dvd.msw == 0?
	cntlz	 r9,  r4			# dvd.lsw
	bne	cr0, sd3			# dvd.msw == 0
	a	 r0,  r0,  r9			# all dvd.LZ
sd3:					# Compute total Divisor Leading 0s
	cntlz	 r9,  r5			# dvs.msw
	cmpi	cr5,  r9,  32			# dvd.msw == 0?
	cntlz	r10,  r6			# dvs.lsw
	bne	cr5, sd4			# dvs.msw == 0
	a	 r9,  r9, r10			# all dvs.LZ
sd4:					# Is Divisor > Dividend?
	cmp	cr0,  r0,  r9			# dvd.LZ to dvs.LZ
	sfi	r10,  r0,  64			# dvd.LZ -> dvd.SD
	bgt	cr0, sd13			# dvs > dvd (quo=0)
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
	blt	cr0, sd5			# Jumps if not
	sr	 r8,  r3,  r7			# MSW shifted into LSW
	cal	 r7, 0(r0)			# final tmp.msw
	b	sd6				#
sd5:						#
	sr	 r8,  r4,  r9			# Shift LSW
	sfi	 r7,  r9,  32			# Bits to rotate (left)
	sl	 r7,  r3,  r7			# Isolate LSBits of MSW
	or	 r8,  r8,  r7			# Insert MSBits of LSW
	sr	 r7,  r3,  r9			# final tmp.msw
sd6:					# Preshift Divisor, Preclear Quotient
						# r3:r4 = r3:r4 << r0
	cmpi	cr0,  r0,  32			# 32 or more bits?
	si	 r9,  r0,  32			#
	blt	cr0, sd7			# Jumps if not
	sl	 r3,  r4,  r9			# LSW shifted into MSW
	cal	 r4, 0(r0)			# final dvd.lsw
	b	sd8				#
sd7:						#
	sl	 r3,  r3,  r0			# Shift MSW
	sfi	 r9,  r0,  32			#
	sr	 r9,  r4,  r9			#
	or	 r3,  r3,  r9			# final dvd.msw
	sl	 r4,  r4,  r0			# final dvd.lsw
sd8:					# Prepare constants for the main loop
	cal	r10, -1(r0)			#
	ai	 r7,  r7,   0			# tmp.lsw (Clear CY)
sd9:					# Main Loop
	ae	 r4,  r4,  r4			# dvd.lsw (lsb = CY)
	ae	 r3,  r3,  r3			# dvd.msw (lsb from lsw.msb)
	ae	 r8,  r8,  r8			# tmp.lsw (lsb from dvd carry)
	ae	 r7,  r7,  r7			# tmp.msw
						#
	sf	 r0,  r6,  r8			#
	sfe.	 r9,  r5,  r7			#
	blt	sd10				# Jmp !CY
	oril	 r8,  r0,   0			# Move lsw
	oril	 r7,  r9,   0			# Move msw
	ai	 r0, r10,   1			# Set CY
sd10:						#
	bdn	sd9				# Decrement CTR, branch !0
					# Rotate in the final Quotient bit
	ae	 r4,  r4,  r4			# quo.lsw (lsb = CY)
	ae	 r3,  r3,  r3			# quo.msw (lsb from lsw)
sd11:					# Set final Quotient sign
	bge	cr6, sd12			# Skip if positive result
	sfi	 r4,  r4,   0			# Toggle sign, Quotient LSW
	sfze	 r3,  r3			# Toggle sign, Quotient MSW
sd12:					# Set final Remainder sign
	oril	 r6,  r8,   0			# rmd.lsw = tmp.lsw
	oril	 r5,  r7,   0			# rmd.msw = tmp.msw
	bger	cr7				# Return if positive result
	sfi	 r6,  r6,   0			# Toggle sign, Remainder LSW
	sfze	 r5,  r5			# Toggle sign, Remainder MSW
	br					# Return
sd13:					# Quotient is 0 (dvs > dvd)
	oril	 r8,  r4,   0			# tmp.lsw = dvd.lsw
	oril	 r7,  r3,   0			# tmp.msw = dvd.msw
	cal	 r4, 0(r0)			# dvd.lsw = 0
	oril	 r3,  r4,   0			# dvd.msw = 0
	b	sd11				# Fix the sign(s)

	S_EPILOG(__divi64)
