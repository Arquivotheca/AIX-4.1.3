# @(#)98	1.20  src/bos/usr/ccs/lib/libc/POWER/longjmp.s, libcsys, bos411, 9428A410j 3/22/94 14:33:54
#*******************************************************************
#
# COMPONENT_NAME: (LIBCSYS) Standard C Library System Functions
#
# FUNCTIONS: setjmp, longjmp, sigsetjmp, siglongjmp, _setjmp, _longjmp,
#	     jmpsavefpr, jmprestfpr
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#*******************************************************************

include(sys/comlink.m4)

#-------------------------------------------------------------------
# NAME: setjmp
#
# FUNCTION: The setjmp function saves its calling environment in its
#	jump buffer argument for later use by the longjmp function.
#
# NOTES:
#	Input:	r3 -> label_t for jump buffer
#
#	Output:	r3 = 0
#-------------------------------------------------------------------

	.set	jmpmask, 0	# signal mask part 1 in jump buffer
	.set	jmpmask1, 4	# signal mask part 2 in jump buffer
	.set	jmpret, 8	# return address offset in jump buffer
	.set	jmpstk, 12	# stack pointer offset in jump buffer
	.set	jmptoc, 16	# TOC pointer offset in jump buffer
	.set	jmpregs, 20	# registers offset in jump buffer
	.set	jmpcr,96	# condition reg offset in jump buffer
	.set	jmprsv1,100	# reserved (alignment)
	.set	jmpfpr,104	# floating point regs offset in jump buffer
	.set	jmpbckch,248	# backchain to prev. stack frame in buffer
	.set	jmprsv2,252	# reserved (used to be from 248--defect 37969)
	.set	jmpbufsize,256	# total length = 256 bytes

	.extern .sigprocmask

	S_PROLOG(setjmp)

	.file	"longjmp.s"

	mfcr	r4 			# save the condition reg
	st	r4, jmpcr(r3)
	mflr	r4			# copy the return address
	st	r1, jmpstk(r3)		# save the stack pointer
	l	r7, 0(r1)		# move backchain pointer into r7
	st	r7, jmpbckch(r3)	# save backchain to prev. stack frame 
	st	r2, jmptoc(r3)		# save the TOC pointer
	st	r4, jmpret(r3)		# save the return address
	stm	r13, jmpregs(r3)	# save non-volatile regs
	bl	.jmpsavefpr[PR]		# save non-volatile fp regs
	mr	r13, r3			# copy pointer to jump buffer
	stu	r1, -stkmin(r1)		# buy a stack frame
	mr	r5, r3			# where to return signal mask
	cal	r3, 0(0)		# set sigprocmask how argument
	cal	r4, 0(0)		# set sigprocmask set argument (NULL)
	bl	.sigprocmask		# retrieve the current signal mask
	cror	15, 15, 15
	cal	r1, stkmin(r1)		# drop the stack frame we bought
	cal	r3, 0(0)		# return 0 from setjmp
	l	r6, jmpret(r13)		# get return address
	mtlr	r6			# restore link register
	l	r13, jmpregs(r13)	# restore reg to entry value

	S_EPILOG
	FCNDES(setjmp)

#-------------------------------------------------------------------
# NAME: longjmp
#
# FUNCTION: The longjmp function restores the environment saved by
#	the most recent invocation of the setjmp function in the
#	same invocation of the program, with the corresponding
#	jump buffer argument.  If there has been no such invocation,
#	the behavior is undefined.  If the function containing the
#	invocation of the setjmp macro has terminated execution in
#	the interim, the longjmperror function is called.
#
# EXECUTION ENVIRONMENT:
#	The longjmp function executes correctly in contexts of
#	interrupts, signals and any of their associated functions.
#	However, if the longjmp function is invoked from a nested
#	signal handler, the behavior is undefined.
#
# NOTES:
#	Input:	r3 -> label_t for jump buffer
#		r4 = return code for caller of setjmp
#
#	Output:	r3 = return code passed by caller, if not 0
#		   = 1 if caller tried to return 0
#-------------------------------------------------------------------

	.extern	.sigcleanup
	.extern	.longjmperror
	.extern	.abort

	S_PROLOG(longjmp)
	
	mr	r13, r3			# save jmp_buf address
	mr	r14, r4			# save return value
	stu	r1, -stkmin(r1)		# buy a stack frame
	bl	.sigcleanup		# restore saved signal mask
	cror	15, 15, 15
	cal	r1, stkmin(r1)		# drop the stack frame we bought
	mr	r3, r13			# restore jmp_buf address
	mr	r4, r14			# restore return value	
	l       r5, jmpret(r3)		# restore return address
	l       r1, jmpstk(r3)		# restore stack pointer
	l	r7, jmpbckch(r3)	# move backchain pointer to r7
	st	r7, 0(r1)              	# restore backchain pointer
	l       r2, jmptoc(r3)		# restore TOC pointer
	bl	.jmprestfpr[PR]		# restore non-volatile fp regs
	cmpi	cr0, r4, 0		# check the return value
	mtlr	r5			# return to location after setjmp call
	lm      r13, jmpregs(r3)	# restore non-volatile regs
	l	r5, jmpcr(r3)		# restore condition regs 2-4
	mtcrf	0x38, r5
	mr	r3, r4			# load return value
	bne	cr0, jmp1		# if the return value is 0
	cal	r3, 1(0)		# make the return value be 1
jmp1:
	S_EPILOG
	FCNDES(longjmp)

#-------------------------------------------------------------------
# NAME: sigsetjmp
#
# FUNCTION: The sigsetjmp function works like the setjmp function.
#	If the save mask argument is not zero, the sigsetjmp function
#	will also save the process's current signal mask as part of
#	the calling environment.
#
# NOTES:
#	Input:	r3 -> label_t for jump buffer
#		r4 -> save mask flag
#
#	Output:	r3 = 0
#-------------------------------------------------------------------

	.extern .sigprocmask

	S_PROLOG(sigsetjmp)

	mfcr	r5 			# save the condition reg
	st	r5, jmpcr(r3)
	cmpi	cr0, r4, 0		# check the save mask argument
	mflr	r5			# copy the return address
	st	r1, jmpstk(r3)		# save the stack pointer
	st	r2, jmptoc(r3)		# save the TOC pointer
	st	r5, jmpret(r3)		# save the return address
	stm	r13, jmpregs(r3)	# save the non-volatile regs
	bl	.jmpsavefpr[PR]		# save non-volatile fp regs
	mr	r13, r3			# copy pointer to jump buffer
	mr	r5, r3			# where to return signal mask
	cal	r3, -1(0)		# assume signal mask won't be saved
	st	r3, jmpmask(r5)		# put -1 in signal mask
	st	r3, jmpmask1(r5)
	cal	r3, 0(0)		# set sigprocmask how argument
	cal	r4, 0(0)		# set sigprocmask set argument (NULL)
	beq	ss_skipit		# if save mask argument was not zero
	stu	r1, -stkmin(r1)		# buy a stack frame
	bl	.sigprocmask		# retrieve the current signal mask
	cror	15, 15, 15
	cal	r1, stkmin(r1)		# drop the stack frame we bought
ss_skipit:
	cal	r3, 0(0)		# return 0 from setjmp
	l	r6, jmpret(r13)		# get return address
	mtlr	r6			# restore link register
	l	r13, jmpregs(r13)	# restore reg to entry value

	S_EPILOG
	FCNDES(sigsetjmp)

#-------------------------------------------------------------------
# NAME: siglongjmp
#
# FUNCTION: The siglongjmp function works like the longjmp function.
#	The siglongjmp function will restore the saved signal mask if
#	and only if the jump buffer argument was initialized by a call
#	to the sigsetjmp function with a non-zero save mask argument.
#
# NOTES:
#	Input:	r3 -> label_t for jump buffer
#		r4 = return code for caller of setjmp
#
#	Output:	r3 = return code passed by caller, if not 0
#		   = 1 if caller tried to return 0
#-------------------------------------------------------------------

	.extern	.sigcleanup
	.extern	.longjmperror
	.extern	.abort

	S_PROLOG(siglongjmp)
	
	mr	r13, r3			# save jmp_buf address
	mr	r14, r4			# save return value
	l	r5, jmpmask(r3)		# get signal mask
	l	r15, jmpmask1(r3)	
	cmpi	cr0, r5, -1		# is first word of mask -1?
	cmpi	cr1, r15, -1		# is second word -1?
	crand	cr0*4+eq, cr0*4+eq, cr1*4+eq
	beq	cr0, sl_skipit	    	# don't reset signal mask
	stu	r1, -stkmin(r1)		# buy a stack frame

				# set up for call to sigprocmask
	mr	r4, r3			# pointer to signal mask
	cal	r3, 2(0)		# SIG_SETMASK
	cal	r5, 0(0)		# no returned mask
	bl	.sigprocmask		# restore saved signal mask
	cror	15, 15, 15

	cal	r1, stkmin(r1)		# drop the stack frame we bought
	mr	r3, r13			# restore jmp_buf address
	mr	r4, r14			# restore return value
sl_skipit:
	l       r5, jmpret(r3)		# restore return address
	l       r1, jmpstk(r3)		# restore stack pointer
	l       r2, jmptoc(r3)		# restore TOC pointer
	bl	.jmprestfpr[PR]		# restore non-volatile fp regs
	cmpi	cr0, r4, 0		# check the return value
	mtlr	r5			# return location after sigsetjmp call
	lm      r13, jmpregs(r3)	# restore non-volatile regs
	l	r5, jmpcr(r3)		# restore condition regs 2-4
	mtcrf	0x38, r5
	mr	r3, r4			# load return value
	bne	cr0, sigjmp1		# if the return value is 0
	cal	r3, 1(0)		# make the return value be 1
sigjmp1:
	S_EPILOG
	FCNDES(siglongjmp)

#-------------------------------------------------------------------
# NAME: _setjmp
#
# FUNCTION: The _setjmp function works like the setjmp function.
#	However, the _setjmp function saves only the stack context,
#	it does not save the signal mask.
#
# NOTES:
#	Input:	r3 -> label_t for jump buffer
#
#	Output:	r3 = 0
#-------------------------------------------------------------------


	S_PROLOG(_setjmp)

	mfcr	r4 			# save the condition reg
	st	r4, jmpcr(r3)
	mflr	r4			# copy the return address
	st	r1, jmpstk(r3)		# save the stack pointer
	l	r7, 0(r1)		# move backchain pointer into r7
	st	r7, jmpbckch(r3)	# save backchain to prev. stack frame 
	st	r2, jmptoc(r3)		# save the TOC pointer
	st	r4, jmpret(r3)		# save the return address
	stm	r13, jmpregs(r3)	# save non-volatile regs
	bl	.jmpsavefpr[PR]		# save non-volatile fp regs
	mtlr	r4			# restore link register
	cal	r3, 0(0)		# return 0 from _setjmp

	S_EPILOG

#-------------------------------------------------------------------
# NAME: _longjmp
#
# FUNCTION: The _longjmp function works like the longjmp function.
#	However, the _longjmp function restores only the stack context,
#	it does not restore the signal mask.
#
# NOTES:
#	Input:	r3 -> label_t for jump buffer
#		r4 = return code for caller of _setjmp
#
#	Output:	r3 = return code passed by caller, if not 0
#		   = 1 if caller tried to return 0
#-------------------------------------------------------------------

	.extern	.longjmperror
	.extern	.abort

	S_PROLOG(_longjmp)
	
	l       r5, jmpret(r3)		# restore return address
	l       r1, jmpstk(r3)		# restore stack pointer
	l	r7, jmpbckch(r3)	# move backchain pointer to r7
	st	r7, 0(r1)              	# restore backchain pointer
	l       r2, jmptoc(r3)		# restore TOC pointer
	bl	.jmprestfpr[PR]		# restore non-volatile fp regs
	cmpi	cr0, r4, 0		# check the return value
	mtlr	r5			# return to location after _setjmp call
	lm      r13, jmpregs(r3)	# restore non-volatile regs
	l	r5, jmpcr(r3)		# restore condition regs 2-4
	mtcrf	0x38, r5
	mr	r3, r4			# load return value
	bne	cr0, _jmp1		# if the return value is 0
	cal	r3, 1(0)		# make the return value be 1
_jmp1:
	S_EPILOG

#-------------------------------------------------------------------
# NAME: jmpsavefpr
#
# FUNCTION: An internal subroutine to save the volatile floating
#	point registers in the jump buffer.
#
# NOTES:
#	Input:	r3 -> label_t for jump buffer
#
#	Output:	r3 -> label_t for jump buffer
#-------------------------------------------------------------------

	S_PROLOG(jmpsavefpr)

	stfd	f14, jmpfpr (r3)
	stfd	f15, jmpfpr +  1 * 8 (r3)
	stfd	f16, jmpfpr +  2 * 8 (r3)
	stfd	f17, jmpfpr +  3 * 8 (r3)
	stfd	f18, jmpfpr +  4 * 8 (r3)
	stfd	f19, jmpfpr +  5 * 8 (r3)
	stfd	f20, jmpfpr +  6 * 8 (r3)
	stfd	f21, jmpfpr +  7 * 8 (r3)
	stfd	f22, jmpfpr +  8 * 8 (r3)
	stfd	f23, jmpfpr +  9 * 8 (r3)
	stfd	f24, jmpfpr + 10 * 8 (r3)
	stfd	f25, jmpfpr + 11 * 8 (r3)
	stfd	f26, jmpfpr + 12 * 8 (r3)
	stfd	f27, jmpfpr + 13 * 8 (r3)
	stfd	f28, jmpfpr + 14 * 8 (r3)
	stfd	f29, jmpfpr + 15 * 8 (r3)
	stfd	f30, jmpfpr + 16 * 8 (r3)
	stfd	f31, jmpfpr + 17 * 8 (r3)

	S_EPILOG

#-------------------------------------------------------------------
# NAME: jmprestfpr
#
# FUNCTION: An internal subroutine to restore the volatile floating
#	point registers from saved values in the jump buffer.
#
# NOTES:
#	Input:	r3 -> label_t for jump buffer
#
#	Output:	r3 -> label_t for jump buffer
#-------------------------------------------------------------------

	S_PROLOG(jmprestfpr)

	lfd	f14, jmpfpr (r3)
	lfd	f15, jmpfpr +  1 * 8 (r3)
	lfd	f16, jmpfpr +  2 * 8 (r3)
	lfd	f17, jmpfpr +  3 * 8 (r3)
	lfd	f18, jmpfpr +  4 * 8 (r3)
	lfd	f19, jmpfpr +  5 * 8 (r3)
	lfd	f20, jmpfpr +  6 * 8 (r3)
	lfd	f21, jmpfpr +  7 * 8 (r3)
	lfd	f22, jmpfpr +  8 * 8 (r3)
	lfd	f23, jmpfpr +  9 * 8 (r3)
	lfd	f24, jmpfpr + 10 * 8 (r3)
	lfd	f25, jmpfpr + 11 * 8 (r3)
	lfd	f26, jmpfpr + 12 * 8 (r3)
	lfd	f27, jmpfpr + 13 * 8 (r3)
	lfd	f28, jmpfpr + 14 * 8 (r3)
	lfd	f29, jmpfpr + 15 * 8 (r3)
	lfd	f30, jmpfpr + 16 * 8 (r3)
	lfd	f31, jmpfpr + 17 * 8 (r3)

	S_EPILOG
