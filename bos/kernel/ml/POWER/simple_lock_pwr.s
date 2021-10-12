# @(#)73	1.15  src/bos/kernel/ml/POWER/simple_lock_pwr.s, sysml, bos412, 9445C412a 10/25/94 12:38:58
#
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS:	simple_lock_pwr
#		simple_unlock_pwr
#		simple_lock_try_pwr
#		rsimple_lock_pwr
#		rsimple_lock_try_pwr
#		rsimple_unlock_pwr
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

undefine(`_POWER_601')
undefine(`_POWER_PC')

	.file	"simple_lock_pwr.s"
	.machine "pwr"

	.using	low, 0
include(systemcfg.m4)
include(macros.m4)
#
#
# FUNCTION: acquire simple lock
#
# CALL:
#       void simple_lock(simple_lock_t l)
#
# NOTES:
#       This function handles the case where the lock is available.
# If some real work needs to be done slock is called.
# However, since the glink has been avoided, it is necessary
# to buy a stack frame prior to calling slock to do the
# real work.
#
# lockword definition:
#
#        ----------------------------
#       |             |              |
#       |  I W L X X  |  owner_id    |
#       |             |              |
#         ----------------------------
#	
#	I		Interlock bit
#	W		Waiting bit
#	L		Lock bit
#	owner_id	thread_id
#
# PSEUDO CODE:
#
# void simple_lock(simple_lock_t l);
# 
# {
# 
# register thread_t    myself = curthread;
# register int lockword;
# register int id;
# register int oldpri;
#
#              oldpri = i_disable(INTMAX);
#
#              id = myself->t_tid;
#
#              lockword =  *l;
# #ifdef DEBUG
#              assert ( (lockword & OWNER_MASK) != id);
# #endif
#              if (!(lockword & LOCKBIT)){
#                      lockword |= (id | LOCKBIT);
#                      myself->t_lockcount++;
#              }
# 
#              else {
#                      slock(l,oldpri);
#              }
#              i_enable(oldpri);
# }
#
#
# REGISTERS
#              r3       lockaddr
#              r4       Trconflag
#              r5       curthread
#              r6       id
#              r7       lockword
#              r9       t_lockcount
#              r10      msr saved
#              r11      scratch
#              r12      mst pointer
#
#	S_PROLOG(simple_lock_pwr)
	.csect	simple_lock_pwr_overlay[RO]
	.set	STKMIN, 96		# minimum stack
DATA(simple_lock_pwr):
	.globl	DATA(simple_lock_pwr)

	mfmsr	r10                     # save the MSR value in r10 
	cal	r11, DISABLED_MSR(0)	# calculate a disabled MSR
	mtmsr   r11                     # disable interrupts

ifdef(`_POWER_MP',`
',`
ifdef(`DEBUG',`
#	No caller of simple_lock() should be an interrupt handler.
#	Serialization with interrupt handlers should call disable_lock().
#	In the uni-processor environment, it is enough to disable and
#	not call simple_lock().  The call to simple_lock() is ignored
#	in disable_lock().
	GET_CSA(cr0,r11,r12)		# get current mst
	l	r11, mstprev(r12)	# check for interrupt handler
	tnei	r11, nullA		# trap if not thread
')
')


ifdef(`DEBUG',`
	liu	r4,0x0800		# debug only spin count
')

sl_grant_lock:
	l       r7,0(r3)                # load the value of the lockword
	GET_CURTHREAD(cr0,r11,r5)	# get current thread
	cmpi	cr0, r7, 0
	l       r6,t_tid(r5)            # get thread id
ifdef(`DEBUG',`
	si	r4,r4,1			# decrement spin count	
	rlinm	r11,r7,0,OWNER_MASK	# clear all bits but owner bit
	teqi	r4,0			# been here too long--die!
	teq     r11,r6                  # assert lock is not recursive
')
	lhz     r9,t_lockcount(r5)      # get lockcount field
	bne-    sl_check_waiting
sl_take_lock:
	ai      r9,r9,1                 # thread: increment lockcount
	st      r6,0(r3)                # grab the lock
	sth     r9,t_lockcount(r5)	# store modified lockcount

sl_exit:
ifdef(`DEBUG',`
	rlinm	r11,r6,0,OWNER_MASK	# clear all bits but owner bit
	teqi	r11,0			# trap if lock not taken!
')
	mtmsr   r10                     # enable interrupts
	br

sl_check_waiting:
	rlinm.	r7, r7, 0, ~WAITING	# mask everything but waiting bit
	oriu	r6, r6, WAITING>16	# assume lock free, but waiters
	beq	sl_take_lock		# branch if lock free

	# lock is busy
sl_spinning:
	GET_CSA(cr0,r11,r12)		# get current mst
ifdef(`_POWER_MP',`
ifdef(`_SLICER',`
	mtmsr	r10			# enable interrupts briefly
	cal	r11, DISABLED_MSR(0)	# calculate a disabled MSR
	mtmsr   r11                     # re-disable interrupts
')
')
# a thread is allowed to spin more than maxspin cycles if its intpri 
# is not INTBASE.
# This means:
#	a) it's an interrupt handler
#	b) the lock is used to serialize a thread/interrupt critical section
# Note:  this should never be the case on a UP system that the caller is
#        disabled AND the lock is not free!

	lbz     r4, mstintpri(r12)      # Save the old intr priority for slock
	cal     r11, INTMAX(0)          # disabled interrupt priority
	cmpi    cr0, r4, INTBASE        # check if proc was at INTBASE
	bne	cr0, sl_grant_lock

#       Fix the pages around the current stack pointer.
#       This sequence may fault on the stack.
#       Whenever anyone faults with mststackfix non-null, resume()
#       will carefully touch the same area we are touching here.
#

	stb     r11, mstintpri(r12)     # Set priority to disabled
	st      r1, mststackfix(r12)    # fix area around the stack pointer
ifdef(`_POWER_MP',`
ifdef(`_SLICER',`
	GET_PPDA(cr0, r9)		# clear stack fix flag for MP LRU
	.machine "pwr"
	lil	r0, 1
	stb	r0, ppda_stackfix(r9)
')
')
	l       r0, STACKTOUCH(r1)
	l       r10, 0(r1)              # superstition - second load to a
	l       r0, -STACKTOUCH(r1)     # diff reg to machine isn't "too" smart

#
#	Must buy a stack frame in order to restore callers toc
#
        mflr    r5                              # Save link reg
						# address of caller in 3rd C parameter
        st      r5, stklink(r1)                 # Store return addr to caller
        stu     r1, -(8+STKMIN)(r1)             # Back up the stack pointer
	st	r2, stktoc(r1)			# save caller toc
                                                # (buy stack frame)
	l	r2, DATA(g_toc)			# get kernel toc

					# r4 has old mstpri whick slock expects
	bla     ENTRY(slock_pwr)	# branch to slock to handle lock miss
	.extern ENTRY(slock_pwr)

        l       r0, (8+STKMIN+stklink)(r1)	# Reload return address
        l       r2, stktoc(r1)                  # restore caller toc
        cal     r1, (8+STKMIN)(r1)              # Pop the stack frame
        mtlr    r0                              # Return addr to LR
	br					# return to caller

#	FCNDES(simple_lock_pwr)

#       SIMPLE_UNLOCK
#
#
#       fast path front end to simple_unlock
#       this routine checks the normal case of releasing
#       a lock that is not being waited for by anyone
#
#
#       PSEUDO CODE
#       
#void simple_unlock (simple_lock_t l)
#
#{
# register owner_t    myself = curthread;
# register int ipri;
#
#              ipri = i_disable(INTMAX);
#       
# #ifdef DEBUG
#              assert( *l == myself->t_tid )
# #endif
#
#              if (!(*l & WAITING)) {
#                     *l = SIMPLE_LOCK_AVAIL;
#                     TRCHKGT(HKWD_KERN_UNLOCK,l,*l);
#                     myself->t_lockcount--;
#                     i_enable(ipri);
#                     return;
#              }
#
#              sunlock(l);
#
#}
#
# REGISTERS
#              r3       lockaddr
#              r4       Trconflag
#              r5       curthread
#              r6       id
#              r7       lockword
#              r8       msr saved
#              r9       t_lockcount
#              r10/r11 scratch
#
#
#	S_PROLOG(simple_unlock_pwr)
	.csect	simple_unlock_pwr_overlay[RO]
DATA(simple_unlock_pwr):
	.globl	DATA(simple_unlock_pwr)

	mfmsr   r8                      # save the MSR value in r8 
	cal	r11, DISABLED_MSR(0)	# calculate a disabled MSR
	mtmsr   r11                     # disable interrupts

su_release_lock:
ifdef(`_POWER_MP',`
#	This works since interrupts are disabled
	GET_PPDA(cr0, r9)		# get PPDA in r9
	.machine "pwr"
	l	r5, ppda_curthread(r9)	# get current thread
',`
	GET_CURTHREAD(cr0,r11,r5)	# load curthread
')

ifdef(`DEBUG',`
	l       r6,t_tid(r5)            # get current thread id
')

	l       r7,0(r3)                # get the value of the lockword
	lhz     r9,t_lockcount(r5)      # get t_lockcount
	rlinm.	r11,r7,0,WAITING	# test waiting bit
	cal     r10,SIMPLE_LOCK_AVAIL(0)
	bne     su_call_sunlock		# set: branch to sunlock
	                                # not set
ifdef(`DEBUG',`
	tne     r7, r6                  # assert that this caller holds lock
')
	si      r9,r9,1                 # thread: decrement lockcount
	st      r10,0(r3)               # release the lock

	sth     r9,t_lockcount(r5)	# store modified lockcount

su_exit:
	mtmsr   r8
	br

su_call_sunlock:
#
#       Must buy a stack frame in order to restore callers toc
#
        mflr    r4                              # Save link reg
                                                # address of caller in 2nd C parameter
        st      r4, stklink(r1)                 # Store return addr to caller
        stu     r1, -(8+STKMIN)(r1)             # Back up the stack pointer
        st      r2, stktoc(r1)                  # put caller toc onto stack
                                                # (buy stack frame)
	l	r2, DATA(g_toc)			# get kernel toc
	mtmsr	r8			# allow interrupts--first thing we do
					# in sunlock_pwr() is to call i_disable(INTMAX)
	bla     ENTRY(sunlock_pwr)      # branch to sunlock to handle waiters
	.extern ENTRY(sunlock_pwr)

        l       r0, (8+STKMIN+stklink)(r1)	# Reload return address
        l       r2, stktoc(r1)                  # restore caller toc
        cal     r1, (8+STKMIN)(r1)              # Pop the stack frame
        mtlr    r0                              # Return addr to LR
        br                                      # return to caller

#	FCNDES(simple_unlock_pwr)

#	SIMPLE_LOCK_TRY
# 
#	this function tries to acquire a simple lock
#
#
#boolean_t simple_lock_try(simple_lock_t l)
#
#{
#
# register thread_t    myself = curthread;
# register int lockword;
# register int id;
#
#       disable_ints();
#       id = myself->t_tid;
#       lockword = *l;
#       if (!(lockword & LOCKBIT)){
#              lockword |= (id | LOCKBIT);
#              TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,
#			l,*((simple_lock *)l),LOCK_SWRITE_TRACE);
#              myself->t_lockcount++;
#              enable_ints();
#              return(TRUE);
#       }
#       TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_MISS,
#		l,*((simple_lock *)l),LOCK_SWRITE_TRACE);
#       return(FALSE);
#}
#
# REGISTERS
#		r3	lockaddr
#		r4	Trconflag
#		r5	curthread
#		r6	id
#		r7	lockword
#		r8	scratch
#		r9	t_lockcount
#		r10	msr saved
#		r11	scratch
#		r12	mst pointer
#
#	S_PROLOG(simple_lock_try_pwr)
	.csect	simple_lock_try_pwr_overlay[RO]
DATA(simple_lock_try_pwr):
	.globl	DATA(simple_lock_try_pwr)

	mfmsr   r10                     # save the MSR value in r10 
	cal	r11, DISABLED_MSR(0)	# calculate a disabled MSR
	mtmsr   r11                     # disable interrupts

ifdef(`_POWER_MP',`
',`
ifdef(`DEBUG',`
#	No caller of simple_lock_try() should be an interrupt handler.
#	Serialization with interrupt handlers should call disable_lock().
#	In the uni-processor environment, it is enough to disable and
#	not call simple_lock().  The call to simple_lock() is ignored
#	in disable_lock().
	GET_CSA(cr0,r11,r12)		# get current mst
	l	r11, mstprev(r12)	# check for interrupt handler
	tnei	r11, nullA		# trap if not thread
')
')
	l	r7,0(r3)		# load the value of the lockword
	GET_CURTHREAD(cr0,r11,r5)	# load curthread
	cmpi	cr0, r7, 0
	l	r6,t_tid(r5)		# get thread id
	lhz	r9,t_lockcount(r5)	# get lockcount field
ifdef(`DEBUG',`
	rlinm	r11,r7,0,OWNER_MASK	# clear all bits but owner bit
	teq	r11,r6			# assert lock is not recursive
')
	bne	slt_check_waiting
slt_take_lock:
	ai	r9,r9,1			# thread: increment lockcount
	st	r6,0(r3)		# grab the lock
	sth	r9,t_lockcount(r5)	# store modified lockcount

	# return TRUE
	li	r3,TRUE
ifdef(`DEBUG',`
	rlinm	r11,r6,0,OWNER_MASK	# clear all bits but owner bit
	teqi	r11,0			# trap if lock not taken!
')
	mtmsr	r10			# enable interrupts
	br

slt_check_waiting:
	rlinm.	r7, r7, 0, ~WAITING	# mask everything but waiting bit
	oriu	r6, r6, WAITING>16	# assume lock free, but waiters
	beq	slt_take_lock		# branch if lock free

slt_lock_fail:
	lil	r3,FALSE		# return FALSE
	mtmsr	r10			# enable interrupts
	br

#	FCNDES(simple_lock_try_pwr)

ifdef(`_POWER_MP',`
#***************************************************************************
#
# FUNCTION: acquire simple recursive spin lock (JUST RETURN FOR PWR)
#
# CALL:
#	void rsimple_lock(simple_lock_t l)
#
#	NOTE THAT THIS SUBROUTINE MUST BE USED ONLY IN THE OVERFLOW
#	HANDLER
#	ABD MUST BE PAIRED WITH A CALL TO rsimple_unlock() (which can be
#	in the C code overflow hamdler).
#
# NOTES:
#	this lock is used only for the overflow handler which needs the
#	lock to be recursively acquired if the overflow happened while
#	in a machine dependant portion of the code which holds that lock.
#	This function does not get a stack frame.  If the lock
# 	is not available it spins.
#	this function is entered disabled and xlate off.
#
#
# REGISTERS
#		r3	lockaddr
#		r4/r5	scratch 
#
#       S_PROLOG(rsimple_lock_pwr)
        .csect  rsimple_lock_pwr_overlay[RO]
DATA(rsimple_lock_pwr):
        .globl  DATA(rsimple_lock_pwr)

	br

        .csect  rsimple_lock_try_pwr_overlay[RO]
DATA(rsimple_lock_try_pwr):
        .globl  DATA(rsimple_lock_try_pwr)

	br

#	SIMPLE_UNLOCK_SPIN_RECURSIVE  (JUST RETURN FOR PWR)
#
#	if the lock was acquired recursive its recursion bit is set
#	called disbaled and xlate off.
#
#	NOTE THAT THIS SUBROUTINE MUST BE USED ONLY IN THE OVERFLOW
#	HANDLER
#
# REGISTERS
#		r3	lockaddr
#		r4/r5	scratch 
#
#       S_PROLOG(rsimple_unlock_pwr)
        .csect  rsimple_unlock_pwr_overlay[RO]
DATA(rsimple_unlock_pwr):
        .globl  DATA(rsimple_unlock_pwr)

	br

#       FCNDES(rsimple_unlock_pwr)

')

include(pri.m4)
include(mstsave.m4)
include(low_dsect.m4)
include(lock_def.m4)
include(machine.m4)
include(proc.m4)
include(param.m4)
include(i_machine.m4)
include(trchkid.m4)

