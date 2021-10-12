# @(#)57        1.2  src/bos/usr/ccs/lib/libc/POWER/atomic_op.s, libcsys, bos41B, 9505A 1/20/95 04:53:56
#*******************************************************************
#
# COMPONENT_NAME: (LIBCSYS) Standard C Library System Functions
#
# FUNCTIONS: fetch_and_add, fetch_and_and, fetch_and_or,
#	     compare_and_swap
#
# ORIGINS: 27 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#*******************************************************************

	.file	"atomic_op.s"

#*******************************************************************
#
# This macro checks for alignment on the pointer as well as
# tests the processor type for POWER_RS
#
#*******************************************************************

	.set SYS_CONFIG,r11
	.set SCFG_IMP,r12

define(ATOMIC_PROLOG,
	`
	.machine "pwr"

#	load _system_configuration struct 
	LTOC(SYS_CONFIG,_system_configuration, data)

#	scheduling optimization--alignment check
	rlwinm	r7,r3,0,WORD_ALIGNED

#	load scfg_impl
	l	SCFG_IMP,scfg_impl(SYS_CONFIG)

#	scheduling optimization--alignment check
	twnei	r7, 0

#	test for POWER_RS_ALL
	andil.	SCFG_IMP,SCFG_IMP,POWER_RS_ALL
')

	.extern	.cs

#*******************************************************************
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
#*******************************************************************
	S_PROLOG(fetch_and_add)

	ATOMIC_PROLOG			# check alignment and processor
	bne	cr0,add_POWER

	or	r8,r3,r3		# copy address to scratch
add_601:
	.machine "ppc"
	lwarx	r3,0,r8			# load atomic location value
	add	r5,r3,r4		# add delta
	stwcx.	r5,0,r8			# store new value
	beqlr+				# predict successful store
	b	add_601			# retry

add_POWER:
	.machine "pwr"	
	mflr	r0			# grab the link register
	st	r3, -4(r1)		# save atomic pointer
	st	r0, stklink(r1)		# save link register on caller's stack
	st	r4, -8(r1)		# save delta
add_cs_retry:
	l	r3, -4(r1)		# get atomic pointer
	l	r8, -8(r1)		# get delta
	l	r4,0(r3)		# load "old" value
	st	r4, -12(r1)		# save "old" value
	stu	r1, -stkmin(r1)		# buy a stack frame for me
	a	r5,r4,r8		# create "new" value
	bl	.cs			# call fast SVC
	nop
	cmpi	cr0,r3,0		# check success
	cal	r1, stkmin(r1)		# restore stack pointer
	bne	add_cs_retry
	
	l	r0, stklink(r1)		# recover saved link register
	l	r3, -12(r1)		# load "old" value for return
	mtlr	r0			# restore link register
	
	S_EPILOG(fetch_and_add)
	FCNDES(fetch_and_add)

#*******************************************************************
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
#*******************************************************************
	S_PROLOG(fetch_and_and)

	ATOMIC_PROLOG			# check alignment and processor
	bne	cr0,and_POWER

	or	r8,r3,r3		# copy address to scratch
and_601:
	.machine "ppc"
	lwarx	r3,0,r8			# load atomic location value
	and	r5,r3,r4		# and value with mask
	stwcx.	r5,0,r8			# store new value
	beqlr+				# predict successful store
	b	and_601			# retry

and_POWER:
	.machine "pwr"
	mflr	r0			# grab the link register
	st	r3, -4(r1)		# save atomic pointer
	st	r0, stklink(r1)		# save link register on caller's stack
	st	r4, -8(r1)		# save mask
and_cs_retry:
	l	r3, -4(r1)		# get atomic pointer
	l	r8, -8(r1)		# get mask
	l	r4, 0(r3)		# load "old" value
	st	r4, -12(r1)		# save "old" value
	stu	r1, -stkmin(r1)		# buy a stack frame for me
	and	r5, r4, r8		# create "new" value
	bl	.cs			# call fast SVC
	nop
	cmpi	cr0,r3,0		# check success
	cal	r1, stkmin(r1)		# restore stack pointer
	bne	and_cs_retry

	l	r0, stklink(r1)		# recover saved link register
	l	r3, -12(r1)		# load "old" value for return
	mtlr	r0			# restore link register
	
	S_EPILOG(fetch_and_and)
	FCNDES(fetch_and_and)

#*******************************************************************
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
#*******************************************************************
	S_PROLOG(fetch_and_or)

	ATOMIC_PROLOG			# check alignment and processor
	bne	cr0,or_POWER

	or	r8,r3,r3		# copy address to scratch
or_601:
	.machine "ppc"
	lwarx	r3,0,r8			# load atomic location value
	or	r5,r3,r4		# or value with mask
	stwcx.	r5,0,r8			# store new value
	beqlr+				# predict successful store
	b	or_601			# retry

or_POWER:
	.machine "pwr"
	mflr	r0			# grab the link register
	st	r3, -4(r1)		# save atomic pointer
	st	r0, stklink(r1)		# save link register on caller's stack
	st	r4, -8(r1)		# save mask
or_cs_retry:
	l	r3, -4(r1)		# get atomic pointer
	l	r8, -8(r1)		# get mask
	l	r4, 0(r3)		# load "old" value
	st	r4, -12(r1)		# save "old" value
	stu	r1, -stkmin(r1)		# buy a stack frame for me
	or	r5, r4, r8		# create "new" value
	bl	.cs			# call fast SVC
	nop
	cmpi	cr0,r3,0		# check success
	cal	r1, stkmin(r1)		# restore stack pointer
	bne	or_cs_retry
	
	l	r0, stklink(r1)		# recover saved link register
	l	r3, -12(r1)		# load "old" value for return
	mtlr	r0			# restore link register
	
	S_EPILOG(fetch_and_or)
	FCNDES(fetch_and_or)

#*******************************************************************
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
#*******************************************************************
	S_PROLOG(compare_and_swap)

	ATOMIC_PROLOG			# check alignment and processor
	bne	cr0,cs_POWER

	or	r8,r3,r3		# copy atomic address
	lwz	r10,0(r4)		# r10 = old_value
cs_601:
	.machine "ppc"
	lwarx	r9,0,r8			# load atomic location value
	cmplw	cr0,r10,r9		# are they equal?
	lil	r3,TRUE			# assume success
	bne	swap_failed		# not equal, branch
	stwcx.	r5,0,r8			# store new value
	beqlr+				# predict successful store
	b	cs_601			# retry

swap_failed:				# atomic != original
	stw	r9,0(r4)		# store atomic value in original
	lil	r3,FALSE		# return FALSE
	br

cs_POWER:
	.machine "pwr"
	mflr	r0			# grab the link register
	st	r3, -4(r1)		# save atomic pointer
	st	r4, -8(r1)		# save "old" pointer
	st	r0, stklink(r1)		# save link register on caller's stack
	l	r4, 0(r4)		# load "old" value
	stu	r1, -stkmin(r1)		# buy a stack frame for me
	bl	.cs			# call fast SVC
	nop
	cal	r1, stkmin(r1)		# restore stack pointer
	cmpi	cr0, r3, 0		# check success
	l	r0, stklink(r1)		# recover saved link register
	lil	r3, TRUE		# assume success
	mtlr	r0			# restore link register
	beqlr+				# predict successful swap

	l	r4, -4(r1)		# load atomic pointer
	l	r5, -8(r1)		# load "old" pointer
	l	r4, 0(r4)		# load current value
	lil	r3, FALSE		# return FALSE
	st	r4, 0(r5)		# store atomic value in "old"

	S_EPILOG(compare_and_swap)
	FCNDES(compare_and_swap)

	.toc
	TOCE(_system_configuration, data)
	
include(systemcfg.m4)
include(lock_def.m4)
include(sys/comlink.m4)
