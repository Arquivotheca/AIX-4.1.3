# @(#)25        1.3  src/bos/kernel/ml/POWER/pr_flih.s, sysml, bos411, 9428A410j 3/7/94 22:20:38
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS: DATA
#		ENTRY
#		code
#		mstiar
#		mstmsr
#		ppda_curproc
#
#   ORIGINS: 27,83
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1989,1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************


	.file "pr_flih.s"
	.machine "com"

#******************************************************************************
#
# NAME:	 pr_flih
#
# FUNCTION:
#       The program flih is architecture independent.  The flih
#       syncs the floating point unit on floating point exceptions
#       since fp imprecise exceptions are not serialized with
#       respect to each other on PPC.  It passes control to p_slih()
#       to do all the real work.  The value of SRR1 must be
#       retrieved from the mst since a PTE overflow can occur in the
#       flih.
#
#       This function is written in intersection assembler for all
#       architectures.
#
# PSEUDO CODE:
#       pr_flih()
#       {
#               struct mstsave *intr_mst;
#
#               state_save();
#               begin_interrupt(PR_LEVEL, INTMAX<<8);
#               convert srr1 to exception code;
#               if (floating point exception)
#                       sync fpu;
#
#               intr_mst = ppdp->csa->prev;
#       #ifdef _THREADS
#               p_slih(intr_mst, intr_mst->iar, excpt_code,
#                                                       ppdp->curthread);
#       #else
#               p_slih(intr_mst, intr_mst->iar, excpt_code,
#                                                       ppdp->curproc);
#       #endif
#               finish_interrupt();
#       }
#
#******************************************************************************

	.csect	pr_sect[PR]
DATA(pr_flih):
	.globl	DATA(pr_flih)
ENTRY(pr_flih):
	.globl	ENTRY(pr_flih)

	mfspr	r0, LR				# do full state save
	bl	ENTRY(state_save)
	.extern	ENTRY(state_save)

#	Set new	interrupt priority; perform interrupt setup processing

	lil	r3, PR_LEVEL			# call begin interrupt
	lil	r4, INTMAX*256
	bl	ENTRY(begin_interrupt)
	.extern	ENTRY(begin_interrupt)

#	Compute	the code to pass to p_slih to indicate type of interrupt

	
	l	r12, mstmsr(INTR_MST)	# Fetch	SRR1=flags set by hdwe,	old MSR
	cntlz	r5, r12			# Bit number of	first '1' bit
	cal	r5, (EXCEPT_FLOAT-11)(r5) # Convert bit	number to code (parm 3):
#					      00000000001000...	=> FLOAT
#					      00000000000100...	=> INV_OP
#					      00000000000010...	=> PRIV_OP
#					      00000000000001...	=> TRAP
#	Note that hardware spec	guarantees that	top 11 bits will be '0'	and
#	that exactly one of the	next four will be '1'.

#	No support for imprecise interrupt yet

	rlinm.	r12, r12, 0, SRR_PR_IMPRE 	# check for imprecise interrupt
	beq	cr0, not_fpim			# branch if not imprecise	
	sync					# sync floating point unit
	lil	r5, EXCEPT_FLOAT_IMPRECISE	# imprecise exception code
not_fpim:

#	Call the second	level interrupt	handler

	mr	r3, INTR_MST			# offending mst
	l	r4, mstiar(INTR_MST)		# excepting old IAR
ifdef(`_THREADS',`
	l	r6, ppda_curthread(PPDA_ADDR)	# current thread pointer
',`
	l	r6, ppda_curproc(PPDA_ADDR) # current proc pointer
')
       .extern	ENTRY(p_slih)
	bl	ENTRY(p_slih)			# Call the Program SLIH

#	Finish interrupt processing; resume interrupted	program

	b	ENTRY(finish_interrupt)		# all done
	.extern	ENTRY(finish_interrupt)


include(flihs.m4)
include(i_machine.m4)
include(scrs.m4)
include(mstsave.m4)
include(except.m4)
include(machine.m4)
include(ppda.m4)
