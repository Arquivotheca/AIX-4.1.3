# @(#)28	1.7  src/bos/kernel/ml/POWER/dbi_flih.m4, sysml, bos411, 9428A410j 6/1/94 12:22:37
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: dbi_flih
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1992, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NOTE: This is a debug extension to the PI handler.  It allows
# debugger to be used in the majority to the assembler code
#
# This is not product level code.  This function is conditionally
# built using the MLDEBUG m4 flag
#
#****************************************************************************

ifdef(`DBI_FLIH',`
	.machine "any"

#	Use the kernel patch area for stack and save area

	.set old_csa, 0x6000-4
	.set dmst, old_csa-(mstend-mstsave)

dbpr_flih:

	# store off a few registers

	st	r0, dmst+(mstgpr-mstsave)+0*4(0)
	st	r1, dmst+(mstgpr-mstsave)+1*4(0)
	mfcr	r0
	st	r0, dmst+(mstcr-mstsave)(0)

	#
	# if the PI was not a trap go to the system PI flih
	#
	mfspr	r1, SRR1
	rlinm.	r1, r1, 0, 0x00020000		# check for trap instruction
	beq	cr0, not_trap
	
	#
	# check that instruction is debugger trap
	#
	mfspr	r1, SRR1
	rlinm	r1, r1, 31, MSR_DR		# users IR into DR
	oril	r1, r1, MSR_ME			# add ME
	mtmsr	r1				# set MSR
	mfspr	r1, SRR0
	isync
	l	r0, 0(r1)
	lil	r1, MSR_ME			# turn off xlate
	mtmsr	r1
	isync
	cau	r1, 0, 0x7c80
	oril	r1, r1, 0x0008
	cmp	cr0, r0, r1
	beq	cr0, pr_trap

	cau	r1, 0, 0x7c81
	oril	r1, r1, 0x0808
	cmp	cr0, r0, r1
	beq	cr0, pr_trap

	lil	r1, MSR_ME
	mtmsr	r1
	isync
not_trap:
	#
	# was not a trap so restore state and go to standard filh
	#
	l	r1, syscfg_arch(0)
	cmpi	cr0, r1, POWER_PC
	st	r15, ra_save(0)		# save the value of r15
	bne	not_ppc
	mtspr	SPRG1, r15
not_ppc:
	GET_PPDA(cr0,r15)
	l	r0, dmst+(mstcr-mstsave)(0)
	mtcr	r0
	l	r0, dmst+(mstgpr-mstsave)+0*4(0)
	l	r1, dmst+(mstgpr-mstsave)+1*4(0)

#
# emulate flih prlog linkage
#

	st	r0, ppda_save0(r15)	# save a register
	b	DATA(pr_flih)
	.extern	DATA(pr_flih)

pr_trap:
	#
	# save GPRs
	#
	cal	r1, dmst(0)
	stm	r2, mstgpr+2*4(r1)

	#
	# save segment registers
	#
	mfsr	r16, sr0
	mfsr	r17, sr1
	mfsr	r18, sr2
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
	stm	r16, mstsr(r1)

	#
	# save other misc registers
	#

	# 603/604: save MQ emulation from
	# csa to dmst.
	# if (__power_ppc() && !__power_601())
	#	l r30, mstmq(r1)
	# else
	#	mfspr	r30, MQ
	#
	# CR already saved in mstcr(r1)

	l	r24, syscfg_arch(0)	 # get architecture
	cmpi	cr1, r24, POWER_PC	 # check for ppc
	bne	cr1, load_from_mq	 # branch if not ppc
	l	r24, syscfg_impl(0)	 # get processor implementation
	cmpi	cr1, r24, POWER_601	 # check for 601
	beq	cr1, load_from_mq	 # branch if 601 else
	GET_CSA(cr0,r24,r25)		 # get current save area pointer
	l	r30, mstmq(r25)		 # load MQ from csa
	b	save_misc_regs
load_from_mq:
	mfspr	r30, MQ

save_misc_regs:
	mfspr	r24, SRR0
	mfspr	r25, SRR1
	l	r26, mstcr(r1)
	mflr	r27
	mfspr	r28, CTR
	mfspr	r29, XER
	stm	r24, mstiar(r1)

	#
	# get kernel addressability
	#
	l	r16, DATA(g_ksrval)
	mtsr	sr0, r16
	mfsr	r16, sr2
	rlinm	r16, r16, 0, ~sr_k_bit
	mtsr	sr2, r16
	l	r16, DATA(g_kxsrval)
	mtsr	sr14, r16
	cal	r16, DISABLED_MSR(0)
	mtmsr	r16
	isync

	#
	# call debugger
	#
	#	r1 - stack
	#	r2 - TOC
	#	r3 - mst pointer
	#	r4 - reason code
	#	r5 - 0
	#

	mr	r3, r1
	cal	r1, -stkmin(r1)
	l	r2, DATA(g_toc)
	lil	r4, DBG_TRAP
	lil	r5, 0

	# point csa at this temporary mst

	GET_CSA(cr0,r31,r16)
	LTOC(r17, old_csa, data)
	st	r16, DATA(old_csa)(r17)
	GET_PPDA(cr0,r31)
	st r1, ppda_csa(r31)
	CSA_UPDATED(cr0, r6, r1)

	bl	ENTRY(debugger)
	.extern ENTRY(debugger)

	st r16, ppda_csa(r31)	# r31 persists across the call
	CSA_UPDATED(cr0, r6, r16)
	
	#
	# turn off xlate
	#
	lil	r16, MSR_ME(0)
	mtmsr	r16
	isync

	#
	# restore segment registers
	#
	cal	r1, dmst(0)
	lm	r16, mstsr(r1)
	mtsr	sr0, r16
	mtsr	sr1, r17
	mtsr	sr2, r18
	mtsr	sr3, r19
	mtsr	sr4, r20
	mtsr	sr5, r21
	mtsr	sr6, r22
	mtsr	sr7, r23
	mtsr	sr8, r24
	mtsr	sr9, r25
	mtsr	sr10, r26
	mtsr	sr11, r27
	mtsr	sr12, r28
	mtsr	sr13, r29
	mtsr	sr14, r30
	mtsr	sr15, r31

	#
	# restore misc registers
	#
	lm	r24, mstiar(r1)
	mtspr	SRR0, r24
	mtspr	SRR1, r25
	mtlr	r27
	mtctr	r28
	mtxer	r29

	# 603/604: restore MQ emulation from
	# dmst to csa.
	# if (__power_ppc() && !__power_601())
	#	st r30, mstmq(csa)
	# else
	#	mtmq	r30
	#
	# Restore CR after MQ emulation

	l	r24, syscfg_arch(0)	# get architecture
	cmpi	cr1, r24, POWER_PC	# check for ppc
	bne	cr1, move_to_mq		# branch if not ppc
	l	r24, syscfg_impl(0)	# get processor implementation
	cmpi	cr1, r24, POWER_601	# check for 601
	beq	cr1, move_to_mq		# branch if 601 else
	GET_CSA(cr0,r24,r25)		# get current save area pointer
	st	r30, mstmq(r25)		# store MQ in emulation
	b	restore_misc_regs
move_to_mq:
	mtmq	r30

restore_misc_regs:
	mtcr	r26
	
	#
	# restore GPRs
	#
	lm	r2, mstgpr+2*4(r1)
	l	r0, mstgpr(r1)
	l	r1, mstgpr+1*4(r1)

	rfi
	.machine "com"

	.toc
	TOCL(old_csa, data)
	.csect ENTRY(low[PR])
',)
