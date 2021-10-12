# @(#)82	1.6  src/bos/usr/ccs/lib/libc/POWER/memset.s, libcasm, bos41J, 9514B 4/7/95 16:21:37
#
# COMPONENT_NAME: (LIBCGEN) Standard C Library General Routines
#
# FUNCTIONS: memset
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
#-----------------------------------------------------------------------
#
#       R3   Address of memory
#       R4   The set_value byte
#       R5   Length of memory

	S_PROLOG(memset)

	.file	"memset.s"

	cmpi	cr1, r5, 0		# check for <= 0

	rlinm	r6, r4, 0, 0xff		# move value to r6
	sri.	r0, r5, 5		# r0 = length / 32
	rlimi	r6, r6, 8, 0xff00
	mr	r4, r3			# set store address
	rlimi	r6, r6, 16, 0xffff0000
	bler	cr1			# return if length <=0


	mtctr	r0			# load count register for store loop
	rlinm	r0, r5, 0, 0x1f		# set r0 = length % 32

	mr	r5, r6			# set r5 - r12 with set value
	mr	r7, r6
	mr	r8, r6
	mr	r9, r6
	mr	r10, r6
	mr	r11, r6
	mr	r12, r6

# setup is done 

	beq	small			# branch if length < 32


loop:
	stsi	r5, r4, 32		# set memory values
	ai	r4, r4, 32		# update address
	bdn	loop

small:		
	mtxer	r0			# xer = remainder
	stsx	r5, 0, r4		# set memory values

	S_EPILOG
	FCNDES(memset)
