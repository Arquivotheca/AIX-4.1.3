# @(#)37	1.1.1.16  src/bos/kernel/ml/POWER/state.m4, sysml, bos41J, 9520A_a 5/16/95 14:33:54
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS: NONE
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

include(macros.m4)

#*****************************************************************************
#
# NAME:	state_save
#
# FUNCTION:
#	The value of r15 is saved in a machine dependent location
#	(SPRG1 for PPC;	low memory for Power).	All the	rest of	the
#	thread	state is in registers.	On exit	from this routine,
#	all the	thread	state, except floating point registers,	is
#	in the mst.
#
#	The values of SRR0 and SRR1 must be saved before turning on
#	translation.  This is because the PPC architecture allows
#	these registers	to be used when	a software TLB reload
#	occurs.
#
#	This is	an architecture	dependent function.  It	is built
#	from a single #ifdef'ed	source that produces two binaries.
#	The function is	reached	with the branch	table.
#
#	     The routine must:
#
#	 1.  Establish read/write addressability to the	kernel and
#	     user segments.
#
#	 2.  Set translate on, interrupts disabled (except machine
#	     check).
#
#	 3.  Clear the reservation on PPC machines
#
#	 4.  Save the rest of the GPRs and special registers
#	     (except floating point) in	the mst	pointed	to by
#	     ppda[N].csa.
#
#	 5.  Save all the segment registers.
#
#	 6.  Establish read/write addressability to the	kernel
#	     extension segment.
#
#	 7.  Get a new mstsave area from mstack.
#
#	 8.  Initialize	context-dependent variables --
#	     mstkjmpbuf,execbranch, and	address	space.
#
#	 9.  Assert that if an interrupt handler was interrupted,
#	     its run-time interrupt stack has not overflowed.
#
#	10.  Set up the	stack pointer (r1) and TOC pointer (r2)	for
#	     calling C routines	from the FLIHs.
#
#	11.  As	a convenience to callers, on return r13	is set to
#	     point to the mst which this routine acquires (new
#	     value of csa), r14	is set to point	to the previous	mst
#	     (old value	of csa), and r15 continues to point at the
#	     ppda.
#
# REGISTERS ON EXIT:
#	r1     Address of stack (for C)
#	r2     Address of kernel TOC
#	r13    Address of current  mst save area (new value of csa)
#	r14    Address of previous mst save area (old value of csa)
#	r15    PPDA address
#	sr0    R/W access to kernel segment
#	sr2    R/W access to process segment
#	sr14   R/W access to kernel extension segment
#
# EXECUTION ENVIRONMENT:
#	Called xlate off at the	start of flihs and swtch.
#
#	state_save(
#	       struct ppd * ppdp)      /* address of ppda for this processor */
#	{
#		int sr2val;		/* interrupted sr2 value */
#		struct mstsave *mstp;	/* mst to save into */
#
#		/* save	some registers and seg regs to get started.  r0	was
#		 * saved at ppdp->save[0] by the flih prolog
#		 */
#
#		ppdp->save[1] =	r1;
#		ppdp->save[7] = r2;
#		ppdp->save[2] =	mfsr(0);
#		sr2val = ppdp->save[3] = mfsr(2);
#		ppdp->save[4] =	mfsrr0();
#		ppdp->save[5] =	mfsrr1();
#
#		/* get kernel addressability
#		 */
#		mtsr(0,	g_ksrval);
#		mtsr(2,	sr2val & !(Ks|Kp));	/* clears the S	power bit */
#
#		/* move	r15 value and zero low memory variable before xlate
#		 * is turned on	since ra_save0 is read only
#		 */
#	#ifdef _POWER_RS
#		ppdp->save[6] =	ra_save0;
#		ra_save0 = 0;
#	#endif
#
#		/* save	the SPRG1 value	before turning xlate on, so the	software
#		 * tlb reload handler can use it
#		 */
#	#ifdef _POWER_PC
#		ppdp->save[6] =	mfspr(SPRG1)
#	#endif
#		/* Turn	on xlate, but continue to run disabled
#		 */
#		mtmsr(MSR_IR|MSR_DR|MSR_AL|(mfmsr() & MSR_ME));
#
#	#ifdef _POWER_PC
#		isync();
#
#	#endif
#
#		mstp = ppdp->csa;
#
#		/* save	gprs
#		 */
#		mstp->gpr[2..14,16..31]	= r2..r14,r16..r31;
#		mstp->gpr[0] = ppdp->save[0];
#		mstp->gpr[1] = ppdp->save[1];
#
#		/* get the value of r15	saved earlier
#		 */
#		mstp->gpr[15] =	ppdp->save[6];
#
#		/* save	more registers
#		 */
#		mstp->iar = ppdp->save[4];
#		mstp->msr = ppdp->save[5];
#		mstp->cr = mfcr();
#		mstp->ctr = mfctr();
#		mstp->xer = mfxer();
#	#ifdef _POWER_RS
#		mstp->mq = mfmq();
#	#endif
#	#ifdef _POWER_PC
#		if (_power_601())
#		{
#			mstp->mq = mfmq();
#		}
#		else
#		{
#			/* Save data bats if in use
#			 */
#			if (mstp->ioalloc)
#			{
#				mstp->dbats[0..3] = dbat(0..3);
#			}
#			/* Always invalidate data BATs
#			 */
#			dbatu(0..4) = 0;
#		}
#	#endif
#		mstp->sr[1,3..15] = sr1, sr3..sr15;
#		mstp->sr[0] = ppdp->save[2];
#		mstp->sr[2] = ppdp->save[3];
#
#		mtsr(14, kernex);
#
#	#ifdef _POWER_PC
#		/* for kernel extension	load and user IO access
#		 */
#		isync();
#	#endif
#
#		allocate new mst
#
#		setup C	run-time environment
#
#		check interrupted stack	pointer	for overflow;
#	}
#
#*****************************************************************************

	.using	low, r0

ifdef(`_POWER_RS',`ENTRY(state_save_rs):',)
ifdef(`_POWER_PC',`ENTRY(state_save_pc):',)

	st	r1, ppda_save1(PPDA_ADDR) # Save r1 so we have something
					#   to work with
	st	r0, ppda_save7(PPDA_ADDR) # get another work register

#	Save segment registers 0 (kernel) and 2	(user);	reset the key (k)
#	bits in	each register so that we have read/write access	to these
#	segments.

	mfsr	r1, sr0			# Kernel segment
	mfsr	r0, sr2			# get process private seg (u-block)
	st	r1, ppda_save2(PPDA_ADDR) # old	save sr0
	st	r0, ppda_save3(PPDA_ADDR) # save sr2
	l	r1, DATA(g_ksrval)	# Load kernel segment register value
	rlinm	r0, r0, 0, ~SR_KsKp	# clear "k" bit(s)
	mtsr	sr0, r1			# load kernel seg reg
	mtsr	sr2, r0

#	Save the values	of SRR0, and SRR1 before turning on xlate.  This
#	must be	done since software TLB	reload interrupt are permitted
#	us use these two registers

	mfspr	r1, SRR0		# get old iar
	mfspr	r0, SRR1		# get old msr
	st	r1, ppda_save4(PPDA_ADDR)
	st	r0, ppda_save5(PPDA_ADDR)

ifdef(`_POWER_RS',`
#	copy saved value of r15	to private save	area.  This could be
#	read-only memory (if a future release) so this must be done before
#	xlate is turned	on.  Also zero the memory to avoid a security hole
	l	r1, DATA(ra_save)(0)	# get the saved	r15 value
	lil	r0, 0
	st	r1, ppda_save6(PPDA_ADDR) # copy to local save area
	st	r0, DATA(ra_save)(0)
',)
ifdef(`_POWER_PC',`
#	copy saved value of r15	to private save	area.  Do this before turning
#	on translation to allow	the TLB	reload handler to use SPRG1
	mfspr	r1, SPRG1		# move SPRG1 to the local save area
	st	r1, ppda_save6(PPDA_ADDR)
')

#	Set translate on in the	machine	state register (MSR) before
#	saving the rest	of the interrupted program's state.  This is
#	necessary because csa may point	to a save area in the u-block,
#	which is pinned	in memory (while the thread is	running) but is
#	addressed with a virtual address.  If an interrupt-level program
#	was running, csa points	to a save area in the mst stack, pinned
#	in the kernel segment.

	mfmsr	r1			# get old MSR value
	rlinm	r1, r1, 0, MSR_ME	# preserver ME bit
	oril	r1, r1, MSR_IR|MSR_DR|MSR_AL # tun on xlate and AL
	mtmsr	r1			# new msr value
ifdef(`_POWER_PC',`isync',)

#	Get addressability to the mst via csa.	Save the rest of the
#	thread	state currently	still in registers into	the mst.


	l	r1, ppda_csa(PPDA_ADDR)	# Get current save area pointer
	l	r0, ppda_save7(PPDA_ADDR)
	stm	r2, mstgpr+2*4(r1)	# Save GP regs 2 through 31

#	move previously saved values of r0, r1, and r15 to mst
	l	r16, ppda_save0(PPDA_ADDR) # reload saved r0 from ppda
	l	r17, ppda_save1(PPDA_ADDR) # reload saved r1 from ppda
	l	r18, ppda_save6(PPDA_ADDR) # reload saved r15 from ppda
	st	r16, mstgpr(r1)		# save r0 in proper place
	st	r17, mstgpr+1*4(r1)	# save r1 in proper place
	st	r18, mstgpr+15*4(r1)	# save r15 in proper place

#	Load machine registers into GP registers for saving via	stm

ifdef(`_POWER_PC',`
	l	r25, syscfg_impl(0)	# get processor type
')

	l	r26, ppda_save4(PPDA_ADDR) # get IAR
	l	r27, ppda_save5(PPDA_ADDR) # get MSR
	mfcr	r28			# Condition register
	mr	r29, r0			# Move saved link register
ifdef(`_POWER_PC',`
	cmpi	cr1, r25, POWER_601	# check for 601
',)
	mfctr	r30			# Count	register
	mfxer	r31			# Fixed	point exception	register
ifdef(`_POWER_PC',`
	beq+	cr1, mq_save		# branch if  601
	l	r12, mstioalloc(r1)	# check for kernel BAT usage
	lil	r0, 0			# invalid BAT
	cmpi	cr0, r12, 0		# check if BATs in use
	mtdbatu	3, r0			# kill BAT 3 (never saved )
	bne-	cr0, bat_save		# save BATs if in use
					#      (r12-r14, r1, DBAT0-2)
bat_save_ret:
	mtdbatu	2, r0			# invalidate existing DBATs
	mtdbatu	1, r0
	mtdbatu	0, r0
	b	no_mq_save		# skip MQ save
',)

mq_save:
	mfspr	r25, MQ
	st	r25, mstmq(r1)		# store	mq to mst
no_mq_save:

	stm	r26, mstiar(r1)		# Save IAR et al. in mst

#	Save all the segment registers

	l	r16, ppda_save2(PPDA_ADDR) # get saved sr0
	mfsr	r17, sr1
	l	r18, ppda_save3(PPDA_ADDR) # get saved sr2
	mfsr	r19, sr3
	mfsr	r20, sr4
	mfsr	r21, sr5
	mfsr	r22, sr6
	mfsr	r23, sr7
	mfsr	r24, sr8
	mfsr	r25, sr9
	mfsr	r26, sr10
	mfsr	r27, sr11
	mfsr	r28, sr12
	mfsr	r29, sr13
	mfsr	r30, sr14
	mfsr	r31, sr15

	l	r12, DATA(g_kxsrval)	# Kernel extension segment reg value
	stm	r16, mstsr(r1)		# Save the segment regs	in the mst
	mtsr	sr14, r12		# load kernel extension	segment

ifdef(`_POWER_PC',`isync',)		# for bus access and sr14 load

#	Get a new mst save area

	mr	INTR_MST, r1		# Return old mst address in INTR_MST
	l	NEW_MST, ppda_mstack(PPDA_ADDR) # New mst
	l	r3, mstprev(INTR_MST)	# Get addr previous mst
	cal	r0, -framesize(NEW_MST)	# Store	address	of next	available
	st	r0, ppda_mstack(PPDA_ADDR) # for next interrupt
	st	NEW_MST, ppda_csa(PPDA_ADDR) # Store new current save area
	CSA_UPDATED(unused, unused, NEW_MST)

	cmpi	cr0, r3, NULL		# Test for NULL	previous mst
	lil	r0, 0
	liu	r4, NEW_ADSPACE		# get new address space allocation mask
	sth	r0, mstintpri(NEW_MST)	# Clear	priority and back-track
	st	r0, mstkjmpbuf(NEW_MST)	# Clear	jump buffer pointer
	st	r0, excbranch(NEW_MST)	# Clear	jump buffer pointer
	st	r4, mstsralloc(NEW_MST)	# set allocation vector

#	setup TOC and stack and return

	l	r2, DATA(g_toc)		# load kernel toc
	cal	r1, -stkmin(NEW_MST)	# Point	to a real stack	frame
	beqr	cr0			# return if no previous interupt

#	If interrupted program was an interrupt	handler	the stack pointer
#	(r1) should not	be inside new mst save.	 If it is, interrupted program
#	has over-flowed	stack.

	l	r3, mstgpr+1*4(INTR_MST) # get saved stack pointer
	cal	r4, (mstend-mstsave-framesize)(NEW_MST)	# r2 = low addr	end mst
	sf	r3, r4,	r3		# r3 = r3 - r4
	tllei	r3, framesize		# Trap if overflow
	br

#******************************************************************************
#
# NAME:	resume
#
# FUNCTION: resume
#	This is	the general state restore routine.  All	interrupts
#	and thread dispatches exit through this	routine.
#	resume() is entered with MSR(EE)=0.  This is an	external
#	entry point, called from low.s and vmvcs.s.  resume() is
#	architecture dependent.	 There is one source ifdef'ed for
#	all supported architectures.
#
#	resume has the following functions:
#
#	   + Call resume trace hook if trace is	enabled
#
#	   + Check for sig_slih
#
#		+ Must be thread
#
#		+ Must have a signal suppression count of zero.
#
#		+ Must have a signal/termination/suspension/swapping request
#
#	   + Restore non-floating point	state
#
#	   + Restore user segment registers
#
#
#	   + Restore user level	IO access
#
#	All the	registers except floating point	registers are
#	reloaded from the mstsave area.	 The IAR and MSR are copied
#	from the mst to	SRR0 and SRR1, and an "rfi" instruction	is
#	issued,	which resumes the interrupted program.
#
#	On entry nothing is assumed to be in the GPRs.	R/W access
#	to kernel segment, kernel extension segment, and user
#	private	segment	is assumed via segment regs 0, 14, and 2.
#
#	This is	an architecture	dependent function.  It	is built
#	from a single #ifdef'ed	source that produces two binaries.
#	The function is	reached	with the branch	table.
#
# PSEUDO CODE:
#
#	resume()
#	{
#		struct mstsave *mstp;
#		extern struct ppd *ppdp;	/* ppda	for this processor */
#
#
#	#ifdef _POWER_RS
#		ppdp = &ppda[0];
#	#endif
#	#ifdef _POWER_PC
#		ppdp = mfspr(SPRG0);
#
#		/* crash if there is a reservation hanging around
#		 */
#	#endif
#		mstp = ppdp->csa;
#
#		/* Do system trace hook	if tracing is enabled
#		 */
#		if (Trconflag[0])
#			trchook(HKWD_KERN_RESUME, mstp->iar);
#
#		if ((mstp->prev == NULL) && (ppdp->curthread->t_suspend == 0))
#			if ((ppdp->curproc->p_int & SSIGSLIH) ||
#					(ppdp->curthread->t_flags & TSIGSLIH))
#				call_sigslih();
#				/* does not return */
#
#	#ifdef _POWER_RS
#
#		/* Do machine specific interrupt priority manipulations
#		 */
#		if (__power_rs2())
#		{
#			mtcil(i_data.pma[mstp->intpri].cil);
#		}
#		else
#		{
#			EIM0 = i_data.pma[mstp->intpri].eim0;
#			EIM1 = i_data.pma[mstp->intpri].eim1;
#		}
#	#endif
#
#		/* copy	sr0 and	sr2 to ppda to be loaded with xlate off
#		 */
#		ppdp->save[0] =	mstp->sr[0];
#		ppda->save[1] =	mstp->sr[2]
#
#		/* load	segment	registers
#		 */
#		sr(1,3..15) = mstp->sr[1,3..15];
#
#		/* set up display access rights in segment registers/bats
#                * for graphics thread
#                */
#		if (ppdp->curthread->t_graphics && resuming to user mode)
#		{
#			call sr_bat_display_setup();
#		}
#
#		mtlr(mstp->lr);
#		mtctr(mstp->ctr);
#		mtxer(mstp->xer);
#	#ifdef _POWER_RS
#		mtmq(mstp->mq);
#	#endif
#	#ifdef _POWER_PC
#		/* clear reservation
#		 */
#		stcx(&dummy)
#		if (!__power_601())
#			if (mstp->ioalloc)
#				dbat(0..3) = mstp->dbat[0..3];
#		else
#			mtmq(mstp->mq)
#	#endif
#		mtcr(mstp->cr);
#
#		/* place iar and msr into save area, to	load SRR0 and SRR1 with
#		 * xlate off
#		 */
#		ppdp->save[2] =	mstp->iar;
#		ppdp->save[3] =	mstp->msr;
#		ppdp->save[4..6] = mstp->gpr[0..2]
#
#		/* The rest of this routine is run using r0-r2 as working
#		 * register
#		 */
#		gpr(3..31) = mstp->gpr[3..31];
#
#		/* Turn	of xlate
#		 */
#		mtmsr(MSR_ME);
#		mtsr(0,	ppdp->save[0]);
#		mtsr(2,	ppdp->save[1]);
#		mtsrr0(ppdp->save[2]);
#		mtsrr1(ppdp->save[3]);
#		gpr(0..2) = ppdp->save[4..6];
#		rfi();
#	}
#
#******************************************************************************

ifdef(`_POWER_RS',`ENTRY(resume_rs):',)
ifdef(`_POWER_PC',`ENTRY(resume_pc):',)
	lbz	r0,DATA(Trconflag)	# get current trace mode
	GET_PPDA(cr0, PPDA_ADDR)
ifdef(`_POWER_RS',`.machine "pwr"')
ifdef(`_POWER_PC',`.machine "ppc"')
	cmpi	cr0, r0, 0
ifdef(`MLDEBUG',`crxor	cr0*4+eq, cr0*4+eq, cr0*4+eq',) # allways do trace


	l	r2, DATA(g_toc)		# load kernel TOC
	l	r30, ppda_curthread(PPDA_ADDR) # get current thread pointer
	l	INTR_MST, ppda_csa(PPDA_ADDR) # Load ptr to mst to resume
	l	r31, t_procp(r30)	# get current process pointer
	bne	cr0, resume_trace	# (INTR_MST,PPDA_ADDR, volatile regs)
resume_trace_ret:

#	if we are resuming a thread in user mode, then do further checking
#	to see if the thread must execute sig_slih().

	lha	r28, t_suspend(r30)	# get t_suspend count
	l	r29, mstprev(INTR_MST)	# get mstprev field
	cmpi	cr0, r28, 0		# check for zero t_suspend count
	cmpi	cr1, r29, NULL		# NULL mstprev means a thread
	crand	cr2*4+eq, cr0*4+eq, cr1*4+eq # !suspend && (mstprev == NULL)
	beql	cr2, check_sigslih

ifdef(`_POWER_RS',`

#	set interrupt priority for Power machines

	l	r11, syscfg_impl(0)	# get processor	implementation
	lbz	r4, mstintpri(INTR_MST)	# Load inter. priority from mst
	cmpi	cr5, r11, POWER_RS2	# check	if on RS2
	LTOC(r12, i_data, data)		# Load addr of "i_data"	table
	beq	cr5, rs2_int		# branch if on a RS2

	rlinm	r4, r4,	3, 0x7F8	# Multiply by 8	for word index
	a	r4, r12, r4		# Get address of masks
	cau	r10, 0,	BUID0		# Gain access to BUID0,	where the
	lsi	r11, r4, 8		# Load new mask	value
	mtsr	sr15, r10		# load buid0
	cau	r9, 0, 0xF000		# get EIM address
	stsi	r11, r9, 8		# write	new EIM	values
	b	rs_int_set

#	There is assumed to be enough instruction between this the CIL load
#	and the rfi for the CIL load to take effect.

rs2_int:
	lbzx	r11, r4, r12		# load new CIL value
	rlinm	r11, r11, 8, 0xFF00	# generate ILCR	command
	mtspr	ILCR, r11		# set new interrupt priority

rs_int_set:
',)

ifdef(`_POWER_PC',`
ifdef(`DEBUG',`
	#	check that no software interrupts are pending
	lbz	r12, mstintpri(INTR_MST) # get current intpri
	cmpi	cr0, r12, INTBASE
	bne	skip_trap
	lhz	r11,ppda_softpri(PPDA_ADDR)
ifdef(`_POWER_MP',`
	li	r12, 0x8000>INTOFFL0	# trace is using an offlevel interrupt
	andc	r11, r11, r12		# ignore pending OFFL0 interrupts
')
	tnei	r11, 0
skip_trap:
')

',)

#	Restore	segment	regs and other miscellaneous regs

#	If we are about to resume a thread in user mode, we fetch the
#	segment registers directly from u_adspace to ensure a consistent
#	process address space among threads. Another thread may have
#	altered it while we were suspended and thus our MST may not
#	contain an accurate view of it.

	l	r11, t_userp(r30)	# get user structure address
	beq	cr2, SR_from_U
SR_from_MST:
	lm	r16, mstsr(INTR_MST)	# Pick up seg regs 0-15
	creqv	4*cr2+eq, 4*cr2+eq, 4*cr2+eq  # set cr2 eq bit
	b	misc_regs
SR_from_U:
	l	r3, t_graphics(r30)
	lm	r16, u_adspace_sr(r11)	# Pick up seg regs 0-15
	cmpi	cr2, r3, 0
misc_regs:
	st	r16, ppda_save0(PPDA_ADDR) # Save sr0 and sr2 in the ppda
	st	r18, ppda_save1(PPDA_ADDR)

ifdef(`_POWER_PC',`isync')
	# Load all the segment registers except 0 and 2, which we are
	# still using
	mtsr	sr15, r31
	mtsr	sr14, r30
	mtsr	sr13, r29
	mtsr	sr12, r28
	mtsr	sr11, r27
	mtsr	sr10, r26
	mtsr	sr9, r25
	lm	r25, mstiar(INTR_MST)	# Pick up other	special	regs
	mtsr	sr8, r24
	mtsr	sr7, r23
	mtsr	sr6, r22
	mtsr	sr5, r21
	mtsr	sr4, r20
	mtsr	sr3, r19
	mtsr	sr1, r17

	st	r25, ppda_save2(PPDA_ADDR)
	st	r26, ppda_save3(PPDA_ADDR)

	mtctr	r29			# Count	register

	mtxer	r30			# Exception register

ifdef(`_POWER_RS',`
	mtmq	r31			# Multiplier quotient
',)
ifdef(`_POWER_PC',`
	l	r24, syscfg_impl(0)	# get processor	implementation
	l	r23, mstioalloc(INTR_MST) # check for kernel BAT use
	cmpi	cr1, r24, POWER_601	# check	for 601
	cal	r26, ppda_save2(PPDA_ADDR) # get address for stwcx
	cmpi	cr7, r23, 0		# check if BATs in use
	stwcx.	r25, 0, r26		# clear reservation
	beq+	cr1, mq_load		# branch if 601
	beq+	cr7, bat_restore_ret	# branch if BATs are not in use
	b	bat_restore		# restore BATs
					#   DBATS(0..3), r12, r13, ,r14,INTR_MST
					#
mq_load:
	.machine "601"
	mtmq	r31			# restore mq
	.machine "ppc"
bat_restore_ret:
no_mq_load:
',)

	# resume() is 32 Kbytes away from sr_bat_display_setup()
	beq     cr2, no_srbat_setup

	# set up segment registers/bats for graphics thread
	# in case we are resuming to the user mode
	mr	r26, r3
	bl      sr_bat_display_setup  	# (r26, r29, r31, cr0, cr1)
	.extern	DATA(sr_bat_display_setup)

#	Reload the GP registers	and the	final two segment registers.
no_srbat_setup:
	l	r29, mstgpr+0*4(INTR_MST) # load r0
	l	r30, mstgpr+1*4(INTR_MST) # load r1
	l	r31, mstgpr+2*4(INTR_MST) # load r2

	mtcr	r27			# restore cr	(after runtime check)
	mtlr	r28			# Link register

	lil	r0, DISABLED_REAL_MSR	# xlate	off msr value
	st	r29, ppda_save4(PPDA_ADDR) # move gpr to ppda
	st	r30, ppda_save5(PPDA_ADDR)
	st	r31, ppda_save6(PPDA_ADDR)

	mr	r2, INTR_MST		# move mst pointer to r2
	mr	r1, PPDA_ADDR			# move ppda pointer to r1
	lm	r3, mstgpr+3*4(r2)	# restore r3-r31

	mtmsr	r0			# turn off xlate
ifdef(`_POWER_PC',`isync',)

#	restore the final state (with xlate off) and rfi

	l	r0, ppda_save0(r1)	# get sr0
	l	r2, ppda_save1(r1)	# get sr2
	mtsr	sr0, r0			# restore seg regs
	mtsr	sr2, r2

	l	r0, ppda_save2(r1)	# get iar
	l	r2, ppda_save3(r1)	# get msr
	mtspr	SRR0, r0		# load for rfi
	mtspr	SRR1, r2		# load for rfi

	l	r0, ppda_save4(r1)	# restore r0
	l	r2, ppda_save6(r1)	# restore r2
	l	r1, ppda_save5(r1)	# restore r1

	rfi				# return

ifdef(`_POWER_PC',`
#******************************************************************************
#
# NAME: bat_save
#
# FUNCTION:
#	Save bat registers to mstsave.  Only bats 0-2 are saved
#
# NOTES:
#	non-standard linkage
#
# EXECUTION ENVIORNMENT:
#	called only from state save.
#	Only called for Power PC (not 601)
#
# REGISTERS:
#	INPTUT:
#		r1 - mstsave
#	USED:
#		r12-r14
#
#******************************************************************************

bat_save:
	mfdbatu	r12, 0
	mfdbatl	r13, 0
	mfdbatu	r14, 1

	st	r12, mstdbat+0*4(r1)
	st	r13, mstdbat+1*4(r1)
	st	r14, mstdbat+2*4(r1)

	mfdbatl	r12, 1
	mfdbatu	r13, 2
	mfdbatl	r14, 2

	st	r12, mstdbat+3*4(r1)
	st	r13, mstdbat+4*4(r1)
	st	r14, mstdbat+5*4(r1)

	b	bat_save_ret

#******************************************************************************
#
# NAME: bat_restore
#
# FUNCTION:
#	restore bat registers from mstsave.  Only bats 0-2 are restored
#
# NOTES:
#	non-standard linkage
#
# EXECUTION ENVIORNMENT:
#	called only from resume.
#	Only called for Power PC (not 601)
#
# REGISTERS:
#	INPTUT:
#		INTR_MST - mstsave
#	USED:
#		r10-r12
#
#******************************************************************************

bat_restore:
	l	r10, mstdbat+0*4(INTR_MST)
	l	r11, mstdbat+1*4(INTR_MST)
	l	r12, mstdbat+2*4(INTR_MST)

	mtdbatu	0, r10
	mtdbatl	0, r11
	mtdbatu	1, r12

	l	r10, mstdbat+3*4(INTR_MST)
	l	r11, mstdbat+4*4(INTR_MST)
	l	r12, mstdbat+5*4(INTR_MST)

	mtdbatl	1, r10
	mtdbatu	2, r11
	mtdbatl	2, r12

	b	bat_restore_ret
')

#******************************************************************************
#
# NAME: check_sigslih
#
# FUNCTION:
#	Checks to see if the thread should be terminated, suspended, swapped
# out or have a signal delivered (in short if sigslih should be called). If
# it does then this function does not return.
#
# NOTES:
#	non-standard linkage
#
# EXECUTION ENVIRONMENT:
#	called from resume (only for threads returning to user mode)
#
# REGISTER:
#	INPUT:
#		r14 - mst to be resumed (INTR_MST)
#		r15 - ppda address (PPDA_ADDR)
#		r30 - curthread pointer
#		r31 - curproc pointer
#		cr2(eq) - ! must be preserved !
#	USED:
#		r10,r11,r12,cr0,cr1
#
#*******************************************************************************

check_sigslih:
	l	r11, p_int(r31)		# get process level flags
	l	r12, t_flags(r30)	# get thread level flags
	rlinm.	r10, r11, 0, SSIGSLIH	# check process level
	cror	cr1*4+eq, cr0*4+eq, cr0*4+eq # !SSIGSLIH
	rlinm.	r10, r12, 0, TSIGSLIH	# check thread level
	crand	cr1*4+eq, cr1*4+eq, cr0*4+eq # !SSIGSLIH && !TSIGSLIH
	beqr	cr1			# return if negative

	b	ENTRY(call_sigslih)
	.extern	ENTRY(call_sigslih)

#*******************************************************************************
#
# NAME: resume_trace
#
# FUNCTION:
#	Called when trace is enabled to do resume trace hook.  This is really
# part of resume(), but is here to avoid bringing it into the cache when trace
# is not enabled
#
# NOTES:
#	non-standard linkage
#
# REGISTERS:
#	INPUT
#		r2 - kernel TOC
#		r14 - mst to be resumed (INTR_MST)
#		r15 - ppda address (PPDA_ADDR)
#	USED
#		all volatile registers
#
#*******************************************************************************

resume_trace:
	l	r12, mstprev(INTR_MST)
	cmpi	cr0, r12, NULL
	crnor	0, cr0*4+eq, cr0*4+eq	# CR bit 0 == 1	=> not equal
	mfcr	r4			# Fetch	results	of comparison
	rlinm	r3, r4, 1, 0x00000001	# Isolate proc/not proc	bit in low end

#	Call the trace routine,	specifying:
#	hookword	: hook ID | proc/not proc
#	iar		: iar to resume
# these	aren't traced now:
#	msr		: msr to resume
#	priorities	: priority/backtrack to	resume
#	curproc		: current process
#	mst		: mst to resume

	cau	r3, r3,	(HKWD_KERN_RESUME | HKTY_LT)>16
	l	r4, mstiar(INTR_MST)
ifdef(`MLDEBUG',`
	l	r5, mstmsr(INTR_MST)
	lhz	r6, mstintpri(INTR_MST)
	l	r7, ppda_curthread(PPDA_ADDR)
	mr	r8, INTR_MST
	bl	ENTRY(mltrace)
	.extern	ENTRY(mltrace)
',`
	bl	ENTRY(trchook)		 # Call	the trace routine
       .extern	ENTRY(trchook)
')
	b	resume_trace_ret



	.toc
	TOCE(i_data, data)

include(low_dsect.m4)
include(scrs.m4)
include(proc.m4)
include(machine.m4)
include(mstsave.m4)
include(param.m4)
include(trchkid.m4)
include(systemcfg.m4)
include(intr.m4)
include(m_types.m4)
include(flihs.m4)
include(user.m4)
include(dispauth.m4)
