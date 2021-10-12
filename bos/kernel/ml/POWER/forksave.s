# @(#)48	1.3  src/bos/kernel/ml/POWER/forksave.s, sysml, bos411, 9428A410j 3/15/93 09:20:20
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: Maching Dependent Fork Routines
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

	.file	"forksave.s"
	.machine "com"

#-------------------------------------------------------------------
# Subroutine Name: forksave
#
# Function: Called by fork() to save the register context of the
#           child process, so that the child can be dispatched after
#           the parent puts it on the ready queue.
#
# Input:
#       r3        = &u.u_save
# Output:
#	r1, r2 and r13-r31 saved in u-block.
#	function value in u-block set to return 1
#	iar in u-block set to return address
#	returns function value of 0 to the caller (parent process)
#-------------------------------------------------------------------

	S_PROLOG(forksave)

	.using	mstsave, r3
	mflr	r0			# return address
	stm     r13, mstgpr+13*4(r3)	# save r13 thru r31
	st      r1, mstgpr+4(r3)	# save r1 (stack)
	st	r2, mstgpr+8(r3)	# save r2 (toc)
	cal	r4, 1(0)		# child returns 1
	st	r4, mstgpr+12(r3)	# store in r3 (return value)
	st	r0, mstiar(r3)		# starting address for child
	.drop	r3
	cal	r3, 0(0)		# parent returns 0
	S_EPILOG

#-------------------------------------------------------------------
# Subroutine Name: forksregs
#
# Function: Called by as_fork() to save the current segment registers
#		in the segment register save area of an adspace_t.
# Input:
#       r3        = address of srval[0]
# Output:
#	sregs 0-15 saved in srval array
#-------------------------------------------------------------------

	S_PROLOG(forksregs)
	mfsr	r4,sr0			# get sregs 0-7
	mfsr	r5,sr1
	mfsr	r6,sr2
	mfsr	r7,sr3
	mfsr	r8,sr4
	mfsr	r9,sr5
	mfsr	r10,sr6
	mfsr	r11,sr7
	stsi	r4,r3,32		# save sregs 0-7
	ai	r3,r3,32		# increment r3
	mfsr	r4,sr8			# get sregs 8-15
	mfsr	r5,sr9
	mfsr	r6,sr10
	mfsr	r7,sr11
	mfsr	r8,sr12
	mfsr	r9,sr13
	mfsr	r10,sr14
	mfsr	r11,sr15
	stsi	r4,r3,32		# save sregs 8-15
	S_EPILOG

#------------------------#
#   include files
#------------------------#
include(mstsave.m4)
