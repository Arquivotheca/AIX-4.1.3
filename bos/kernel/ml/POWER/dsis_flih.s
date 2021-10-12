# @(#)29	1.2.1.15  src/bos/kernel/ml/POWER/dsis_flih.s, sysml, bos41J, 9522A_all 5/30/95 18:12:42
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS: ds_flih, is_flih, dsis_flih
#
#   ORIGINS: 27, 83
#
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

	.file "dsis_flih.s"
	.machine "com"
	.using low, r0

include(macros.m4)
	
#*******************************************************************************
#
# NAME:	dsis_flih
#
# FUNCTION:
#	There is one routine to	service	both ISIs and DSIs -- the
#	dsis_flih.  The	first thing the	dsi_flih must do is check
#	for dsi	in the alignment check handler.	 If this has
#	occurred all registers changed by the alignment	check
#	handler	are reloaded from the ppda.  This makes	the DSI
#	always restart on the instruction that generated the
#	alignment interrupt.  Note that	the alignment flih uses	the
#	DAR after turning translation on, so this order	is a
#	requirement.
#
#	The flih has a special path for	PTE group overflow.  The
#	flih does a partial state save to the ppda.  The dssave	area
#	is used	for the	partial	state save.  The dsis flih can not
#	alter any registers or fields in the ppda, since this
#	portion	of the flih can	interrupt ANY code with	that runs
#	with xlate on (state_save(), dsis_flih(), etc.).  The
#	reservation is not cleared by the flih (check_overflow should
#	clear the reservation).	 If a translation fault	occurred and
#	a page table overflow has occurred at some point then the flih
#	calls v_reload().  v_reload() is called in real mode.
#	No run-time stack is provided and only a limited set of
#	registers are available	(r15-ppda, r3 -	for return code).
#	v_reload() returns 0 if it handled the exception. The
#	pte overflow path does an in-line resume.  This	does not
#	return through finish_interrupt().
#
#	The DSISR and DAR must be save to the PPDA before calling
#	state_save() (state save turns xlate on), since	a PTE
#	overflow interrupt could occur during state_save() and alter
#	their values.
#
#	The dsis_flih also contains code for the Power compare-and-
#	swap system call.  The cs() call has a different
#	implementation on Power	and Power PC.  The Power version
#	relies on the DSI handler to restart the system	call if	a
#	page fault occurs.  The	iar is moved back to handler must
#	enforce	the problem state setting of the MSR(EE	PR DR IR ME
#	FP IP IE) bits as an application could bypass the SVC
#	instruction and	branch directly	into the kernel	code.
#
#	The logic for handling data address breakpoints	is in the
#	debugger.  The flih checks for a DABR hit and passes control
#	to the debugger.  The check is done after fixing up a DSI in
#	the alignment check handler.  This makes DABR exceptions
#	that occurred in the alignment check handler appear to have
#	been caused by the misaligned instruction.  This assumes
#	that DABR exceptions can be restarted without examining	the
#	instruction that generated the exception.  All logic to
#	handle the DABR	must be	in the debugger.  The debugger
#	returns	0 if the exception was processed by the	debugger as
#	a DABR exception.  If the debugger returns a non-zero value,
#	the flih treats	the exception as a page	fault and passes the
#	page fault handler control.  A second reason for calling the
#	debugger here is to allow the DABR's use in VMM	critical
#	sections (avoid	back-tracking).
#
#	For non-direct store segments the ds_flih() runs the page
#	fault handler at INTMAX	unless the faulter was at INTBASE or
#	is a thread at INTMAX.	In this	case the page fault
#	handler	is run at INTPAGER.  IO	exception handling is
#	processed in io_exception().  io_exception() is	always
#	entered	at INTMAX.
#
#       A graphics thread accesses a processor bus display with a
#       segment register of value T = 0, Ks = Kp = 1, SID = 0x7FFFFE will
#       take a DSI.  dsis_flih() will fill out the except[] structure in the
#       faulting mstsave and call p_slih().
#
#	These functions are written in intersection assembler for all
#	architectures.
#
#	ds_flih()
#	{
#		dsis_flih(DS_LEVEL, mfdsisr(), mfdar());
#	}
#
#	is_flih()
#	{
#		dsis_flih(IS_LEVEL, mfsrr1(), mfsrr0());
#	}
#
#	dsis_flih(
#		int flag,
#		int dsisr,
#		int dar)
#	{
#		short pri_back;
#		uint  srval;                    /* fault srval */ 
#		uint  feaddr;                   /* fault eaddr */
#		struct mstsave *new_mst;	/* mst currently executing on */
#		struct mstsave *intr_mst;	/* new_mst->prev */
#		struct mstsave *fault_mst;	/* mst where fault occurred */
#		struct ppd *ppdp;	        /* ppda	for this processor */
#		
#
#		/* partial state save
#		 */
#		ppdp->dssave[0..7] = r1..r8;
#
#		/* This	must be	done before checking for PTE overflow, so that
#		 * a dsi never returns to the alignment	check handler.	If the
#		 * fault happened in the alignment check handler, restore the
#		 * registers changed by	the al_flih
#		 */
#		if (flag == DS_LEVEL &&	SRR0 > alignlo && SRR0 < alignhi)
#		{
#			SRR0,SRR1,lr,cr,xer = ppdp->al_save[0..4];
#			r25..r31 = ppdp->as_save[5..15];
#		}
#
#		if ((dsisr & DSIR_PFT))
#		{
#			if (v_reload() == 0)
#			{
#				r1..r8 = ppdp->dssave[0..7];
#				rfi();
#			}
#		}
#
#		/* save	before turning on xlate	in state_save()
#		 */
#		ppdp->dsisr = dsisr;
#		ppdp->dar = dar;
#		ppdp->dsi_flag = flag;
#
#		/* restore partial state
#		 */
#		r1..r8 = ppdp->dssave[0..7];
#
#		state_save();
#
#		/* Save	faulting in current mst
#		 */
#		new_mst	= ppdp->csa;
#		intr_mst = fault_mst = new_mst->prev;
#
#		/* get faulting eaddr from ppda and faulting srval from mst
#		 */
#		srval =	fault_mst->as.srval[ppdp->dar>>28];
#               feaddr =  ppdp->dar;
#
#		if (flag == DS_LEVEL)
#		{
#			/* If fault was	in the compare-and-swap	fast svc then
#			 * make	it appear that the fault occurred on the svc
#			 * instruction.	 Also set the faulting msr to problem
#			 * state
#			 */
#			if (fault_mst->iar >= cslo_pwr &&
#						fault_mst->iar <= cshi_pwr)
#			{
#				fault_mst->iar = .cs;
#				disown_fp(ppdp->curthread->t_tid);
#				fault_mst->msr = (fault_mst->ctr & MSR_AL) |
#					MSR_PR|MSR_EE|MSR_DR|MSR_IR|MSR_ME;
#
#
#			}
#
#                       /* handle exceptions taken on accessing processor bus
#                        * displays or bat displays for graphics threads
#                        */
#                       if ((srval | KPBIT) == INV_PB_SEGREG_PPC)
#			{
#                           goto call_p_slih(feaddr,faulting EA,
#                 		        EXCEPT_GRAPHICS_SID,srval,dsisr);
#			}	
#
#			/* handle T = 1 IO exceptions 
#			 */
#			if (ppdp->dsisr	& DSISR_IO)
#				goto call_io_exception;
#
#                       /* handle ecowx/eciwx with invalid EAR
#                        */
#			if (ppdp->dsisr	& DSISR_EAR)
#		        {
#			    goto call_p_slih(ear,feaddr,EXCEPT_INVAL_EAR,
#				        srval,dsisr);
#			}
#
#                       /* handle DABR exception 
#                        */
#			if (ppdp->dsisr	& DSISR_DABR)
#			{
#				if (debugger(fault_mst,	DBG_DABR, 0) ==	0)
#					finish_interrupt();
#			}
#		}
#
#		/* Determine the priority to run the page fault	handler	at.
#		 * Also	mark this as an	original page fault
#		 */
#		if ( (fault_mst->intpri	>= INTPAGER) ||
#			((fault_mst->intpri == INTMAX) &&
#			(fault_mst->prev == NULL)) )
#		{
#			pri_back = (INTPAGER <<	8)|ORGPFAULT;
#		}
#		else
#		{
#			pri_back = (INTMAX << 8)|ORGPFAULT;
#		}
#
#		/* Check for a back track page fault
#		 */
#		if (fault_mst->backt !=	0)
#		{
#			/* increment count of system back tracks
#			 */
#	#ifdef _POWER_MP
#			ATOMIC_INC(vminfo.backtrks++);
#	#else
#			vminfo.backtracs++;
#	#endif
#			/* free	mst.  new_mst and intr_mst are adjusted	to point
#			 * to new mst frames
#			 */
#			new_mstp = intr_mst;
#			ppdp->mstack = ppdp->csa;
#			ppdp->csa = intr_mst;
#			intr_mst = intr_mst->prev;
#			pri_back = (INTPAGER <<	8)|BKTPFAULT;
#		}
#
#		begin_interrupt(ppdp->dsi_flag,	pri_back);
#
#		/* gain	addressability to VMM data structures
#		 */
#		mtsr(VMSR, vmker.vmsrval);
#		mtsr(PTASR, vmker.ptasr);
#		isync();
#
#		/* Check if this is an original	fault or a back	tracking fault.
#		 * and do the appropriate error	logging.  Note that this logic
#		 * does	not store into the mst the page	fault handler is to be
#		 * run on
#		 */
#		if (fault_mst->backt ==	0)
#		{
#			fault_mst->except[0] = ppdp->dar;
#			fault_mst->except[1] = ppdp->dsisr;
#			fault_mst->except[2] = srval;
#			fault_mst->except[3] = ppdp->dar;
#			fault_mst->except[4] = (flag == DS_FLIH) ? EXCEPT_DSI : EXCEPT_ISI;
#		}
#		else
#		{
#			intr_mst->except[1] = DSISR;
#			intr_mst->except[2] = srval;
#			intr_mst->except[3] = ppdp->dar;
#		}
#
#		/* Turn	on EE bit if not at INTMAX
#		 */
#		if (new_mstp->intpri !=	INTMAX)
#			MSR_EE = 1;
#
#		/* Call	vmm to process page fault
#		 */
#		if (flag == DS_FLIH)
#		{
#			v_dpfget(ppdp->dar, srval, fault_mst, ppdp->dsisr);
#		}
#		else
#		{
#			v_ipfget(ppdp->dar, srval, fault_mst, ppdp->dsisr);
#		}
#
#		finish_interrupt();
#
#	call_io_exception:
#		begin_interrupt(DS_LEVEL, INTMAX<<8);
#		io_exception(ppdp->dar,	intr_mst->as.srval[DAR>>28],
#					fault_mst, ppdp->dsisr);
#		finish_interrupt();
#	}
#
#*******************************************************************************

	.csect	dsis_flih_sect[PR]
DATA(is_flih):					# ISI entry point
	.globl	DATA(is_flih)
ENTRY(is_flih):
	.globl	ENTRY(is_flih)
	st	r1, ppda_dssave0(PPDA_ADDR)	# save registers
	st	r2, ppda_dssave1(PPDA_ADDR)
	st	r7, ppda_dssave6(PPDA_ADDR)
	st	r8, ppda_dssave7(PPDA_ADDR)
	lil	r1, IS_LEVEL			# load flag parameter
	mfspr	r7, SRR0			# address of fault
	mfspr	r8, SRR1			# status register
	b	dsis_flih			# branch to common flih

DATA(ds_flih):					# DSI entry point
	.globl	DATA(ds_flih)
ENTRY(ds_flih):
	.globl	ENTRY(ds_flih)
	st	r1, ppda_dssave0(PPDA_ADDR)	# save a register
	st	r2, ppda_dssave1(PPDA_ADDR)
	st	r7, ppda_dssave6(PPDA_ADDR)
	st	r8, ppda_dssave7(PPDA_ADDR)
	lil	r1, DS_LEVEL			# load flag parameter
	mfspr	r7, DAR				# address of fault
	mfspr	r8, DSISR			# status register

dsis_flih:
	st	r3, ppda_dssave2(PPDA_ADDR)
	st	r4, ppda_dssave3(PPDA_ADDR)
	st	r5, ppda_dssave4(PPDA_ADDR)
	st	r6, ppda_dssave5(PPDA_ADDR)

	l	r2, DATA(g_toc)			# get kernel TOC
	mfcr	r6				# save condition register
	mfspr	r3, SRR0			# get IAR of fault
	LTOC(r4, alignlo, data)			# get start of al_flih
	LTOC(r5, alignhi, data)			# get end of al_flih

	cmpl	cr2, r3, r4			# compare IAR to alignlo
	cmpl	cr3, r3, r5			# compare IAR to alignhi
	cmpi	cr1, r1, DS_LEVEL		# check	for DSI	fault
	crand	cr2*4+gt, cr2*4+gt, cr3*4+lt	# iar >	alignlo	&& iar < alignhi
	crand	cr2*4+gt, cr2*4+gt, cr1*4+eq	#   && flag == DS_LEVEL
	bgt	cr2, fix_al_fault		# branch if fault in al_flih
fix_al_fault_ret:

# Determine if reload handling is required -- if this fault is due to a
# missing translation then perform reload checks

	andiu.	r3, r8,	DSISR_PFT>16		# check	for no translation
	bne	cr0, check_overflow		# continue if other exception

check_overflow_ret:

# Reload handling returns here if a reload was not performed

	st	r0, ppda_save0(PPDA_ADDR)
	st	r8, ppda_dsisr(PPDA_ADDR)	# save DSISR before xlate is on
	st	r1, ppda_dsi_flag(PPDA_ADDR)	# save interrupt type
	st	r7, ppda_dar(PPDA_ADDR)		# save DAR

	mtcr	r6				# restore condition register
	l	r1, ppda_dssave0(PPDA_ADDR)	# restore registers saved
	l	r2, ppda_dssave1(PPDA_ADDR)
	l	r3, ppda_dssave2(PPDA_ADDR)
	l	r4, ppda_dssave3(PPDA_ADDR)
	l	r5, ppda_dssave4(PPDA_ADDR)
	l	r6, ppda_dssave5(PPDA_ADDR)
	l	r7, ppda_dssave6(PPDA_ADDR)
	l	r8, ppda_dssave7(PPDA_ADDR)

	mflr	r0				# do full state	save
	bl	ENTRY(state_save)
	.extern	ENTRY(state_save)

# REGISTER USAGE:
#	cr3(eq)	1 - fault_mst->backt ==	0, 0 - fault_mst->backt	!= 0
#	cr4(eq)	1 - ISI, 0 - DSI
#	r13 = NEW_MST (csa)
#	r14 = INTER_MST	(csa->prev)
#	r15 = PPDA_ADDR	(ppda pointer)
#	r16-r18 = scratch registers
#       r19 = faulting eaddr
#       r20 = faulting segment register vaule
#	r21-r25	= scratch registers
#	r28 = faulting intpri
#	r29 = fault_mst	(csa at	time of	fault)
#	r30 = DS_LEVEL - DSI, IS_LEVEL - ISI
#	r31 = DSISR

	l	r30, ppda_dsi_flag(PPDA_ADDR)	# get type of interrupt
	l	r31, ppda_dsisr(PPDA_ADDR)	# get saved DSISR
	cmpi	cr4, r30, IS_LEVEL		# check	for DSI/ISI
	mr	r29, INTR_MST			# csa at time of fault
	lbz	r28, mstintpri(INTR_MST)	# faulting intpri

	l	r19, ppda_dar(PPDA_ADDR)	# get fault address
	cal	r21, mstsr(INTR_MST)		# address of segment registers
	rlinm	r22, r19, 6, 0x3c		# segreg num * 4
	lx	r20, r21, r22			# get SR value

	cmpli	cr2, r28, INTPAGER		# check faulting intpri

	lil	r27, EXCEPT_ISI			# preset flag for ISI
	beq	cr4, inst_fault			# branch if ISI

	l	r21, mstiar(r29)		# get faulting IAR
	lil	r27, EXCEPT_DSI			# set flag for type

#	check for fault	in cs()

	cmpli	cr6, r21, cslo_pwr-1		# compare iar to start of cs()
	cmpli	cr7, r21, cshi_pwr+1		# compare iar to end of	cs()
	crand	cr6*4+gt, cr6*4+gt, cr7*4+lt	# iar >= cslo && iar <=	cshi
	bgtl	cr6, fix_csfault		# branch if fault in cs()
						#  (volatile regs,r16,r17)
						#  (cr not changed)

#       handle exceptions taken on accessing processor bus displays
#       for graphics threads

        cau     r8,0,INV_PB_SEGREG_PPC.h
        oriu    r9,r20,KPBIT             	# r9 = fault srval with Kp set
        oril    r8,r8,INV_PB_SEGREG_PPC.l 	# r8 = special segid (0x607FFFFE)
	mr	r3,r19
	mr	r4,r19
        cmpl    cr0,r9,r8
        cal     r5,EXCEPT_GRAPHICS_SID(0)   	# r5 = EXCEPT_GRAPHICS_SID
	mr	r6,r20

	# r3 = faulting eaddr, r4 = faulting eaddr, r5 = EXCEPT_GRAPHICS_SID
	# r6 = faulting sreg, r31 = dsisr
        beq-    call_p_slih             	# branch if special graphics exception

	# handle DABR, io_exception, or EAR type DSI
	andiu.	r0,r31,((DSISR_DABR | DSISR_IO | DSISR_EAR)>16)&0xffff
	bnel-	cr0, call_dabr_io_ear           #  (volatile regs,r16)

#	compute	interrupt priority for page fault handler

inst_fault:
	lbz	r21, mstbackt(r29)		# get faulting back track state
	lil	r4, INTPAGER*256|ORGPFAULT	# priority normal page faults
	cmpi	cr3, r21, 0			# check	for back track fault
	LTOC(r21, vmker, data)
	bltl	cr2, not_int_pager		# branch if intpri < INTPAGER
						#   (cr0,cr1,r4,r16)
	l	r22, vmmsrval(r21)		# get VMM sr value
	l	r23, ptasrval(r21)		# get PTA sr value
	bnel	cr3, back_track			# branch if backtracking fault
						#   scratch
						#	r0,cr0,r3
						#   changes
						#	r1,r4,NEW_MST, INTR_MST

	mtsr	vmmsr, r22			# VMM segments
	mtsr	ptasr, r23
DATA(isync_dsis):				# patched at SI	time
	.globl	DATA(isync_dsis)
	isync

	mr	r3, r30				# r3 = level, r4 - new priority
	bl	ENTRY(begin_interrupt)		# (volatile, r16-r18)
	.extern	ENTRY(begin_interrupt)

	mr	r3,r19                          # faulting address
	mr	r4,r20                          # faulting SR
	lbz	r25, mstintpri(NEW_MST)		# get new interrupt priority
	lil     r7, 0x1				# set backtrack parameter r7,
						# reset below if not backtrack
	cmpi	cr0, r25, INTMAX		# check	for INTMAX

	bne	cr3, backt_fault		# branch if backt track	fault
	lil     r7, 0x0				# reset backtrack parameter r7
	st	r3, except(r29)			# save DAR
	st	r27,except4(r29) 		# set exception type
# fault_mst and	INTR_MST are same if we	did not	back track
						
backt_fault:
	mfmsr	r0				# get msr for enable
	st	r31, except1(INTR_MST)		# DSISR	value
	st	r4, except2(INTR_MST)		# seg reg value
	st	r3, except3(INTR_MST)		# DAR value

	beq	cr0, run_intmax			# branch if running at INTMAX
	oril	r0, r0,	MSR_EE			# turn on EE
run_intmax:

# v_dpfget/v_ipfget
#	r3 - DAR, r4 - srval, r5 - fault_mst, r6 - dsisr, r7 - backtrack case

	mr	r5, r29				# fault_mst
	mr	r6, r31				# DSISR
	mtmsr	r0				# enable interrupts if needed
	beq	cr4, call_ipfget		# branch if ISI
	bl	ENTRY(v_dpfget)			# call slih
	.extern	ENTRY(v_dpfget)
	b	ENTRY(finish_interrupt)
	.extern	ENTRY(finish_interrupt)

call_ipfget:
	bl	ENTRY(v_ipfget)			# call slih
	.extern	ENTRY(v_ipfget)
	b	ENTRY(finish_interrupt)

#*******************************************************************************
#
# NAME:	fix_al_fault
#
# FUNCTION:
#	Called from the	dsis_flih when it has been determined that a fault
# has occurred in the alignment	check flih.  This function reloads the
# registers altered by the al_flih.  This makes	it appear that the fault
# was caused by	the instruction	that generated the alignment check
#
# EXECUTION ENVIRONMENT:
#	Internal function of disi_flih
#
# NOTES:
#	This is never called on a ISI fault
#
# REGISTERS:
#	INPUT:
#		r15 - PPDA_ADDR
#	OUTPUT:
#		SSR0,SRR1,lr,cr(contained in r6),xer,r27..r31 reloaded from ppda
#		No other registers altered
#
#*******************************************************************************
	
fix_al_fault:
	lm	r27, SAVE_SRR0(PPDA_ADDR)	# load misc. registers
	mtspr	SRR0, r27			# restore misc.	registers
	mtspr	SRR1, r28
	mtlr	r29
	mr	r6, r30				# update saved cr value
	mtxer	r31
	lm	r25, SAVE_R25(PPDA_ADDR)	# reload gprs
	b	fix_al_fault_ret		# return

#*******************************************************************************
#
# NAME:	check_overflow
#
# FUNCTION:
#	Glue code between dsis_flih and	v_reload().  Calls v_reload
# and checks the return	code.  If the return code is zero gprs are
# restored and a rfi is	issued.
#
# EXECUTION ENVIRONMENT:
#	Internal function of dsis_flih
#	Translate off, interrupts disabled
#
# SAVED STATE
#
#       r1-r8   in ppda_dssave[0-7]
#       r15     in ra_save (PWR) or SPRG1 (PPC)
#       cr      in r6
#
# REGISTERS USED
#
#       INPUT:
#               r2 - kernel TOC
#               r6 - saved cr value
#               r7 - DAR/SRR0
#               r15 - PPDA_ADDR
#
#       AVAILABLE:
#               cr, r3, r4, r5
#
#******************************************************************************

DATA(overflow):
        .globl  DATA(overflow)

check_overflow:
	l	r3,syscfg_arch(0)		# get architecture
	l	r5,syscfg_impl(0)		# get implementation
	cmpi	cr3,r3,POWER_PC			# check for Power PC
	andil.	r5,r5,POWER_RS_ALL|POWER_601	# check for machines having MQ
	cror	cr2*4+eq,cr0*4+eq,cr0*4+eq	# move MQ check to cr2
	bne	cr3,no_lock

# first clear any reservation
        cal     r5,syscfg_arch(0)               # get architecture addr (e.g.)
	.machine "ppc"
        stwcx.  r3, 0, r5                       # to clear reservation
	.machine "com"				# even on UP

# Set up 'C' environment by saving registers in reload save area

no_lock:
	l	r3,ppda_pmapstk(r15)		# r3 -> V=R reload stack
	st 	r0,-32*4(r3)			# save GPR0
	st 	r1,-31*4(r3)			# save GPR1
	stm	r6,-26*4(r3)			# save GPR6-31
ifdef(`_KDB',`
	l	r29,ppda_dssave0(r15)
	bne	cr3,noppc_1
	.machine "ppc"				# SPRG1 is used by break-points
	mfspr	r25, SPRG1
	.machine "com"
noppc_1:
')
	cal	r1,-stkmin-32*4(r3)		# set stack pointer
	mfspr	r26,SRR0			# save SRR0, SRR1 in order to
	mfspr	r27,SRR1			#   set breakpoints later on
	mflr	r28				# save LR
ifdef(`_KDB',`
	st 	r29,stkback(r1)			# set back-chain for debug
')
	mfctr	r29				# save CTR
	mfxer	r30				# save XER
	beq	cr2,no_mq_save			# branch if no mq
	.machine "any"
	mfmq	r31				# save MQ
	.machine "com"

no_mq_save:
	beq	cr3,mfsri_ppc			# branch if Power PC
	.machine "pwr"
	mfsri	r3,r0,r7			# get sreg value via DAR on PWR
	.machine "com"
	b	mfsri_done

mfsri_ppc:
	.machine "ppc"
 	mfsrin	r3,r7				# get sreg value via DAR on PPC
	.machine "com"

# Call 'C' routine to determine if fault is a valid reload fault.
#
# rc = v_reload(sid,pno)

mfsri_done:
	rlinm	r3,r3,0,0x00ffffff		# set sid parameter
	rlinm	r4,r7,20,0x0000ffff		# set pno parameter from DAR
	bl	ENTRY(v_reload)			# call 'C' reload handler
	.extern	ENTRY(v_reload)

	cmpi	cr4, r3, 0			# check	return code
	mtspr	SRR0, r26			# restore SRR0
	mtspr	SRR1, r27			# restore SRR1
ifdef(`_KDB',`
	bne	cr3,noppc_2
	.machine "ppc"
	mtspr	SPRG1, r25			# restore SPRG1 on PPC
	.machine "com"
noppc_2:
')
	mtlr	r28				# restore LR
	mtctr	r29				# restore CTR
	mtxer	r30				# restore XER
	beq	cr2,no_mq_rest			# branch if no mq
	.machine "any"
	mtmq	r31				# restore MQ
	.machine "com"

no_mq_rest:
	lm	r6,6*4+stkmin(r1)		# restore GPR6-31
	l	r0,0*4+stkmin(r1)		# restore GPR0
	l	r1,1*4+stkmin(r1)		# restore GPR1

	bne	cr4, check_overflow_ret		# return to flih if no reload

# Reload was performed so restore state and resume from here

	mr	r8, r15				# r8->PPDA
	beq	cr3, r15_ppc
	l	r15, ra_save(0)			# restore r15 on PWR
	b	r15_done

r15_ppc:
	.machine "ppc"
	mfspr	r15, SPRG1			# restore r15 on PPC
	.machine "com"

r15_done:
	mtcr	r6				# restore cr
	l	r1, ppda_dssave0(r8)		# restore r1-r8
	l	r2, ppda_dssave1(r8)
	l	r3, ppda_dssave2(r8)
	l	r4, ppda_dssave3(r8)
	l	r5, ppda_dssave4(r8)
	l	r6, ppda_dssave5(r8)
	l	r7, ppda_dssave6(r8)
	l	r8, ppda_dssave7(r8)
DATA(dsis_nop):					# for 603e 1.4 workaround
	.globl	DATA(dsis_nop)			# patched to sync at si time
	sync
	rfi					# DSI done

#*******************************************************************************
#
# NAME:	call_io_exception
#
# FUNCTION:
#	Called from dsis_flih for DSI to T=1 space.  This routine simply
# calls	the PIO	exception slih -- io_exception().
#
# REGISTERS:
#	INPUT:
#		r1 - stack
#		r2 - kernel toc
#		r13 - NEW_MST current mst
#		r14 - INTR_MST faulting	mst
#		r15 - PPDA_ADDR
#		r19 - faulting eaddr
#               r20 - faulting srval	
#
#*******************************************************************************

call_io_exception:
	lil	r3, DS_LEVEL		# Interrupt level 3 for	0x300
	lil	r4, INTMAX*256		# Interrupt priority
	bl	ENTRY(begin_interrupt)  # (volatile, r16-r18)

	mr	r3, r19
	mr	r4, r20                 # faulting sreg value 
	mr	r5, INTR_MST		# faulting mst
	l	r6, ppda_dsisr(PPDA_ADDR) # get	DSISR
	bl	ENTRY(io_exception)	# call io exception handler
	.extern	ENTRY(io_exception)

	b	ENTRY(finish_interrupt)	# resume thread


#*******************************************************************************
#
# NAME: call_p_slih
#
# FUNCTION:
#       Called from dsis_flih for exceptions caused by ecowx/eciwx with an 
#       invalid EAR or by accessing displays on the processor bus.
#       This routine simply fills out the exception structure and calls p_slih
#
# REGISTERS:
#       INPUT:
#               r1 - stack
#               r2 - kernel toc
#               r13 - NEW_MST current mst
#               r14 - INTR_MST faulting mst
#               r15 - PPDA_ADDR
# 		r3 = faulting eaddr or ear content
# 		r4 = faulting eaddr
#		r5 = exception code
# 		r6 = faulting sreg
#               r31 - dsisr register
#
#*******************************************************************************

call_p_slih:
        st      r3,except(INTR_MST)     	# fill out except structure
        st      r31,except1(INTR_MST)
        st      r6,except2(INTR_MST)
        cal     r3,EXCEPT_DSI(0)
        st      r4,except3(INTR_MST)
        st      r3,except4(INTR_MST)
	mr	r31,r5				# save exception code in r31

	lil	r3, DS_LEVEL			# Interrupt level 3 for	0x300
	lil	r4, INTMAX*256			# Interrupt priority
	bl	ENTRY(begin_interrupt)  	# (volatile, r16-r18)
	
        mr      r3,INTR_MST             	# r3 = faulting mstsave
	l	r4,except3(INTR_MST)		# r4 = faulting effective address
	mr	r5,r31				# r5 = exception code
	l	r6, ppda_curthread(PPDA_ADDR)	# r6 = thread pointer
        bl      ENTRY(p_slih)
        .extern ENTRY(p_slih)

        b       ENTRY(finish_interrupt) 	# resume


#*******************************************************************************
#
# NAME:	fix_csfault
#
# FUNCTION:
#	Called when a DSI occurred in the cs() fast SVC.  This only occurs
# on Power machines.  This function makes the DSI appear to have been caused
# by the first instruction in cs().
#
# EXECUTION ENVIRONMENT:
#	Internal routine called	for dsis_flih (DSI only)
#
# REGISTERS:
#	INPUT:
#		r2 - kernel TOC
#		r14 - INTR_MST faulting	mst
#		r15 - PPDA_ADDR	address	of PPDA
#	USED:
#		volatile registers
#		r16
#		r17
#		cr - preserved
#
#*******************************************************************************

fix_csfault:

	mfcr	r17				# save condion register
	l	r4, ppda_curthread(PPDA_ADDR)	# thread pointer
	l	r3, t_tid(r4)			# tid of current thread
	mflr	r16				# save link register
	bl	ENTRY(disown_fp)		# force	FP bits	in MSR to known
	.extern	ENTRY(disown_fp)		#  state
	mtlr	r16

	LTOC(r3, cs, data)			# function descriptor
	l	r3, 0(r3)			# entry	point
	st	r3, mstiar(INTR_MST)		# change IAR

	l	r3, mstctr(INTR_MST)		# save MSR

#	Force msr bits to a user mode state

	andil.	r3, r3,	MSR_AL
	oril	r3, r3,	MSR_PR|MSR_EE|MSR_DR|MSR_IR|MSR_ME
	st	r3, mstmsr(INTR_MST)		# update msr
	mtcr	r17				# restore condition register
	br					# return


#*******************************************************************************
#
# NAME:	back_track
#
# FUNCTION:
#	Called when the	faulting mst is	in back	track state.  This function
# frees	the mst	allocated during state save and	initializes the	faulting
# mst to use to	re-execute to page fault handler.  It also sets	up the
# parameter to begin_interrupt()
#
# EXECUTION ENVIRONMENT:
#	Internal function of dsis_flih
#
# REGISTERS:
#	INPUT:
#		r2 - kernel toc
#		r13 - INTR_MST faulting	MST (altered by	this function)
#		r14 - NEW_MST current MST (altered by this function)
#	USED:
#		r1 - new stack pointer
#		r4 - new interrupt priority returned here
#		cr0,r0,r3 - scratch
#
#*******************************************************************************

back_track:
	LTOC(r3, vmminfo, data)			# increment back track count
	cal	r3, backtrks(r3)
	ATOMIC_INC(cr0,	r4, r3)

	st	NEW_MST, ppda_mstack(PPDA_ADDR)	# give back mst
	st	INTR_MST, ppda_csa(PPDA_ADDR)
	CSA_UPDATED(cr0, r4, INTR_MST)

	mr	NEW_MST, INTR_MST
	l	INTR_MST, mstprev(INTR_MST)
	
	liu	r0, NEW_ADSPACE			# initialize adspace
	st	r0, mstsralloc(NEW_MST)
	cal	r1, -stkmin(NEW_MST)		# set new stack pointer
	lil	r0, 0				# clear exception handler
	st	r0, excbranch(NEW_MST)
	st	r0, mstkjmpbuf(NEW_MST)

	lil	r4, INTPAGER*256|BKTPFAULT	# priority for begin_interrupt
	br

#*******************************************************************************
#
# NAME:	call_dabr
#
# FUNCTION:
#	Call the debugger when a DABR interrupt	is enabled.  If	the debugger
# claims the interrupt then we are done
#
# REGISTERS:
#	INPUT:
#		r1 - stack
#		r2 - TOC
#		r13 - NEW_MST
#		r14 - INTR_MST
#		r15 - PPDA_ADDR
#		r19 - faulting eaddr
#               r20 - faulting srval	
#		lr  - return address to dsis_flih
#	ALTERED:
#		all volatile, r16
#
#*******************************************************************************

call_dabr:

	mr	r3, INTR_MST			# faulting mst
	lil	r4, DBG_WATCH			# exception code
	lil	r5, 0

	mflr	r16				# call debugger
	bl	ENTRY(debugger)
	.extern	ENTRY(debugger)
	mtlr	r16

	cmpi	cr0, r3, 0			# check	return code
	bner					# return if non-zero return code
	b	ENTRY(finish_interrupt)		# resume

#*******************************************************************************
#
# NAME: not_int_pager
#
# FUNCTION:
#	Called when the faulting priority is < INTPAGER.  This function
# computes the interrupt priority to run the pager at.  The priority
# is:
#	pri = (fault_mst->prev == NULL && fault_mst->intpri == INTMAX) ?
#		INTPAGER : INTMAX
#
# REGISTERS
#	INPUT:
#		r28 - fault_mst->intpri
#		r29 - fault_mst
#	ALTERS:
#		cr0,cr1
#		r4 - new priority and back track flag loaded here
#		r16 - scratch
#
#*******************************************************************************

not_int_pager:
	l	r16, mstprev(r29)		# fault_mst->mstprev
	cmpi	cr0, r28, INTMAX		# check for fault at INTMAX
	cmpi	cr1, r16, NULL			# check for thread
	crand	cr0*4+eq, cr0*4+eq, cr1*4+eq	# thread && fault at INTMAX
	lil	r4, INTPAGER*256|ORGPFAULT	# INTPAGER return
	beqr					# return if thread at INTMAX
	lil	r4, INTMAX*256|ORGPFAULT	# all other cases
	br


#*******************************************************************************
#
# NAME:	call_dabr_io_ear
#
# FUNCTION:
#	Call the debugger when a DABR interrupt	is enabled.  If	the debugger
# claims the interrupt then we are done
#
# REGISTERS:
#	INPUT:
#		r1 - stack
#		r2 - TOC
#		r13 - NEW_MST
#		r14 - INTR_MST
#		r15 - PPDA_ADDR
#		r19 - faulting eaddr
#               r20 - faulting srval	
#		lr  - return address to dsis_flih
#	ALTERED:
#		all volatile, r16
#
#*******************************************************************************


call_dabr_io_ear:

	# handle T = 1 IO exception
	rlinm.	r0, r31, 0, DSISR_IO		# check for IO exception
	bne+	cr0, call_io_exception		# branch if T=1 DSI

	rlinm.	r0,r31,0,DSISR_DABR		# check for DABR execption
	bne-	cr0,call_dabr			# call debugger	for DABR int
						#  (all volatile,r16)
	# handle exceptions taken when executing ecowx/eciwx with 
	# an invalid EAR

	.machine "any"
	mfear	r3				
	.machine "com"
	mr	r4,r19
        cal     r5,EXCEPT_INVAL_EAR(0)   	# r5 = EXCEPT_INVAL_EAR
	mr	r6,r20

	# r3 = faulting EAR, r4 = faulting eaddr, r5 = EXCEPT_INVAL_EAR
	# r6 = faulting sreg, r31 = dsisr
	b       call_p_slih			# branch if EAR exception

	.toc
	TOCE(alignlo, data)
	TOCE(alignhi, data)
	TOCE(cs, data)
	TOCE(vmker, data)
	TOCE(vmminfo, data)

include(flihs.m4)
include(i_machine.m4)
include(scrs.m4)
include(mstsave.m4)
include(low_dsect.m4)
include(machine.m4)
include(except.m4)
include(vmker.m4)
include(vminfo.m4)
include(systemcfg.m4)
include(param.m4)
include(proc.m4)
include(dbg_codes.m4)
