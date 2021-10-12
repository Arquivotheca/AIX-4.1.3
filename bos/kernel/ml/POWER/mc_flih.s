# @(#)49	1.2  src/bos/kernel/ml/POWER/mc_flih.s, sysml, bos411, 9428A410j 3/7/94 22:27:51
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: mc_flih
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential	Restricted when
# combined with	the aggregated modules for this	product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT	International Business Machines	Corp. 1992,1993
# All Rights Reserved
#
# US Government	Users Restricted Rights	- Use, duplication or
# disclosure restricted	by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

	.file "mc_flih.s"
	.machine "com"

#******************************************************************************
#
# NAME:	mc_flih:
#
# FUNCTION:
#	Machine	checks are non-recoverable.  The machine check flih
#	is architecture	independent.  The machine check	slih is	run
#	with xlate on, and the AL bit set (for Power).
#
#	The machine check flih must begin with the following
#	instruction sequence to	prevent	checkstops on some RS1
#	machines.  This	sequence is at the machine check vector	and
#	it destroys r1 (machine	checks are not restartable).  The
#	code (followed by the flih_prolog) is overlaid at SI time
#	(Power).  On PPC the standard flih_prlog is put	at the
#	machine	check vector.
#
#		dcs
#		bl	next_instr
#	next_intsr:
#		mflr	r1
#		ai	r1, r1,	12
#		cli	0, r1
#		ics
#
# PSEUDO CODE:
#
#	int mc_flih()
#	{
#		struct mstsave *intr_mst;
#		state_save();
#		begin_interrupt(MC_LEVEL, INTMAX<<8);
#		intr_mst = ppdp->csa;
#		mac_check(intr_mst->iar, intr_mst->msr,	intr_mst);
#	}
#
#******************************************************************************

	.csect mc_flih_sect[PR]
DATA(mc_flih):
	.globl	DATA(mc_flih)
ENTRY(mc_flih):
	.globl	ENTRY(mc_flih)

	mfspr	r0, LR				# call state save
	bl	ENTRY(state_save)
	.extern	ENTRY(state_save)

	lil	r3, MC_LEVEL			# call begin interrupt
	lil	r4, INTMAX*256
	bl	ENTRY(begin_interrupt)
	.extern	ENTRY(begin_interrupt)

	l	r3, mstiar(INTR_MST)		# get faulting iar
	l	r4, mstmsr(INTR_MST)		# get faulting MSR
	mr	r5, INTR_MST			# faulting mst
	b	ENTRY(mac_check)		# call slih
	.extern	ENTRY(mac_check)		# does not return


include(flihs.m4)
include(i_machine.m4)
include(scrs.m4)
include(mstsave.m4)

