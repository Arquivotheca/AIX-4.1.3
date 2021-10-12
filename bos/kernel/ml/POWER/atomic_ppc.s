# @(#)64        1.8  src/bos/kernel/ml/POWER/atomic_ppc.s, sysml, bos411, 9428A410j 5/31/94 10:26:50
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS:	fetch_and_add_ppc
#		fetch_and_add_h_ppc
#		fetch_and_and_ppc
#		fetch_and_or_ppc
#		compare_and_swap_ppc
#		test_and_set_ppc
#		fetch_and_nop_ppc
#		_check_lock_ppc
#		_clear_lock_ppc
#
# ORIGINS: 27 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

	.file	"atomic_ppc.s"
	.machine "ppc"

	.using	low, 0

#****************************************************************************
#
#  NAME:  fetch_and_add
#
#  FUNCTION:  atomically increment a memory location
#
# 	int fetch_and_add(atomic_p,int)
#
#  INPUT STATE:
#	r3 = ptr to the atomic location
#	r4 = delta
#
#  OUTPUT STATE:
#	r3 = prior value from the memory location
#
#  EXECUTION ENVIRONMENT:
#	can be called from thread or interrupt.
#	may only page fault when called at INTBASE.
#
#*****************************************************************************

	.csect	fetch_and_add_ppc_overlay[RO]
DATA(fetch_and_add_ppc):
	.globl	DATA(fetch_and_add_ppc)
	rlwinm	r7,r3,0,WORD_ALIGNED	# check alignment
	or	r6,r3,r3		# copy address to scratch

retry_add:
	lwarx	r3,0,r6			# load atomic location value
	twnei	r7,0	    	    	# trap if not aligned
	add	r5,r4,r3		# add delta
	stwcx.	r5,0,r6			# store new value
	beqlr+				# predict successful store

	b	retry_add		# retry

#****************************************************************************
#
#  NAME:  fetch_and_add_h
#
#  FUNCTION:  atomically increment a half word memory location
#
# 	ushort fetch_and_add_h(atomic_h,int)
#
#  INPUT STATE:
#	r3 = ptr to the half word location
#	r4 = delta
#
#  OUTPUT STATE:
#	r3 = prior value from the memory location
#
#  EXECUTION ENVIRONMENT:
#	can be called from thread or interrupt.
#	may only page fault when called at INTBASE.
#
#*****************************************************************************

	.csect	fetch_and_add_h_ppc_overlay[RO]
DATA(fetch_and_add_h_ppc):
	.globl	DATA(fetch_and_add_h_ppc)
# 
# The lwarx/stwcx pair take a word-aligned value (evenly divisible by 4).
# Since the value passed into this routine is the address of a short, there
# are two conditions to handle here: first, the address passed in (N) is
# 2-byte aligned.  In this case, the word-aligned address we generate for
# the lwarx/stwcx (by anding with 0xfffffffc) becomes N-2.  When we load
# the word from N-2 into a register the half-word we are interested in
# resides in bits 16-31 (the "lower" half of the word).  In the other case,
# the address passed in (N) is 4-byte aligned.  In this case, the anding of
# 0xfffffffc leaves N as N, and now when we load the word into a register,
# the half-word we are interested in resides in bits 0-15 (the "upper" half
# of the word.
#
	andi.	r7,r3,WORD_ALIGNED	# test for word alignment
	rlwinm	r8,r3,0,HALF_WORD_ALIGNED # test for half-word alignment
	rlwinm	r6,r3,0,0xfffffffc	# lwarx needs word-aligned address
	beq	cr0,add_upper		# branch if word in upper half

retry_add_lower:    	    	    	# lower half-word (bits 16-31)
	lwarx	r5,0,r6			# load atomic location value
	twnei	r8,0			# assert if not half-word aligned
	add	r7,r4,r5		# add delta to r5 into r7
	rlwinm	r3,r5,0,0xffff		# return value masked to half-word
	rlwimi	r5,r7,0,0xffff		# new value inserted into lower half
	stwcx.	r5,0,r6			# store new value
	beqlr+				# predict successful store

	b	retry_add_lower		# retry

add_upper:	    	    	    	# upper half-word (bits 0-15)
	slwi	r4,r4,16		# shift increment to upper half-word

retry_add_upper:
	lwarx	r3,0,r6			# load atomic location value
	twnei	r8,0			# assert if not half-word aligned
	add	r7,r4,r3		# add shifted increment
	stwcx.	r7,0,r6			# store new value
	rlwinm	r3,r3,16,0xffff		# move to least significant short
	beqlr+				# predict successful store

	b	retry_add_upper		# retry


#****************************************************************************
#
#  NAME:  fetch_and_and
#
#  FUNCTION:  atomically AND bits in a memory location
#
# 	uint fetch_and_and(atomic_p,uint)
#
#  INPUT STATE:
#	r3 = ptr to the atomic location
#	r4 = mask
#
#  OUTPUT STATE:
#	r3 = prior value from the memory location
#
#  EXECUTION ENVIRONMENT:
#	can be called from thread or interrupt.
#	may only page fault when called at INTBASE.
#
#*****************************************************************************

	.csect	fetch_and_and_ppc_overlay[RO]
DATA(fetch_and_and_ppc):
	.globl	DATA(fetch_and_and_ppc)
	rlwinm	r7,r3,0,WORD_ALIGNED	# check alignment
	or	r6,r3,r3		# copy address to scratch

retry_and:
	lwarx	r3,0,r6			# load atomic location value
	twnei	r7,0	    	    	# trap if not aligned
	and	r5,r4,r3		# and value with mask
	stwcx.	r5,0,r6			# store new value
	beqlr+				# predict successful store

	b	retry_and		# retry

#****************************************************************************
#
#  NAME:  fetch_and_or
#
#  FUNCTION:  atomically OR bits in a memory location
#
# 	uint fetch_and_or(atomic_p,uint)
#
#  INPUT STATE:
#	r3 = ptr to the atomic location
#	r4 = mask
#
#  OUTPUT STATE:
#	r3 = prior value from the memory location
#
#  EXECUTION ENVIRONMENT:
#	can be called from thread or interrupt.
#	may only page fault when called at INTBASE.
#
#*****************************************************************************

	.csect	fetch_and_or_ppc_overlay[RO]
DATA(fetch_and_or_ppc):
	.globl	DATA(fetch_and_or_ppc)
	rlwinm	r7,r3,0,WORD_ALIGNED	# check alignment
	or	r6,r3,r3		# copy address to scratch

retry_or:
	lwarx	r3,0,r6			# load atomic location value
	twnei	r7,0	    	    	# trap if not aligned
	or	r5,r4,r3		# or value with mask
	stwcx.	r5,0,r6			# store new value
	beqlr+				# predict successful store

	b	retry_or		# retry

#****************************************************************************
#
#  NAME:  compare_and_swap
#
#  FUNCTION:  compare the atomic location with the original value passed.
#		if the original value equals the current value, the new
#		value is stored into the atomic location and TRUE is returned.
#		Otherwise, the original value is modified to the current value
#		and FALSE is returned.
#
# 	boolean_t compare_and_swap(atomic_p,int *,int)
#
#  INPUT STATE:
#	r3 = ptr to the atomic location
#	r4 = ptr to original value of atomic location
#	r5 = new value
#
#  OUTPUT STATE:
#	TRUE  - if swap occurred
#	FALSE - if swap did not take place
#
#  EXECUTION ENVIRONMENT:
#	can be called from thread or interrupt.
#	may only page fault when called at INTBASE.
#
#*****************************************************************************

	.csect	compare_and_swap_ppc_overlay[RO]
DATA(compare_and_swap_ppc):
	.globl	DATA(compare_and_swap_ppc)
	lwz	r10,0(r4)		# r10 = old_value
	or	r6,r3,r3		# copy atomic address

retry_swap:
	lwarx	r9,0,r6			# load atomic location value
	rlwinm	r7,r6,0,WORD_ALIGNED	# check word alignment
	cmplw	cr0,r10,r9		# are they equal?
	twnei	r7,0			# assert if not word aligned
	lil	r3,TRUE			# assume success
	bne	swap_failed		# not equal, branch
	stwcx.	r5,0,r6			# store new value
	beqlr+				# predict successful store
	b	retry_swap		# retry

swap_failed:				# atomic != original
	stw	r9,0(r4)		# store atomic value in original

	lil	r3,FALSE		# return FALSE
	br


#****************************************************************************
#
#  NAME:  test_and_set
#
#  FUNCTION:  atomically test and set a memory location
#
# 	boolean_t test_and_set(atomic_p,int)
#
#  INPUT STATE:
#	r3 = ptr to the atomic location
#	r4 = mask
#
#  OUTPUT STATE:
#	TRUE  - if new mask differs from old mask
#	FALSE - otherwise
#
#  EXECUTION ENVIRONMENT:
#	can be called from thread or interrupt.
#	may only page fault when called at INTBASE.
#
#*****************************************************************************

	.csect	test_and_set_ppc_overlay[RO]
DATA(test_and_set_ppc):
	.globl	DATA(test_and_set_ppc)
	or	r5,r3,r3		# copy atomic address
tas_retry:
	lwarx	r8,0,r5			# load atomic location value
	rlwinm	r7,r5,0,WORD_ALIGNED	# check aligment
	and.	r9,r8,r4		# test the bit
	lil	r3,TRUE			# assume success
	twnei	r7,0			# assert if not word aligned
	bne	cr0,tas_failed		# was the bit already on?
	or	r9,r8,r4		# set the bit
	stwcx.	r9,0,r5			# no--store value with bit on
	beqlr+				# predict sucess
	b 	tas_retry		# store failed, retry

tas_failed:				# bit was already set
	lil	r3,FALSE		# this TAS failed
	br


#****************************************************************************
#
#  NAME:  fetch_and_nop
#
#  FUNCTION:  atomically read a memory location
#
# 	int fetch_and_nop(atomic_p)
#
#  INPUT STATE:
#	r3 = ptr to the atomic location
#
#  OUTPUT STATE:
#	r3 = value fetched
#
#  EXECUTION ENVIRONMENT:
#	can be called from thread or interrupt.
#	may only page fault when called at INTBASE.
#
#*****************************************************************************

	.csect	fetch_and_nop_ppc_overlay[RO]
DATA(fetch_and_nop_ppc):
	.globl	DATA(fetch_and_nop_ppc)
	rlwinm	r7,r3,0,WORD_ALIGNED	# check word alignment

retry_nop:
	lwarx	r4,0,r3			# load atomic location value
	twnei	r7,0			# trap if not aligned
	stwcx.	r4,0,r3			# store back same value
	bne+	retry_nop		# if lost reservation, retry
	or	r3,r4,r4		# return value fetched
	br

#****************************************************************************
#
#  NAME:  _check_lock
#
#  FUNCTION:	ISYNC if MP
#		compare the atomic location with the original value passed.
#		if the original value equals the current value, the new
#		value is stored into the atomic location and TRUE is returned.
#		Otherwise, the original value is modified to the current value
#		and FALSE is returned.
#
# 	boolean_t _check_lock(atomic_p,int,int)
#
#  INPUT STATE:
#	r3 = ptr to the atomic location
#	r4 = original value of atomic location
#	r5 = new value
#
#  OUTPUT STATE:
#	0 - if swap occurred
#	1 - if swap did not take place
#
#  EXECUTION ENVIRONMENT:
#	can be called from thread or interrupt.
#	may only page fault when called at INTBASE.
#
#*****************************************************************************

	.csect	_check_lock_ppc_overlay[RO]
DATA(_check_lock_ppc):
	.globl	DATA(_check_lock_ppc)
	or	r6,r3,r3		# copy atomic address

retry_check:
	lwarx	r9,0,r6			# load atomic location value
	cmplw	cr0,r4,r9		# are they equal
	lil	r3,0			# good return code
	bne	_check_failed		# not equal, branch
	stwcx.	r5,0,r6			# store new value
ifdef(`_POWER_MP',`
	isync				# to prevent speculative loads
')
	beqlr+				# predict successful store
	b	retry_check

_check_failed:				# atomic != original
	lil	r3,1			# bad return code
	br

#****************************************************************************
#
#  NAME:  _clear_lock
#
#  FUNCTION:	SYNC if MP
#		store value into atomic word location
#
# 	void _clear_lock(atomic_p, int)
#
#  INPUT STATE:
#	r3 = ptr to the atomic location
#	r4 = value to store
#
#  OUTPUT STATE:
#
#  EXECUTION ENVIRONMENT:
#	can be called from thread or interrupt.
#	may only page fault when called at INTBASE.
#
#  NOTE:
#	For ABI compliance, register r12 CANNOT be used is _clear_lock().
#
#*****************************************************************************

	.csect	_clear_lock_ppc_overlay[RO]
DATA(_clear_lock_ppc):
	.globl	DATA(_clear_lock_ppc)

ifdef(`_POWER_MP',`
	sync
')
	stw	r4,0(r3)		# store new value
	br

include(low_dsect.m4)
include(lock_def.m4)
include(machine.m4)
