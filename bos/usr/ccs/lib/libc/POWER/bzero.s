# @(#)80	1.4  src/bos/usr/ccs/lib/libc/POWER/bzero.s, libcmem, bos411, 9428A410j 2/14/94 11:25:51
#
# COMPONENT_NAME: (LIBCGEN) Standard C Library General Routines
#
# FUNCTIONS: bzero
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
#   bzero:
#
#   This routine implements a general purpose storage zero.
#   Storage can be of any length and start on any boundary.
#
#   This subroutine is implemented using stores, rather than zeroing
#   the cache line (for now) so xlate off will work.
#   Eventually, zeroing the cache line is probably the answer.
#
#   The C entry description is:         void bzero(addr, len);
#
#   input registers are:
#         LR - return address
#         r3 - address of string to be zeroed
#         r4 - length of the string
#
#-----------------------------------------------------------------------


        S_PROLOG(bzero)                 # This is the C name

	.file	"bzero.s"

	cmpi	cr0, r4, 16		# check for small store
        cal     r5,0(0)			# init store values
	cal	r6,0(0)
	bgt	cr0, med_move		# branch if > 16
	cmpi	cr0, r4, 0		# check for <= 0
	cal	r7, 0(0)
	cal	r8, 0(0)
	bler				# return if length <= 0

do_store:
	mtxer	r4			# set move length
	stsx	r5, 0, r3		# zero memory
	br

med_move:
	sri.	r0, r4, 5		# r0 = length / 32
	cal	r7, 0(0)
	cal	r8, 0(0)
	cal	r9, 0(0)
	cal	r10, 0(0)
	cal	r11, 0(0)
	cal	r12, 0(0)
	beq	do_store		# branch if can be done in one
					# store

	mtctr	r0			# set loop count
	andil.	r4, r4, 0x1f		# r4 = remainder


z_loop:
	stsi	r5, r3, 32		# clear 32 bytes
	ai	r3, r3, 32		# move to next chuk
	bdn	z_loop

	mtxer	r4			# clear the remaining bytes
	stsx	r5, 0, r3

	S_EPILOG

	FCNDES(bzero)


