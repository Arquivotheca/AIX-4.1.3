# @(#)42	1.2  src/bos/usr/ccs/lib/libc/POWER/whatbucket.s, libcmem, bos411, 9428A410j 2/14/94 11:23:16
#
#   COMPONENT_NAME: LIBCMEM
#
#   FUNCTIONS: whatbucket
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993,1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# This routine calculates the bucket number for a given requested size.
# This is the assembly language version of the Berkley allocator's whatbucket
# function.
#
include(sys/comlink.m4)

	S_PROLOG(_WhatBucket)

	.file	"whatbucket.s"

	cmpi	cr0, r3, 4088
	ai	r7, r3, 8		# add size of prefix
	liu	r4, 0x8000		# This mask will be shifted right by
					#   the number of leading zeros in r3
	bge	cr0, large_block	# for big numbers call slow code
	cntlz	r5, r7			# Count the leading zeroes in R3
	sr	r6, r4, r5		# Shift 0x80000000 right by the number
					#   of leading zeroes.
	cmpl	cr0, r6, r7		# Is our mask big enough?
	sfi	r3, r5, 29		# Calc bucket no.
	bltr    cr0			# return to caller if result good
	si	r3, r3, 1		# No, decrement.
	br				# i'm outta here

large_block:
	# Large blocks will use the existing 'whatbucket' function since it
	# has some very bizarre mapping behavior.  
	#
	# Setup to call function
	mflr	r0			# grab the link register
	st	r0, stklink(1)		# save link register on caller's stack
	stu	r1, -stkmin(r1)		# buy a stack frame for me

	.extern ENTRY(__whatbucket)
	bl	ENTRY(__whatbucket)	# use the existing cryptic version
	cror	15,15,15		# nop -- binder may it change to 
					#   l r2,stktoc(r1) if necessary

	# Return
	cal	r1, stkmin(r1)		# Restore stack pointer
	l	r0, stklink(r1)		# Recover saved link register
	mtlr	r0			# Restore link register

	S_EPILOG
	FCNDES(_WhatBucket)

