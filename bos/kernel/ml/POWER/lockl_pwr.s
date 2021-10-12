# @(#)71        1.9  src/bos/kernel/ml/POWER/lockl_pwr.s, sysml, bos41J, 9521A_all 5/23/95 18:29:17
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS:	lockl_pwr
#		unlockl_pwr
#
#   ORIGINS: 27, 83
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
#   LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

	.file	"lockl.s"
	.machine "pwr"

	.using	low, 0

undefine(`_POWER_601')
undefine(`_POWER_PC')



include(systemcfg.m4)
include(scrs.m4)
include(macros.m4)
#***************************************************************************
#
# FUNCTION: acquire logical lock
#
# CALL:
#	int lockl(lock_word, flag)
#	lock_t *lock_word;
#	int flag
#
# NOTES:
#	This function handles the case were lock is available.
# If some real work needs to be done klockl is called.
#	This function does not get a stack frame.  If the lock
# is not available then control is passed to klockl without
# changing the contents of the link register. klockl will
# return to the caller of this routine
#
# PSUEDO CODE:
#	old intpri = csa->intpri
#	disable interrupts via MSR
#	owner = *lock_word;
#
#	if (owner == LOCK_AVAIL) 	/* lock was available? */
#		goto grant_lock;
#
#	if (owner == t->t_tid)   	/* lock already mine? */
#		rc = LOCK_NEST;		/* nested lock return code */
#		goto check_trace;
#
#	if (flags & LOCK_NDELAY)	/* requester won't wait? */
#		rc = LOCK_FAIL;		/* request failed */
#		goto check_trace;
#
#	set stack fix flag
#	touch stack	
#
#	call klockl(lock_word, flag, old intpri) /* will not return here */
#
# grant_lock:
#	*lock_word = t->t_tid;	/* claim the lock */
#	rc = LOCK_SUCC
#
# check_trace:
#       if (Trconflag[0])
#               trace call
#
# enable:
#	i_enable(ipri);
#	return(rc);
#
#**************************************************************************

	.csect	lockl_pwr_overlay[RO]
DATA(lockl_pwr):
	.globl	DATA(lockl_pwr)

	lbz     r11, DATA(Trconflag)    # load first byte of trace flags array
	cmpi    cr7, r11, 0             # check "trace active" indicator

	mfmsr	r6			# save the MSR value in r6
	cal	r11, DISABLED_MSR(0)	# calculate a disabled MSR
	mtmsr	r11			# disable interrupts

#	N.B. this code "cheats" - it disables but DOES NOT reset mstintpri.
#	this has two consequences.  One, since mstintpri has its old value,
#	if we page fault on the lockword and are restarted the EIS/EIM
#	will be restored correctly.
#	The second consequence it that if the caller expects to page fault
#	on the lockword he had better be at a priority that is allowed to
#	page fault.  This implies more work for someone running somewhere
#	between INTBASE and INTMAX who expects to page fault on a lock -	
#	but make the lock/unlock path length MUCH shorter for the normal
#	case.  If a program is REALLY using both a lock and priority for
#	synchronization it should probably get the lock, then disable,
#	do its thing, enable, and then release the lock!.

ifdef(`_POWER_MP',`
#	This works since interrupts are disabled
	GET_PPDA(cr1, r9)		# get PPDA in r9
	.machine "pwr"
	l	r12, ppda_csa(r9)	# get current mst
	l	r5, ppda_curthread(r9)	# get current thread
',`
	GET_CSA(cr1, r11, r12)		# get current mst 
	GET_CURTHREAD(cr1, r11, r5)	# get pointer to current thread
')

ifdef(`DEBUG',`
	l	r11, mstprev(r12)	# can only be called at process level
	tnei	r11, 0
')
	l	r7, 0(r3)		# get the value of the lockword
	
	cmpi	cr0, r7, LOCK_AVAIL	# check if lock is available
	l	r10, t_tid(r5)		# load thread id
	lhz	r8, t_lockcount(r5)	# load lockcount field
	bne	lock_miss		# branch if lock is busy

	st	r10, 0(r3)		# grant lock by storing tid in lock word
	ai	r8, r8, 1		# increment lockcount
	cal	r9, hkwd_LOCK_TAKEN(0)	# setup sub-hook
	sth	r8, t_lockcount(r5)	# store new lockcount
	cal	r11, LOCK_SUCC(0)	# load return value

lockl_check_trace:
	bne	cr7, lockltrace		# trace call

lockl_enable_ints:
	mtmsr	r6			# enable interupts
	mr	r3, r11			# load return code and exit
	cror	0,0,0			# 603 specific work around
	br
	
lockltrace:
	mflr	r5
	st	r5, stklink(r1)		# save link register

	st	r6, -4(r1)		# save msr
	st	r11, -8(r1)		# save return value

	mr	r8, r4			# 6th parm is flags
	mr	r7, r5			# 5th parm is return addr
	mr	r6, r10			# 4th parm is tid
	l	r5, 0(r3)		# 3rd parm is lockword value
	mr	r4, r3			# 2nd parm is lockword addr
	mr	r3, r9		# setup sub-hook
	cau     r3, 0, (HKWD_KERN_LOCKL | HKTY_GT)>16 # 1st parm is hook type

#	significant registers saved across call to trchook
#
#	r6  = original msr value
#	r11 = return value

	bla     ENTRY(trchook)         	# call trace routine
       .extern  ENTRY(trchook)

	l	r6, -4(r1)		# restore msr
	l	r11, -8(r1)		# restore return value

	l	r5, stklink(r1)		# restore link register
	mtlr	r5

	b 	lockl_enable_ints

lock_miss:
	rlinm	r11, r7, 0, LOCKL_OWNER_MASK # keep only tid
	cmp	cr0, r11, r10		# owner == curthread ?
	cal	r11, LOCK_NEST(0)	# calculate the return code
	cal	r9, hkwd_LOCK_RECURSIVE(0)
	beq	lockl_check_trace	# if thread already has the lock
					# then return

	lbz	r5, mstintpri(r12)	# Save the old intr priority for klockl
	rlinm.	r11, r4, 0, LOCK_NDELAY	# check the NDELAY flag

	cmpi	cr1, r5, INTBASE	# check if proc was at INTBASE
	cal	r11, LOCK_FAIL(0)	# set up return code
	cal	r9, hkwd_LOCK_BUSY(0)	# set up trace hook
	bne	lockl_check_trace	# if LOCK_NDELAY was set then branch

#	Fix the pages around the current stack pointer.
#	This sequence may fault on the stack.
#	Whenever anyone faults with mststackfix non-null, resume()
#	will carefully touch the same area we are touching here.
#	

	cal	r11, INTMAX(0)		# disabled interrupt priority
	stb	r11, mstintpri(r12)	# Set priority to disabled
	bne	cr1, fixed_stack1	# branch if we were not at INTBASE.
					#  we already have a fixed stack

	st	r1, mststackfix(r12)	# fix area around the stack pointer
ifdef(`_POWER_MP',`
ifdef(`_SLICER',`
	GET_PPDA(cr0, r9)		# clear stack fix flag for MP LRU
	.machine "pwr"
	lil	r0, 1
	stb	r0, ppda_stackfix(r9)
')
')
	l	r0, STACKTOUCH(r1)
	l	r10, 0(r1)              # superstition - second load to a
	l	r0, -STACKTOUCH(r1)     # diff reg to machine isn't "too" smart

fixed_stack1:                           # r5 has old mstpri whick klock expects
	ba	ENTRY(klockl)		# branch to klockl to handle lock miss
	.extern	ENTRY(klockl)   	# we count on klockl to enable at end

#
#	fast path front end to unlockl
#	this routine checks the normal case of releasing
#	a lock that is not being waited for by anyone
#
#unlockl(lock_t *lock_word)
#{
#	struct proc *p,*nextp;
#	int ipri;
#	ipri = i_disable(INTMAX);
#	t = curthread;
#	assert(*lock_word == t->t_tid);		/* caller must own the lock */
#	if (*lock_word & WAITING)
# 	 	kunlock();			/* never returns */
#	*lock_word = LOCK_AVAIL;
#	i_enable(ipri);
#	return;
			
	.csect	unlockl_pwr_overlay[RO]
DATA(unlockl_pwr):
	.globl	DATA(unlockl_pwr)

	lbz     r4, DATA(Trconflag)    	# load first byte of trace flags array
	cmpi    cr7, r4, 0             	# check "trace active" indicator

	mfmsr	r9			# save the MSR value in r9
	cal	r11, DISABLED_MSR(0)	# calculate a disabled MSR
	mtmsr	r11			# disable interrupts

#	N.B. see lockl comments above about disable "cheat"

ifdef(`_POWER_MP',`
#	This works since interrupts are disabled
	GET_PPDA(cr0, r8)
	l	r10, ppda_curthread(r8)	# get pointer to current thread
	l	r12, ppda_csa(r8)	# get current mst
',`
	GET_CURTHREAD(cr0, r11, r10)	# get pointer to current thread
	GET_CSA(cr1, r11, r12)		# get current mst 
')
	l	r6, t_tid(r10)		# make sure this thread holds lock
	bne	cr7, unlockltrace	# branch if trace active

unlocklcont:
	cal	r7, LOCK_AVAIL(0)	# calculate the new lockword value
	l	r0, 0(r3)		# if we are going to fault do it now!
	rlinm	r11, r0, 0, LOCKL_OWNER_MASK # keep only tid
	lhz	r8, t_lockcount(r10)	# get t_lockcount
	tne	r11, r6			# assert that this thread holds lock

	rlinm.	r11, r0, 0, WAITING	# check waiting bit
	bne	cr0, call_kunlock	# call back end if waiting bit set

	st	r7, 0(r3)		# release the lock

	si	r8, r8, 1		# decrement lock count
	sth	r8, t_lockcount(r10)	# update t_lockcount

unlockl_enable_ints:
	mtmsr	r9			# enable interupts
	br
	
call_kunlock:
#	Fix the pages around the current stack pointer.
#	This sequence may fault on the stack.
#	Whenever anyone faults with mststackfix non-null, resume()
#	will carefully touch the same area we are touching here.
#	

	lbz	r4, mstintpri(r12)	# save the old intr priority for kunlockl
	cal	r11, INTMAX(0)		# disabled interrupt priority
	cmpi	cr1, r4, INTBASE	# check if thread was at INTBASE
	stb	r11, mstintpri(r12)	# Set priority to disabled
	bne	cr1, fixed_stack2	# branch if we were not at INTBASE.
					#  we already have a fixed stack

	st	r1, mststackfix(r12)	# fix area around the stack pointer
	l	r0, STACKTOUCH(r1)
	l	r10, 0(r1)              # superstition - second load to a
	l	r0, -STACKTOUCH(r1)     # diff reg to machine isn't "too" smart

fixed_stack2:				# r4 has old mstpri which kunlock expects
	ba	ENTRY(kunlockl)		# branch to kunlockl to handle hard case
	.extern	ENTRY(kunlockl)		# we count on klockl to enable at end

unlockltrace:
	mflr	r7
	st	r7, stklink(r1)		# save link register
	st	r3, -4(r1)		# save lockword address
	st	r6, -8(r1)		# save tid
	st	r10, -12(r1)		# save curthread
	st	r12, -16(r1)		# save current mst
	st	r9, -20(r1)		# save current msr

					# 5th parm is return addr
					# 4th parm is tid
	l	r5, 0(r3)		# 3rd parm is lockword value
	mr	r4, r3			# 2nd parm is lockword addr
	cau     r3, 0, (HKWD_KERN_UNLOCKL | HKTY_GT)>16 # 1st parm is hook type

#	significant registers saved across call to trchook
#
#	r3  - lockword address
#	r6  - tid
#       r9  - msr
#	r10 - curthread
#	r12 - current mst

	bla     ENTRY(trchook)         	# call trace routine
       .extern  ENTRY(trchook)

	l	r9, -20(r1)		# restore current msr
	l	r12, -16(r1)		# restore current mst
	l	r10, -12(r1)		# restore curthread
	l	r6, -8(r1)		# restore tid
	l	r3, -4(r1)		# restore lockword address
	l	r7, stklink(r1)		# restore link register
	mtlr	r7

	b 	unlocklcont

include(pri.m4)
include(mstsave.m4)
include(lock_def.m4)
include(low_dsect.m4)
include(machine.m4)
include(proc.m4)
include(param.m4)
include(i_machine.m4)
include(trchkid.m4)
