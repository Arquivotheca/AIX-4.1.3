# @(#)81	1.5  src/bos/usr/ccs/lib/libc/POWER/memcmp.s, libcasm, bos41J, 9514B 4/7/95 16:21:35
#
# COMPONENT_NAME: (LIBCGEN) Standard C Library General Routines
#
# FUNCTIONS: memcmp bcmp
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1990, 1991
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#-----------------------------------------------------------------------
#
#       R3   Address of first block of memory
#       R4   Address of second block of memory
#       R5   Length of memory
#

	.file	"memcmp.s"

	S_PROLOG(memcmp)

	cmp	cr1, r3, r4		# check for equal addresses
	sri.	r12, r5, 2		# r12 = length / 4
	rlinm	r11, r5, 0, 0x3		# r11 = length % 4
	lil	r7, 0			# zero registers for remainder move
	beq	cr1, retequ		# return equal if equall address
	mr	r8, r7
	mtxer	r11			# set up for move of remainder
	beq	cr0, small		# branch if less than 4 bytes to
					# compare

	mtctr	r12			# number of 4 byte chunks to compare
loop:
	lsi	r5, r3, 4		# load memory to compare
	lsi	r6, r4, 4
	cmpl	cr0, r5, r6		# compare 4 bytes at a time
	ai	r3, r3, 4		# increment for next load
	ai	r4, r4, 4
	bne	retdiff			# return if not equal
	bdn	loop			# compare next word

# since r7 and r8 have been zeroed 0 to 3 bytes can be loaded and compared
# to produce to correct result

small:
	lsx	r7, 0, r3		# load remaining bytes
	lsx	r8, 0, r4
	cmpl	cr0, r7, r8		# compare the results
	bne	cr0, retdiff		# branch if not equal

retequ:
	lil	r3, 0			# return equal
	br

retdiff:
	bgt	cr0, greater
	lil	r3, -1			# return less than
	br
greater:
	lil	r3, 1			# return greater than

	S_EPILOG
	FCNDES(memcmp)


