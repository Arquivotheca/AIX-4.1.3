# @(#)34	1.24  src/bos/kernel/ml/POWER/flih_util.s, sysml, bos41J, 9511A_all 3/7/95 13:12:37
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS: swtch, locked_swtch
#	begin_interrupt, finish_interrupt, call_dispatch,
#	run_dispatch, call_sigslih
#
#   ORIGINS: 27, 83
#
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1989, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************
#
# NOTES:
#	Module contains architecture independent utilities used by
# flihs and other state management code
#
#****************************************************************************


	.file "flih_util.s"
	.machine "com"
	.using	low,0
	.csect	flih_util[PR]

include(macros.m4)

#****************************************************************************
#
# NAME:	call_sigslih
#
# FUNCTION:
#	sig_slih() is called if an event preventing the thread to be
#	normally resumed occurred. Usually signals (hence the name),
#	it may also be a suspension or swapping or termination request.
#	If it's a signal, it copies the user's mst to the user's stack
#	via the signal context structure.  sig_slih() then changes the
#	user's mst so that sig_deliver() is called with the signal
#	context structure as a parameter. If it's a termination request,
#	it changes the user's mst so that kexit() is called. It returns
#	zero so that this routine free the mst and call resume.
#	If it's a suspension/swapping request, it calls sched_swapout()
#	which changes the state of the thread.  It returns one so that
#	this routine call the dispatcher to resume another thread.
#	sig_slih() is run with bactracking set in the mst.  This allows the
#	function restart if a page fault occurs during the context
#	copy.
#
# EXECUTION ENVIRONMENT:
#	called from check_sigslih and from the SVC backend
#
# REGISTERS ON ENTRY:
#	r2 - kernel toc
#	r14 - current csa value (INTR_MST)
#	r15 - ppda address (PPDA_ADDR)
#
#*****************************************************************************

ENTRY(call_sigslih):
	.globl	ENTRY(call_sigslih)

#	Get a new mst to run the "slih"

	l	NEW_MST, ppda_mstack(PPDA_ADDR)	# New mst
	cal	r0, -framesize(NEW_MST)		# get next available mst
	st	r0, ppda_mstack(PPDA_ADDR)	# update next mst pointer
        cal     r0, INTMAX*256(0)               # INTMAX
	st	NEW_MST, ppda_csa(PPDA_ADDR)	# store	new csa	value
	sth	r0,mstintpri(NEW_MST)		# Set priority + backtrack
	CSA_UPDATED(cr0, r3, NEW_MST)

	cal	r0, 0(0)
	liu	r3, NEW_ADSPACE			# new seg reg allocation mask
	st	r0, mstkjmpbuf(NEW_MST)		# Clear	jump buffer pointer
	st	r0, excbranch(NEW_MST)		# Clear	jump buffer pointer
	st	r3, mstsralloc(NEW_MST)		# set allocation vector

#	run the "slih"

	cal	r1, -stkmin(NEW_MST)		# Point	to a real stack	frame
	bl	ENTRY(sig_slih)			# Call the signal "slih"
       .extern	ENTRY(sig_slih)

#	check whether this thread is still resumable

	cmpi	cr0, r3, 1			# dispatcher needed ? cr0(eq)
ifdef(`_POWER_MP',`
	lil	r4, 0				# proc_int_lock is held
')
	beq	cr0, run_dispatch		# yes, find another thread

	st	NEW_MST, ppda_mstack(PPDA_ADDR)	# Free mst
	st	INTR_MST, ppda_csa(PPDA_ADDR)	# update csa
	CSA_UPDATED(cr0, r3, INTR_MST)

	b	ENTRY(resume)			# no, resume the current thread

#*******************************************************************************
#
# NAME:	swtch
#
# FUNCTION:
#	The "swtch" routine saves the state of its caller (which
#	must be the currently-executing thread) in the same manner
#	as the FLIHs do.  It then invokes dispatch() to select
#	another thread to run.  There will always be such a
#	thread, for the "waitproc" threads are always ready.
#	Eventually the thread which called swtch() will be
#	redispatched, at the point after its call to swtch().
#
#	This interface is handled as much like an ordinary first-
#	level interrupt handler as possible, but there are some
#	differences.  For example, not all the state of the caller
#	need be preserved across the (ordinary) call to swtch(), and
#	so it is permissible to bash r0 in the course of making sure
#	that interrupts are disabled.
#
#	The bulk of the thread state is saved by the normal
#	state_save() subroutine.  The saved state
#	reflects the caller's return address as the value in the
#	IAR, so that when the thread is resumed it will be at the
#	point following the call to swtch().  swtch() must turn off
#	translation, since it loads values into SRR0 and SRR1.
#
# EXECUTION ENVIRONMENT:
#	Called from the process environment only
#
# PSEUDO CODE:
#
#	swtch()
#	{
#	       struct ppda *ppdp;
#	       int oldmsr;
#
#	       /* save caller's	MSR, then turn off xlate and interrupts
#		*/
#	       oldmsr =	mfmsr();
#	       mtmsr(DISABLED_REAL_MSR);
#
#	       ra_save = r15;			/* a no-op on PPC */
#	       if (__power_pc())
#		       mtspr(SPRG1, r15);
#
#	       /* load r15 with	the ppda for the current processor
#		*/
#	       ppdp = GET_PPDA();
#
#	       /* set up saved IAR and MSR to be that of the caller
#		*/
#	       mtsrr0(mflr());
#	       mtsrr1(oldmsr);
#
#	       /* do a state save as if	an interrupt had occurred
#		*/
#	       state_save();
#
#	       /* let the dispatcher choose another process
#		*/
#	       run_dispatch(DISPATCH);
#
#	}
#
#*******************************************************************************

ENTRY(swtch):	       # Real entry point name
	.globl	ENTRY(swtch)

	l	r4, syscfg_arch(0)	# get machine architecture
	mfmsr	r12			# get callers msr
	cmpi	cr0, r4, POWER_PC	# check	for PPC
	lil	r0, DISABLED_REAL_MSR	# xlate	off, interrupts	disabled
	mtmsr	r0			#   except for machine check

DATA(isync_swtch_1):
	.globl DATA(isync_swtch_1)
	isync     			# make sure mtmsr is noticed

	st	r15, ra_save(0)		# save r15 for state save
	bne	cr0, no_sprs		# branch if not	PPC machine
	.machine "ppc"
	mtspr	SPRG1, r15		# put r15 in SPR for PCP
	.machine "com"
no_sprs:
	GET_PPDA(cr0, PPDA_ADDR)	# load PPDA address

#	Arrange	for state_save to save MSR contents which reflect supervisor
#	state; interrupt enable/disable	will come from mstintpri on re-dispatch.
#
#	Save link register in reg 0; put link register contents	into SRR0,
#	which state_save will save as IAR contents; call state_save.

	mfspr	r0, LR			# state_save stores r0 into mstlr
	mtspr	SRR1, r12
	mtspr	SRR0, r0		# Claim	that return address is IAR value

	bl	ENTRY(state_save)	# Save all regs	except floating	point
	.extern	ENTRY(state_save)

#	On return from state_save, the following registers are set up:
#	    r1	   Address of stack (for C)
#	    r2	   Address of kernel TOC
#	    r13	   Address of current  mst save	area (new value	of csa)
#	    r14	   Address of previous mst save	area (old value	of csa)
#	    r15	   Address of ppda (PPDA_ADDR)
#	    sr0	   R/W access to kernel	segment
#	    sr2	   R/W access to process segment
#	    sr14   R/W access to kernel	extension segment

#	Call the dispatcher

	crorc	cr0*4+eq, cr0*4+eq, cr0*4+eq	# set eq bit to	run dispatcher
ifdef(`_POWER_MP',`
	lil	r4, 1			# proc_int_lock is not held
')
	b	run_dispatch		# Call the dispatcher, and resume

ifdef(`_POWER_MP',`
ENTRY(locked_swtch):
	.globl	ENTRY(locked_swtch)

	l	r4, syscfg_arch(0)	# get machine architecture
	mfmsr	r12			# get callers msr
	cmpi	cr0, r4, POWER_PC	# check	for PPC	
	lil	r0, DISABLED_REAL_MSR	# xlate	off, interrupts	disabled
	mtmsr	r0			#   except for machine check

DATA(isync_swtch_2):
	.globl DATA(isync_swtch_2)
	isync     			# make sure mtmsr is noticed

	st	r15, ra_save(0)		# save r15 for state save
	bne	cr0, no_sprs1		# branch if not	PPC machine
	.machine "ppc"
	mtspr	SPRG1, r15		# put r15 in SPR for PCP
	.machine "com"
no_sprs1:
	GET_PPDA(cr0, PPDA_ADDR)	# load PPDA address

#	Arrange	for state_save to save MSR contents which reflect supervisor
#	state; interrupt enable/disable	will come from mstintpri on re-dispatch.
#
#	Save link register in reg 0; put link register contents	into SRR0,
#	which state_save will save as IAR contents; call state_save.

	mfspr	r0, LR			# state_save stores r0 into mstlr
	mtspr	SRR1, r12
	mtspr	SRR0, r0		# Claim	that return address is IAR value

	bl	ENTRY(state_save)	# Save all regs	except floating	point
	.extern	ENTRY(state_save)

#	On return from state_save, the following registers are set up:
#	    r1	   Address of stack (for C)
#	    r2	   Address of kernel TOC
#	    r13	   Address of current  mst save	area (new value	of csa)
#	    r14	   Address of previous mst save	area (old value	of csa)
#	    r15	   Address of ppda (PPDA_ADDR)
#	    sr0	   R/W access to kernel	segment
#	    sr2	   R/W access to process segment
#	    sr14   R/W access to kernel	extension segment

#	Call the dispatcher

	crorc	cr0*4+eq, cr0*4+eq, cr0*4+eq	# set eq bit to	run dispatcher
	lil	r4, 0			# proc_int_lock is held
	b	run_dispatch		# Call the dispatcher, and resume
')
	
#******************************************************************************
#
# NAME:	begin_interrupt
#
# FUNCTION:
#	This subroutine does the initial set-up for the first level
#	interrupt handlers, after the interrupted program's state
#	has been saved by the state_save() routine.  The only
#	machine dependent code in this function is the interrupt
#	priority adjustment.  This is handled with a run-time check.
#	This function is not architecture dependent.
#
# REGISTERS ON ENTRY:
#
#	r1 - stack
#	r2 - Kernel TOC
#	r3 - "level" number for	kernel trace ('1' for
#	    interrupt to vector	0x100, etc.)
#	r4 - Desired interrupt priority	number * 256,
#	    backtrack flag. (r4) == -1 if not to set priority
#	r13 - Current mstsave area   (NEW_MST)
#	r14 - Previous mstsave area  (INTR_MST)
#	r15 - Private memory area
#	lr - Return address
#
#       WARNING:
#		destroys r16-r18
#
# EXECUTION ENVIRONMENT:
#	Called from flihs
#
# RETURNS:
#	None
#
# PSEUDO CODE:
#	void
#	begin_interrupt(
#	       int interrupt_level,	       /* flih identifier */
#	       int newpri)		       /* new interrupt	priority */
#	{
#	       extern struct ppd *ppdp;	       /* ppda for this	processor */
#
#	       /* if tracing is	enabled	do flih	trace hook
#		*/
#	       if (Trconflag[0])
#	       {
#		       trchook(HKWD_KERN_FLIH|interrupt_level, iar);
#	       }
#
#	       /* if caller does not want priority changed return
#		*/
#	       if (newpri == -1)
#		       return;
#
#	       /* Update software interrupt priority and backtracking stuff
#		*/
#	       (short)ppdp->csa->intpri	= newpri;
#
#	       /* if priority requested	is INTMAX return as this function
#		*  is entered with EE bit clear
#		*/
#	       pri = newpri >> 8;
#	       if (pri == INTMAX)
#		       return;
#
#		if (__power_pc())
#		{
#			/*
#		 	 * All PPC models support the software-managed model;
#			 * therefore, no work needs to be done (i.e.
#			 * no need to write CPPR).
#		 	 */
#			return;
#		}
#		else if (__power_rs2())
#		{
#			cil = i_data.cil[pri];
#			return;
#		}
#		else
#		{
#			EIM0 = i_data.pma[mstp->intpri].eim0;
#			EIM1 = i_data.pma[mstp->intpri].eim1;
#		}
#	}
#
#
#******************************************************************************

ENTRY(begin_interrupt):
	.extern	ENTRY(begin_interrupt)

ifdef(`FLIH_LEDS',`
	stm	r29, -8(r1)		# save some registers
	mr	r30, r3			# save r3
	mr	r31, r4			# save r4
	l	r4, int_count(0)	# get interrupt count
	ai	r4, r4, 1		# inc count
	st	r4, int_count(0)	# update result
	rlinm	r3, r3, 24, 0xff000000	# move level number
	rlimi	r3, r4, 20, 0x00f00000	# insert count
	mflr	r29			# save link register
	bl	ENTRY(write_leds)	# write leds
	.globl	ENTRY(write_leds)
	mtlr	r29			# restore link register
	mr	r3, r30			# restore r3
	mr	r4, r31			# restore r4
	lm	r29, -8(r1)		# restore registers
',)

ifdef(`MLDEBUG',`
	lil	r0, 1			# always trace when debug is on
',`
	lbz	r0,DATA(Trconflag)	# get current trace mode
')

	cmpi	cr0,r0,0		# is trace on?
	mr	r18, r4			# Save priority	value across call
	beq	beg_10			# trace	is off,	skip call
	mfspr	r16, lr			# Save LR across call
	mr	r17, r3			# Save "level" number for trace

#	Call the trace routine,	specifying:
#	hookword	: hook ID | interrupt level
#	iar		: iar of interrupted program
# these	aren't traced now:
#	msr		: msr of interrupted program
#	priorities	: new priority/backtrack, old priority/backtrack
#	curproc		: current process
#	mst		: interrupted mst

	cau	r3, r17, (HKWD_KERN_FLIH | HKTY_LT)>16
	l	r4, mstiar(INTR_MST)
ifdef(`MLDEBUG',`
	l	r5, mstmsr(INTR_MST)
	lhz	r6, mstintpri(INTR_MST)
	rlimi	r6,r18,16,0,15		# form newpri/oldpri
	l	r7, ppda_curthread(r15)
	mr	r8, INTR_MST
	bl	ENTRY(mltrace)
	.extern	ENTRY(mltrace)
',`
	bl	ENTRY(trchook)		# Call the trace routine
       .extern	ENTRY(trchook)
')

	mtspr	lr, r16			# Restore LR

beg_10:
#	Set actual priority in mst; load proper	value into EIM

	cmpi	cr0, r18, -1		# See if prio is to be set
	rlinm	r5, r18, 24, 0xFF	# Isolate priority value
	cmpi	cr1, r5, INTMAX		# Check	for INTMAX
	l	r11, syscfg_arch(0)	# get machine architecture
	l	r12, syscfg_impl(0)	# get machine implementation

	beqlr+	cr0			# Return if not	to set prio
	cmpi	cr0, r11, POWER_PC	# check	for PPC
	sth	r18, mstintpri(NEW_MST)	# Store	new inter. priority in mst
	beqr	cr1			# Return if INTMAX; don't enable EE
	cmpi	cr1, r12, POWER_RS2	# check	for RS2

	beqr	cr0 	    	    	# No more work for PPC
	LTOC(r6,i_data,data)		# Load addr of "i_data"	table
	beq	cr1, beg_dis_rs2	# branch to RS2	disable	function

#
#	RS1/RSC	interrupt disable sequence
#

	rlinm	r5, r5,	3, 0x7F8	# Multiply priority by 8 for word index
	a	r5, r5,	r6		# Calculate address of masks
	lsi	r11, r5, 8		# load new mask	values
	cau	r10, 0,	BUID0		# Gain access to BUID0,	where the
	cau	r9, 0, 0xF000		# get EIM address
	mtsr	sr15, r10
	stsi	r11, r9, 8		# write	new EIM	values
	br				# Return

 	.machine "com"

beg_dis_rs2:
	lbzx	r11, r6, r5		# load new CIL value
	rlinm	r11, r11, 8, 0xFF00	# generate Update CIL command
	mtspr	ILCR, r11
	muli	r3, r3,	1		# Delay	to make	sure the CIL
	muli	r3, r3,	1		# is updated
	br

#*******************************************************************************
#
# NAME:	finish_interrupt
#
# FUNCTION:
#      finish_interrupt() is called at the end of every flih.
#      and from i_dosoft. Its first function is to check for
#      pending software interrupts.
#      There are 2 variables to check, i_softis & i_softpri, they
#      are strategicly located in two shorts of the same word.
#      If either is non-zero then call_softint() is called to
#      sort them out.

#      i_softint() is called if any software interrupts are
#      pending.  i_softint() checks if the mst to be resumed is
#      enabled sufficiently before allowing a software interrupt
#      handler to run. If i_softint() can not executed the
#      software interrupt handler, it schedules a hardware
#      interrupt to occur when the machine becomes enabled.
#
#      The runrun flag is used by process management to indicate
#      that the dispatcher needs to be called. When runrun is set,
#      run_dispatch() is executed to set up a run-time environment
#      for the dispatcher.  Processes resumed with a fixed stack
#      also run through run_dispatch() since it contains the
#      stack-touching logic.
#
# EXECUTION ENVIRONMENT:
#      This is a internal interface called only by flihs.
#
# RETURNS: None
#
# PSEUDO CODE:
#
#      void finish_interrupt()
#      {
#	       extern struct ppd *ppdp;	       /* ppda for this	processor */
#
#	       /* check	for pending software interrupt
#		*/
#	       if (ppdp->softis || ppda->softpri)
#	       {
#		       cleanup_mst;
#		       call_softint();
#	       }
#
#	       assert(ppdp->csa->ioalloc == 0);
#
#	       /* return interrupt mst to stack	frame
#		*/
#	       free_mst;
#
#	       /* call the dispatcher if runrun	is set,	or the stack needs to be
#		* touched
#		*/
#
#	       if (ppdp->csa->prev == NULL)
#	       {
#		       if (runrun)
#		       {
#			       /* dispatch and stack touch
#				*/
#			       run_dispatch(DISPATCH);
#		       }
#		       else if (ppdp->csa->stackfix)
#		       {
#			       /* stack	touch only
#				*/
#			       run_dispatch(0);
#		       }
#	       }
#
#	       free_mst();
#	       resume();
#
#      }
#
#
#*******************************************************************************

ENTRY(finish_interrupt):
	.globl	ENTRY(finish_interrupt)

	lil	r3, DISABLED_MSR		# load a disabled MSR value
	mtmsr	r3				# disable interrupts
	l	r10, ppda_softis(PPDA_ADDR)	# get software interrupt flag
	l	r11, mstprev(INTR_MST)		# get mst to resume
	cmpi	cr0, r10, 0			# check	for software interrupts
	cmpi	cr3, r11, NULL			# check	for thread mst

	bnel	cr0, call_softint		# call i_softint if softis set
	lbz	r7, ppda_ficd(PPDA_ADDR)	# load finish-int call dispatch
	l	r0, DATA(runrun)		# get value of runrun
	l	r12, mststackfix(INTR_MST)	# check	for fixed stack
	cmpi	cr5, r7, 0			# check for ficd set
	cmpi	cr2, r0, 0			# check	for runrun set
	bne	cr3, call_resume		# call resume if not a thread

	l	r11, mstioalloc(NEW_MST)	# check for iomem_atts
	crand	cr2*4+eq, cr2*4+eq, cr5*4+eq	# runrun==0 and ficd==0
	cmpi	cr4, r12, NULL			# check	for fixed stack
	tnei	r11, 0				# trap if not zero
	beq	cr2, runrun_clear		# branch if runrun clear
	crorc	cr0*4+eq, cr0*4+eq, cr0*4+eq	# set eq bit for run_dispatch
ifdef(`_POWER_MP',`
	lil	r4, 1				# proc_int_lock is not held
')
	b	run_dispatch			# call run_dispatch

runrun_clear:
	beq	cr4, stackfix_clear		# branch if not	a fixed	stack
	crxor	cr0*4+eq, cr0*4+eq, cr0*4+eq	# clear	eq bit
ifdef(`_POWER_MP',`
						# no need to set r4, no dispatch
')
	b	run_dispatch			# call run_dispatch
stackfix_clear:

#	Back up	to the previous	mst save area

call_resume:
	st	NEW_MST, ppda_mstack(PPDA_ADDR)	# free current mst
	st	INTR_MST, ppda_csa(PPDA_ADDR)	# backup up to previous	mst
	CSA_UPDATED(cr0, r3, INTR_MST)
	b	ENTRY(resume)			# branch to resume
	.extern	ENTRY(resume)

#*******************************************************************************
#
# NAME:	call_softint
#
# FUNCTION:
#	Called when software interrupts have been detected.  This routine
# is glue code between finish_interrupt and i_softint()/i_softmod.
# It initializes the current mst and runtime stack and then calls
# i_softint() or i_softmod().  Only volatile registers are destroyed
# by this routine
#
# EXECUTION ENVIRONMENT:
#	Called only from finish_interrupt.  MSR_EE=0
#
#	When called csa	points to a mst that can be used to process
# any queued interrupts.  This mst needs to be re-initialized before use.
#
# REGISTERS ON ENTRY:
#	r2 - kernel toc
#	r10 - i_softis & i_softpri values
#	r13 - address of mst for interrupt handler (NEW_MST)
#	r14 - address of interrupted mst (INTR_MST)
#	r15 - address of PPDA (PPDA_ADR)
#
# RETURNS:
#	None
#
#*******************************************************************************
	.set	tframe,	8			# size of local	stack frame

call_softint:
	lil	r0, 0				# initialize mst
	mflr	r12				# save return address
	st	r0, mstkjmpbuf(NEW_MST)		# clear	exception handler
	cau	r3, 0, NEW_ADSPACE		# address space	allocation
	st	r0, excbranch(NEW_MST)		# clear	fast exception handler
	liu	r0, INTMAX*256			# intpri+back tracking
	sth	r0, mstintpri(NEW_MST)		# set priority and back track
	st	r3, mstsralloc(NEW_MST)		# initialize seg reg allocation

ifdef(`_KDB',`
#
# Linkage conventions are respected to allow stack trace working fine
# Two frames are reserved.
#
	cal	r1, -stkmin(NEW_MST)		# setup	1st frame
	lil	r0, 0
	st	r12, stklink(r1)		# save link register
	st	r0, stkback(r1)			# clean back-chain for debug
	stwu	r1, -stkmin(r1)			# set back-chain for debug
',`
	st	r12, -tframe(NEW_MST)		# save link register
	cal	r1, -(stkmin+tframe)(NEW_MST)	# setup stack
') # _KDB
	rlinm.	r0, r10, 0, 0xffff0000		# i_softis
	rlinm	r3, r10, 16, 0xffff0000		# i_softpri
	bne	call_soft1

	.extern	ENTRY(i_softmod)
	bl	ENTRY(i_softmod)		# must be software managed
	b	call_soft2			#	model

call_soft1:
	bl	ENTRY(i_softint)		# call HW assisted software
						#	int handler
	.extern	ENTRY(i_softint)

call_soft2:
ifdef(`_KDB',`
	l	r12, (stklink+stkmin)(r1)	# restore link register
',`
	l	r12, stkmin(r1)			# restore link register
') # _KDB
	mtlr	r12
	br

#******************************************************************************
#
# NAME:	call_dispatch/run_dispatch
#
# FUNCTION:
#	call_dispatch() allows the dispatcher to choose a new
#	thread to run. call_dispatch() buys an interrupt
#	stack frame for use as a run-time stack while executing the
#	dispatcher.  It must be called with xlate on and interrupts
#	disabled.
#
#	Another function of call_dispatch() is touching the user
#	stack if the thread was running under a fixed stack.
#	Because of this touch, the function places the mst into
#	back-tracking state in case the stack touch page faults.
#
#	When releasing the interrupt stack call_dispatch() must
#	refetch the prev pointer from the mst.  This is because in a
#	threaded environment the dispatcher changes the prev pointer
#	when a new thread is dispatched.
#
#	call_dispatch() is the external interface.  It is called
#	from a thread. The function run_dispatch() does the work.
#	This is the internal interface used by the flihs and
#	switch().  run_dispatch() is called with a interrupt mst
#	already allocated.  The dispatch parameter flags if the
#	dispatcher is to be run.  A stack touch is always
#	performed (if required)
#
# EXECUTION ENVIRONMENT:
#	Called with interrupts disabled.
#
# REGISTERS ON ENTRY:
#	call_dispatch()
#		MSR(EE)=0
#		r2 - kernel toc
#
#	run_dispatch()
#		r2 - kernel toc
#		r4 - no_proc_int_lock (if POWER_MP)
#		r13 - address of mst for interrupt handler (NEW_MST)
#		r14 - address of interrupted mst (INTR_MST)
#		r15 - address of PPDA (PPDA_ADR)
#		cr0(eq)	- 1 run	dispatcher
#
#	call_dispatch()
#	{
#	       buy_mst();
#	       run_dispatch(DISPATCH);
#	}
#
#	run_dispatch(
#	       int dispatch)
#	{
#	       extern struct ppd *ppdp;	       /* ppda for this	processor */
#	       struct proc *new_proc;	       /* new process to dispatch */
#	       struct mstsave *mstp;	       /* process environment mst */
#
#
#	       /* Choose new process thread to run
#		*/
#	       if (dispatch)
#	       {
#		       new_proc	= dispatch();
#	       }
#	       else
#	       {
#		       new_proc	= ppdp->curproc;
#	       }
#
#	       /* The process private segment must be updated to point to new
#		* process that is running.
#		*/
#	       mtsr(PROCESS_PRIVATE, new_proc->p_adspace);
#	       isync();
#	       mstp = ppdp->csa->prev;
#
#	       /* If any queued interrupts are present and the current
#		* priority allows them to be processed then go process
#		* any queued interrupts.
#		*/
#	       if (i_softpri != 0  && i_softpri < mstp->mstintpri )
#			finish_interrupt();
#
#	       /* Check	if a stack touch is required
#		*/
#	       if (mstp->stackfix)
#	       {
#			ppdp->stackfix = 1;
#		       /* Get addressability to	the kernel stack.
#			*/
#		       mtsri(mstp->stackfix, mstp->sr[mstp->stackfix >>	28]);
#		       isync();
#
#		       /* Put into backtracking	state in case of a page	fault.
#			*/
#		       (short)ppdp->csa->backt = BKTPFAULT;
#
#		       trash = *mstp->stackfix;
#		       trash = *(mstp->stackfix	+ STACKTOUCH);
#		       trash = *(mpsp->stackfix	- STACKTOUCH);
#	       }
#              else
#	       {
#	       	       ppdp->stackfix = 0;
#              }
#
#	       free_mst();
#
#	       goto resume;
#	}
#
#******************************************************************************

ENTRY(call_dispatch):
	.globl	ENTRY(call_dispatch)

	GET_PPDA(cr0, PPDA_ADDR)		# load PPDA pointer
	l	NEW_MST, ppda_mstack(PPDA_ADDR)
	l	INTR_MST, ppda_csa(PPDA_ADDR)	# allocate an mst
	cal	r0, -framesize(NEW_MST)		# Back up the pointer to the
	st	r0, ppda_mstack(PPDA_ADDR)	#   available mst
	st	NEW_MST, ppda_csa(PPDA_ADDR)	# Update csa
	CSA_UPDATED(cr0, r3, NEW_MST)
	crorc	cr0*4+eq, cr0*4+eq, cr0*4+eq	# set eq bit and fall through to
						#   run_dispatch
ifdef(`_POWER_MP',`
	lil	r4, 1				# proc_int_lock is not held
')

run_dispatch:

#	Get an mstsave area to run the dispatcher and to do stack touch

	lil	r0, 0
	liu	r3, NEW_ADSPACE		# segment register allocation
	st	r0, mstkjmpbuf(NEW_MST)	# Clear	jump buffer pointer
	st	r0, excbranch(NEW_MST)	# Clear	jump buffer pointer
	sth	r0, mstintpri(NEW_MST)	# Clear	intpri/backtrack
	st	r3, mstsralloc(NEW_MST)	# Set seg reg allocation vector



	cal	r1, -stkmin(NEW_MST)	# Set stack ptr	for call to dispatch()

#	Call the dispatcher to select a	(potentially new) process

	l	r3, ppda_curthread(PPDA_ADDR) # get curthread value
	l	r3, t_procp(r3)		# ! dispatch returns a proc pointer !
	bne	cr0, no_dispatch	# don't	call dispatcher
	bl	ENTRY(dispatch)		#  runrun is not set
       .extern	ENTRY(dispatch)
no_dispatch:

#	Pick up	(possibly different) user private segment register value
#	dispatch returns process table address in r3

	l	r30, p_adspace(r3)	# User's sr2 value
	rlinm	r30, r30, 0, ~SR_KsKp	# Clear	the "k"	bit
	mtsr	sr2, r30		# Restore it to	seg reg
	isync				# PPC requires this

#	NEW_MST is an mstsave area bought out of the mststacks and is still
#	valid but INTR_MST is the standard mstsave area in the uthread
#	structure and may now be at a completely different address. dispatch()
#	has already relinked NEW_MST and INTR_MST via the mstprev field, we
#	just need to fetch it from there.

	l	INTR_MST, mstprev(NEW_MST)

#	Check for any queued interrupts that may be able to be processed
#	as a result of this new priority due to a new process/thread.
#
#	This same logic can be found in i_softmod().  Process only
#	queued interrupts that are more favored than the current priority.
#	Finish_interrupt is where the next set of decisions are if work
#	is pending
#
#	/* Only process more favored hardware interrupts, process
#	 * decrementer, off-levels, and iodone priorities only at
#	 * INTBASE
#	 */
#	if( ((queued_priority < INTTIMER) &&
#	     (queued_priority < current_priority)) ||
#	    (current_priority == INTBASE) )
#	{
#		go process queued interrupts
#	}
#

	lhz	r7, ppda_softpri(PPDA_ADDR) # get interrupt priority queue
	lbz	r8, mstintpri(INTR_MST)	# Get interrupt priority
	rlwinm	r7, r7, 16, 0xffff0000	# move priority queue to top half
	l	r6, mststackfix(INTR_MST) # Critical section flag
	cntlzw	r7, r7			# generate value of highest queued intr
	lil	r9, 1
	cmp	cr1, r7, r8		# check if able to process interrupts
	cmpi	cr6, r7, INTTIMER
	cmpi	cr7, r8, INTBASE
	cmpi	cr0, r6, 0		# Check	value of critical section flag
	cror	cr5*4+eq,cr6*4+lt,cr7*4+eq
	bge+	cr1, no_interrupt	# no interrupt processing needed
	beq-	cr5, ENTRY(finish_interrupt)	# interrupt processing needed

#	If resuming critical section, touch the	stack in various places

no_interrupt:
	beq	cr0, no_touch		# Branch if no "touch" needed

#	set stack fix flag only used in MP

	stb	r9, ppda_stackfix(PPDA_ADDR)

#	get the	user stack segmsnt
	cal	r7, mstsr(INTR_MST)	# get address of faulting mst's	sr
	rlinm	r8, r6,	6, 0x3c		# get 4	* (stack segreg	num)
	lx	r9, r8,	r7		# load the stack seg reg value
	rlinm	r9, r9,	0, ~SR_KsKp	# clear	the key	bit
	mtsri	r9, 0, r6		# load seg reg for stack touch
	isync

	cal	r7, (INTMAX<8)|BKTPFAULT(0)
	sth	r7, mstintpri(NEW_MST)	#   in case of page-fault
	l	r7, 0(r6)		# Touch	where stack pointer points,
	l	r7, STACKTOUCH(r6)	#   and	above
	l	r7,-STACKTOUCH(r6)	#   and	below
	b	free_mst

no_touch:
	stb	r6, ppda_stackfix(PPDA_ADDR)


#	Back up	to the previous	mst save area
free_mst:
	st	NEW_MST, ppda_mstack(PPDA_ADDR)	# Our mst becomes next available
	st	INTR_MST, ppda_csa(PPDA_ADDR) #	Interrupted mst	becomes	current
	CSA_UPDATED(cr0, r3, INTR_MST)

	b	ENTRY(resume)
	.extern	ENTRY(resume)

	.toc
	TOCE(i_data, data)

include(machine.m4)
include(mstsave.m4)
include(i_machine.m4)
include(scrs.m4)
include(flihs.m4)
include(low_dsect.m4)
include(trchkid.m4)
include(systemcfg.m4)
include(param.m4)
include(proc.m4)
