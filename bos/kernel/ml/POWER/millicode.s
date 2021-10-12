# @(#)50	1.1.1.1  src/bos/kernel/ml/POWER/millicode.s, sysml, bos411, 9428A410j 6/17/93 14:31:59
#*****************************************************************************
#
# COMPONENT_NAME: SYSML
#
# FUNCTIONS: generic integer functions (Power/Power PC implementation)
#	MILLICODE FUNCTIONS:
#	     __mulh
#	     __mull
#	     __divss
#	     __divus	
#	     __quoss
#	     __quous
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1992,1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#          *** NOTE ***
#
# When using these routines, the compiler saves/restores only
# those registers which will actually be used; it has knowledge
# of the internal semantics.  Do NOT change the code to use
# any additional registers without agreement from the compiler
# architects!
#****************************************************************************

	.file "millicode.s"

# generate a traceback table with the tocless flag on.
define(_DF_MILLICODE, `0x4,0x0,0x0,0x0,0x0,0x0,0x0')dnl
define(MILLICODE_EPILOG,
	`br
	 _DF(_DF_MILLICODE)')



ifdef(`_POWER_RS',`

	.machine "pwr"

#-------------------------------------------------------------------
# Subroutine Name: __mulh (Power Version)
#
# Function: calculate high-order 32 bits of the signed integer
#           product arg1 * arg2
#
# Input:
#       r3 = arg1 (signed int)
#	r4 = arg2 (signed int)
# Output:
#	r3 = high-order 32 bits of r3 * r4
# General Purpose Registers Set/Used:
#	r3, r4
# Special Purpose Registers Set/Used:
#	mq
# Condition Register Fields Set/Used:
#	None.
#-------------------------------------------------------------------

DATA(mulh_pwr):
	.globl	DATA(mulh_pwr)
	mul	r3,r3,r4		# r3 = r3 * r4
	MILLICODE_EPILOG

#-------------------------------------------------------------------
# Subroutine Name: __mull (Power Version)
#
# Function: calculate 64 bits of signed product arg1 * arg2,
#	    (returned in two 32-bit registers)
#           
#
# Input:
#       r3 = arg1 (signed int)
#	r4 = arg2 (signed int)
# Output:
#	r3 = high-order 32 bits of r3 * r4
#	r4 = low-order 32 bits of r3 * r4
# General Purpose Registers Set/Used:
#	r3, r4
# Special Purpose Registers Set/Used:
#	mq
# Condition Register Fields Set/Used:
#	None.
#-------------------------------------------------------------------

DATA(mull_pwr):
	.globl	DATA(mull_pwr)
	mul	r3,r3,r4		# r3 = r3 * r4
	mfspr	r4,mq			# get low order part
	MILLICODE_EPILOG

#-------------------------------------------------------------------
# Subroutine Name: __divss (Power Version)
#
# Function: calculate 32-bit quotient and 32-bit remainder
#	    of arg1/arg2, where arg1 and arg2 are 32-bit signed ints
#           
#
# Input:
#       r3 = arg1 - dividend (signed int)
#	r4 = arg2 - divisor  (signed int)
# Output:
#	r3 = quotient of arg1/arg2  (signed int)
#	r4 = remainder of arg1/arg2 (signed int)
#
#	For division by zero and overflow (i.e. -2^32 / -1)
#	the quotient and remainder are undefined and may
#	vary by platform.
#
# General Purpose Registers Set/Used:
#	r3, r4
# Special Purpose Registers Set/Used:
#	mq
# Condition Register Fields Set/Used:
#	None.
#-------------------------------------------------------------------

DATA(divss_pwr):
	.globl	DATA(divss_pwr)
	divs	r3,r3,r4		# quotient to r3
	mfspr	r4,mq			# get remainder to r4
	MILLICODE_EPILOG

#-------------------------------------------------------------------
# Subroutine Name: __divus (Power Version)
#
# Function: calculate unsigned 32-bit quotient and 32-bit remainder
#	    of arg1/arg2, where arg1 and arg2 are unsigned ints
#           
#
# Input:
#       r3 = arg1 - dividend (unsigned int)
#	r4 = arg2 - divisor  (unsigned int)
# Output:
#	r3 = quotient of arg1/arg2  (unsigned int)
#	r4 = remainder of arg1/arg2 (unsigned int)
#
#	For division by zero the quotient and remainder
#	are undefined and may vary by platform.
#
# General Purpose Registers Set/Used:
#	r0, r3, r4
# Special Purpose Registers Set/Used:
#	mq
# Condition Register Fields Set/Used:
#	cr0, cr1
#-------------------------------------------------------------------

DATA(divus_pwr):
	.globl	DATA(divus_pwr)
	cmpl	cr0,r4,r3	# r4 > r3 (unsigned)?
	cmpi	cr1,r4,0x1	# r4 compared to 1
	mtspr	mq,r3		# for div instructions
	cal	r0,0(0)		# move zero to r0
	bgt	cr0,DIVUS.3	# for arg2 > arg1 (unsigned)
	blt	cr1,DIVUS.2	# for case arg2 < 1 (**signed**)
	beq	cr0,DIVUS.2	# for case arg1 == arg2
	beq	cr1,DIVUS.1	# for case of arg1 = 1
	div	r3,r0,r4	# (0 || mq) / r4
	mfspr	r4,mq		# get remainder
	br			# return
DIVUS.1:			# here if arg1 == 1
	cal	r4,0(0)		# move 0 to remainder
	br			# return
DIVUS.2:			# here if arg1 == arg2 or
				# (arg2 > 0x7FFFFFFF) && (arg2 < arg2)
	sf	r4,r4,r3	# r4 = r3 - r4 (quotient)
	cal	r3,1(0)		# quotient is 1
	br			# return
DIVUS.3:			# here if arg2 > arg2
	oril	r4,r3,0x0	# move r3 to r4 (remainder)	
	cal	r3,0(0)		# move 0 to quotient
	MILLICODE_EPILOG

#-------------------------------------------------------------------
# Subroutine Name: __quoss (Power Version)
#
# Function: calculate the 32-bit quotient of arg1/arg2, signed
#
# Input:
#       r3 = arg1 - dividend (signed int)
#	r4 = arg2 - divisor  (signed int)
#
#	For division by zero and overflow (i.e. -2^32 / -1)
#	the quotient and remainder are undefined and may
#	vary by platform.
#
# Output:
#	r3 = quotient of arg1/arg2 (signed int)
# General Purpose Registers Set/Used:
#	r3,r4
# Special Purpose Registers Set/Used:
#	mq
# Condition Register Fields Set/Used:
#	None.
#-------------------------------------------------------------------

DATA(quoss_pwr):
	.globl	DATA(quoss_pwr)
	divs	r3,r3,r4		# quotient to r3
	MILLICODE_EPILOG

#-------------------------------------------------------------------
# Subroutine Name: __quous (Power Version)
#
# Function: calculate the 32-bit quotient of arg1/arg2, unsigned
#
# Input:
#       r3 = arg1 - dividend (unsigned int)
#	r4 = arg2 - divisor  (unsigned int)
# Output:
#	r3 = quotient of arg1/arg2 (unsigned int)
#
#	For division by zero the quotient and remainder
#	are undefined and may vary by platform.
#
# General Purpose Registers Set/Used:
#	r0, r3, r4
# Special Purpose Registers Set/Used:
#	mq
# Condition Register Fields Set/Used:
#	cr0, cr1
#-------------------------------------------------------------------

DATA(quous_pwr):
	.globl	DATA(quous_pwr)
	cmpl	cr0,r4,r3	# r4 > r3 (unsigned)?
	cmpi	cr1,r4,0x01	# r4 == 1?
	mtspr	mq,r3		# for div instruction
	cal	r0,0(0)		# r0 to contain 0x0
	bgt	cr0,QUOUS.3	# for arg2 > arg1 (unsigned)
	blt	cr1,QUOUS.2	# for arg2 < 1 (**signed**)
	beq	cr0,QUOUS.2	# for case arg1 == arg2
	beq	cr1,QUOUS.1	# for case of arg1 == 1
	div	r3,r0,r4	# (0 || mq) / r4
				# falls thru to br...
QUOUS.1:			# branch here if arg2 == 1; result is arg1,
	br			# which is already in r3
QUOUS.2:			# here if arg1 == arg2 or
				# (arg2 > 0x7FFFFFFF) && (arg2 < arg2)
	cal	r3,1(0)		# quotient is 1
	br			# return
QUOUS.3:			# branch here if r4 > r3 (unsigned)
	cal	r3,0(0)		# zero is result
	MILLICODE_EPILOG
',)

ifdef(`_POWER_PC',`

	.machine "ppc"

#-------------------------------------------------------------------
# Subroutine Name: __mulh (Power PC Version)
#
# Function: calculate high-order 32 bits of the integer
#           product arg1 * arg2
#
# Input:
#       r3 = arg1 (signed int)
#	r4 = arg2 (signed int)
# Output:
#	r3 = high-order 32 bits of r3 * r4
# General Purpose Registers Set/Used:
#	r3, r4
# Special Purpose Registers Set/Used:
#	None.
# Condition Register Fields Set/Used:
#	None.
#-------------------------------------------------------------------

DATA(mulh_ppc):
	.globl	DATA(mulh_ppc)
	mulhw	r3,r3,r4		# r3 = (high 32 bits) r3 * r4
	MILLICODE_EPILOG

#-------------------------------------------------------------------
# Subroutine Name: __mull (Power PC Version)
#
# Function: calculate 64 bits or product arg1 * arg2,
#	    (returned in two 32-bit registers)
#           
#
# Input:
#       r3 = arg1 (signed int)
#	r4 = arg2 (signed int)
# Output:
#	r3 = high-order 32 bits of r3 * r4
#	r4 = low-order 32 bits of r3 * r4
# General Purpose Registers Set/Used:
#	r0, r3, r4
# Special Purpose Registers Set/Used:
#	None.
# Condition Register Fields Set/Used:
#	None.
#-------------------------------------------------------------------

DATA(mull_ppc):
	.globl	DATA(mull_ppc)
	mullw	r0,r3,r4		# low part of result
	mulhw	r3,r3,r4		# high part of result
	ori	r4,r0,0x0		# move low part to correct reg.
	MILLICODE_EPILOG

#-------------------------------------------------------------------
# Subroutine Name: __divss (Power PC Version)
#
# Function: calculate 32-bit quotient and 32-bit remainder
#	    of arg1/arg2, where arg1 and arg2 are signed ints
#           
#
# Input:
#       r3 = arg1 (signed int)
#	r4 = arg2 (signed int)
# Output:
#	r3 = quotient of arg1/arg2  (signed int)
#	r4 = remainder of arg1/arg2 (signed int)
# General Purpose Registers Set/Used:
#	r0, r3, r4
# Special Purpose Registers Set/Used:
#	None.
# Condition Register Fields Set/Used:
#	None.
#-------------------------------------------------------------------

DATA(divss_ppc):
	.globl	DATA(divss_ppc)
	divw	r0,r3,r4		# r0 = r3/r4
	mullw	r4,r0,r4		# r4 = (r3/r4) * r4
	subf	r4,r4,r3		# remainder: r4 = r3 - ((r3/r4) * r4)
	ori	r3,r0,0x0		# quotient to correct reg.
	MILLICODE_EPILOG

#-------------------------------------------------------------------
# Subroutine Name: __divus (Power PC Version)
#
# Function: calculate 32-bit quotient and 32-bit remainder
#	    of arg1/arg2, where arg1 and arg2 are unsigned ints
#           
#
# Input:
#       r3 = arg1 (unsigned int)
#	r4 = arg2 (unsigned int)
# Output:
#	r3 = quotient of arg1/arg2  (unsigned int)
#	r4 = remainder of arg1/arg2 (unsigned int)
# General Purpose Registers Set/Used:
#	r0, r3, r4
# Special Purpose Registers Set/Used:
#	None.
# Condition Register Fields Set/Used:
#	None.
#-------------------------------------------------------------------

DATA(divus_ppc):
	.globl	DATA(divus_ppc)
	divwu	r0,r3,r4		# r0 = r3/r4 (unsigned)
	mullw	r4,r0,r4		# r4 = (r3/r4) * r4
	subf	r4,r4,r3		# remainder
	ori	r3,r0,0x0		# quotient to correct register
	MILLICODE_EPILOG

#-------------------------------------------------------------------
# Subroutine Name: __quoss (Power PC Version)
#
# Function: calculate the 32-bit quotient of arg1/arg2, signed
#
# Input:
#       r3 = arg1 (signed int)
#	r4 = arg2 (signed int)
# Output:
#	r3 = quotient of arg1/arg2 (signed int)
# General Purpose Registers Set/Used:
#	r3,r4
# Special Purpose Registers Set/Used:
#	None.
# Condition Register Fields Set/Used:
#	None.
#-------------------------------------------------------------------

DATA(quoss_ppc):
	.globl	DATA(quoss_ppc)
	divw	r3,r3,r4
	MILLICODE_EPILOG

#-------------------------------------------------------------------
# Subroutine Name: __quous (Power PC Version)
#
# Function: calculate the 32-bit quotient of arg1/arg2, unsigned
#
# Input:
#       r3 = arg1 (unsigned int)
#	r4 = arg2 (unsigned int)
# Output:
#	r3 = quotient of arg1/arg2 (unsigned int)
# General Purpose Registers Set/Used:
#	r3, r4
# Special Purpose Registers Set/Used:
#	None.
# Condition Register Fields Set/Used:
#	None.
#-------------------------------------------------------------------

DATA(quous_ppc):
	.globl	DATA(quous_ppc)
	divwu	r3,r3,r4
	MILLICODE_EPILOG

',)



