# @(#)75  1.2  src/bos/kernel/ml/POWER/pr_flih_ppc.m4, sysml, bos411, 9430C411a 7/27/94 02:44:04
#****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTION: pr_flih_ppc
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Copyright (C) Bull S.A. 1994
# LEVEL 1, 5 Years Bull Confidential Information
#
#****************************************************************************

#****************************************************************************
#                                                   
# Name:	pr_flih_ppc
#
# Function:
#	Called from flih_prolog_ppc().
#       The pr_flih_ppc() routine is entered due to a program interrupt
#	on POWER_PC machines. For invalid instruction interrupt and 
#	privileged instruction interrupt, emulation is performed first.
#	Otherwise, the usual pr_flih() is called. 
#       The pr_flih_ppc() runs with interrupts disabled. It does a partial 
#	state save in a Per Processor Descriptor Area (PPDA), handles the
#	instruction interrupt and returns to the faulting instruction + 4.
#       This FLIH may cause a data storage interrupt in which case the
#       ds_flih alters the saved state so that we return to the instruction
#       that caused the invalid instruction interrupt after we have processed
#	the data storage interrupt (we get the program instruction interrupt
#	again but do not the page fault again). This part is common with the
#	alignment handler.
#
# Dependencies:
#       Any changes to state saved here requires changes to ds_flih code
#       that retrieves it.
#
# Upon entry:
#	GPR 0  saved in ppda_save0
#	GPR 15 saved in SPRG1
#	GPR 15 contains the address of the PPDA
#
#****************************************************************************

	.set	SRR_PR_EMUL,	0x000C0000	# SRR_PR_INVAL | SRR_PR_PRIV

	.machine "ppc"

DATA(pr_flih_ppc):
	.globl	DATA(pr_flih_ppc)	

	stw	r31, SAVE_R31(r15)		# don't use stm/lm any more...
	stw	r30, SAVE_R30(r15)		# save r30
	mfcr	r31				# save CR
	stw	r29, SAVE_R29(r15)		# save r29
	mfspr	r30, SRR1			# get SRR1
	stw	r28, SAVE_R28(r15)		# save r28
	rlwinm.	r29, r30, 0, SRR_PR_EMUL 	# check for invalid or privileged instruction
	stw	r27, SAVE_R27(r15)		# save r27
	mfxer	r28				# get XER
	beq-	cr0, call_pr_flih		# branch if no emulation required
	stw	r26, SAVE_R26(r15)		# save r26
	stw	r31, SAVE_CR(r15)		# save CR
	stw	r28, SAVE_XER(r15)		# save XER
	mfspr	r27, SRR0			# get SRR0 
	mflr	r26				# get LR
	stw	r25, SAVE_R25(r15)		# save r25
	stw	r30, SAVE_SRR1(r15)		# save SRR1
	mfmsr	r28				# r28 = MSR DR=0
	stw	r26, SAVE_LR(r15)		# save LR
	stw	r27, SAVE_SRR0(r15)		# save SRR0
	ori	r29, r28, MSR_DR		# r29 = MSR DR=1 Data Relocation on
	mr	r25, r15			# move ppda addr into r25
	mfspr	r15, SPRG1			# load up original contents of r15
	mtmsr	r29				# turn on Data Relocation
	isync					# make sure DR is noticed

	# get faulting instruction (can page fault in MP)
	lwz	r26, 0(r27)

	mtmsr	r28				# turn off Data Relocation
	isync					# make sure DR is noticed
	b	DATA(power_asm_emulate)		# call emulation

call_pr_flih:
	mtcr	r31				# restore CR
	lwz	r28, SAVE_R28(r15)		# restore r28
	lwz	r29, SAVE_R29(r15)		# restore r29
	lwz	r30, SAVE_R30(r15)		# restore r30
	lwz	r31, SAVE_R31(r15)		# restore r31
	b	DATA(pr_flih)			# call pr_flih
	.extern	DATA(pr_flih)

