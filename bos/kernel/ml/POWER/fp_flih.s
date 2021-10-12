# @(#)39	1.10  src/bos/kernel/ml/POWER/fp_flih.s, sysml, bos411, 9428A410j 4/13/94 14:28:41
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: fp_flih
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential	Restricted when
# combined with	the aggregated modules for this	product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT	International Business Machines	Corp. 1989, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************
 

	.file "fp_flih.s"
	.machine "com"

include(flihs.m4)
include(systemcfg.m4)
include(macros.m4)

#******************************************************************************#
#									       #
#     Floating point unavailable FLIH					       #
#									       #
#******************************************************************************#

# Name:	 fp_flih

# Function:

#	This routine is	entered	due to "Floating Point Not Available" interrupt
#	whenever a program attempts to use a FP	register or perform any	FP
#	operation with the FP Available	bit off	in the MSR.

#	N.B.:  It is illegal to	use floating point instructions	in interrupt
#	       handlers	because	of the lazy save/restore philosophy regarding
#	       floating	point resources.  If "fpna" occurs while executing
#	       interrupt-level code, we	should trap to the debugger to find
#	       the culprit.

#   This code has the following	tasks:

#    * Save the	state of the interrupted program, establish r/w	addressability
#	 to segments 0 (kernel segment), 2 (user segment), and 14 (kernel ext
#	 segment), get a new mst save area, and	establish a C environment.
#	 All this is done by "save".
#    * Call disown_fp to save the current FP owner's state, if any, in the
#	 appropriate mst save area.
#    * Load this process's FP state from its mst save area, and	mark it	as
#	 the owner of the FP regs.
#    * Set this	process's MSR(FE) bit in the mst if the	process	has
#      requested that it run in	synchronous mode for floating point.
#    * Set this	process's MSR(IE) bit in the mst if it has requested 
#      to run with imprecise floating point interrupts.
#      
#      It is NOT a valid state to have BOTH fpinfo(FP_SYNC_TASK) &
#      fpinfo(FP_IMP_INT) set. In other	words, a task cannot request BOTH
#      precise And imprecise floating point interrupts at the same time.
#      This code does not check	for this case, as fp_cpusync(),	& sigreturn()
#      prevent it from occurring.
#
#    * Mark in the mst that this process has `used' floating point (fpeu).
#    * Resume the interrupted program, with "FP	Available" set in the MSR.

       .set	all_fields, 0xFF	# Set all 4-bit	fields in FPSCR
       .set	null_procp, 0		# Value	in fp_owner which signifies
					#   "no	owner"

#	Save reg 0 in low memory; save link reg	in reg 0

	.csect	fp_flih_sect[PR]
DATA(fp_flih):
	.globl	DATA(fp_flih)
ENTRY(fp_flih):
	.globl	ENTRY(fp_flih)

	mfspr	r0, LR

#	Call the state-saving routine, which will set up kernel	and user
#	segment	r/w addressability and stash all the state except the
#	floating point registers in the	mst pointed to by csa.

	bl	ENTRY(state_save)
	.extern	ENTRY(state_save)

#	On return from state_save, the following registers are set up:
#	    r1	   Address of stack (for C)
#	    r2	   Address of kernel TOC
#	    r13	   Address of current  mst save	area (new value	of csa)
#	    r14	   Address of previous mst save	area (old value	of csa)
#	    r15    ppda
#	    sr0	   R/W access to kernel	segment
#	    sr2	   R/W access to process segment
#	    sr14   R/W access to kernel	extension segment

#	Set new	interrupt priority; perform interrupt set-up processing

	cal	r3, 8(0)		# Interrupt "level" is 8 for 0x400
	cal	r4, INTMAX*256(0)	# New priority,	plus backtrack flag of 0
	bl	ENTRY(begin_interrupt)
	.extern	ENTRY(begin_interrupt)

#	Find current owner of floating point registers

	l	r3, ppda_fpowner(r15)	# Pick up owning proc struct, or "null"

	cmpi	cr0, r3, null_procp	# See if anyone	currently owns FP
	beq	fp_10			# Skip call to disown_fp if not
ifdef(`_POWER_MP',`
ifdef(`DEBUG',`
	# We should never get here in a _POWER_MP environment.
	# Currently, the FPU is disowned on every dispatch.
	teq	r3, r3
')
')
	l	r3, t_tid(r3)		# Pick up thrd id itself
	bl	ENTRY(disown_fp[PR])	# Call "disown"	routine
       .extern	ENTRY(disown_fp[PR])
fp_10:

#	Check to ensure	that FP	instructions not issued	by int-level routine.
#	Establish the interrupted program as the FP owner.

	l	r0, mstprev(INTR_MST)	# Get backchain	to previous mst, if any

	l	r3, ppda_curthread(r15)	# Load addr of curthread pointer
	l	r4, t_fpuct(r3)		# Get unavailable count
	ai	r4, r4, 1		#    & increment it
	st	r4, t_fpuct(r3)		# 

	cmpi	cr0, r0, 0		# Previous mst pointer SHOULD BE NULL

       .using	mstsave, INTR_MST
	l	r4, mstmsr		# Pick up failing MSR value
	mfmsr	r5			# Get our own MSR value

	beq	fp_20			# Skip trap if interrupted program is
	TRAP				#   a process; trap if interrupt handler
					# use static debugger trap
fp_20:

	st	r3, ppda_fpowner(r15)	# Save proc struct addr	in "fp_owner"
	lbz	r3, mstfpinfo		# Retrieve floating point flags	field
	oril	r4, r4,	MSR_FP		# Turn on "FP Available" bit in	MSR
					# Set FE-Synchronous-mode if requested
	rlimi	r4, r3,	FP_SYNC_IMP_S, MSR_FE
					# Set IE-Imprecise-mode	if requested
	rlimi	r4, r3,	FP_SYNC_IMP_S, MSR_IE
	st	r4, mstmsr		# Store	MSR so "restore" picks it up

	cal	r3, FP_USED(0)		# Indicate this	task used floating pt
	stb	r3, mstfpeu		#	 "	      "

	oril	r5, r5,	MSR_FP		# Give ourselves access	so we can
	mtmsr	r5			#   load the FP	registers
	isync				# make sure FP is seen

#	Load the FP regs out of	the mst	save area; take	particular care
#	with the FPSCR since it's accessed in such an arcane fashion.

	lfd	f31, mstfpscr -	4	# Pick up 32 "don't care" bits plus
					#   the	32 bits	of the FPSCR
	lfd	f0,  mstfpr +  0 * 8
	lfd	f1,  mstfpr +  1 * 8
	lfd	f2,  mstfpr +  2 * 8
	lfd	f3,  mstfpr +  3 * 8
	lfd	f4,  mstfpr +  4 * 8
	mtfsf	all_fields, f31		# Restore the entire FPSCR
	lfd	f5,  mstfpr +  5 * 8
	lfd	f6,  mstfpr +  6 * 8
	lfd	f7,  mstfpr +  7 * 8
	lfd	f8,  mstfpr +  8 * 8
	lfd	f9,  mstfpr +  9 * 8
	lfd	f10, mstfpr + 10 * 8
	lfd	f11, mstfpr + 11 * 8
	lfd	f12, mstfpr + 12 * 8
	lfd	f13, mstfpr + 13 * 8
	lfd	f14, mstfpr + 14 * 8
	lfd	f15, mstfpr + 15 * 8
	lfd	f16, mstfpr + 16 * 8
	lfd	f17, mstfpr + 17 * 8
	lfd	f18, mstfpr + 18 * 8
	lfd	f19, mstfpr + 19 * 8
	lfd	f20, mstfpr + 20 * 8
	lfd	f21, mstfpr + 21 * 8
	lfd	f22, mstfpr + 22 * 8
	lfd	f23, mstfpr + 23 * 8
	lfd	f24, mstfpr + 24 * 8
	lfd	f25, mstfpr + 25 * 8
	lfd	f26, mstfpr + 26 * 8
	lfd	f27, mstfpr + 27 * 8
	lfd	f28, mstfpr + 28 * 8
	lfd	f29, mstfpr + 29 * 8
	lfd	f30, mstfpr + 30 * 8
	lfd	f31, mstfpr + 31 * 8

       .drop	r14

#	Finish interrupt processing; resume interrupted	program

	b	ENTRY(finish_interrupt)
	.extern	ENTRY(finish_interrupt)

	.toc

ifdef(`_POWER_MP',`
include(low_dsect.m4)
',`
include(ppda.m4)
')
include(i_machine.m4)
include(scrs.m4)
include(mstsave.m4)
include(proc.m4)
include(machine.m4)
include(user.m4)
