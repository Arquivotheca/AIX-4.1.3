# @(#)94	1.3  src/bos/kernel/ml/POWER/forkstack.s, sysml, bos411, 9428A410j 3/22/94 15:57:19
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS:  execve
#		fork
#		kexit
#		kfork
#		kload
#		_load
#		loadbind
#		unload
#
#   ORIGINS: 27, 83
#
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************


	.file	"forkstack.s"
	.machine "com"


#*******************************************************************************
#
# The routines included in this file are system call wrapper to execute on
# the special fork stack.
#
# On entry from SVC the following registers are set :
#	r1  - stack pointer to only one stack frame on own kstack
#	r3-r10 - parameters
#	r24 - u
#	r25 - ut (csa)
#	r26 - user lr
#	r27 - user cr
#	r28 - curproc
#	r29 - curthread
#	r30 - scratch (used to save return code)
#	r31 - scratch (used to save return address)
#
# On exit to SVC, r1 and r24-r29 must still hold accurate values, therefore
# some are updated (r1, r25, r28, r29 for fork; r1, r25 for exec). r3 must
# hold the SVC return code.
#
#*******************************************************************************

	
#*******************************************************************************
#
# NAME: fork
#
# FUNCTION: binary compatibility for app. statically linked with old fork() sc
#
#*******************************************************************************

	S_PROLOG(fork)

	b	ENTRY(kfork)
	.extern ENTRY(kfork)

	S_EPILOG

	FCNDES(fork)


#*******************************************************************************
#
# NAME: kfork
#
# FUNCTION: system call wrapper to execute on the fork stack
#
# ASSUMPTION: fork has no parameter
#
#*******************************************************************************

	S_PROLOG(kfork)

#	Save current return address

	mflr	r31

#	Acquire special fork stack

	mr	r3, r28			# current proc pointer
	bl	ENTRY(acquire_forkstack)# returns forkstack address
	.extern ENTRY(acquire_forkstack)

#	Switch to the fork stack

	cal	r1, -stkmin(r3)		# buy one stack frame on forkstack
ifdef(`_KDB',`
	l	r12, t_stackp(r29)	# user stack pointer
	st	r12, stkback(r1)	# back chain to old stack
')

#	Invoke real function

	bl	ENTRY(kforkx)
	.extern ENTRY(kforkx)

#	Save return code

	mr	r30, r3

#	When the child returns, curproc, curthread and csa/ut are no longer
#	valid. They are refetched thanks to u which is still valid and
#	thanks to the assumption that the child is single-threaded.

	cmpi	cr0, r3, 0
	bne	cr0, fork_no_refetch	# not returning as child

	l	r28, u_procp(r24)	# refetch curproc
	l	r29, p_thrdlist(r28)	# refetch curthread
	l	r25, t_uthreadp(r29)	# refetch csa/ut

fork_no_refetch:

#	Switch back to normal stack

	l	r11, ut_kstack(r25)	# kernel stack pointer
	cal	r1, -stkmin(r11)	# buy one stack frame on normal stack
ifdef(`_KDB',`
	l	r12, t_stackp(r29)	# user stack pointer
	st	r12, stkback(r1)	# back chain to old stack
')

#	Release special fork stack

	mr	r3, r28			# current proc pointer
	bl	ENTRY(release_forkstack)
	.extern ENTRY(release_forkstack)

#	Restore return address and return code

	mtlr	r31			# return address
	mr	r3, r30			# return code

	S_EPILOG

	FCNDES(kfork)


#*******************************************************************************
#
# NAME: execve
#
# FUNCTION: system call wrapper to execute on the fork stack
#
# ASSUMPTION: exec has three parameters
#
#*******************************************************************************

	S_PROLOG(execve)

#	Save current return address

	mflr	r31

#	Buy a stack frame in order to save parameters in the current one

	st	r3, stkp1+0*4(r1)
	st	r4, stkp1+1*4(r1)
	st	r5, stkp1+2*4(r1)
	st	r31, stklink(r1)
	stu	r1, -stkmin(r1)

#	Acquire special fork stack

	mr	r3, r28			# current proc pointer
	bl	ENTRY(acquire_forkstack)# returns forkstack address
	.extern ENTRY(acquire_forkstack)

#	Restore parameters

	l	r0, stkmin+stkp1+0*4(r1)# cannot lose r3 yet
	l	r4, stkmin+stkp1+1*4(r1)
	l	r5, stkmin+stkp1+2*4(r1)

#	Switch to the fork stack

	cal	r1, -stkmin(r3)		# buy one stack frame on forkstack
	mr	r3, r0			# get r3 parameter back for good
ifdef(`_KDB',`
	l	r12, t_stackp(r29)	# user stack pointer
	st	r12, stkback(r1)	# back chain to old stack
')

#	Invoke real function

	bl	ENTRY(execvex)
	.extern	ENTRY(execvex)

#	Save return code

	mr	r30, r3

#	When unsuccessful, csa/ut is no longer valid because the process
#	has lost all its thread but this one which has been moved in the
#	default location. It is refetched thanks to curthread which is
#	still valid.
#	When successful, never returns here, execexit() is used instead.

	l	r25, t_uthreadp(r29)	# refetch csa/ut

#	Switch back to normal stack

	l	r11, ut_kstack(r25)	# kernel stack pointer
	cal	r1, -stkmin(r11)	# buy one stack frame on normal stack
ifdef(`_KDB',`
	l	r12, t_stackp(r29)	# user stack pointer
	st	r12, stkback(r1)	# back chain to user stack
')

#	Release special fork stack

	mr	r3, r28			# current proc pointer
	bl	ENTRY(release_forkstack)
	.extern ENTRY(release_forkstack)

#	Restore return address and return code

	mtlr	r31			# return address
	mr	r3, r30			# return code

	S_EPILOG

	FCNDES(execve)


#*******************************************************************************
#
# NAME: kexit
#
# FUNCTION: system call wrapper to execute on the fork stack
#
# ASSUMPTION: exit has one parameter
#
# NOTE: Not always called as a system call, therefore we only assume the
# following to be available on entry:
#	r1 - caller's normal stack pointer
#	r3 - parameter
#
# never returns therefore no need to save any registers we may trash
#
#*******************************************************************************

	S_PROLOG(kexit)

#	Save current return address

	mflr	r31

#	Save parameter

	mr	r30, r3

#	Acquire special fork stack

	LTOC(r11, __ublock, data)	# address of ublock
	l	r28,u_procp+(ub_user-ublock)(r11) # current proc pointer
	mr	r3, r28
	bl	ENTRY(acquire_forkstack)# returns forkstack address
	.extern ENTRY(acquire_forkstack)

#	Switch to the fork stack if not a kproc and not already on it
#	(Note: we assume the fork stack is the only stack in segment 2)

	l	r11, p_flag(r28)	# process flags
	mr	r12, r1			# current stack pointer
	rlinm.	r11, r11, 0, SKPROC	# check kproc flag
	rlinm	r10, r12, 4, 0x0000000F	# get segment number for stack
	bne	cr0, exit_no_forkstack
	cmpi	cr0, r10, 2		# compare to PRIVSEG
	beq	cr0, exit_no_forkstack
	cal	r1, -stkmin(r3)		# buy one stack frame on forkstack
	st	r31, stklink(r12)	# save old lr
	st	r12, stkback(r1)	# back chain to old stack

exit_no_forkstack:

#	Restore parameter

	mr	r3, r30

#	Invoke real function

	bl	ENTRY(kexitx)
	.extern ENTRY(kexitx)

#	Does not return

	S_EPILOG

	FCNDES(kexit)


#*******************************************************************************
#
# NAME: kload
#
# FUNCTION: system call wrapper to execute on the fork stack
#
# ASSUMPTION: kload has three parameters
#	(max. of the number of parameters for load, unload and loadbind)
#
# NOTE: a fourth parameter containing the address of the real function is added
#
#*******************************************************************************

	S_PROLOG(kload)

#	Save current return address

	mflr	r31

#	Buy a stack frame in order to save parameters in the current one

	st	r3, stkp1+0*4(r1)
	st	r4, stkp1+1*4(r1)
	st	r5, stkp1+2*4(r1)
	st	r6, stkp1+3*4(r1)
	st	r31, stklink(r1)
	stu	r1, -stkmin(r1)

#	Acquire special fork stack

	mr	r3, r28			# current proc pointer
	bl	ENTRY(acquire_forkstack)# returns forkstack address
	.extern ENTRY(acquire_forkstack)

#	Restore parameters

	l	r0, stkmin+stkp1+0*4(r1)# cannot lose r3 yet
	l	r4, stkmin+stkp1+1*4(r1)
	l	r5, stkmin+stkp1+2*4(r1)
	l	r6, stkmin+stkp1+3*4(r1)

#	Switch to the fork stack

	cal	r1, -stkmin(r3)		# buy one stack frame on forkstack
	mr	r3, r0			# get r3 parameter back for good
ifdef(`_KDB',`
	l	r12, t_stackp(r29)	# user stack pointer
	st	r12, stkback(r1)	# back chain to old stack
')

#	Reload SR1 with TEXTSEG

	l	r0, u_adspace_sr+1*4(r24)
	mtsr	sr1, r0
	isync

#	Invoke real function

	mtctr	r6
	bctrl

#	Reload SR1 with KSTACKSEG

	l	r0, p_kstackseg(r28)
	mtsr	sr1, r0
	isync

#	Save return code

	mr	r30, r3

#	Switch back to normal stack

	l	r11, ut_kstack(r25)	# kernel stack pointer
	cal	r1, -stkmin(r11)	# buy one stack frame on normal stack
ifdef(`_KDB',`
	l	r12, t_stackp(r29)	# user stack pointer
	st	r12, stkback(r1)	# back chain to user stack
')

#	Release special fork stack

	mr	r3, r28			# current proc pointer
	bl	ENTRY(release_forkstack)
	.extern ENTRY(release_forkstack)

#	Restore return address and return code

	mtlr	r31			# return address
	mr	r3, r30			# return code

	S_EPILOG

	FCNDES(kload)


#*******************************************************************************
#
# NAME: _load
#
# FUNCTION: system call wrapper to execute on the fork stack
#
# ASSUMPTION: _load has no more than three parameters
#
#*******************************************************************************

	S_PROLOG(_load)

#	Get the address of loadx has a fourth parameter

	LTOC(r6, loadx, entry)

#	Branch to common code

	b	ENTRY(kload)
	.extern	ENTRY(kload)

	S_EPILOG

	FCNDES(_load)


#*******************************************************************************
#
# NAME: unload
#
# FUNCTION: system call wrapper to execute on the fork stack
#
# ASSUMPTION: unload has no more than three parameters
#
#*******************************************************************************

	S_PROLOG(unload)

#	Get the address of unloadx has a fourth parameter

	LTOC(r6, unloadx, entry)

#	Branch to common code

	b	ENTRY(kload)
	.extern	ENTRY(kload)

	S_EPILOG

	FCNDES(unload)


#*******************************************************************************
#
# NAME: loadbind
#
# FUNCTION: system call wrapper to execute on the fork stack
#
# ASSUMPTION: loadbind has no more than three parameters
#
#*******************************************************************************

	S_PROLOG(loadbind)

#	Get the address of loadbindx has a fourth parameter

	LTOC(r6, loadbindx, entry)

#	Branch to common code

	b	ENTRY(kload)
	.extern	ENTRY(kload)

	S_EPILOG

	FCNDES(loadbind)


	.toc
	TOCE(__ublock, data)
	TOCE(loadx, entry)
	TOCE(loadbindx, entry)
	TOCE(unloadx, entry)

include(proc.m4)
include(mstsave.m4)
include(user.m4)
include(low_dsect.m4)
