# @(#)66        1.30  src/bos/kernel/ml/POWER/disable_lock.m4, sysml, bos41J, 9511A_all 3/7/95 13:12:57
#
#*****************************************************************************
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS:	i_disable
#		disable_lock
#		i_enable
#		unlock_enable
#
#   ORIGINS: 27 83
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
#   LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

#*****************************************************************************
# NOTES:
#	These procedures are pinned and only fault if called on a
#	pageable stack. Touches the stack while @ INTMAX and disabled
#	to ensure the pages are pinned.
#
#	No vm critical section
#	No fixed stack
#	No backtrack
#	Can page fault on callers stack
#
#   RS2 info:
#   NOTE: After executing a "mtspr   ILCR,GPRx" that changes the current
#	  interrupt level, a "mtmsr" setting MSR(EE) to b'1' will enable
#	  external interrupts after the new interrupt level is effective
#	  under either of the following conditions:
#
#	  a) five fixed point instructions of any type are placed between the
#	     mtspr and the mtmsr
#
#	  b) two fixed point multiplies are placed between the mtspr and
#	     the mtmsr
#
#	  The requirement is that two cycles separate the "mtspr" from the
#	  "mtmsr".  The fixed point multiplies each take two cycles to complete
#	  and are restricted to executing in the B unit.  One may overlap the
#	  execution of the "mtspr" resulting in a three cycle separation or the
#	  first may follow the "mtspr" causing a four cycle separation.
#
#	  If there is useful work to be done, there are other alternatives
#	  such as:
#
#		1) execute the "mtmsr	ILCR,GPRx"
#		2) execute a simple fixed-point op (not multiply)
#		3) execute a fixed-point multiply
#		4) execute the "mtmsr" setting MSR(EE)=b'1'
#
#	  This sequence ensures a minimum of two cycles and a maximum of three
#	  cycles between the critical instructions.
#
#-----------------------------------------------------------------------#

##page;
#-----------------------------------------------------------------------#
# /*
#  *  i_disable -- disable interrupts to selected priority.
#  *
#  *  disable_lock -- disable interrupts to selected priority and lock
#  *		      the selected simple_lock.
#  *
#  *  NOTE: i_disable/disable_lock are overlays, therefore the external
#  *	    labels will be different.
#  *
#  *	On entry:
#  *	   r2 = user TOC as a result of feature 146891
#  *	   r3 = New interrupt prio level (parameter 1)
#  *	   r4 = simple lock addr
#  *	Returns:
#  *	   r3 = Old interrupt priority level
#  *
#  *	exit:	selected interrupts disabled
#  *		current kernel stack region logically pinned
#  */
# int disable_lock(new,lockaddr)
# int new;				/* desired interrupt priority */
# simple_lock_t lockaddr;		/* selected lock */
# {
#	register int old;		/* previous interrupt priority */
#	register char *fp;		/* current stack pointer */
#	volatile char touch;		/* used to touch stack pages */
#	register int  *p;		/* I/O space pointer to EIS */
#
# i_disable_ep:			/* entry point for i_disable() */
#	lockaddr = NULL;
#
# disable_lock_ep:		/* entry point for disable_lock() */
#	old = csa->intpri;	
#	if (old > new)	{	
#		MSR &= ~EE;		
#		if (old == INTBASE)  {	
#			csa->intpri = INTMAX;
#			csa->stackfix = fp;	/* mark stack "fixed" */
# ifdef _POWER_MP
#			ppdp->stackfix = 1;
# endif
#			touch = fp[STACKTOUCH];
#			touch = fp[0];
#			touch = fp[-STACKTOUCH];
#		}
#		csa->intpri = new;
#		if (new != INTMAX) {
# ifdef _POWER_RS1
#			ldsr(15,0x80000000);
#			p = (int *) 0xF0000000;
#			p->EIM0 = i_data[new];
#			p->EIM1 = 0;
# endif
# ifdef _POWER_RS2
#			CIL = i_data[new]
# endif
#			MSR |= EE;
#		}
#	}
#	if (lockaddr)
#		simple_lock(lockaddr);
#	return(old);
# }
#-----------------------------------------------------------------------#
ifdef(`_POWER_RS1',`
	.csect	disable_lock_rs1_ovl[PR] 
DATA(disable_lock_rs1):
	.globl	DATA(disable_lock_rs1)
')
ifdef(`_POWER_RS2',`
	.csect	disable_lock_rs2_ovl[PR] 
DATA(disable_lock_rs2):
	.globl	DATA(disable_lock_rs2)
')
ifdef(`_POWER_PC',`
ifdef(`_INSTRUMENTATION',`
	.csect	disable_lock_instr_ppc_ovl[PR] 
DATA(disable_lock_instr_ppc):
	.globl	DATA(disable_lock_instr_ppc)
',`
ifdef(`_POWER_MP',`
	.csect	disable_lock_sof_ovl[PR] 
DATA(disable_lock_sof):
	.globl	DATA(disable_lock_sof)
')
')
')

ifdef(`_POWER_PC', `
	.machine "ppc"

	.extern ENTRY(i_dosoft)
')

##page;

	.using	low, r0
#
# NOTE:  No other instructions should be placed before i_disable_ep
#        or between i_disable_ep and disable_lock_ep.
#        This 'hardcodes' the disable_lock entry point to be 4 bytes
#        from the i_disable entry point.
#	 This must exist whether or not we are compiled with _POWER_MP!
#
i_disable_ep:
	li	r4, 0			# i_disable()--lockaddr == NULL
					# This is tested before locking

disable_lock_ep:
	GET_CSA(cr0, r6, r5)		# r5 -> current save area
	cmpi	cr6,r3,INTMAX		# check if maximum disable req
	lbz	r6, mstintpri(r5)	# r6 = old int priority

ifdef(`INTRDEBUG',`
#
#	mtrace whacks r7, r8, r9, r10, cr0
#	       stores r3, r4, r5, r6
#
	mr	r11, r3			# Save target priority lr
	mflr	r0
#
#	Must save toc in r12 and get kernel toc to r2.  mltrace
#	must not use r12.
#
	mr	r12,r2			# save caller toc
	l	r2,DATA(g_toc)		# get kernel toc
	lil	r3, 0x4953		# IDIS for i_disable
        oriu	r3, r3, 0x4944
	bla	ENTRY(mltrace)
	.extern	ENTRY(mltrace)

	mr	r3, r11			# move it back
	mr	r2,r12			# restore caller toc
	mtlr	r0
')

	cmp	cr7, r6, r3
	cmpi	cr1, r6, INTBASE

	cmpi	cr0,r4,0		# check for NULL lockaddr
	li	r9,1			# used for ppda_stackfix
	ble+	cr7,i_dis002		# branch if already sufficiently disabled

	bne-	cr1,i_dis001		# branch if previously disabled

ifdef(`_POWER_MP',`
	GET_PPDA(cr0, r8)		
	stb	r9, ppda_stackfix(r8)	# set stack fix flag for MP LRU
')
#
#	Fix the pages around the current stack pointer.
#	This sequence may fault on the stack.
#	Whenever anyone faults with mststackfix non-null, resume()
#	will carefully touch the same area we are touching here.
#
	st	r1, mststackfix(r5)	# indicate stack fix in effect
	l	r0, STACKTOUCH(r1)
	l	r10, 0(r1)
	l	r0, -STACKTOUCH(r1)
#
#	Set new interrupt priority.
#	If new priority = INTMAX then return with interrupts disabled via MSR.
#	Else load proper value into EIS and re-enable interrupts via MSR.
#
i_dis001:
	stb	r3,mstintpri(r5)	# set new interrupt priority
	bne-	cr6,i_dis001a		# branch if not INTMAX

	mfmsr	r7			# r7 = machine state register	
	rlinm	r0, r7, 0, ~MSR_EE	# turn off External interrupt Enable
	mtmsr	r0			# disable interrupts
	b	i_dis002		# New level completely disabled

i_dis001a:
ifdef(`_POWER_RS1',`
#
# RS1 uniqueness - modify EIM0/1
#
	rlinm	r3, r3, 3, 0x7F8	# Mult by 8 to get mask index
	l	r10,DATA(g_data)	# Load addr of interrupt mask table
	mfsr	r9, sr15		# Save segment reg 15
	a	r3, r3, r10		# get address of mask entry
	cau	r8, 0, BUID0		# Set segment register 15
	mtsr	sr15, r8		#   to allow I/O access
	lsi	r11, r3, 8		# load eim0 and eim1
	cau	r3, 0, 0xF000		# calculate EIM address
	stsi	r11, r3, 8		# write new EIMs
	mtsr	sr15, r9		# Restore segment reg 15
')
ifdef(`_POWER_RS2',`
#
# RS2 uniqueness - modify CIL
#
	l	r10,DATA(g_data)	# Load addr of interrupt mask table
	lbzx	r11, r10, r3		# Get level to disable to
	rlinm	r11, r11, 8, 0xFF00	# Move it over 8 bits
#
#  The ICO_UPDCIL command is a binary 0 so there is no real need to
#  include the instruction to insert it since bits 0-7 of r11 are already
#  0.
	mtspr	ILCR, r11		# Set CIL
	muli	r3, r3, 1		# Delay
	muli	r3, r3, 1		# Delay
')

#	If monoprocessor return to caller, passing back the "old" value 
#	of interrupt priority (unless _POWER_MP is defined).
#	If new prio is INTMAX, MSR is still disabled; else MSR is enabled.

i_dis002:
ifdef(`_POWER_MP',`

#       Check to see if function called is i_disable()
	mr	r3, r6			# set return value
	beqlr+				# return from i_disable()
	
#	If multiprocessor or _POWER_MP kernel invoke simple_lock
#
#	register values from "i_disable" portion of code
#	r3 - scratch
#	r4 - lockword address
#	r5 - csa
#	r6 - old priority
#
ifdef(`_POWER_PC',`
ifdef(`_INSTRUMENTATION',`
	lbz	r3, DATA(Trconflag)	# load first byte of trace flags array
	lwz	r8,0(r4)		# load pointer to statistics structure
	cmpi	cr6, r3, 0		# check "trace active" indicator
')

ifdef(`DEBUG',`
	lil	r3,-1			# debug only spin count
')

ifdef(`_INSTRUMENTATION',`
	rlinm.	r7,r8,0,INSTR_ON	# test if instrumentation enabled
	crorc	cr7*4+eq, cr7*4+eq, cr7*4+eq # set eq in cr7 for no misses
	bne+	sl_grant_lock_instr	# branch if lock instrumented
')

sl_grant_lock:

	lwarx	r7,0,r4			# load the value of the lockword
	GET_CURTHREAD(cr0,r9,r5)	# get curthread
	cmpi	cr0,r7,0
	lwz	r10,t_tid(r5)		# load thread id
ifdef(`DEBUG',`
	subic	r3,r3,1			# decrement spin count
	rlinm	r11,r7,0,OWNER_MASK	# clear all bits but owner bit
	tweqi	r3, 0			# been here too long--die!
	tweq	r11,r10			# assert lock is not recursive
')
	lhz	r9,t_lockcount(r5)	# load lockcount field
ifdef(`_INSTRUMENTATION',`
	bne-	sl_miss			# trace miss
',`
	bne-	sl_grant_lock		# retry until successful
')
	stwcx.	r10,0,r4		# grab the lock
	addi	r9,r9,1			# thread: increment lockcount
	bne-	sl_grant_lock		# retry until successful
	isync

	sth	r9,t_lockcount(r5)	# store modified lockcount
ifdef(`_INSTRUMENTATION',`
	bne-	cr6,sl_trace		# branch if trace is active
')
	mr	r3, r6			# return previous priority
	br

ifdef(`_INSTRUMENTATION',`
sl_miss:
	bne+	cr7, sl_grant_lock	# branch if miss already recorded
	crxor	cr7*4+eq, cr7*4+eq, cr7*4+eq # clear eq in cr7 to record miss
	beq+	cr6, sl_grant_lock	# branch if trace disabled

	mfcr	r11
	li	r8, 0			# 6th parm is lockname
	mflr	r7			# 5th parm is return addr
	stw	r6, -4(r1)		# save return value
	stw	r4, -8(r1)		# save lockaddr
	stw	r11, -12(r1)		# save CR
	stw	r2, -16(r1)		# save user toc on stack
	l	r2,DATA(g_toc)		# get kernel toc to r2
	stw	r7, stklink(r1)		# save link register
	li	r6, LOCK_SWRITE_TRACE	# 4th parm is trace type
	lwz	r5, 0(r4)		# 3rd parm is lockword value
					# 2nd parm is lockword addr
					# 1st parm is hook type
	li	r3, (hkwd_LOCK_DISABLED | hkwd_LOCK_MISS)
	addis	r3, r3, (HKWD_KERN_LOCK | HKTY_GT)>16 

#	significant registers saved across call to trchook
#
#	r6 = original return value
#	r3 = original lockaddr

	bla	 ENTRY(trchook)		# call trace routine
	.extern	 ENTRY(trchook)

	lwz	r5, stklink(r1)		# restore link register
	lwz	r2, -16(r1)		# restore user toc
	lwz	r11, -12(r1)		# restore CR
	lwz	r4, -8(r1)		# restore lockaddr
	lwz	r6, -4(r1)		# restore return value
	mtlr	r5
	mtcr	r11
	b	sl_grant_lock

sl_trace:
	li	r8, 0			# 6th parm is lockname
	mflr	r7			# 5th parm is return addr
	stw	r7, stklink(r1)		# save link register
	stw	r2, -8(r1)		# save user toc on stack
	l	r2,DATA(g_toc)		# get kernel toc to r2
	stw	r6, -4(r1)		# save return value
	li	r6, LOCK_SWRITE_TRACE	# 4th parm is trace type
	lwz	r5, 0(r4)		# 3rd parm is lockword value
					# 2nd parm is lockword addr
					# 1st parm is hook type
	li	r3, (hkwd_LOCK_DISABLED | hkwd_LOCK_TAKEN)
	addis	r3, r3, (HKWD_KERN_LOCK | HKTY_GT)>16 

#	significant registers saved across call to trchook
#
#	r6 = original return value
	bla	 ENTRY(trchook)		# call trace routine
	.extern	 ENTRY(trchook)

	lwz	r5, stklink(r1)		# restore link register
	lwz	r2, -8(r1)		# restore user toc
	lwz	r6, -4(r1)		# restore return value
	mtlr	r5
	mr	r3, r6			# return previous priority
	br

sl_grant_lock_instr:

	lwarx	r7,0,r8			# load the value of the lockword
	GET_CURTHREAD(cr0,r9,r5)	# get curthread
	cmpi	cr0,r7,0
	lwz	r10,t_tid(r5)		# load thread id
ifdef(`DEBUG',`
	subic	r3,r3,1			# decrement spin count
	rlinm	r11,r7,0,OWNER_MASK	# clear all bits but owner bit
	tweqi	r3, 0			# been here too long--die!
	tweq	r11,r10			# assert lock is not recursive
')
	lhz	r9,t_lockcount(r5)	# load lockcount field
	bne-	cr0, sl_spinning_instr

	stwcx.	r10,0,r8		# grab the lock
	addi	r9,r9,1			# thread: increment lockcount
	bne-	sl_grant_lock_instr
	isync

ifdef(`DEBUG',`
	mflr	r11
	stw	r11,lock_lr(r8)
	stw	r10,lock_caller(r8)
	mfspr	r11,PID
	stw	r11,lock_cpuid(r8)
')
	lwz	r10,acquisitions(r8)	# load acquisitions field 
	lwz	r11,misses(r8)		# load misses field
	addic.	r10,r10,1		# increment acquisitions
	stw	r10,acquisitions(r8)
	beq	cr7,sl_incr_lockcount_instr	# if has spun
	addi	r11,r11,1		# increment misses
	stw	r11,misses(r8)

sl_incr_lockcount_instr:
	sth	r9,t_lockcount(r5)	# store modified lockcount
	bne	cr6,sl_trace_instr	# branch if trace is active
	mr	r3, r6			# return previous priority
	br

sl_spinning_instr:
	bne+	cr7, sl_grant_lock_instr     # branch if miss already recorded
	crxor	cr7*4+eq, cr7*4+eq, cr7*4+eq # clear eq in cr7 to record miss
	beq+	cr6, sl_grant_lock_instr     # branch if trace disabled

	mfcr	r11
	mflr	r7			# 5th parm is return addr
	stw	r6, -4(r1)		# save return value
	stw	r8, -8(r1)		# save instr lockaddr
	stw	r11, -12(r1)		# save CR
	stw	r4, -16(r1)		# save real lockaddr
	stw	r2, -20(r1)		# save user toc on stack
	l	r2,DATA(g_toc)		# get kernel toc to r2
	stw	r7, stklink(r1)		# save link register
	li	r6, LOCK_SWRITE_TRACE	# 4th parm is trace type
	lwz	r5, 0(r8)		# 3rd parm is lockword value
	lwz	r8,lockname(r8)		# 6th parm is lockname
					# 2nd parm is lockword addr
					# 1st parm is hook type
	li	r3, (hkwd_LOCK_DISABLED | hkwd_LOCK_MISS)
	addis	r3, r3, (HKWD_KERN_LOCK | HKTY_GT)>16 

#	significant registers saved across call to trchook
#
#	r6 = original return value
#	r8 = original lockaddr

	bla	 ENTRY(trchook)		# call trace routine
	.extern	 ENTRY(trchook)

	lwz	r5, stklink(r1)		# restore link register
	lwz	r4, -16(r1)		# restore real lockaddr
	lwz	r2, -20(r1)		# restore user toc
	lwz	r11, -12(r1)		# retore CR
	lwz	r8, -8(r1)		# restore instr lockaddr
	lwz	r6, -4(r1)		# restore return value
	mtlr	r5
	mtcr	r11
	b	sl_grant_lock_instr

sl_trace_instr:
	mflr	r7			# 5th parm is return addr
	stw	r7, stklink(r1)		# save link register
	stw	r6, -4(r1)		# save return value
	stw	r2, -8(r1)		# save user toc on stack
	l	r2,DATA(g_toc)		# get kernel toc to r2
	li	r6, LOCK_SWRITE_TRACE	# 4th parm is trace type
	lwz	r5, 0(r8)		# 3rd parm is lockword value
	lwz	r8,lockname(r8)		# 6th parm is lockname
					# 2nd parm is lockword addr
					# 1st parm is hook type
	li	r3, (hkwd_LOCK_DISABLED | hkwd_LOCK_TAKEN)
	addis	r3, r3, (HKWD_KERN_LOCK | HKTY_GT)>16 

#	significant registers saved across call to trchook
#
#	r6 = original return value

	bla	 ENTRY(trchook)		# call trace routine
	.extern	 ENTRY(trchook)

	lwz	r5, stklink(r1)		# restore link register
	lwz	r6, -4(r1)		# restore return value
	lwz	r2, -8(r1)		# restore user toc
	mtlr	r5
	mr	r3, r6			# return previous priority
	br
')					# _INSTRUMENTATION
',` # _POWER_MP && !_POWER_PC
#
#	register values from "i_disable" portion of code
#	r3 - scratch
#	r4 - lockword address
#	r5 - csa
#	r6 - old priority
#
	mfmsr	r10			# save the MSR value in r10 
	cal	r11, DISABLED_MSR(0)	# calculate a disabled MSR
	mtmsr	r11			# disable interrupts

ifdef(`DEBUG',`
	liu	r3,0x0800		# debug only spin count
')

sl_try_lock:
	l	r7,0(r4)		# load the value of the lockword
	GET_CURTHREAD(cr0,r11,r12)	# load curthread
	cmpi	cr0, r7, 0
	l	r8,t_tid(r12)		# get thread id
ifdef(`DEBUG',`
	si	r3,r3,1			# decrement spin count
	rlinm	r11,r7,0,OWNER_MASK	# clear all bits but owner bit
	teqi	r3, 0			# been here too long--die!
	teq	r11,r8			# assert lock is not recursive
')
	lhz	r9,t_lockcount(r12)	# get lockcount field
	bne	sl_try_lock

#	lock is free
	ai	r9,r9,1			# thread: increment lockcount
	st	r8,0(r4)		# grab the lock
	sth	r9,t_lockcount(r12)

	mr	r3, r6			# return previous priority
	mtmsr	r10			# enable interrupts
	br
')
',` # !_POWER_MP
	mr	r3,r6
	br
')

##page;
#-----------------------------------------------------------------------#
# /*
#  *  i_enable -- enables interrupts to selected priority.
#  *
#  *  unlock_enable -- unlock the selected simple_lock and
#  *		       enables interrupts to selected priority.
#  *
#  *  NOTE: i_enable/unlock_enable are overlays, therefore the external
#  *	    labels will be different.
#  *
#  *	On entry:
#  *	   r2 = user TOC pointer
#  *	   r3 = Old interrupt prio level (parameter 1)
#  *	   r4 = lockaddr
#  *	Returns:
#  *	   none
#  *
#  *	exit:
#  *		selected interrupts enabled based on priority
#  *		stack unpinned if level is restored to INTBASE (ie. if
#  *		at process level)
#  */
#
# void unlock_enable(old,lockaddr)
# {
#
# i_enable_ep:			/* entry point for i_enable() */
#	goto su_enable;
#
# unlock_enable_ep:		/* entry point for unlock_enable() */
#	simple_unlock(lockaddr);
#
# su_enable:
#	current = csa->intpri;	
#	if (old > current)	
#	{
#		MSR &= ~EE;		  /* Disable interrupts via MSR	      */
#		csa->intpri = old;	  /* Set previous value of mstintpri  */
#		if (old == INTBASE)
#		{
# ifdef _POWER_MP
#			ppdp->stackfix = 0;
# endif
#			mststackfix = NULL;	
#		}
#		if ((old == INTBASE) &&	     /* Not disabled            */
#		    (csa->prev == NULL)) &&  /* Resuming a process      */
#		    (runrun)                 /* runrun set              */
#		{
# ifdef _POWER_PC
#			if (clz32(ppdp->i_softpri) < csa->intpri)
#				i_dosoft();
# endif
#			if (runrun)
#				swtch();	  /* Run the dispatcher */
#		} else {
# ifdef _POWER_PC
#			if (clz32(ppdp->i_softpri) < csa->intpri)
#				i_dosoft();
# endif
# 		}		
# ifdef _POWER_RS1
#		ldsr(15,0x80000000);	  /* Update EIM and MSR		      */
#		p = (int *) 0xF0000000;
#		p->EIM0 = i_data[olc];
#		p->EIM1 = 0;
# endif
# ifdef _POWER_RS2
#		CIL = i_data[new]
# endif
#		MSR |= EE;
#	}	
# }
#-----------------------------------------------------------------------#

ifdef(`_POWER_RS1',`
	.csect	unlock_enable_rs1_ovl[PR] 
DATA(unlock_enable_rs1):
	.globl	DATA(unlock_enable_rs1)
')
ifdef(`_POWER_RS2',`
	.csect	unlock_enable_rs2_ovl[PR] 
DATA(unlock_enable_rs2):
	.globl	DATA(unlock_enable_rs2)
')
ifdef(`_POWER_PC',`
ifdef(`_INSTRUMENTATION',`
	.csect	unlock_enable_instr_ppc_ovl[PR] 
DATA(unlock_enable_instr_ppc):
	.globl	DATA(unlock_enable_instr_ppc)
',`
ifdef(`_POWER_MP',`
	.csect	unlock_enable_sof_ovl[PR] 
DATA(unlock_enable_sof):
	.globl	DATA(unlock_enable_sof)
')
')
')

       .using	low, r0
#
# NOTE:  No other instructions should be placed before i_enable_ep
#        or between i_enable_ep and unlock_enable_ep.
#        This 'hardcodes' the unlock_enable entry point to be 4 bytes
#        from the i_enable entry point.
#        This must exist whether or not we are compiled with _POWER_MP!
i_enable_ep:
ifdef(`_POWER_MP',`
	b	su_enable		# i_enable()--skip unlock code
',`
	nop				# fall into su_enable
')

unlock_enable_ep:
ifdef(`_POWER_MP',`
ifdef(`_POWER_PC',`
	sync				# wait for writes completed in memory

ifdef(`_INSTRUMENTATION',`
	lwz	r8,0(r4)		# load pointer to statistics structure
	lbz	r10, DATA(Trconflag)	# load first byte of trace flags array
	cmpi	cr6, r10, 0		# check "trace active" indicator
')

	GET_CURTHREAD(cr0,r9,r10)

ifdef(`DEBUG',`
	lwz	r6,t_tid(r10)		# get current thread id
')
	lhz	r9,t_lockcount(r10)	# get t_lockcount
	li	r12,SIMPLE_LOCK_AVAIL
	subic	r9,r9,1			# thread: decrement lockcount


ifdef(`_INSTRUMENTATION',`
	rlinm.	r7,r8,0,INSTR_ON	# test if instrumentation enabled
	bne+	su_instr		# branch if lock instrumented
')


ifdef(`DEBUG',`
su_retry:
	lwarx	r7,0,r4			# get the value of the lockword
	twne	r7, r6			# assert that this thread/IH holds lock
	stwcx.	r12,0,r4		# release the lock
	bne-	su_retry
',`
	stw	r12,0(r4)		# release the lock
')
	sth	r9,t_lockcount(r10)	# store modified lockcount

su_check_trace:
ifdef(`_INSTRUMENTATION',`
	beq+	cr6, su_enable		# branch if trace is not active

su_trace:
	mflr	r6			# 4th parm is return address
	stw	r6, stklink(r1)		# save link register
	stw	r3, -4(r1)		# save ipri
	stw	r2, -8(r1)		# save user toc
	lwz	r2, DATA(g_toc)		# get kernel toc
	lwz	r10, 0(r4)		# 3rd parm is lockword value
					# 2nd parm is lockword addr
					# 1st parm is hook type
	lil	r3, hkwd_LOCK_DISABLED
	addis	r3, r3, (HKWD_KERN_UNLOCK | HKTY_GT)>16 

#	significant registers saved across call to trchook
#
#	r3 = ipri
#	r6 = link register

	bla	ENTRY(trchook)		# call trace routine
	.extern	ENTRY(trchook)

	lwz	r3, -4(r1)
	lwz	r2, -8(r1)		# restore user toc

	lwz	r6, stklink(r1)		# restore link register
	mtlr	r6

	b	su_enable

su_instr:

ifdef(`DEBUG',`
	mflr	r11
	stw	r11,unlock_lr(r8)
	stw	r6,unlock_caller(r8)
	mfspr	r11,PID
	stw	r11,unlock_cpuid(r8)

su_retry_instr:
	lwarx	r7,0,r8			# get the value of the lockword
	twne	r7, r6			# assert that this thread/IH holds lock
	stwcx.	r12,0,r8		# release the lock
	bne-	su_retry_instr
',`
	stw	r12,0(r8)		# release the lock
')
	sth	r9,t_lockcount(r10)	# store modified lockcount

su_check_trace_instr:
	beq+	cr6, su_enable		# branch if trace is not active

su_trace_instr:
	mflr	r6			# 4th parm is return address
	stw	r6, stklink(r1)		# save link register
	stw	r3, -4(r1)		# save ipri
	stw	r2, -8(r1)		# save user toc
	lwz	r2, DATA(g_toc)		# get kernel toc

	lwz	r7,lockname(r8)		# 5th parm is lockname
	lwz	r10, 0(r8)		# 3rd parm is lockword value
					# 2nd parm is lockword addr
					# 1st parm is hook type
	lil	r3, hkwd_LOCK_DISABLED
	addis	r3, r3, (HKWD_KERN_UNLOCK | HKTY_GT)>16 

#	significant registers saved across call to trchook
#
#	r3 = ipri
#	r6 = link register

	bla	ENTRY(trchook)		# call trace routine
	.extern	ENTRY(trchook)

	lwz	r3, -4(r1)
	lwz	r2, -8(r1)		# restore user toc

	lwz	r6, stklink(r1)		# restore link register
	mtlr	r6

	# fall into su_enable
')					# _INSTRUMENTATION
',` # _POWER_MP && !_POWER_PC

	mfmsr	r8			# save the MSR value in r8 
	cal	r11, DISABLED_MSR(0)	# calculate a disabled MSR
	mtmsr	r11			# disable interrupts

su_release_lock:
	GET_PPDA(cr0,r11)		# get ppda
	l	r10, ppda_curthread(r11) # load curthread

ifdef(`DEBUG',`	
	l	r6,t_tid(r10)		# get current thread id
')

	lhz	r9,t_lockcount(r10)	# get t_lockcount
	cal	r12,SIMPLE_LOCK_AVAIL(0)
	si	r9,r9,1			# thread: decrement lockcount

ifdef(`DEBUG',`
	l	r7,0(r4)		# get the value of the lockword
	tne	r7, r6			# assert that this caller holds lock
')
	st	r12,0(r4)		# release the lock
	sth	r9,t_lockcount(r10)	# store modified lockcount

su_check_trace:
	mtmsr	r8			# restore MSR
')
')
su_enable:
	GET_CSA(cr0, r6, r5)		# r5 -> current save area

	.using	mstsave, r5
	lbz	r6, mstintpri		# r6 = current interrupt priority

ifdef(`INTRDEBUG',`
#
#	Must save toc in r12 and get kernel toc to r2.  mltrace
#	must not use r12.
#
	mr	r12,r2			# save caller toc
	l	r2,DATA(g_toc)		# get kernel toc
	mr	r4, r3			# Save target priority & lr
	mflr	r0
	lil	r3, 0x4e41		# IENA for i_enable
	oriu	r3, r3, 0x4945

	bla	ENTRY(mltrace)

	mr	r3, r4			# move it back
	mr	r2,r12			# restore user toc
	mtlr	r0
')
	l	r8, mstprev		# r8 = previous save area ptr, or null
	l	r9, DATA(runrun)	# Dispatchers "runrun" flag
	cmp	cr0, r3, r6		# If current priority more favored than
	bler	cr0			#   old, do not change current
	cmpi	cr0, r3, INTBASE	# Check for input pri = INTBASE
	cmpi	cr1, r8, nullA		# Check for back pointer = null
	cmpi	cr6, r9, 0		# Check for runrun flag set

	mfmsr	r7			# r7 = machine state register	
	rlinm	r0, r7, 0, ~MSR_EE	# Disable interrupts altogether
	mtmsr	r0
	stb	r3, mstintpri		# Restore old interrupt priority

	crand	cr1*4+eq, cr1*4+eq, cr0*4+eq	# Set up condition register bit
	crandc	cr1*4+eq, cr1*4+eq, cr6*4+eq	#  for "call swtch" condition

#	If returning to INTBASE, then mark as no longer running on
#	fixed stack.  This effectively "unpins" the stack.

	bne	cr0, en_10		# Branch around if not INTBASE
	cal	r0, 0(0)		# Store zero in fixed stack
	st	r0, mststackfix		#   pointer
ifdef(`_POWER_MP',`
	GET_PPDA(cr0, r9)		# clear stack fix flag
	stb	r0, ppda_stackfix(r9)
')
en_10:

#	If returning to INTBASE (enabling), resuming a process, and runrun <> 0,
#	call swtch() which will run the dispatcher. 

	bne	cr1, en_15		# Branch if not to run dispatcher
ifdef(`_POWER_PC',`
ifdef(`_POWER_MP',`
	lhz	r7,ppda_softpri(r9)	# r7 - interrupt queue
',`
	lhz	r7,softpri		# r7 - interrupt queue
')
	rlwinm	r7,r7,16,0xffff0000	# r7 - move i_softpri to upper bits
	cntlzw	r0,r7			# r0 - find most favored priority
	cmp	cr0,r3,r0		# cr0 - can process any interrupts?
	mflr	r0			# r0 - get link register
	ble+	en_13			# if no soft ints to process

	stw	r0,stklink(r1)		# save link register
	bla	ENTRY(i_dosoft)		# process soft ints
	lwz	r8,DATA(runrun)		# r8 - re-fetch runrun
	lwz	r0,stklink(r1)		# r0 - retrieve link register
	mtlr	r0			# restore link register
en_13:
	cmpi	cr0,r8,0		# is runrun set?
	beq+	en_25			# return if runrun not set

')
#	Buy a stack frame so that we can legally call swtch(), preserving
#	the link register.

	mflr	r0			# Save link reg
	st	r31, stkr31(r1)		# Store callers r31
	st	r0, stklink(r1)		# Store return addr to caller
	stu	r1, stkpush(r1)		# Back up the stack pointer (buy frame)
	st	r2, stktoc(r1)		# save user toc

	l	r2,DATA(g_toc)		# get kernel toc
	mr	r31, r3			# Save new prio across call
	bla	ENTRY(swtch)		# Call swtch()
	.extern	ENTRY(swtch)
	mr	r3, r31			# Get new prio again

	l	r0, stkpop+stklink(r1)	# Reload return address
	l	r2, stktoc(r1)		# restore user toc	
	cal	r1, stkpop(r1)		# Pop the stack frame
	mtlr	r0			# Return addr to LR
	l	r31, stkr31(r1)		# Reload callers r31
ifdef(`_POWER_PC',`
	b	en_25
')
en_15:
ifdef(`_POWER_PC',`
#
# check i_softpri
#
ifdef(`_POWER_MP',`
	lhz	r7,ppda_softpri(r9)	# r7 - interrupt queue
',`
	lhz	r7,softpri		# r7 - interrupt queue
')
	rlwinm	r7,r7,16,0xffff0000	# r7 - move i_softpri to upper bits
	cntlzw	r0,r7			# r0 - find most favored priority
	cmp	cr0,r3,r0		# cr0 - can process any interrupts?
	mflr	r0			# r0 - get link register
	ble+	cr0, en_20		# continue if no soft ints to process

	stw	r0,stklink(r1)		# save link register
	bla	ENTRY(i_dosoft)		# process soft ints
	lwz	r0,stklink(r1)		# r0 - retrieve link register
	mtlr	r0			# restore link register
')
en_20:
ifdef(`_POWER_RS1',`
#
# RS1 uniqueness - modify EIM0/1
#

#	Set EIM to state indicated by newly-restored value of mstintpri
#
#	Must get int mask table addr to r4.
#
	l	r4,DATA(g_data)		# Load addr of interrupt mask table
	mfsr	r9, sr15		# Save segment reg 15
	cau	r8, 0, BUID0		# Set segment register 15
	rlinm	r5, r3, 3, 0x7F8	# Mult by 8 to get mask index
	a	r5, r4, r5		# Get address of mask
	mtsr	sr15, r8		#   to allow I/O access

	lsi	r11, r5, 8		# load mask
	cau	r5, 0, 0xF000		# calculate EIM address
	stsi	r11, r5, 8		# update interrupt mask
	mtsr	sr15, r9		# Restore segment reg 15
')
ifdef(`_POWER_RS2',`
#
# RS2 uniqueness - modify CIL
#
#
#	Must get int mask table addr to r4.
#
	l	r4,DATA(g_data)		# Load addr of interrupt mask table
	lbzx	r11, r4, r3		# Get level to disable to
	rlinm	r11, r11, 8, 0xFF00	# Move it over 8 bits
#
# The ICO_UPDCIL command is a binary 0 so there is no real need to
# include the instruction to insert it since bits 0-7 of r11 are already
# 0.
	mtspr	ILCR, r11		# Set CIL
	muli	r3, r3, 1		# Delay
	muli	r3, r3, 1		# Delay
')
en_25:
#	We are potentially enabling something, so set bit in MSR to
#	enable external interrupts.

	mfmsr	r7			# Enable interrupts via MSR
	oril	r7, r7, MSR_EE
	mtmsr	r7
	br

	_DF(_DF_NOFRAME)

	.toc
	TOCE(debabend,data)

include(param.m4)
include(systemcfg.m4)
include(scrs.m4)
include(pri.m4)
include(proc.m4)
include(lock_def.m4)
include(low_dsect.m4)
include(mstsave.m4)
include(intr.m4)
include(machine.m4)
include(trchkid.m4)
