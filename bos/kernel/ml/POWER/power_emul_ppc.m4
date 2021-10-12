# @(#)74  1.3  src/bos/kernel/ml/POWER/power_emul_ppc.m4, sysml, bos411, 9430C411a 7/27/94 02:34:42
#****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: power_asm_emulate
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

	.set	OE, 21		# update XER if OE bit is set in opcode
	.set	Rc, 31		# update  CR if Rc bit is set in opcode

	.machine "ppc"

#****************************************************************************
#
# Try C emulation: call power_emul()
#
#****************************************************************************

call_c_emul:
	# Restore initial context and r15 with ppda addr.
	# r15 is already saved in SPRG1.
	# get SRR0,SRR1,LR,CR,XER from savearea
	lwz	r27, SAVE_SRR0(r25)	# load SRR0
	lwz	r28, SAVE_SRR1(r25)	# load SRR1
	lwz	r29, SAVE_LR(r25)	# load LR
	lwz	r30, SAVE_CR(r25)	# load CR
	lwz	r31, SAVE_XER(r25)	# load XER
        mtspr   SRR0, r27		# restore SRR0 
        mtspr   SRR1, r28		# restore SRR1 
        mtlr   	r29                  	# restore LR 
        mtcr    r30                     # restore CR
        mtxer   r31                 	# restore XER
	mr	r15, r25		# move ppda addr to r15
	# restore r26-r31
	lwz	r26, SAVE_R26(r25)	# restore r26
	lwz	r27, SAVE_R27(r25)	# restore r27
	lwz	r28, SAVE_R28(r25)	# restore r28
	lwz	r29, SAVE_R29(r25)	# restore r29
	lwz	r30, SAVE_R30(r25)	# restore r30
	lwz	r31, SAVE_R31(r25)	# restore r31
        lwz	r25, SAVE_R25(r15)	# restore r25

	# Part of pr_flih() code duplicated here.

	mfspr	r0, LR			# do full state save
	bl	ENTRY(state_save)
	.extern	ENTRY(state_save)

	# Set new interrupt priority; perform interrupt setup processing
	li	r3, PR_LEVEL
	li	r4, INTMAX*256
	bl	ENTRY(begin_interrupt)
	.extern	ENTRY(begin_interrupt)

#	Compute	the code to pass to p_slih to indicate type of interrupt

	lwz	r12, mstmsr(INTR_MST)	  # Fetch SRR1=flags set by hdwe, old MSR
	cntlzw	r5, r12			  # Bit number of first '1' bit
	la	r5, (EXCEPT_FLOAT-11)(r5) # Convert bit	number to code (parm 3):
#					      00000000001000...	=> FLOAT
#					      00000000000100...	=> INV_OP
#					      00000000000010...	=> PRIV_OP
#					      00000000000001...	=> TRAP
#	Note that hardware spec	guarantees that	top 11 bits will be '0'	and
#	that exactly one of the	next four will be '1'.

#	No support for imprecise interrupt yet

	rlwinm.	r12, r12, 0, SRR_PR_IMPRE 	# check for imprecise interrupt
	beq+	cr0, not_fp_imp			# branch if not imprecise	
	sync					# sync floating point unit
	li	r5, EXCEPT_FLOAT_IMPRECISE	# imprecise exception code
not_fp_imp:

	# Call power_emul() with p_slih() parameters.

	mr	r3, INTR_MST			# offending mst
	lwz	r4, mstiar(INTR_MST)		# excepting old IAR
	lwz	r6, ppda_curthread(PPDA_ADDR)	# current thread pointer

	.extern	ENTRY(power_emul)
	bl	ENTRY(power_emul)

	# Finish interrupt processing; resume interrupted program
	b	ENTRY(finish_interrupt)
	.extern	ENTRY(finish_interrupt)



#****************************************************************************
#
# Final stage of power_asm_emulate(): store result, update XER and CR
# then go to "al_ret" label in al_flih.m4 code.
#
#****************************************************************************
pr_st:
	# store r31 into RT
	rlwinm	r30, r26, 14, 0xF8	# r30 = bits(6-10) of opcode, RT*8
       .using   low, r30
	la	r30, loadtab(r30)
       .drop    r30
        mtlr    r30                     # LR = branch addr in loadtab
	mfcr	r29			# save CR
        blrl    			# store r31 into RT

	# is OE bit set
	mtcr	r26
	bc+	false, OE, pr_Rc
	mfxer	r31
	stw	r31, SAVE_XER(r25)	# OE=1: store XER back to save area

pr_Rc:
	# is Rc bit set
	bc+	false, Rc, pr_ret	 # leave if RC=0
        lwz	r31, SAVE_CR(r25)	 # Rc=1: load CR from save area
	lwz	r30, SAVE_XER(r25)	 # load XER from save area
	rlwimi	r31, r29, 0, 0xe0000000	 # insert LT, GT, EQ bits in CR0
	rlwimi	r31, r30, 29, 0x10000000 # insert XER[SO] bit in CR0[SO] 
	stw	r31, SAVE_CR(r25)	 # store new CR in save area

pr_ret:
	# update statistics counter and return
	mr	r31, r2			# save r2
	lwz	r2, DATA(g_toc)		# load kernel toc
	LTOC(r30, emulate_count, data)
	lwz	r29, 0(r30)		# load,
	addi	r29, r29, 1		# incremente and
	stw	r29, 0(r30)		# store counter
	mr	r2, r31			# restore r2
	b	al_ret			# restore and return.


#******************************************************************************
#
# power_asm_emulate
#
#	The power_asm_emulate is only called from pr_flih_ppc() and perform
#	the power emulation. If assembler emulation fail then call emulation
#	written in C : power_emul().
#
# Upon entry:
#               r15 restored
#               r25 = address of the PPDA
#               r26 = opcode of the faulting instruction
#               r27 = address of the faulting instruction
#         MSR = r28 = MSR:DR=0
#               r29 = MSR:DR=1
#               r30 = scratch
#               r31 = scratch
#
#******************************************************************************

DATA(power_asm_emulate):
	.globl DATA(power_asm_emulate)
	# get primary opcode
	rlwinm	r31, r26, 6, 0x3f
	cmpi 	cr0, r31, 9
	cmpi	cr7, r31, 31
	beq-	cr0, emul_dozi		# asm emulation for dozi instruction
	bne-	cr7, call_c_emul	# try C emulation

#****************************************************************************
# Try to find extended opcode for primary opcode = 31
#               r25 = save area addr
#               r26 = opcode faulting instruction
#               r27 = SRR0 = faulting instruction addr
#         MSR = r28 = MSR:DR=0
#               r29 = MSR:DR=1
#               r30 = scratch
#               r31 = primary opcode
#****************************************************************************

	rlwinm	r30, r26, 31, 0x3ff	# extended opcode and OE tested
	cmpi	cr0, r30, 0x115		# lscbx lscbx.
	cmpi	cr7, r30, 0x153		# mfspr
	beq+	cr0, emul_lscbx
	cmpi	cr6, r30, 0x1d3		# mtspr
	beq+	cr7, emul_mfspr
	beq+	cr6, emul_mtspr

	rlwinm	r30, r26, 31, 0x1ff	# extended opcode without OE
	cmpi	cr0, r30, 0x06b		# mul mul. mulo mulo.
	cmpi	cr7, r30, 0x14b		# div div. divo divo.
	beq+	cr0, emul_mul
	beq+	cr7, emul_div
	cmpi	cr0, r30, 0x16b		# divs divs. divso divso.
	cmpi	cr7, r30, 0x108		# doz doz. dozo dozo.
	beq+	cr0, emul_divs
	beq+	cr7, emul_doz
	cmpi	cr0, r30, 0x168		# abs abs. abso abso.
	cmpi	cr7, r30, 0x1e8		# nabs nabs. nabso nabso.
	beq+	cr0, emul_abs
	beq+	cr7, emul_nabs
	b	call_c_emul		# call C code emulation

#****************************************************************************
# Emulation code 
#****************************************************************************
emul_mul:
	# get content of RA
	rlwinm	r30, r26, 19, 0xF8	# r30 = bits(11-15) of opcode, RA*8
	mfsr	r27, sr2
	.using	low, r30
	la	r30, stortab(r30)	# r30 = branch addr in stortab for RA
	.drop 	r30
	stw	r27, SAVE_WORK1(r25)	# save original sr2
	mtlr 	r30			# LR = branch address in stortab
	rlwinm	r27, r27, 0, ~SR_KsKp	# clear "k" bit(s)
	blrl				# set r31 = (RA)
	mr	r28, r31		# save (RA)
	# get content of RB
	rlwinm	r30, r26, 24, 0xF8	# r30 = bits(16-20) of opcode, RB*8
	mtsr	sr2, r27		# ensure r/w access to mst
	.using	low, r30
	la	r30, stortab(r30)	# r30 = branch addr in stortab for RB
	.drop 	r30
	lwz	r27, ppda_csa(r25)	# mst address
	mtlr 	r30			# LR = branch address in stortab
	blrl				# set r31 = RB
	# compute low order 32 bits of result
	mullwo.	r30, r31, r28		# low order 32 bits and set CR0 & XER
	mtmsr	r29			# turn on Data Relocation
	isync
	xori	r29, r29, MSR_DR	# toggle DR
	stw	r30, mstmq(r27)		# low order 32 bits in MQ emulation
	mtmsr	r29			# turn off Data Relocation
	isync
	lwz	r30, SAVE_WORK1(r25)	# load original sr2 value
	mulhw	r31, r31, r28		# compute high order 32 bits of result
	mtsr	sr2, r30		# restore sr2
	b	pr_st			# store into RT

emul_doz:
	# get content of RA -> r31
	rlwinm	r30, r26, 19, 0xF8	# r30 = bits(11-15) of opcode, RA*8
	.using	low, r30
	la	r30, stortab(r30)	# r30 = branch addr in stortab for RA
	.drop 	r30
	mtlr 	r30			# LR = branch address in stortab
	blrl				# set r31=(RA)
	# get content of RB -> r31
	rlwinm	r30, r26, 24, 0xF8	# r30 = bits(16-20) of opcode, RB*8
	mr	r29, r31		# save (RA)
	.using	low, r30
	la	r30, stortab(r30)	# r30 = branch addr in stortab for RB
	.drop 	r30
	mtlr 	r30			# LR = branch address in stortab
	blrl				# set r31 = RB
	cmp	0, r29, r31		# if RA > RB
	li	r28, 0			# return (RT)=0
	bgt+	doz_pos
	subfo.	r31, r29, r31		# compute (RT), set CR0 & XER
	b	pr_st			# and store
doz_pos:
	addo.	r31, r28, r28		# set CR0 & XER
	b	pr_st

emul_lscbx:
	# update statistics counter: don't modify r29 here
	# because alignment use it.
	mr	r31, r2			# save r2
	lwz	r2, DATA(g_toc)		# load kernel toc
	LTOC(r30, emulate_count, data)
	lwz	r28, 0(r30)		# load,
	addi	r28, r28, 1		# incremente and
	stw	r28, 0(r30)		# store counter
	mr	r2, r31			# restore r2

	# Reuse code from al_flih.m4
	# Compute effective addr (pseudo dar) in r27 and
	# emulate code between "l_str_cmp" and "next" label.
	# Then branch in al_flih.m4 at "next" label
	
	# get content of RB -> r31
	rlwinm	r30, r26, 24, 0xF8	# r30 = bits(16-20) of opcode, RB*8
        stb     r30, SAVE_RB(r25)   	# save RB
	.using	low, r30
	la	r30, stortab(r30)	# r30 = branch addr in stortab for RB
	.drop 	r30
	mtlr 	r30			# LR = branch address in stortab
	blrl				# set r31 = (RB)

	rlwinm	r30, r26, 19, 0xF8	# r30 = bits(11-15) of opcode, RA*8
        stb     r30, SAVE_RA(r25)   	# save RA
	cmpi	cr0, r30, 0		# branch if RA = r0
	mr	r27, r31		# save (RB)
	beq-	lscbx_pos		# DAR = (RB)
	.using	low, r30		# else get content of RA -> r31
	la	r30, stortab(r30)	# r30 = branch addr in stortab for RA
	.drop 	r30
	mtlr 	r30			# LR = branch address in stortab
	blrl				# set r31=RA
	add	r27, r27, r31		# DAR = (RA) + (RB)
lscbx_pos:
	# l_str_cmp code emulation with DAR modification
	stw	r26, SAVE_WORK1(r25)	# save original instruction to scratch
	stw	r27, SAVE_WORK2(r25)	# save DAR
	lwz	r31, SAVE_XER(r25)	# load XER from save area
	andi.	r30, r31, 0x7F		# mask off to get byte count
	cmpi	0, r30, 0		# if byte count <= 0
	ble-	get_out    		# then go to get_out in al_flih.m4
	rlwinm	r28, r31, 24, 0xFF	# get match byte
	stb	r28, SAVE_MATCH(r25)	# save match byte 
	addi	r27, r27, -1		# prepare DAR for update loads
	add	r30, r30, r27		# get address of last load
	rlwinm	r26, r26, 14, 0xF8	# r26 = bits(6-10) of opcode, RT*8
	stb	r26, SAVE_RT(r25)   	# save copy of RT
	b	next			# branch in al_flih.m4 code

emul_div:
	# upon entry:   r25 = save area addr
	#               r26 = opcode faulting instruction
	#               r27 = SRR0 = faulting instruction addr
	#         MSR = r28 = MSR:DR=0
	#               r29 = MSR:DR=1
	#               r30 = scratch
	#               r31 = primary opcode
	#
	# emulation for: div RD,RA,RB
	#
	#   if (RA)=0 and (RB)>0
	#   then
	#	divwuo RD,MQ,RB   /* to clear XER[OV] */
	#	compute remainder
	#   else
	#	call C code and use divi64()
	#
	# Note: work is done with MSR_DR=1 and sr2 access enable
	#

	# get content of RA
	rlwinm	r30, r26, 19, 0xF8	# r30 = bits(11-15) of opcode, RA*8
	.using	low, r30
	la	r30, stortab(r30)	# r30 = branch addr in stortab for RA
	.drop 	r30
	mtlr 	r30			# LR = branch address in stortab
	blrl				# set r31=RA
	mfsr	r27, sr2
	cmpi	cr0, r31, 0		# if (RA) <> 0 
	rlwinm	r30, r26, 24, 0xF8	# r30 = bits(16-20) of opcode, RB*8
	bne-	call_c_emul		# call C emulation
	# get content of RB
	.using	low, r30
	la	r30, stortab(r30)	# r30 = branch addr in stortab for RB
	.drop 	r30
	stw	r27, SAVE_WORK1(r25)	# save original sr2
	mtlr 	r30			# LR = branch address in stortab
	rlwinm	r27, r27, 0, ~SR_KsKp	# clear "k" bit(s)
	blrl				# set r31 = RB
	mtsr	sr2, r27		# ensure r/w access to mst
	cmpi	cr0, r31, 0		# if (RB) < 0
	lwz	r27, ppda_csa(r25)
	blt-	call_c_emul		# call C emulation
	mtmsr	r29			# turn on Data Relocation
	isync
	lwz	r30, mstmq(r27)		# get content of emulated MQ
	divwuo	r29, r30, r31		# compute quotient MQ/RB and update XER
	mullw	r31, r31, r29		# compute (MQ/RB)*RB
	subfc.	r30, r31, r30		# compute remainder MQ-((MQ/RB)*RB) and set CR0
	stw	r30, mstmq(r27)		# save remainder in MQ
	mtmsr	r28			# turn off Data Relocation
	isync
	lwz	r30, SAVE_WORK1(r25)	# load original sr2 value
	mr	r31, r29		# r31=quotient and
	mtsr	sr2, r30		# restore sr2
	b	pr_st			# store into RT

emul_abs:
	# get content of RA
	rlwinm	r30, r26, 19, 0xF8	# r30 = bits(11-15) of opcode, RA*8
	.using	low, r30
	la	r30, stortab(r30)	# r30 = branch addr in stortab for RA
	.drop 	r30
	mtlr 	r30			# LR = branch address in stortab
	blrl				# set r31=(RA)
	cmpi	cr0, r31, 0
	li	r30, 0
	bge	abs_pos			# branch if (RA) >= 0
	nego.	r31, r31		# set CR0 & XER
	b	pr_st
abs_pos:
	addo.	r31, r30, r31		# set CR0 & XER
	b	pr_st

emul_divs:
	# get content of RB
	rlwinm	r30, r26, 24, 0xF8	# r30 = bits(16-20) of opcode, RB*8
	mfsr	r27, sr2
	.using	low, r30
	la	r30, stortab(r30)	# r30 = branch addr in stortab for RB
	.drop 	r30
	stw	r27, SAVE_WORK1(r25)	# save original sr2
	mtlr 	r30			# LR = branch address in stortab
	rlwinm	r27, r27, 0, ~SR_KsKp	# clear "k" bit(s)
	blrl				# set r31 = (RB)
	# get content of RA -> r31
	rlwinm	r30, r26, 19, 0xF8	# r30 = bits(11-15) of opcode, RA*8
	mr	r28, r31		# save (RB)
	.using	low, r30
	la	r30, stortab(r30)	# r30 = branch addr in stortab for RA
	.drop 	r30
	mtsr	sr2, r27		# ensure r/w access to mst
	mtlr 	r30			# LR = branch address in stortab
	lwz	r27, ppda_csa(r25)	# for access to emulated MQ
	blrl				# set r31 = (RA)
	cmpi	cr0, r28, -1		# divw return undefined RD
	mtmsr	r29			# turn on Data Relocation
	isync
	xori	r29, r29, MSR_DR	# toggle DR
	bne+	cr0, divs_pos		# branch if (RA) <> -1
	rlwinm	r30, r31, 1, -1		# rotate RB
	cmpi	cr0, r30, 1		# compare RB with -2**31
	bne+	cr0, divs_pos		# branch if (RB) <> -2**31
	li	r28, 0			# emulate -2**31/-1
	divwo   r30, r31, r28		# RT=-2**31; overflow in XER
	cmpi	cr0, r28, 0		# MQ=0; update CR0
	stw	r28, mstmq(r27)		# save (MQ)=0 in mst
	mtmsr	r29			# turn off Data Relocation
	isync
	lwz	r30, SAVE_WORK1(r25)	# load original sr2 value
	mtsr	sr2, r30		# restore sr2
	b	pr_st			# store r31=-2**31 into RT
divs_pos:
	divwo	r30, r31, r28		# compute quotient & detect overflow
	mullw	r28, r30, r28		# compute remainder RB = RA - ((RA/RB) * RB)
	subf.	r28, r28, r31		# and update CR0
	ori	r31, r30, 0x0		# quotient to correct reg.
	stw	r28, mstmq(r27)		# save r28=(MQ) in mst
	mtmsr	r29			# turn off Data Relocation
	isync
	lwz	r30, SAVE_WORK1(r25)	# load original sr2 value
	mtsr	sr2, r30		# restore sr2
	b	pr_st			# store r31 into RT

emul_nabs:
	# get content of RA -> r31
	rlwinm	r30, r26, 19, 0xF8	# r30 = bits(11-15) of opcode, RA*8
	.using	low, r30
	la	r30, stortab(r30)	# r30 = branch addr in stortab for RA
	.drop 	r30
	mtlr 	r30			# LR = branch address in stortab
	blrl				# set r31=RA
	mfxer	r30			# clear XER[OV] in case OE is set 
	rlwinm	r30, r30, 0, 0xbfffffff
	cmpi	cr0, r31, 0
	mtxer	r30
	ble	nabs_pos		# branch if RA <= 0
	neg.	r31, r31		# set CR0
	b	pr_st
nabs_pos:
	li	r30, 0
	add.	r31, r30, r31		# set CR0
	b	pr_st

emul_dozi:
	# get content of RA -> r31
	rlwinm	r30, r26, 19, 0xF8	# r30 = bits(11-15) of opcode, RA*8
	.using	low, r30
	la	r30, stortab(r30)	# r30 = branch addr in stortab for RA
	.drop 	r30
	mtlr 	r30			# LR = branch address in stortab
	blrl				# set r31=RA
	# get immediate field -> r30
	andi.	r30, r26, 0xFFFF	# mask off to get SIMM
	extsh	r30, r30		# extend 32 bits
	cmp	0, r31, r30		# if RA > SIMM
	li	r29, 0			# return RT=0
	bgt-	dozi_pos
	subf	r29, r31, r30		# compute RT
dozi_pos:
	mr	r31, r29		# load r31
	rlwinm	r30, r26, 14, 0xF8	# r30 = bits(6-10) of opcode, RT*8
       .using   low, r30
	la	r30, loadtab(r30)	# r30 = branch addr in loadtab for RT
       .drop    r30                     #
        mtlr    r30                     # LR = branch addr in loadtab
        blrl    			# store r31 into RT
	b	pr_ret			# restore and return

emul_mfspr:
	# get SPR field -> r27
	rlwinm	r27, r26, 21, 0x3FF	# r27 = bits(11-20) of opcode
	mfsr	r31, sr2
	cmpi	cr0, r27, 0		# if MQ encoding
	mr	r30, r31		# save original sr2 in r30
	lwz	r27, ppda_csa(r25)
	rlwinm	r31, r31, 0, ~SR_KsKp	# clear "k" bit(s)
	bne-	cr0, call_c_emul	# call C code emulation
	mtsr	sr2, r31		# ensure r/w access to mst
	# get content of emulated MQ
	mtmsr	r29			# turn on Data Relocation
	isync
	lwz	r31, mstmq(r27)		# load MQ from mst
	mtmsr	r28			# turn off Data Relocation
	isync
	mtsr	sr2, r30		# restore sr2
	b	pr_st			# store r31 into RT :OE and Rc always null
					# so is correct to branch to pr_st

emul_mtspr:
	# get SPR field -> r30
	rlwinm	r30, r26, 21, 0x3FF	# r30 = bits(11-20) of opcode
	mfsr	r28, sr2
	cmpi	cr0, r30, 0		# if MQ encoding
	mr	r27, r28		# save original sr2 in r27
	rlwinm	r30, r26, 14, 0xF8	# r30 = bits(6-10) of opcode, RT*8
	bne-	cr0, call_c_emul	# call C code emulation
	# get content of RT -> r31
       .using   low, r30
	la	r30, stortab(r30)	# r30 = branch addr in stortab for RT
       .drop    r30
        mtlr    r30                     # LR = branch addr in stortab
	rlwinm	r28, r28, 0, ~SR_KsKp	# clear "k" bit(s)
        blrl    			# set r31 = RT
	mtsr	sr2, r28		# ensure r/w access to mst
	lwz	r30, ppda_csa(r25)
	# store r31 in MQ emulation
	mtmsr	r29			# turn on Data Relocation
	isync
	xori	r29, r29, MSR_DR	# toggle DR
	stw	r31, mstmq(r30)
	mtmsr	r29			# turn off Data Relocation
	isync
	lwz	r30, SAVE_WORK1(r25)	# load original sr2 value
	mtsr	sr2, r27		# restore sr2
	b	pr_ret			# restore and return

include(i_machine.m4)
include(except.m4)

