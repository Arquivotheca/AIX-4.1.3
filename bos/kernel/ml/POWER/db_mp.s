# @(#)97        1.4  src/bos/kernel/ml/POWER/db_mp.s, sysdb, bos412, 9446C 11/17/94 14:44:13

#
# COMPONENT_NAME: (SYSDB) Kernel Debugger
#
# FUNCTIONS: db_lock_try(), db_unlock(), db_get_processor_num()
#
# ORIGINS: 83
#
#
#
# LEVEL 1,  5 Years Bull Confidential Information

	.file	"db_mp.s"
	.machine "any"

#*******************************************************************************
include(low_dsect.m4)
include(systemcfg.m4)
include(scrs.m4)

#**********************************************************************
#
#  NAME: db_lock_try
#
#  FUNCTION: try to test and set, return the result
#
#  INPUT STATE: r3 is the address of the lock structure
#		r4 is the current cpu number
#
#  OUTPUT STATE: r3 = is the answer of test and set
#
#  NOTE: This routine keeps track of which cpu is holding the lock,
#	as well as the number of recursive locks that are held
#**********************************************************************

	.dsect	db_lock
lockword:	.long	0
lockcount:	.long	0
owning_cpu:	.long	0
	
  	S_PROLOG(db_lock_try)

ifdef(`_POWER_PC',`
	l	r12, syscfg_arch(0)	# get processor	arch. type
	cmpi	cr0, r12, POWER_PC	# check	for Power PC arch.
	bne	cr0, db_lock_not_ppc	# branch if not a Power PC
lock:
	lwz	r5,owning_cpu(r3)	# get owning cpu number
	cmpw	r4,r5			# Do they match?
	beq	lock_count		# All we have to do is count it
	
	lwarx	r5,0,r3
	or.	r5,r5,r5
	bne	lock_w
	stwcx.	r3,0,r3
	bne	lock_w
	
	stw	r4,owning_cpu(r3)	# Say that we own it
lock_count:	
	lwz	r5,lockcount(r3)	# Get lock count
	addi	r5,r5,1			# Add 1 to the count
	stw	r5,lockcount(r3)	# Store count back
	lil	r3,1			# Yes, you have the lock
        isync				# to prevent speculative load
	br

lock_w: lil	r3,0
db_lock_not_ppc:
') # _POWER_PC
#
	S_EPILOG
	FCNDES(db_lock_try)

#*************************************************************************
#
# UNlock a test-and-like semaphore
#
# R3 = lock address
#
#*************************************************************************
 	S_PROLOG(db_unlock)
ifdef(`_POWER_PC',`
	l	r12, syscfg_arch(0)	# get processor	arch. type
	cmpi	cr0, r12, POWER_PC	# check	for Power PC arch.
	bne	cr0, db_unlock_not_ppc	# branch if not a Power PC

	lwz	r4,lockcount(r3)	# Get lock count
	tweqi	r4,0			# Bad if <= 0
	subi	r4,r4,1			# subtract 1
	stw	r4,lockcount(r3)	# Put it back
	or.	r4,r4,r4		# Is it zero?
	li	r4,-1			# Get FFFFFFFF into r4
	bne	unlock_notdone		# If not zero, do not give it up yet

	stw	r4,owning_cpu(r3)	# Now nobody owns the lock
        sync				# to flush store queue
	xor	r4,r4,r4		# Get ready to clear lockword
	st	r4,0(r3)		# Clear the lockword
unlock_notdone:		
	br
db_unlock_not_ppc:
')  # _POWER_PC
#
	S_EPILOG
#*************************************************************************

#**********************************************************************
#
#  NAME: db_get_processor_num
#
#  FUNCTION:
#
#  INPUT STATE:
#
#  OUTPUT STATE: r3 = your processor number
#
#**********************************************************************
	S_PROLOG(db_get_processor_num)
ifdef(`_POWER_MP',`
ifdef(`_POWER_RS',`
ifdef(`_POWER_PC',`
	l	r3, syscfg_arch(0)	# get processor	arch. type
	cmpi	cr0, r3, POWER_PC	# check	for Power PC arch.
	beq	cr0, db_is_ppc		# branch if a Power PC
',) #endif _POWER_PC
	l	r3, proc_arr_addr(0)
	lhz	r3, ppda_cpuid(r3)
	br
',) #endif _POWER_RS
ifdef(`_POWER_PC',`
db_is_ppc:
	.machine "ppc"
	mfspr	r3, SPRG0
	.machine "com"
	lhz	r3, ppda_cpuid(r3)
	br
',) #endif _POWER_PC
',` #else _POWER_MP
	lil	r3, 0(0)
	br
') #endif _POWER_MP

