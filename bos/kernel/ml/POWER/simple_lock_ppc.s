# @(#)72	1.26  src/bos/kernel/ml/POWER/simple_lock_ppc.s, sysml, bos412, 9446B 11/10/94 09:04:44
#
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS:	simple_lock_ppc
#		simple_lock_instr_ppc
#		simple_unlock_ppc
#		simple_unlock_instr_ppc
#		simple_lock_try_ppc
#		simple_lock_try_instr_ppc
#		rsimple_lock_ppc
#		rsimple_lock_instr_ppc
#		rsimple_lock_try_ppc
#		rsimple_lock_try_instr_ppc
#		rsimple_unlock_ppc
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

undefine(`_POWER_RS1')
undefine(`_POWER_RSC')
undefine(`_POWER_RS2')
undefine(`_POWER_RS')

	.file	"simple_lock_ppc.s"
	.machine "ppc"

	.using	low, 0

include(macros.m4)

#***************************************************************************
#
# FUNCTION: acquire simple lock
#
# CALL:
#	void simple_lock(simple_lock_t l)
#
# NOTES:
#	This function handles the case where lock is available.
# If some real work needs to be done slock is called.
#	This function does not get a stack frame.  If the lock
# is not available then control is passed to slock without
# changing the contents of the link register. slock will
# return to the caller of this routine
#
# PSEUDO CODE:
#
# void simple_lock(simple_lock_t l);
# 
# {
# 
# register owner_t    myself = GET_ME();
# register int lockword;
# register int old_value;
# register int id;
# register int spincount=0;
# #ifdef _INSTRUMENTATION
#		l = *l;
# #endif
# 		id = csa()->prev == 0 ? csa()->curthread
#				      : get_processor_id() << 1;
# retry:
# 		lockword =  *l;
# #ifdef DEBUG
#		assert ( (lockword & OWNER_MASK) != id);
# #endif
# 		if (!(lockword & LOCKBIT)){
# 			lockword |= (id | LOCKBIT);
#			myself->t_lockcount++;
#			l->acquisitions++;
#
#			if (spincount)
#				l->misses++;
# 			return;
# 		}
# 
# 		else if (csa()->intpri != INTBASE) {
#				spincount++;
# 				goto retry;
# 		}
# 
# 		else {
# 			slock(l);
# 		}
# }
#
#
# REGISTERS
#		r3	lockaddr
#		r4	Trconflag
#		r5	curthread/curprocessor
#		r6	id
#		r7	lockword
#		r8	statistics structure if instrumented else not used
#		r9	t_lockcount
#		r10/r11 scratch
#
#

ifdef(`_INSTRUMENTATION',`
	.csect simple_lock_instr_ppc_overlay[RO]
	.set	STKMIN, 96		# minimum stack size

DATA(simple_lock_instr_ppc):
	.globl DATA(simple_lock_instr_ppc)
',`
	.csect simple_lock_ppc_overlay[RO]
	.set	STKMIN, 96		# minimum stack size

DATA(simple_lock_ppc):
	.globl DATA(simple_lock_ppc)
')

ifdef(`_INSTRUMENTATION',`
	lbz     r4, DATA(Trconflag)     # load first byte of trace flags array
	lwz	r8,0(r3)		# load pointer to statistics structure
	cmpi    cr7, r4, 0              # check "trace active" indicator
')

ifdef(`_POWER_MP',`
',`
ifdef(`DEBUG',`
	GET_CSA(cr0,r11,r10)		# get current mst
	.machine "ppc"
	lwz	r11,mstprev(r10)	# check for interrupt handler
	twnei	r11, nullA		# trap if not thread
')
')

ifdef(`DEBUG',`
	lil	r12,-1			# debug only spin count
')

ifdef(`_INSTRUMENTATION',`
	rlinm.	r7,r8,0,INSTR_ON	# test if instrumentation enabled
        beq-    sl_grant_lock		# branch if lock not instrumented

sl_grant_lock_instr:

	lwarx	r7,0,r8			# load the value of the lockword
	GET_CURTHREAD(cr0, r9, r5)	# get current thread
	.machine "ppc"
	cmpi	cr0,r7,SIMPLE_LOCK_AVAIL
	lwz	r6,t_tid(r5)		# load thread id
	lhz	r9,t_lockcount(r5)	# load lockcount field

ifdef(`DEBUG',`
	subic	r12,r12,1		# decrement spin count
	rlinm	r11,r7,0,OWNER_MASK	# clear all bits but owner bit
	tweqi	r12, 0			# been here too long--die!
	tweq	r11,r6			# assert lock is not recursive
')
	bne-	sl_check_waiting_instr
sl_take_lock_instr:
	stwcx.	r6,0,r8			# grab the lock
	addi	r9,r9,1			# increment lockcount

	bne-	sl_grant_lock_instr

ifdef(`DEBUG',`
	mflr	r11
	stw	r11,lock_lr(r8)
	stw	r6,lock_caller(r8)
	mfspr	r11,PID
	stw	r11,lock_cpuid(r8)
')
	lwz	r10,acquisitions(r8)	# load acquisitions field 
	addi	r10,r10,1		# increment acquisitions
	stw	r10,acquisitions(r8)	# never miss here

ifdef(`_POWER_MP',`
	isync
')
	sth	r9,t_lockcount(r5)	# store modified lockount

sl_check_trace_instr:
	bne-     cr7,sl_trace_instr	# branch if trace is active
	br

sl_check_waiting_instr:
	rlinm.	r7, r7, 0, ~WAITING	# mask everything but waiting bit
	oriu	r6, r6, WAITING>16	# assume lock free, but waiters
	beq	sl_take_lock_instr	# branch if lock free

sl_spinning_instr:
	GET_CSA(cr0,r11,r10)		# get current mst
	.machine "ppc"

# a thread/proc is allowed to spin more than maxspin cycles if its intpri 
# is not INTBASE.
# This means:
#	a: it is an interrupt handler
#	b: the lock is used to serialize a thread/interrupt critical section

	lbz	r11,mstintpri(r10)	# load csa()->intpri
	cmpi    cr0, r11, INTBASE	# check if proc was at INTBASE
	bne	sl_lock_busy_instr	# caller is disabled

#
#       Must buy a stack frame in order to restore callers toc
#
        mflr    r4                              # Save link reg
                                                # address of caller in 2nd C parameter
        st      r4, stklink(r1)                 # Store return addr to caller
        stu     r1, -(8+STKMIN)(r1)             # Back up the stack pointer
        st      r2,stktoc(r1)                   # put caller toc onto stack
                                                # (buy stack frame)
	l	r2, DATA(g_toc)			# get kernel toc

	bla	ENTRY(slock_ppc)	# branch to slock to handle lock miss
	.extern	ENTRY(slock_ppc)

        l       r0, (8+STKMIN+stklink)(r1) 	# Reload return address
        l       r2,stktoc(r1)                   # restore caller toc
        cal     r1, (8+STKMIN)(r1)              # Pop the stack frame
        mtlr    r0                              # Return addr to LR
        br                                      # return to caller


sl_lock_busy_instr:
	bne-	cr7,sl_trace_miss_instr	# branch if trace active

sl_fast_grant_instr:
ifdef(`DEBUG',`
	lil	r12,-1			# debug only spin count
')
	lhz	r9,t_lockcount(r5)	# reload lockcount
	lwz	r6,t_tid(r5)		# reload thread id
	addi	r9,r9,1			# increment lockcount

sl_fast_loop_instr:
	lwarx	r7,0,r8			# load value of the lockword
ifdef(`DEBUG',`
	subic	r12,r12,1		# decrement spin count
	rlinm	r11,r7,0,WAITING	# check WAITING bit
	tweqi	r12,0			# been here too long--die!
	twnei	r11,0			# WAITING bit should NEVER be set
')
	cmpi	cr0,r7,SIMPLE_LOCK_AVAIL
	bne-	sl_fast_loop_instr
sl_fast_take_instr:
	stwcx.	r6,0,r8			# grab the lock

	bne-	sl_fast_loop_instr

ifdef(`DEBUG',`
	mflr	r11
	stw	r11,lock_lr(r8)
	stw	r6,lock_caller(r8)
	mfspr	r11,PID
	stw	r11,lock_cpuid(r8)
')
	lwz	r10,acquisitions(r8)	# load acquisitions field 
	lwz	r11,misses(r8)		# load misses field
	addi	r10,r10,1		# increment acquisitions
	addi	r11,r11,1		# increment misses (ALWAYS)
	stw	r10,acquisitions(r8)
	stw	r11,misses(r8)

ifdef(`_POWER_MP',`
	isync
')
	sth	r9,t_lockcount(r5)	# store modified lockcount
	bne-    cr7,sl_trace_instr	# branch if trace is active
	br

sl_trace_instr:
	mflr    r7			# 5th parm is return addr
	stw     r7, stklink(r1)         # save link register
	stw	r2, -4(r1)		# save caller toc
	lwz	r2, DATA(g_toc)		# get kernel toc
	li	r6, LOCK_SWRITE_TRACE	# 4th parm is trace type
	lwz     r5, 0(r8)               # 3rd parm is lockword value
	lwz	r8, lockname(r8)	# 6th parm is lockname
	mr      r4, r3                  # 2nd parm is lockword addr
	li	r3, hkwd_LOCK_TAKEN	# 1st parm is hook type
	addis   r3, r3, (HKWD_KERN_LOCK | HKTY_GT)>16 

	bla     ENTRY(trchook)          # call trace routine
	.extern ENTRY(trchook)

	lwz     r5, stklink(r1)         # restore link register
	lwz	r2, -4(r1)		# restore caller toc
	mtlr    r5
	br

sl_trace_miss_instr:
	mfcr	r11
	mflr    r7			# 5th parm is return addr
	stw	r2, -20(r1)		# save caller toc
	lwz	r2, DATA(g_toc)		# get kernel toc
	stw	r8, -4(r1)		# save instr lockaddr
	stw	r5, -8(r1)		# save curthread
	stw	r11, -12(r1)		# save CR
	stw	r3, -16(r1)		# save real lockaddr
	stw     r7, stklink(r1)         # save link register
	li	r6, LOCK_SWRITE_TRACE	# 4th parm is trace type
	lwz     r5, 0(r8)               # 3rd parm is lockword value
	lwz	r8, lockname(r8)	# 6th parm is lockname
	mr      r4, r3                  # 2nd parm is lockword addr
	li	r3, hkwd_LOCK_MISS	# 1st parm is hook type
	addis   r3, r3, (HKWD_KERN_LOCK | HKTY_GT)>16 

	bla     ENTRY(trchook)          # call trace routine
	.extern ENTRY(trchook)

	lwz     r7, stklink(r1)         # restore link register
	lwz	r3, -16(r1)		# restore real lockaddr
	lwz	r11, -12(r1)		# restore CR
	lwz	r5, -8(r1)		# restore curthread
	lwz	r8, -4(r1)		# restore instr lockaddr
	mtlr    r7
	lwz	r2, -20(r1)		# restore caller toc
	mtcr	r11
	b	sl_fast_grant_instr
')					# _INSTRUMENTATION

sl_grant_lock:

	lwarx	r7,0,r3			# load the value of the lockword
	GET_CURTHREAD(cr0, r9, r5)	# get current thread
	.machine "ppc"
	cmpi	cr0,r7,SIMPLE_LOCK_AVAIL
	lwz	r6,t_tid(r5)		# load thread id
	lhz	r9,t_lockcount(r5)	# load lockcount field

ifdef(`DEBUG',`
	subic	r12,r12,1		# decrement spin count
	rlinm	r11,r7,0,OWNER_MASK	# clear all bits but owner bit
	tweqi	r12, 0			# been here too long--die!
	tweq	r11,r6			# assert lock is not recursive
')
	bne-	sl_check_waiting
sl_take_lock:
	stwcx.	r6,0,r3			# grab the lock
	addi	r9,r9,1			# increment lockcount

	bne-	sl_grant_lock

ifdef(`_POWER_MP',`
	isync
')
	sth	r9,t_lockcount(r5)	# store modified lockcount
ifdef(`_INSTRUMENTATION',`
	bne-    cr7,sl_trace		# branch if trace is active
')
	br

sl_check_waiting:
	rlinm.	r7, r7, 0, ~WAITING	# mask everything but waiting bit
	oriu	r6, r6, WAITING>16	# assume lock free, but waiters
	beq	sl_take_lock		# branch if lock free

sl_spinning:
	GET_CSA(cr0,r11,r10)		# get current mst
	.machine "ppc"

# a thread/proc is allowed to spin more than maxspin cycles if its intpri 
# is not INTBASE.
# This means:
#	a: it's an interrupt handler
#	b: the lock is used to serialize a thread/interrupt critical section

	lbz	r11,mstintpri(r10)	# load csa()->intpri
	cmpi    cr0, r11, INTBASE	# check if proc was at INTBASE
	bne	sl_lock_busy		# caller is disabled

#
#       Must buy a stack frame in order to restore callers toc
#
        mflr    r4                              # Save link reg
                                                # address of caller in 2nd C parameter
        st      r4, stklink(r1)                 # Store return addr to caller
        stu     r1, -(8+STKMIN)(r1)             # Back up the stack pointer
        st      r2,stktoc(r1)                   # put caller toc onto stack
                                                # (buy stack frame)
	l	r2, DATA(g_toc)			# get kernel toc

	bla	ENTRY(slock_ppc)	# branch to slock to handle lock miss
	.extern	ENTRY(slock_ppc)

        l       r0, (8+STKMIN+stklink)(r1)	# Reload return address
        l       r2,stktoc(r1)                   # restore caller toc
        cal     r1, (8+STKMIN)(r1)              # Pop the stack frame
        mtlr    r0                              # Return addr to LR
        br                                      # return to caller

sl_lock_busy:
ifdef(`_INSTRUMENTATION',`
	bne-	cr7,sl_trace_miss	# branch if trace active
')

sl_fast_grant:
ifdef(`DEBUG',`
	lil	r12,-1			# debug only spin count
')
	lhz	r9,t_lockcount(r5)	# reload lockcount
	lwz	r6,t_tid(r5)		# reload thread id
	addi	r9,r9,1			# increment lockcount

sl_fast_loop:
	lwarx	r7,0,r3			# load value of the lockword
ifdef(`DEBUG',`
	subic	r12,r12,1		# decrement spin count
	rlinm	r11,r7,0,WAITING	# check WAITING bit
	tweqi	r12,0			# been here too long--die!
	twnei	r11,0			# WAITING bit should NEVER be set
')
	cmpi	cr0,r7,SIMPLE_LOCK_AVAIL
	bne-	sl_fast_loop
sl_fast_take:
	stwcx.	r6,0,r3			# grab the lock

	bne-	sl_fast_loop

ifdef(`_POWER_MP',`
	isync
')
	sth	r9,t_lockcount(r5)	# store modified lockcount
ifdef(`_INSTRUMENTATION',`
	bne-    cr7,sl_trace		# branch if trace is active
')
	br

ifdef(`_INSTRUMENTATION',`
sl_trace:
	li	r8, 0			# 6th parm is lockname
	mflr    r7			# 5th parm is return addr
	stw     r7, stklink(r1)         # save link register
	stw	r2, -4(r1)		# save caller toc
	lwz	r2,DATA(g_toc)		# get kernel toc
	li	r6, LOCK_SWRITE_TRACE	# 4th parm is trace type
	lwz     r5, 0(r3)               # 3rd parm is lockword value
	mr      r4, r3                  # 2nd parm is lockword addr
	li	r3, hkwd_LOCK_TAKEN	# 1st parm is hook type
	addis   r3, r3, (HKWD_KERN_LOCK | HKTY_GT)>16 

	bla     ENTRY(trchook)          # call trace routine
	.extern ENTRY(trchook)

	lwz     r5, stklink(r1)         # restore link register
	lwz	r2, -4(r1)		# restore caller toc
	mtlr    r5
	br

sl_trace_miss:
	mfcr	r11
	li	r8, 0			# 6th parm is lockname
	mflr    r7			# 5th parm is return addr
	stw     r7, stklink(r1)         # save link register
	stw	r2, -16(r1)		# save caller toc
	lwz	r2, DATA(g_toc)		# get kernel toc
	stw	r3, -4(r1)		# save lockaddr
	stw	r5, -8(r1)		# save curthread
	stw	r11, -12(r1)		# save CR
	li	r6, LOCK_SWRITE_TRACE	# 4th parm is trace type
	lwz     r5, 0(r3)               # 3rd parm is lockword value
	mr      r4, r3                  # 2nd parm is lockword addr
	li	r3, hkwd_LOCK_MISS	# 1st parm is hook type
	addis   r3, r3, (HKWD_KERN_LOCK | HKTY_GT)>16 

	bla     ENTRY(trchook)          # call trace routine
	.extern ENTRY(trchook)

	lwz     r7, stklink(r1)         # restore link register
	lwz	r11, -12(r1)		# retore CR
	lwz	r5, -8(r1)		# restore curthread
	lwz	r3, -4(r1)		# restore lockaddr
	lwz	r2, -16(r1)		# restore caller toc
	mtlr    r7
	mtcr	r11
	b	sl_fast_grant
')

#	SIMPLE_UNLOCK
#
#
#	fast path front end to simple_unlock
#	this routine checks the normal case of releasing
#	a lock that is not being waited for by anyone
#
#
#	PSEUDO CODE
#	
#void simple_unlock (simple_lock_t l)
#
#{
#register owner_t    myself = CSA;
#register int id;
#
# #ifdef _INSTRUMENTATION
#	l = *l;
# #endif
#
# #ifdef _POWER_MP
#	sync();
# #endif
#	
#	id = csa()->prev == 0 ? curthread->t_tid : (CPUID << 1) ;
#
#	if (!(*l & INTERLOCK)) {
#		if (!(*l & WAITING)) {
# #ifdef DEBUG
#			assert( *l == id )
# #endif
#			fetch_and_nop((atomic_p)l,SIMPLE_LOCK_AVAIL));
#       		TRCHKGT(HKWD_KERN_UNLOCK,
#				l,*((simple_lock *)l),csa()->prev->iar);
#			if (id & THREAD_BIT)
#				 DECR_LOCKCOUNT(myself));
#			return;
#		}
#	}
#	sunlock(l);
#
#}
#
# REGISTERS
#		r3	lockaddr
#		r4	Trconflag
#		r5	curthread/curprocessor
#		r6	id
#		r7	lockword
#		r8	statistics structure if instrumented else not used
#		r9	t_lockcount
#		r10/r11 scratch
#
#

ifdef(`_INSTRUMENTATION',`
	.csect simple_unlock_instr_ppc_overlay[RO]

DATA(simple_unlock_instr_ppc):
	.globl DATA(simple_unlock_instr_ppc)
',`
	.csect simple_unlock_ppc_overlay[RO]

DATA(simple_unlock_ppc):
	.globl DATA(simple_unlock_ppc)
')

ifdef(`_POWER_MP',`
	sync				# wait for writes completed in memory
')

ifdef(`_INSTRUMENTATION',`
	lwz	r8,0(r3)		# load pointer to statistics structure
')

	GET_CURTHREAD(cr0, r9, r5)	# get current thread
	.machine "ppc"
ifdef(`DEBUG',`
	lwz     r6,t_tid(r5)		# get current thread id
')

ifdef(`_INSTRUMENTATION',`
	lbz     r4, DATA(Trconflag)     # load first byte of trace flags array
	rlinm.	r7,r8,0,INSTR_ON	# test if instrumentation enabled
	cmpi    cr7, r4, 0              # check "trace active" indicator
        beq-    su_retry		# branch if lock not instrumented
ifdef(`DEBUG',`
	mflr	r11
	stw	r11,unlock_lr(r8)
	stw	r6,unlock_caller(r8)
	mfspr	r11,PID
	stw	r11,unlock_cpuid(r8)
')

su_retry_instr:

	lwarx	r7,0,r8			# get the value of the lockword
	lhz     r9,t_lockcount(r5)	# get t_lockcount

	rlinm.	r11,r7,0,(INTERLOCK|WAITING) # test interlock/waiting bits
	li	r12,SIMPLE_LOCK_AVAIL
	bne-	su_check_status_bits_instr # check status bits

ifdef(`DEBUG',`
	twne    r7, r6                  # assert that this thread/IH holds lock
')
	stwcx.	r12,0,r8		# release the lock
	subic	r9,r9,1			# decrement lockcount
	bne-	su_retry_instr		# set: retry

	sth	r9,t_lockcount(r5)	# store modified lockcount

su_check_trace_instr:
	bne-     cr7, su_trace_instr	# branch if trace is active
	br

su_check_status_bits_instr:
	rlinm.	r11,r7,0,WAITING	# check WAITING bit
	beq-	su_retry_instr		# WAITING bit not set--must be INTERLOCK

su_call_sunlock_instr:
#
#       Must buy a stack frame in order to restore callers toc
#
        mflr    r4                              # Save link reg
                                                # address of caller in 2nd C parameter
        st      r4, stklink(r1)                 # Store return addr to caller
        stu     r1, -(8+STKMIN)(r1)             # Back up the stack pointer
        st      r2,stktoc(r1)                   # put caller toc onto stack
                                                # (buy stack frame)
	l	r2, DATA(g_toc)			# get kernel toc

	bla	ENTRY(sunlock_ppc)	# branch to slock to handle lock miss
	.extern	ENTRY(sunlock_ppc)

        l       r0, (8+STKMIN+stklink)(r1)  	# Reload return address
        l       r2,stktoc(r1)                   # restore caller toc
        cal     r1, (8+STKMIN)(r1)              # Pop the stack frame
        mtlr    r0                              # Return addr to LR
        br                                      # return to caller

su_trace_instr:
	mflr    r6			# 4th parm is return addr
	stw     r6, stklink(r1)         # save link register
	stw	r2, -4(r1)		# save caller toc
	lwz	r2, DATA(g_toc)		# get kernel toc

	lwz	r7,lockname(r8)		# 5th parm is lockname
	lwz     r5, 0(r8)               # 3rd parm is lockword value
	mr      r4, r3                  # 2nd parm is lockword addr
					# 1st parm is hook type
	addis   r3, 0, (HKWD_KERN_UNLOCK | HKTY_GT)>16 

#       significant registers saved across call to trchook
#
#       r5 = link register

	bla     ENTRY(trchook)          # call trace routine
       .extern  ENTRY(trchook)

	lwz     r5, stklink(r1)         # restore link register
	lwz	r2, -4(r1)		# restore caller toc
	mtlr    r5
	br

')					# _INSTRUMENTATION

su_retry:

	lwarx	r7,0,r3			# get the value of the lockword
	lhz     r9,t_lockcount(r5)	# get t_lockcount

	rlinm.	r11,r7,0,(INTERLOCK|WAITING) # test interlock/waiting bits
	li	r12,SIMPLE_LOCK_AVAIL
	bne-	su_check_status_bits	# check status bits

ifdef(`DEBUG',`
	twne    r7, r6                  # assert that this thread/IH holds lock
')
	stwcx.	r12,0,r3		# release the lock
	subic	r9,r9,1			# decrement lockcount
	bne-	su_retry		# set: retry

	sth	r9,t_lockcount(r5)

su_exit:
ifdef(`_INSTRUMENTATION',`
	bne-    cr7, su_trace		# branch if trace is active
')
	br

su_check_status_bits:
	rlinm.	r11,r7,0,WAITING	# check WAITING bit
	beq-	su_retry		# WAITING bit not set--must be INTERLOCK

su_call_sunlock:
#
#       Must buy a stack frame in order to restore callers toc
#
        mflr    r4                              # Save link reg
                                                # address of caller in 2nd C parameter
        st      r4, stklink(r1)                 # Store return addr to caller
        stu     r1, -(8+STKMIN)(r1)             # Back up the stack pointer
        st      r2,stktoc(r1)                   # put caller toc onto stack
                                                # (buy stack frame)
	l	r2, DATA(g_toc)			# get kernel toc

	bla	ENTRY(sunlock_ppc)	# branch to slock to handle lock miss
	.extern	ENTRY(sunlock_ppc)

        l       r0, (8+STKMIN+stklink)(r1)	# Reload return address
        l       r2,stktoc(r1)                   # restore caller toc
        cal     r1, (8+STKMIN)(r1)              # Pop the stack frame
        mtlr    r0                              # Return addr to LR
        br                                      # return to caller

ifdef(`_INSTRUMENTATION',`
su_trace:
	mflr    r6			# 4th parm is return addr
	stw     r6, stklink(r1)         # save link register
	stw	r2, -4(r1)		# save caller toc
	l	r2, DATA(g_toc)		# get kernel toc

	lwz     r5, 0(r3)               # 3rd parm is lockword value
	mr      r4, r3                  # 2nd parm is lockword addr
					# 1st parm is hook type
	addis   r3, 0, (HKWD_KERN_UNLOCK | HKTY_GT)>16 

#       significant registers saved across call to trchook
#
#       r5 = link register

	bla     ENTRY(trchook)          # call trace routine
       .extern  ENTRY(trchook)

	lwz     r5, stklink(r1)         # restore link register
	lwz	r2, -4(r1)		# restore caller toc
	mtlr    r5
	br
')

# SIMPLE_LOCK_TRY
# 
# this function try to acquire a simple lock
#
#
#boolean_t simple_lock_try(simple_lock_t l)
#
#{
#
#register owner_t    myself = GET_ME();
#register int lockword;
#register int id;
#int old_value;
#
#	id = csa()->prev == 0 ? GET_ID() : (MY_CPU() << 1) ;
#
#	lockword = *l;
#	if (!(lockword & LOCKBIT)){
#		lockword |= (id | LOCKBIT);
#		TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,
#			l,*((simple_lock *)l),LOCK_WRITE,csa()->prev->lr);
#		return(TRUE);
#	}
#	TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_BUSY,
#		l,*((simple_lock *)l),LOCK_WRITE,csa()->prev->lr);
#	return(FALSE);
#}
#
# REGISTERS
#		r3	lockaddr
#		r4	Trconflag
#		r5	curthread/curprocessor
#		r6	id
#		r7	lockword
#		r8	statistics structure if instrumented else not used
#		r9	t_lockcount
#		r10/r11 scratch
#
#


ifdef(`_INSTRUMENTATION',`
	.csect simple_lock_try_instr_ppc_overlay[RO]

DATA(simple_lock_try_instr_ppc):
	.globl DATA(simple_lock_try_instr_ppc)
',`
	.csect simple_lock_try_ppc_overlay[RO]

DATA(simple_lock_try_ppc):
	.globl DATA(simple_lock_try_ppc)
')

ifdef(`_INSTRUMENTATION',`
	lbz     r4, DATA(Trconflag)     # load first byte of trace flags array
	lwz	r8,0(r3)		# load pointer to statistics structure
	cmpi    cr7, r4, 0              # check "trace active" indicator
')

ifdef(`_POWER_MP',`
',`
ifdef(`DEBUG',`
	GET_CSA(cr0,r11,r10)		# get current mst
	.machine "ppc"
	lwz	r11,mstprev(r10)	# check for interrupt handler
	twnei	r11, nullA		# trap if not thread
')
')

ifdef(`_INSTRUMENTATION',`
	rlinm.	r7,r8,0,INSTR_ON	# test if instrumentation enabled
	beq-	slt_try_lock		# branch if lock not instrumented

slt_try_lock_instr:
	lwarx	r7,0,r8			# load the value of the lockword
	GET_CURTHREAD(cr0, r9, r5)	# get current thread
	.machine "ppc"
	cmpi	cr0,r7,SIMPLE_LOCK_AVAIL
	lwz	r6,t_tid(r5)		# load thread id
	lhz	r9,t_lockcount(r5)	# load lockcount field

ifdef(`DEBUG',`
	rlinm	r11,r7,0,OWNER_MASK	# clear all bits but owner bit
	tweq	r11,r6			# assert lock is not recursive
')
	bne-	slt_check_waiting_instr
slt_take_lock_instr:
	stwcx.	r6,0,r8			# grab the lock
	addi	r9,r9,1			# increment lockcount

	bne-	slt_try_lock_instr

	lwz	r10,acquisitions(r8)	# load acquisitions field 
	addi	r10,r10,1
	stw	r10,acquisitions(r8)
ifdef(`DEBUG',`
	mflr	r11
	stw	r11,lock_lr(r8)
	stw	r6,lock_caller(r8)
	mfspr	r11,PID
	stw	r11,lock_cpuid(r8)
')

ifdef(`_POWER_MP',`
	isync
')
	sth	r9,t_lockcount(r5)	# store modified lockcount

slt_check_trace_instr:
	li	r11,TRUE		# remember return value
	li	r10, hkwd_LOCK_TAKEN	# 1st parm is hook type
	bne-    cr7, slt_trace_instr	# branch if trace is active
	b	slt_exit

slt_check_waiting_instr:
	rlinm.	r7, r7, 0, ~WAITING	# mask everything but waiting bit
	oriu	r6, r6, WAITING>16	# assume lock free, but waiters
	beq	slt_take_lock_instr	# branch if lock free

slt_lock_fail_instr:
	li	r11,FALSE		# remember return value
	li	r10, hkwd_LOCK_BUSY	# 1st parm is hook type
	beq+	cr7, slt_exit    	# branch if trace is not active

slt_trace_instr:
	addis   r10, r10, (HKWD_KERN_LOCK | HKTY_GT)>16 
	mflr	r7			# 5th parm is return addr
	stw	r7, stklink(r1)         # save link register
	stw	r11,-4(r1)		# save return value
	stw	r2, -8(r1)		# save caller toc
	l	r2, DATA(g_toc)		# get kernel toc
	li	r6,LOCK_SWRITE_TRACE	# 4th parm is request type

	lwz	r5, 0(r8)               # 3rd parm is lockword value
	lwz	r8,lockname(r8)		# 6th parm is lockname
	mr	r4, r3                  # 2nd parm is lockword addr
	mr	r3, r10                 # 1st parm is hook type

#       significant registers saved across call to trchook
#
#       r7  = link register
#	r11 = return value

	bla	ENTRY(trchook)          # call trace routine
       .extern	ENTRY(trchook)

	lwz	r11, -4(r1)		# restore return value
	lwz	r5, stklink(r1)         # restore link register
	lwz	r2, -8(r1)		# restore caller toc
	mtlr	r5

	b	slt_exit

')					# _INSTRUMENTATION

slt_try_lock:
	lwarx	r7,0,r3			# load the value of the lockword
	GET_CURTHREAD(cr0, r9, r5)	# get current thread
	.machine "ppc"
	cmpi	cr0,r7,SIMPLE_LOCK_AVAIL
	lwz	r6,t_tid(r5)		# load thread id
	lhz	r9,t_lockcount(r5)	# load lockcount field

ifdef(`DEBUG',`
	rlinm	r11,r7,0,OWNER_MASK	# clear all bits but owner bit
	tweq	r11,r6			# assert lock is not recursive
')
	bne-	slt_check_waiting
slt_take_lock:
	stwcx.	r6,0,r3			# grab the lock
	addi	r9,r9,1			# increment lockcount

	bne-	slt_try_lock

ifdef(`_POWER_MP',`
	isync
')
	sth	r9,t_lockcount(r5)	# store modified lockount

ifdef(`_INSTRUMENTATION',`
	li	r11,TRUE		# remember return value
	li	r10, hkwd_LOCK_TAKEN	# 1st parm is hook type
	bne-    cr7, slt_trace          # branch if trace is active
',`
	li	r3,TRUE			# return TRUE
')

slt_exit:
ifdef(`_INSTRUMENTATION',`
	mr	r3,r11			# r11 holds return value
')
	br

slt_check_waiting:
	rlinm.	r7, r7, 0, ~WAITING
	oriu	r6, r6, WAITING>16
	beq	slt_take_lock

slt_lock_fail:
ifdef(`_INSTRUMENTATION',`
	li	r11,FALSE		# remember return value
	li	r10, hkwd_LOCK_BUSY	# 1st parm is hook type
	beq+	cr7, slt_exit    	# branch if trace is not active
',`
	li	r3,FALSE		# return FALSE
	br
')

ifdef(`_INSTRUMENTATION',`
slt_trace:
	addis   r10, r10, (HKWD_KERN_LOCK | HKTY_GT)>16 
	mflr	r7			# 5th parm is return addr
	stw	r7, stklink(r1)         # save link register
	stw	r11,-4(r1)		# save return value
	stw	r2, -8(r1)		# save caller toc
	l	r2, DATA(g_toc)		# get kernel toc
	li	r6,LOCK_SWRITE_TRACE	# 4th parm is request type

	lwz	r5, 0(r3)               # 3rd parm is lockword value
	mr	r4, r3                  # 2nd parm is lockword addr
	mr	r3, r10                 # 1st parm is hook type

#       significant registers saved across call to trchook
#
#       r7  = link register
#	r11 = return value

	bla	ENTRY(trchook)          # call trace routine
       .extern	ENTRY(trchook)

	lwz	r11, -4(r1)		# restore return value
	lwz	r5, stklink(r1)         # restore link register
	lwz	r2, -8(r1)		# restore caller toc
	mtlr	r5

	b	slt_exit
')

ifdef(`_POWER_MP',`
#***************************************************************************
#
# FUNCTION: acquire simple recursive spin lock
#
# CALL:
#	void rsimple_lock(simple_lock_t l)
#
# NOTES:
#	This routine is used to obtain locks in code that runs with
#	address translation off and with interrupts disabled.  Only
#	one level of recursion is supported and a trap occurs if a
#	second attempt is made to acquire the lock recursively.
#
# REGISTERS
#		r3	lockaddr
#		r4/r5	scratch 
#
ifdef(`_INSTRUMENTATION',`
        .csect  rsimple_lock_instr_ppc_overlay[RO]

DATA(rsimple_lock_instr_ppc):
        .globl  DATA(rsimple_lock_instr_ppc)
',`
        .csect  rsimple_lock_ppc_overlay[RO]

DATA(rsimple_lock_ppc):
        .globl  DATA(rsimple_lock_ppc)
')

ifdef(`_INSTRUMENTATION',`
	crorc	cr6*4+eq, cr6*4+eq, cr6*4+eq # set eq in cr6 bit for no misses
')
	mfspr	r5,SPRG0		# get PPDA
	lwz	r4,0(r3)		# load the value of the lockword
	lhz	r5,ppda_cpuid(r5)	# get logical cpu id
	sli	r5,r5,1			# shift cpu id 1 bit left (must be even)


	cmpi	cr0,r4,0		# test lock busy
	rlinm	r4,r4,0,OWNER_MASK	# get just processor id
	beq	rsl_skip_check		# lock is not held
	cmp	cr0,r4,r5		# test lock recursive
	beq	rsl_exit_recur
rsl_skip_check:
	lwarx	r4,0,r3			# load the value of the lockword
	cmpi	cr0,r4,0		# test lock busy
	bne	rsl_spinning		# spin
	oriu	r5,r5,LOCKBIT>16	# set lockbit
	stwcx.	r5,0,r3			# grab the lock
	bne	rsl_spinning		# spin

	isync				# to avoid speculative execution

ifdef(`_INSTRUMENTATION',`
	lwz	r4,acquisitions(r3)
	lwz	r5,misses(r3)
	addic.	r4,r4,1			# increment acquisitions
	stw	r4,acquisitions(r3)
	beq	cr6,rsl_exit		# branch if no misses
	addic.	r5,r5,1			# increment misses
	stw	r5,misses(r3)
rsl_exit:
')
	br

# if the lock is recursive we set the RECURSION bit as a flag for the 
# rsimple_unlock call

rsl_exit_recur:
	lwz	r4,0(r3)		# reload the value of the lockword
	andiu.	r5,r4,RECURSION>16	# panic if RECURSION bit is already set
	bne	rsl_panic
	oriu	r4,r4,RECURSION>16	# set RECURSION bit
	stw	r4,0(r3)		# nobody is looking at THIS bit
ifdef(`_INSTRUMENTATION',`
	lwz	r4,acquisitions(r3)
	lwz	r5,sleeps(r3)		# we use this field for recursions
	addic.	r4,r4,1			# increment acquisitions
	stw	r4,acquisitions(r3)
	addic.	r5,r5,1			# increment recursions
	stw	r5,sleeps(r3)
')
	br

rsl_panic:
	tweq	r4,r4			# panic RECURSION

rsl_spinning:
ifdef(`_INSTRUMENTATION',`
	crxor	cr6*4+eq, cr6*4+eq, cr6*4+eq # clear eq in cr6 to record miss
') # _INSTRUMENTATION
	b	rsl_skip_check

#***************************************************************************
#
# FUNCTION: try to acquire simple recursive spin lock
#
# CALL:
#	int rsimple_lock_try(simple_lock_t l)
#
# NOTES:
#	This routine is used to obtain locks in code that runs with
#	address translation off and with interrupts disabled.  Only
#	one level of recursion is supported and a trap occurs if a
#	second attempt is made to acquire the lock recursively.
#
# REGISTERS
#		r3	lockaddr
#		r4/r5	scratch 
#
ifdef(`_INSTRUMENTATION',`
        .csect  rsimple_lock_try_instr_ppc_overlay[RO]

DATA(rsimple_lock_try_instr_ppc):
        .globl  DATA(rsimple_lock_try_instr_ppc)
',`
        .csect  rsimple_lock_try_ppc_overlay[RO]

DATA(rsimple_lock_try_ppc):
        .globl  DATA(rsimple_lock_try_ppc)
')

ifdef(`_INSTRUMENTATION',`
	crorc	cr6*4+eq, cr6*4+eq, cr6*4+eq # set eq in cr6 bit for no misses
')
	mfspr	r5,SPRG0		# get PPDA
	lwz	r4,0(r3)		# load the value of the lockword
	lhz	r5,ppda_cpuid(r5)	# get logical cpu id
	sli	r5,r5,1			# shift cpu id 1 bit left (must be even)


	cmpi	cr0,r4,0		# test lock busy
	rlinm	r4,r4,0,OWNER_MASK	# get just processor id
	beq	rslt_skip_check		# lock is not held
	cmp	cr0,r4,r5		# test lock recursive
	beq	rslt_exit_recur
rslt_skip_check:
	lwarx	r4,0,r3			# load the value of the lockword
	cmpi	cr0,r4,0		# test lock busy
	bne	rslt_fail		# fail -- someone else holds lock
	oriu	r5,r5,LOCKBIT>16	# set lockbit
	stwcx.	r5,0,r3			# grab the lock
	bne	rslt_spinning		# spin

	isync				# to avoid speculative execution

ifdef(`_INSTRUMENTATION',`
	lwz	r4,acquisitions(r3)
	lwz	r5,misses(r3)
	addic.	r4,r4,1			# increment acquisitions
	stw	r4,acquisitions(r3)
	beq	cr6,rslt_exit		# branch if no misses
	addic.	r5,r5,1			# increment misses
	stw	r5,misses(r3)
rslt_exit:
')
	li	r3,TRUE			# indicate lock was acquired
	br

# if the lock is recursive we set the RECURSION bit as a flag for the 
# rsimple_unlock call

rslt_exit_recur:
	lwz	r4,0(r3)		# reload the value of the lockword
	andiu.	r5,r4,RECURSION>16	# panic if RECURSION bit is already set
	bne	rslt_panic
	oriu	r4,r4,RECURSION>16	# set RECURSION bit
	stw	r4,0(r3)		# nobody is looking at THIS bit
ifdef(`_INSTRUMENTATION',`
	lwz	r4,acquisitions(r3)
	lwz	r5,sleeps(r3)		# we use this field for recursions
	addic.	r4,r4,1			# increment acquisitions
	stw	r4,acquisitions(r3)
	addic.	r5,r5,1			# increment recursions
	stw	r5,sleeps(r3)
')
	li	r3,TRUE			# indicate lock is held
	br

rslt_panic:
	tweq	r4,r4			# panic RECURSION

rslt_spinning:
ifdef(`_INSTRUMENTATION',`
	crxor	cr6*4+eq, cr6*4+eq, cr6*4+eq # clear eq in cr6 to record miss
') # _INSTRUMENTATION
	b	rslt_skip_check

rslt_fail:
	li	r3,FALSE		# indicate lock was NOT acquired
	br


ifdef(`_INSTRUMENTATION',`
# No instrumented version of rsimple_unlock
',`
#***************************************************************************
#
# FUNCTION: release simple recursive spin lock
#
# CALL:
#	void rsimple_unlock(simple_lock_t l)
#
# NOTES:
#	If the lock was acquired recursive its recursion bit is set.
#	Called disabled and xlate off.
#
# REGISTERS
#		r3	lockaddr
#		r4/r5	scratch 
#
        .csect  rsimple_unlock_ppc_overlay[RO]

DATA(rsimple_unlock_ppc):
        .globl  DATA(rsimple_unlock_ppc)

	sync				# wait for writes completed in memory

rsu_start:
ifdef(`DEBUG',`
	mfspr	r5,SPRG0		# get PPDA
	lwz	r4,0(r3)		# get the value of the lockword
	lhz	r5,ppda_cpuid(r5)	# get logical cpu id
	rlinm	r4,r4,0,OWNER_MASK	# get just processor id of lockword
	sli     r5,r5,1
	tne	r4,r5			# trap if not owner
	
')
	lwz	r4,0(r3)		# get the value of the lockword

# RECURSION bit is used HERE to indicate THIS lock is recursive

	andiu.	r5,r4,RECURSION>16	# test if recursion set
	bne	rsu_exit		# the lock was held recursive
					# the lock must be released 
	lil	r4,SIMPLE_LOCK_AVAIL
	b 	rsu_out
rsu_exit:
	xoriu	r4,r4,RECURSION>16	# reset recursion
rsu_out:
	stw	r4,0(r3)		# no need for stcx here
	br				# return
') # INSTRUMENTATION
') # _POWER_MP

	.toc
	TOCE(debabend,data)

include(pri.m4)
include(scrs.m4)
include(mstsave.m4)
include(lock_def.m4)
include(low_dsect.m4)
include(machine.m4)
include(proc.m4)
include(i_machine.m4)
include(trchkid.m4)
