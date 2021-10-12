# @(#)65        1.5  src/bos/kernel/ml/POWER/atomic_pwr.s, sysml, bos411, 9428A410j 4/6/94 12:00:00
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS:	fetch_and_add_pwr
#		fetch_and_add_h_pwr
#		fetch_and_and_pwr
#		fetch_and_or_pwr
#		compare_and_swap_pwr
#		test_and_set_pwr
#		fetch_and_nop_pwr
#		_clear_lock_pwr
#
# ORIGINS: 27 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
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

	.file	"atomic_pwr.s"
	.machine "pwr"

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

	.csect	fetch_and_add_pwr_overlay[RO]
DATA(fetch_and_add_pwr):
	.globl	DATA(fetch_and_add_pwr)
	mfmsr	r6			# save the MSR value in r6
	mr	r5,r3			# copy address to scratch
	cal	r7,DISABLED_MSR(0)	# calculate a disabled MSR
	mtmsr	r7			# disable interrupts
	l	r3,0(r5)		# load atomic location value
	andil.	r7,r5,WORD_ALIGNED	# check word alignment
	a	r7,r4,r3		# add delta
	st	r7,0(r5)		# store new value
	mtmsr	r6			# enable interrupts
	beqr	cr0 	    	    	# return if aligned
    	TRAP	    	    	    	# trap, if not aligned

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

	.csect	fetch_and_add_h_pwr_overlay[RO]
DATA(fetch_and_add_h_pwr):
	.globl	DATA(fetch_and_add_h_pwr)
	mfmsr	r6			# save the MSR value in r6
	mr	r5,r3			# copy address to scratch
	cal	r7,DISABLED_MSR(0)	# calculate a disabled MSR
	mtmsr	r7			# disable interrupts
	lhz	r3,0(r5)		# load atomic location value
	andil.	r7,r5,HALF_WORD_ALIGNED	# check word alignment
	a	r7,r4,r3		# add delta
	sth	r7,0(r5)		# store new value
	mtmsr	r6			# enable interrupts
	beqr	cr0 	    	    	# return if aligned
    	TRAP	    	    	    	# trap, if not aligned


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

	.csect	fetch_and_and_pwr_overlay[RO]
DATA(fetch_and_and_pwr):
	.globl	DATA(fetch_and_and_pwr)
	mfmsr	r6			# save the MSR value in r6
	mr	r5,r3			# copy address to scratch
	cal	r7,DISABLED_MSR(0)	# calculate a disabled MSR
	mtmsr	r7			# disable interrupts
	l	r3,0(r5)		# load atomic location value
	andil.	r7,r5,WORD_ALIGNED	# check word alignment
	and	r7,r4,r3		# and atomic location
	st	r7,0(r5)		# store new value
	mtmsr	r6			# enable interrupts
	beqr	cr0 	    	    	# return if aligned
    	TRAP	    	    	    	# trap, if not aligned

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

	.csect	fetch_and_or_pwr_overlay[RO]
DATA(fetch_and_or_pwr):
	.globl	DATA(fetch_and_or_pwr)
	mfmsr	r6			# save the MSR value in r6
	mr	r5,r3			# copy address to scratch
	cal	r7,DISABLED_MSR(0)	# calculate a disabled MSR
	mtmsr	r7			# disable interrupts
	l	r3,0(r5)		# load atomic location value
	andil.	r7,r5,WORD_ALIGNED	# check word alignment
	or	r7,r4,r3		# or atomic location
	st	r7,0(r5)		# store new value
	mtmsr	r6			# enable interrupts
	beqr	cr0 	    	    	# return if aligned
    	TRAP	    	    	    	# trap, if not aligned


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
#	r4 = pointer to original value of atomic location
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

	.csect	compare_and_swap_pwr_overlay[RO]
DATA(compare_and_swap_pwr):
	.globl	DATA(compare_and_swap_pwr)
    	mfmsr	r6  	    	    	# save MSR value in r6
	l	r10,0(r4)		# load old value
	cal	r8,DISABLED_MSR(0)	# calculate a disabled MSR
	mtmsr	r8			# disable interrupts
	l	r9,0(r3)		# load atomic location value
	mr	r8,r3	    	    	# copy atomic addr to scratch
	cmp	cr0,r10,r9  	    	# compare values
	rlinm	r7,r3,0,WORD_ALIGNED	# check alignment
    	twnei	r7,0	    	    	# assert if not aligned
    	lil	r3,TRUE(0)  	    	# assume TRUE
    	bne	cs_false    	    	# branch if not equal

    	st	r5,0(r8)    	    	# comparison ==, store new value
    	mtmsr	r6  	    	    	# restore original MSR
    	br	    	    	    	# return

cs_false:	    	    	    	# comparison !=
        st	r9,0(r4)    	    	# put atomic value in old
    	lil	r3,FALSE(0) 	    	# returning FALSE
    	mtmsr	r6  	    	    	# restore original MSR
    	br	    	    	    	# return
	

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

	.csect	test_and_set_pwr_overlay[RO]
DATA(test_and_set_pwr):
	.globl	DATA(test_and_set_pwr)
	mfmsr	r6			# save the MSR value in r6
	rlinm	r10,r3,0,WORD_ALIGNED	# check alignment
	cal	r7,DISABLED_MSR(0)	# calculate a disabled MSR
	mtmsr	r7			# disable interrupts

	l	r8,0(r3)		# load atomic location value
	twnei	r10,0	    	    	# trap if not aligned

	and.	r7,r8,r4		# test the bit
	mr	r11,r3			# copy atomic address
	lil	r3,TRUE(0)		# assume success
	or	r9,r8,r4		# set the bit
	bne	cr0,ts_false		# was the bit already on?
	st	r9,0(r11)		# no--store value with bit on
	mtmsr	r6			# restore entry MSR
	br

ts_false:				# bit was already set
	lil	r3,FALSE(0)		# this TAS failed
	mtmsr	r6			# restore entry MSR
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

	.csect	fetch_and_nop_pwr_overlay[RO]
DATA(fetch_and_nop_pwr):
	.globl	DATA(fetch_and_nop_pwr)
	andil.	r7,r3,WORD_ALIGNED	# assert if not word aligned
	twnei	r7,0

	l	r3,0(r3)		# load atomic location value
	br

#****************************************************************************
#
#  NAME:  _clear_lock
#
#  FUNCTION:	store value into atomic word location
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

	.csect	_clear_lock_pwr_overlay[RO]
DATA(_clear_lock_pwr):
	.globl	DATA(_clear_lock_pwr)
	stw	r4,0(r3)		# store new value
	br

include(low_dsect.m4)
include(lock_def.m4)
include(machine.m4)
