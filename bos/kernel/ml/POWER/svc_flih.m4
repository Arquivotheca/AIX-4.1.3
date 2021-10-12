# @(#)26	1.3.5.27  src/bos/kernel/ml/POWER/svc_flih.m4, sysml, bos41J, 9515B_all 4/10/95 13:52:29
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


#*******************************************************************************
#
# NAME: svc_instr
#
# FUNCTION: All svc's are resolved to descriptors whose iar points here
#
# DETAIL: When a user-level program is linked, the binder inserts glue code
#	  (glink.s) for each unresolved external call.  When the program is
#	  loaded, the loader resolves these calls by setting an entry in
#	  the user's TOC (setting the address of the out-of-module descriptor)
#	  to point to an entry in the svc table (when it is a system call).
#	  This svc table entry contains a dummy function descriptor which has
#	  the svc_instr as its IAR and the svc table index as its TOC.
#	  So when the user makes the system call, his code branches to the glue
#	  code which loads the address of the descriptor (svc table entry),
#	  gets the iar (svc_instr) and toc value (svc table index), loads
#	  the svc table index into register 2 and branches to here.  Note
#	  that the glue code saves and restores the user's TOC (register 2).
#
# NOTES: This code must reside in non-privileged memory (read access for key 1)
#	 since it executes using the user-level segment register 0.
#
# on entry:
#	r1 contains callers stack pointer
#	r2 - svc number (table index)
#	r3-r10 - parameter registers
#
# on exit to svc flih
#	r1 and r2 unmodified
#	link reg contains return address of caller
#
#*******************************************************************************

# The loader uses svc_instruction to initialize svc table entries so that
# all system calls branch to svc_instr

	.globl DATA(svc_instruction)
DATA(svc_instruction):
	.long ENTRY(svc_instr)

	.org	real0+0x3600	# cache align it

ENTRY(svc_instr):
	crorc	cr1*4+eq, cr1*4+eq, cr1*4+eq # set EQ bit for normal SC
	sc

#*****************************************************************************
#
# NAME: sc_flih
#
# FUNCTION:
#       The SC handler must reside in user-readable memory since it
#       manipulates segment register key-bits.  The 4.1 SC handler
#       saves the user stack pointer and user MSR in the u-block/u-
#       thread-block.  The user mode stack pointer is saved for
#       debugging.  The MSR is saved to allow system calls to easily
#       modify the problem state MSR.
#
# REGISTER USAGE:
#	r31 - scratch
#	r30 - scratch
#	r29 - curthread
#	r28 - curproc
#	r27 - saved cr value
#	r26 - saved link register
#	r25 - csa
#	r24 - u
#	cr2 - trace flag status
#
# PSEUDO CODE:
#       sc_flih(
#               int svc_number,                 /* system call identifier */
#               fast_svc_flag)                  /* cr1(eq) = 1 standard sc */
#       {
#               extern struct ppd *ppdp;        /* ppda for this processor */
#
#               /* branch out early for fast SVCs
#                */
#               if (fast_svc_flag)
#                       fast_svc(svc_num);
#
#               /* get addressability to the kernel.
#                */
#               sr0 = ksrval;
#               sr14 =  kxsrval;
#               sr2 = sr2 & !(Ks|Kp);
#               isync();                        /* patched at SI time */
#
#               /* inc p_suspend count to prevent signals from being delivered
#                * in the kernel
#                */
#               ppdp->csa->ut_error = 0;
#               ppdp->curthread->t_suspend = 1;
#
#
#               /* Stash user stack pointer to make debugging easier.  Also save
#                * the user MSR
#                */
#               ppdp->curthread->t_stackp = r1;
#               ppdp->csa->ut_msr = user_msr;
#		sr1 = ppdp->curthread->t_procp->p_kstackseg;
#		isync();
#               r1 = ppdp->csa->ut_kstack - stkmin;
#
#               init_kernel_adspace();
#
#               /* svc flih entered with EE bit off.  Turn it on before
#		 * executing
#                * system call
#                */
#               enable();
#
#               /* check that the svc number requested is in range of the table
#                */
#               if (svc_number > svc_table_entries)
#                       goto bad_svc;
#
#               if (ut.ut_audsvc) {
#			ut.ut_audsvc->svcnum = 0;
#			if (audit_flag==AUDIT_ON && u.u_auditstat==AUDIT_RESUME)
#				ut.ut_audsvc->svcnum = 1;
#		}
#
#		ppdp->syscall++;
#
#               if (Trconflag[0])
#                       trchook(HKWD_KERN_SVC, &svc_table[svc_number]);
#
#               /* Do system call
#                */
#               rc = (*(svc_table[svc_number]))(p1, p2 ,p3, p4, p5, p6, p7, p8);
#
#               /* audit system call if auditing is enabled
#                */
#               if (ut.ut_audsvc && ut.ut_audsvc->svcnum > 1)
#                       auditscall();
#
#               /* trace results of system call if tracing is enabled
#                */
#               if (Trconflag[0])
#                       trchook(HKDW_KERN_SYSCRET, u.u_error);
#
#
#               user_msr = ppdp->csa->ut_msr;
#               errno_addr = ppdp->csa->ut_errno;
#               errno_val = ppdp->csa->ut_error;
#
#		ASSERT(ppdp->curthread->t_lockcount == 0);
#		ASSERT(ppdp->curthread->t_eventlist == 0);
#
#		if (ppdp->curthread->t_boosted) {
#			ppdp->curthread->t_pri = ppdp->curthread->t_sav_pri;
#			swtch();
#		}
#		
#               disable();
#
#               /* Check if the current thread still owns floating
#		 * point.  Clear all FP related bits from user msr if
#		 * floating point is no longer owned
#                */
#		if (ppdp->fp_owner != ppdp->curthread)
#                       user_msr &= !(MSR_FP|MSR_FE|MSR_IE);
#
#               /* decrement signal suspension count.  Perform signal processing
#                * if required.  call_sigslih does not return if called
#                */
#               ppdp->curthread->t_suspend = 0;
#
#		if ((ppdp->curproc->p_int & SSIGSLIH) ||
#					(ppdp->curthread->t_flags & TSIGINTR))
#		{
#			user_mst(user_stack, user_msr);
#			call_sigslih();
#		}
#
#               /* Load the user address space from u_adspace as system
#		 * calls may have altered the address space, RIDs and
#		 * BATs are also loaded here
#                */
#               load_adspace();
#
#               /* set up display access rights in segment registers
#                * for graphics threads
#                */
#		if (curthread->t_graphics) 
#		{
#		    sr_bat_display_setup();
#		}
#
#               kill_volatile_registers();
#
#               load_msr;
#               svc_epilog(rc);
#       }
#
#
#       The svc_epilog() function is the back-end of the SC flih.
#       It is executed in problem state.  This is also the point
#       that threads resume after receiving a signal or being
#       swapped in.  This code restores the information saved onto
#       the user stack at the SC front end, then returns to the
#       application.
#
#       On entry the following values must be in registers
#          o r1 user stack pointer
#          o r3 SC return code
#
#*******************************************************************************
ENTRY(sc_flih):

#	get kernel values for sr0, sr2, and sr14

	l	r12, DATA(g_ksrval)	# kernel segment register value
	mfsr	r11, sr2		# get process private sr value
	l	r0, DATA(g_kxsrval)	# get kernel extension seg.

	bne	cr1, fast_sc		# branch if it is a fast sc

	mtsr	sr0, r12		# load kernel segment
	rlinm	r11, r11, 0, ~SR_KsKp	# clear Ks and Kp bits
	mtsr	sr14, r0		# load kernel extension seg
	mtsr	sr2, r11		# load process private segment
DATA(isync_sc1):			
	.globl	DATA(isync_sc1)	
	isync				# patched at si time

	GET_PPDA(cr0, r11)		# load PPDA pointer
	l	r12, ppda_csa(r11)	# get csa address
	stm	r24, ut_scsave(r12)	# save registers

	mflr	r26			# save callers link register
	mfcr	r27			# save callers cr
	mfctr	r30			# get user MSR value
	mr	r25, r12
	lil	r31, 0

	l	r29, ppda_curthread(r11)# get curthread pointer
	st	r30, ut_msr(r25)	# save user msr value
	st	r1, t_stackp(r29)	# save user stack pointer
	stb	r31, ut_error(r25)	# clear ut_error
	l	r24, t_userp(r29)	# get user structure address
	l	r28, t_procp(r29)	# get curproc pointer
	l	r30, p_kstackseg(r28)	# get kernel stacks segment value
	mtsr	sr1, r30		# load kernel stacks segment
DATA(isync_sc2):
	.globl	DATA(isync_sc2)
	isync

	lil	r30, 1
	l	r12, ut_kstack(r25)	# get kernel stack address
	sth	r30, t_suspend(r29)	# curthread->t_suspend = 1
ifdef(`_KDB',`
	st	r26, stklink(r1)	# lr
') # _KDB
	cal	r1, -stkmin(r12)	# initialize kernel stack
ifdef(`_KDB',`
	l	r0, t_stackp(r29)
	st	r0, stkback(r1)		# back chain to user stack (kdb)
') # _KDB
	l	r0, DATA(svc_table_entries) # number of entries in svc table
	oril	r12, r31, DEFAULT_MSR	# enable interrupts
	cmpl	cr0, r2, r0		# check the SVC number
	mtmsr	r12			# enable interrupts
	l	r30, DATA(svc_table)	# pointer to SVC table
	lbz	r0, DATA(Trconflag)	# flag is non zero if trace enabled
	bge	cr0, bad_sc_num		# branch if SC number too large
	l	r12, ppda_syscall(r11)	# get count of system calls
	muli	r31, r2, 12		# get address
	addi	r12, r12, 1		# increment count of system calls
	st	r12, ppda_syscall(r11)	# store count of system calls
	cmpi	cr2, r0, 0		# check for non-zero trace flag
	liu	r11, 0xE01E		# new seg reg allocation value
					#    vmm/pfs use 11-13 without alloc
	l	r2, DATA(g_toc)		# load kernel TOC
	st	r11, mstsralloc(r25)	# initialize address space
	lx	r12, r30, r31		# get pointer to SVC function

	bnel	cr2, trace_sc_start	# branch if trace is enabled
					#   (r0, r1, r12,cr0167)

	l	r31, ut_audsvc(r25)	# get auditing structure address
	l	r11, 0(r12)		# get function address
	cmpi	cr7, r31, 0		# check if audit structure exists
	mtctr	r11			# load ctr for SVC branch
	bnel	cr7, audit_sc_start	# if audit structure exists, branch
					#	cr0, cr7, r30, r31
	l	r2, 4(r12)		# load new TOC value

ENTRY(sys_call):
	.globl	ENTRY(sys_call)
	bctrl				# branch to system call

	l	r31, ut_audsvc(r25)	# get audit structure address
	l	r2, DATA(g_toc)(0)	# load kernel toc
	cmpi	cr0, r31, 0		# check if audit structure exists
	mr	r30, r3			# save return value
	bgtl	cr0, audit_sc_end	# audit end of system call if audit
	lil	r31, DISABLED_MSR	# disabled MSR value
	bnel	cr2, trace_sc_end	# branch if tracing is enabled
					#  (volatile, r29)
ifdef(`DEBUG',`
#	ASSERT(ppdp->curthread->t_lockcount == 0)
	lha	r3, t_lockcount(r29)
	twnei	r3, 0
#       ASSERT(ppdp->curthread->t_eventlist ==0)
        l       r3, t_eventl(r29)
        twnei   r3,0
')

	l	r3, t_boosted(r29)	# load boosted indicator
	mtmsr	r31			# disable interrupts
	cmpi	cr0, r3, 0		# check if boosted
	mr	r3, r30			# restore system call return value
	bnel-	sc_unboost_thread

	GET_PPDA(cr0, r30)		# load ppda pointer
	l	r31, ppda_fpowner(r30)	# get current floating point owner

#	Note: all volatile registers need to be used before returning
#	to user mode, to avoid a security hole

	mtlr	r26			# restore caller's lr
	mtcr	r27			# restore caller's cr

	lbz	r4, ut_error(r25)	# get ut_error value
	cmp	cr7, r29, r31		# check if current thread owns fpu
	cmpi	cr6, r4, 0		# check for non zero u.u_error
	lil	r5, 0			# kill r5 if branch not taken
	beq	cr6, no_errno		# branch if u.u_error == 0
	l	r5, ut_errnopp(r25)	# get address of pointer to errno
no_errno:

	l	r11, p_int(r28)		# get process level flags
	l	r12, t_flags(r29)	# get thread level flags
	rlinm.	r11, r11, 0, SSIGSLIH	# check process level
	l	r1, t_stackp(r29)	# get user stack pointer
	l	r6, ut_msr(r25)		# get user msr value
	cror	cr1*4+eq, cr0*4+eq, cr0*4+eq # !SSIGSLIH
	lil	r7, 0

	beq	cr7, svc_fp_owner	# branch if floating point owner
	andil.	r6, r6, (~(MSR_FP|MSR_FE|MSR_IE) & 0xffff)
svc_fp_owner:

	rlinm.	r12, r12, 0, TSIGINTR	# check thread level
	sth	r7, t_suspend(r29)	# t_suspend = 0
	bne	cr1, sc_call_sigslih	# branch if SSIGSLIH
	l	r8, u_adspace_sr+0*4(r24)
	l	r9, u_adspace_sr+1*4(r24)
	l	r0, u_adspace_sr+2*4(r24)
	bne	cr0, sc_call_sigslih	# branch if TSIGINTR

#	The following kills the rest of the volatile GPRs

	l	r10, syscfg_impl(0)		# get implementation
	mtxer	r12				# kill xer
	andil.	r10, r10, POWER_RS_ALL|POWER_601 # machines having a MQ
	st	r12, mstmq(r25)			# kill soft mq
	mtctr	r12				# kill counter
	beq	cr0, no_mq			# branch if no mq
	.machine "any"
	mtmq	r12				# kill mq
	.machine "com"
no_mq:

	mtsr	sr1, r9			# load user text segment

	# load remaining segment registers

	l	r26, u_adspace_sr+3*4(r24)
	l	r27, u_adspace_sr+4*4(r24)
	l	r28, u_adspace_sr+5*4(r24)
	mtsr	sr3, r26
	mtsr	sr4, r27
	mtsr	sr5, r28

	l	r26, u_adspace_sr+6*4(r24)
	l	r27, u_adspace_sr+7*4(r24)
	l	r28, u_adspace_sr+8*4(r24)
	mtsr	sr6, r26
	mtsr	sr7, r27
	mtsr	sr8, r28

	l	r26, u_adspace_sr+9*4(r24)
	l	r27, u_adspace_sr+10*4(r24)
	l	r28, u_adspace_sr+11*4(r24)
	mtsr	sr9, r26
	mtsr	sr10, r27
	mtsr	sr11, r28

	l	r26, u_adspace_sr+12*4(r24)
	l	r27, u_adspace_sr+13*4(r24)
	l	r28, u_adspace_sr+14*4(r24)
	mtsr	sr12, r26
	mtsr	sr13, r27
	l	r26,t_graphics(r29)
	mr	r12, r25
	l	r29, u_adspace_sr+15*4(r24)

	cmpi	cr0,r26,0		# test if it is a graphics thread
	mtsr	sr15, r29

	beq     cr0, DATA(isync_sc3)
        
        mflr    r27     
        bl      sr_bat_display_setup   # branch if a graphics thread
        mtlr    r27

DATA(isync_sc3):
	.globl	DATA(isync_sc3)
	isync

	mtsr	sr14, r28               # restore user kext seg register after isync 
	lm	r24, ut_scsave(r12)     # restore caller's registers
	mtsr	sr0, r8			# load user kernel segment
	mtsr	sr2, r0
	mtmsr	r6			# change to problem state
DATA(isync_sc4):
	.globl	DATA(isync_sc4)
	isync

	beqr	cr6			# errno doesn't have to be updated

ENTRY(update_errno):
	.globl	ENTRY(update_errno)

	l	r5, 0(r5)		# get address of errno
	st	r4, 0(r5)		# update errno
	br

#	Traceback with saves_lr and stores_bc bits set (Needed for kdbx)
	_DF(0x0, 0x1, 0x80, 0x0, 0x0, 0x0, 0x0)

#*******************************************************************************
#
# NAME: audit_svc_start
#
# FUNCTION:
#	Called from sc_flih() when audsvc exists.  This routine check the
# audit_flag and then the value of u.u_auditstat and sets aud_svcnum if
# appropriate.
#
# REGISTERS:
#	INPUT:
#		r24 - u
#		r25 - ut
#	USED:
#		cr0, cr7, r30, r31
#
# EXECUTION ENVIRONMENT:
#	Called only from sc_flih
#
#*******************************************************************************

audit_sc_start:
	l	r31, ut_audsvc(r25)		# get address of audit structure
	lil	r30, 0
	sth	r30, aud_svcnum(r31)		# clear aud_svcnum
	LTOC(r30, audit_flag, data)		# get address of auditing flag
	l	r30, 0(r30)			# get audit flag
	cmpi	cr7, r30, 0			# check if audit flag non zero
	beqr					# return if zero

	l	r30, u_auditstat(r24)		# read u.u_auditstat
	cmpi	cr0, r30, 0			# AUDIT_RESUME=0
	bner					# return if not audit resume
	lil	r30, 1				# set svcnum flag to 1
	sth	r30, aud_svcnum(r31)
	br					# return

#*******************************************************************************
#
# NAME: trace_sc_start
#
# FUNCTION:
#	Called from sc_flih when Trconflag[0] is set.  This routine calls
# the SC trace hook.  It preserves all registers.
#
# REGISTERS:
# 	INPUT:
#		r1 - kernel stack
#		r2 - kernel TOC
#		r12 - svc function descriptor
#		r26 - lr from application
#	USED:
#		r0 - scratch
#		cr0,1,6,7
#
# EXECUTION ENVIRONMENT:
#	Called only for sc_flih. preserves all gprs
#
#*******************************************************************************

trace_sc_start:
	st	r2, -12*4(r1)			# save registers
	st	r3, -11*4(r1)
	st	r4, -10*4(r1)
	st	r5, -9*4(r1)
	st	r6, -8*4(r1)
	st	r7, -7*4(r1)
	st	r8, -6*4(r1)
	st	r9, -5*4(r1)
	st	r10,-4*4(r1)
	st	r11, -3*4(r1)
	st	r12, -2*4(r1)
	st	r13, -1*4(r1)

	mflr	r13				# save lr

	liu	r3, (HKWD_KERN_SVC|HKTY_GT)>16	# SVC start trace id
	mr	r4, r12				# system call pointer
	mr	r5, r26				# link register
	lil	r6,0				# secure next three registers
	lil	r7,0
	lil	r8,0

	bl	ENTRY(trchook)			# call trace hook
	.extern	ENTRY(trchook)

	mtlr	r13				# restore lr

	l	r2, -12*4(r1)			# restore registers
	l	r3, -11*4(r1)
	l	r4, -10*4(r1)
	l	r5, -9*4(r1)
	l	r6, -8*4(r1)
	l	r7, -7*4(r1)
	l	r8, -6*4(r1)
	l	r9, -5*4(r1)
	l	r10,-4*4(r1)
	l	r11, -3*4(r1)
	l	r12, -2*4(r1)
	l	r13, -1*4(r1)

	br					# continue

#*******************************************************************************
#
# NAME: audit_sc_end
#
# FUNCTION:
#	Glue code to auditscall routine.  Called when audsvc exists.
#
# REGISTERS
#	INPUT:
#		r1 - kernel stack
#		r2 - kernel toc
#		r25 - ut
#
# EXECUTION ENVIRONMENT:
#	Called only from SC flih
#
#*******************************************************************************

audit_sc_end:
	st	r31, -4(r1)			# save register
	l	r31, ut_audsvc(r25)		# get audsvc address
	lha	r31, aud_svcnum(r31)		# get aud_svcnum
	cmpi	cr0, r31, 1			# check if auditing is enabled
	l	r31, -4(r1)			# reload reg
	bler					# no, return
	stu	r1, -(stkmin+8)(r1)		# buy stack frame
	mflr	r31				# save link register
	bl	ENTRY(auditscall)		# call audint
	.extern	ENTRY(auditscall)		# return
	mtlr	r31
	cal	r1, stkmin+8(r1)		# pop stack
	l	r31, -4(r1)			# restore regs
	br

#*******************************************************************************
#
# NAME: trace_sc_end
#
# FUNCTION:
#	Called from sc_flih, after returning from system call when
# Trchook[0] != 0.
#
# REGISTERS:
#	INPUT:
#		r2 - toc
#		r25 - csa
#
#	USED:
#		all volatile
#
# EXECUTION ENVIRONMENT:
#	Called only from sc_flih
#
#******************************************************************************

trace_sc_end:
	st	r31, -4(r1)
	mflr	r31				# save lr
	liu	r3, (HKWD_KERN_SYSCRET|HKTY_LT)>16 # system call return id
	lbz	r4, ut_error(r25)		# ut_error value
	bl	ENTRY(trchook)			# do trace hook
	.extern	ENTRY(trchook)
	mtlr	r31				# restore link register
	l	r31, -4(r1)
	br					# return

#*******************************************************************************
#
# NAME: sc_call_sigslih
#
# FUNCTION:
#	called from the back end of sc_flih, when returning to user mode
# while sigslih should be called. This function sets up a usermode mst and
# branches to call sig_slih.
#
# REGISTERS:
#	INPUT:
#		r1 - user stack pointer
#		r2 - kernel toc
#		r3 - system call return code
#		r6 - user msr value
#		r24 - u
#		r25 - csa
#		r26 - SC callers link register
#		r30 - ppda
#
# EXECUTION ENVIRONMENT:
#	Called only from sc_flih.
#	All interrupts must be disabled
#
#*******************************************************************************

sc_call_sigslih:

#	set mst to user mode
user_mst:

	mfcr	r0
	st	r1, mstgpr+4*1(r25)		# save parameters for sc_epilog
	st	r3, mstgpr+4*3(r25)
	st	r6, mstmsr(r25)			# set mst to user mode

	lil	r12, -1
	st	r0, mstcr(r25)			# update condition register

#	copy user sregs to mst

	lil	r11, 16				# loop counter
	mtctr	r11
	cal	r10, u_adspace_sr-4(r24)	# user sr values
	cal	r9, mstsr-4(r25)		# mst segregs
sr_loop:
	lu	r8, 4(r10)			# get user segreg
	stu	r8, 4(r9)			# set segreg
	bdn	sr_loop
	
#	kill misc registers

	st	r12, mstmq(r25)			# kill mq
	st	r12, mstctr(r25)		# kill counter
	st	r12, mstxer(r25)		# kill xer
	st	r12, mstlr(r25)			# kill lr

	st	r26, mstiar(r25)		# set iar

# 	kill other volatile GPRs

	st	r12, mstgpr+0*4(r25)
	st	r12, mstgpr+2*4(r25)
	st	r12, mstgpr+4*4(r25)
	st	r12, mstgpr+5*4(r25)
	st	r12, mstgpr+6*4(r25)
	st	r12, mstgpr+7*4(r25)
	st	r12, mstgpr+8*4(r25)
	st	r12, mstgpr+9*4(r25)
	st	r12, mstgpr+10*4(r25)
	st	r12, mstgpr+11*4(r25)
	st	r12, mstgpr+12*4(r25)

#	store other non volatile GPRs

	st	r13, mstgpr+13*4(r25)
	st	r14, mstgpr+14*4(r25)
	st	r15, mstgpr+15*4(r25)
	st	r16, mstgpr+16*4(r25)
	st	r17, mstgpr+17*4(r25)
	st	r18, mstgpr+18*4(r25)
	st	r19, mstgpr+19*4(r25)
	st	r20, mstgpr+20*4(r25)
	st	r21, mstgpr+21*4(r25)
	st	r22, mstgpr+22*4(r25)
	st	r23, mstgpr+23*4(r25)

	lhz	r16, ut_flags(r25)

#	restore user mode GPRs save at front end of sc flih

	lil	r15, 8
	cal	r13, ut_scsave-4(r25)
	cal	r14, mstgpr+24*4-4(r25)
	mtctr	r15
rest_loop:
	lu	r15, 4(r13)
	stu	r15, 4(r14)
	bdn	rest_loop

#	set system call to signal delivery flag.
 
	oril	r16, r16, UTSCSIG
	sth	r16, ut_flags(r25)		

#	call sig_slih

        mr	r15, r30                        # load ppda address
        mr      r14, r25                        # load csa value
        b       ENTRY(call_sigslih)             # invoke sig_slih
        .extern ENTRY(call_sigslih)

#*******************************************************************************
#
# NAME: sr_bat_display_setup
#
# 	Set up access rights to displays controlled by bats or segment registers
# 	This routine assumes that all gruprts for displays controlled by bats or
# 	seg register are in the front of the list. 
#
# REGISTERS used: r26, r29, r31
#
# INPUT:    r26 points to the first gruprt structure 
#
# EXECUTION ENVIRONMENT: called from sc_flih(), resume(), and kgetsig()
#			 with interrupt disabled
#*******************************************************************************
	# program in this fashion for lack of indirect addressing to BATs
	# and the number of DBATs is only 4
	.machine	"any"
	.globl	DATA(sr_bat_display_setup)
sr_bat_display_setup:
	# set up DBAT0			
	lbz	r29,gp_type(r26)	# r29 = type value
	rlinm	r29,r29,0,DISP_MASK	# AND with the mask
	cmpli   cr0,r29,BATTYPE
	bne	cr0, sr_display_setup
	lwz	r29,gp_acw0(r26)	# r29 = batu
	lwz	r31,gp_acw1(r26)	# r31 = batl
	lwz	r26,gp_next(r26)	# r26 = prt to next gruprt in list
	.machine 	"ppc"
	mtdbatu	BAT0,r29
	cmpi	cr0,r26,0		# test if done
	mtdbatl	BAT0,r31
	.machine	"any"
	beqr				# return if done

	# set up DBAT1
	lbz	r29,gp_type(r26)	# r29 = type value
	rlinm	r29,r29,0,DISP_MASK	# AND with the mask
	cmpli   cr0,r29,BATTYPE
	bne	cr0, sr_display_setup
	lwz	r29,gp_acw0(r26)	# r29 = batu
	lwz	r31,gp_acw1(r26)	# r31 = batl
	lwz	r26,gp_next(r26)	# r26 = prt to next gruprt in list
	.machine 	"ppc"
	mtdbatu	BAT1,r29
	cmpi	cr0,r26,0		# test if done
	mtdbatl	BAT1,r31
	.machine	"any"
	beqr				# return if done

	# set up DBAT2
	lbz	r29,gp_type(r26)	# r29 = type value
	rlinm	r29,r29,0,DISP_MASK	# AND with the mask
	cmpli   cr0,r29,BATTYPE
	bne	cr0, sr_display_setup
	lwz	r29,gp_acw0(r26)	# r29 = batu
	lwz	r31,gp_acw1(r26)	# r31 = batl
	lwz	r26,gp_next(r26)	# r26 = prt to next gruprt in list
	.machine 	"ppc"
	mtdbatu	BAT2,r29
	cmpi	cr0,r26,0		# test if done
	mtdbatl	BAT2,r31
	.machine	"any"
	beqr				# return if done

	# set up DBAT3
	lbz	r29,gp_type(r26)	# r29 = type value
	rlinm	r29,r29,0,DISP_MASK	# AND with the mask
	cmpli   cr0,r29,BATTYPE
	bne	cr0, sr_display_setup
	lwz	r29,gp_acw0(r26)	# r29 = batu
	lwz	r31,gp_acw1(r26)	# r31 = batl
	lwz	r26,gp_next(r26)	# r26 = prt to next gruprt in list
	.machine 	"ppc"
	mtdbatu	BAT3,r29
	cmpi	cr0,r26,0		# test if done
	mtdbatl	BAT3,r31
	.machine	"any"
	beqr				# return if done

	# skip the rest of BAT type gruprt structures	
skip_bat:
	lbz	r29,gp_type(r26)	# r29 = type value
	rlinm	r29,r29,0,DISP_MASK	# AND with the mask
	cmpli   cr0,r29,BATTYPE
	bne	cr0, sr_display_setup
	lwz	r26,gp_next(r26)
	b	skip_bat

sr_display_setup:
	cmpli	cr1,r29,SRMCTYPE	
	cmpli	cr0,r29,PBUSTYPE
	cror	cr0+eq, cr0+eq, 4*cr1+eq # combine tests
	bner	cr0			# return if not a SR type display

	lwz	r29,gp_eaddr(r26)	# r29 = starting effective address
	lwz	r31,gp_acw0(r26)	# r31 = seg reg content in gruprt

	lwz	r26,gp_next(r26)	# r26 = prt to next gruprt in list
	mtsrin	r31,r29			# store seg reg content in gruprt to SR
	cmpi	cr0,r26,0		# test if done
	beqr	cr0
	lbz	r29,gp_type(r26)	# r29 = type value
	rlinm	r29,r29,0,DISP_MASK	# AND with the mask
	b	sr_display_setup
	.machine	"com"

#*******************************************************************************
#
# NAME: bad_sc_num:
#
# FUNCTION:
#	Called from sc_flih when system call index is determined to be
# invalid.  Sets u.u_error to EFAULT and branches to bad SC handler.
#
# REGISTERS:
#	INPUT:
#		r24 - u
#		r25 - csa
#		r29 - curthread
#
# EXECUTION ENVIRONMENT:
#	Called only from SC flih
#
#******************************************************************************

bad_sc_num:

	lil	r0, DISABLED_MSR	# disable interrupts
	mtmsr	r0

	lil	r0, 0			# clear t_suspend
	sth	r0, t_suspend(r29)

	lil	r0, EFAULT		# set ut_error to EFAULT
	stb	r0, ut_error(r25)	# in uthread block
	l	r15, ut_msr(r25)	# get callers MSR
	mtctr	r15			# restore count register

#	Restore user segregs, before killing

	l	r0, u_adspace_sr+0*4(r24)
	mtsr	sr0, r0
	l	r0, u_adspace_sr+14*4(r24)
	mtsr	sr14, r0
	l	r0, u_adspace_sr+1*4(r24)
	mtsr	sr1, r0
	l	r0, u_adspace_sr+2*4(r24)
	isync
	mtsr	sr2, r0

	b	bad_sc			# switch to user mode and die

#*******************************************************************************
#
# NAME: bad_sc
#
# FUNCTION:
#	This routine is called when an invalid SC occurs.  It restores the
# caller's (sc,svc) msr and executes an invalid opcode.  It invoced by
# all unused Power SVC vectors, and the sc_flih on invlid system calls
#
#******************************************************************************

bad_sc:
	mfctr	r26			# restore user msr
	mtmsr	r26
	isync
	.long	0			# invalid opcode

#*******************************************************************************
#
# NAME: sc_unboost_thread
#
# FUNCTION:
#	Called from sc_flih() if the priority of the thread had been
# boosted during the system.  This restores the priority of the
# thread and calls swtch().
#
# REGISTERS:
#	INPUT:
#		r29 - curthread
#	USED:
#		r0, r3, r4
#
# EXECUTION ENVIRONMENT:
#	Called only from sc_flih
#
#*******************************************************************************

sc_unboost_thread:

	mflr	r0			# get return addr

	lbz	r3, t_sav_pri(r29)	# load original, non-boosted priority
	cal	r4, 0(0)		# load zero
        stw     r0, stklink(r1)
	stw	r4, t_boosted(r29)	# clear boosted field
	stb	r3, t_pri(r29)		# reset priority

	bla	ENTRY(swtch)		# Call swtch()
	.extern	ENTRY(swtch)

        lwz     r0, stklink(r1)         # reload return address
        mr      r3, r30                 # restore system call return value
	mtlr	r0			# Return addr to LR

	br				# return


