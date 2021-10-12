# @(#)63        1.4  src/bos/kernel/ml/POWER/tlb_flih.s, sysml, bos41J, 9522A_all 5/30/95 18:12:49
#******************************************************************************
#******************************************************************************
#
# COMPONENT_NAME: (SYSML) machine language routines
#
# FUNCTIONS: tlb_flih
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#******************************************************************************

ifdef(`_POWER_603',`
	.file "tlb_flih.s"
	.machine "any"

	.set	IDCMP_Hbit, 0x040		
	.set    PTE_Rbit, 0x100
	.set    PTE_Cbit, 0x080
	.set	PTE_Gbit, 0x008

#******************************************************************************
#
# (1) These three flihs can only use GPR r0, r1, r2, and r3 while TGPR bit
#     in msr is set.
#
# (2) It is necessary to set R bit in PTE for I-fetch tlb miss and 
#     D-read tlb miss, and set both R bit and C bit in PTE for D-write tlb miss
#     and C = 0 exception to keep PTE in sync with TLB
#
# (3) Each of these flihs must fit in 256 bytes.
#
#******************************************************************************

#******************************************************************************
#
# NAME:	 tlb_flih_ifm
#
# FUNCTION:  This is 603 specific interrupt handler for tlb miss on
#            instruction fetch.
#	     This code is overlaid at 0x1000 during system initialization
#
# Execution Environment:  interrupts disabled, I/D translation off	      	
#
#   On entry, the SPR registers setting is the same as that for ISI except that 
#   (a) CRF0 is saved in SRR1,
#   (b) TGPR in msr is set,
#   (c) IMISS register contains the effective address that causes the fault,
#       HASH1 and HASH2 registers contain the physical addresses at which the
#       primary and secondary PTEGs (respectively) reside, and
#       ICMP register contains the first word in the PTE for which the table
#       search is looking (H bit is 0)
#
#   CRF0 needs to be restored and TGPR in msr needs to be reset before exiting
#   from this routine.
#
#   Note that PPC architecture forbits I-fetch from guarded storage.
#   But 603 processor does not check G bit during I-fetch.
#   It depends on this handler to check against I-fetching from guarded
#   stoarge.  The logic in this handler will also prevent the existence of 
#   G=1 I-tlb entries.
#******************************************************************************

DATA(tlb_flih_ifm):
	mfspr	r2,HASH1	# r2 = address of the first entry of primary PTEG
	mfctr	r0		# r0 = saved ctr
	mfspr	r3,ICMP		# r3 = search target
ifm_grp:
	lil	r1,0x8
	addi	r2,r2,-8	# subtract by 8
	mtctr	r1		# set ctr to 8
ifm_next:
	lwzu	r1,8(r2)	# read first word of the entry
	cmp	cr0,r1,r3
	beq	ifm_found
	bdnz+	ifm_next	

	andi.	r1,r3,IDCMP_Hbit  # test H bit 
	mfspr	r2,HASH2	# r2 = address of the first entry of 2nd PTEG
	bne-	cr0,ifm_not_found

	# go for the secondary PTEG
	ori	r3,r3,IDCMP_Hbit  # set H bit
	b	ifm_grp		# search 2nd PTEG

ifm_found:
	lwz	r1,4(r2)	# target found, read the second word of the entry
	mtctr	r0		# restore ctr
	andi.	r3,r1,PTE_Gbit  
	bne-	ifm_Gbit	

	mfspr	r3,SRR1
	ori	r1,r1,PTE_Rbit	# set R bit
	mfspr	r0,IMISS
	mtcrf	0x80,r3		# restore cr0
	mtspr	RPA,r1		
	tlbli	r0		# load instruction tlb entry selected by r0 and 
				# srr1(w) with ICMP and RPA	
	stw	r1,4(r2)	# set R bit in PTE

	# It is NOT necessary to clear bit 0 of SRR1 first.
	# Bit 0 of msr in 603 is reserved and dont care.  

DATA(tlb_nop1):			# 603e 1.4 workaround sync patched at si
	.globl	DATA(tlb_nop1)
	sync
	rfi

ifm_not_found:
	cau	r1,0,SRR_IS_PFT>16 # set bit 1 to indicate PTE miss
	mtctr	r0		# restore ctr

ifm_goto_ISI:
	# reconstruct MSR and SRR1 before branching to is_flih
	# On entry, SRR0 contains the EA of the next instruction to be executed

	mfspr	r3,SRR1
	andi.	r2,r3,0xffff	# r2 = old msr
	mfmsr	r0
	or	r2,r2,r1	
	andi.	r0,r0,0xffff	# clear TGPR bit (14) 
	mtspr	SRR1,r2		
	mtcrf	0x80,r3		# restore cr0
	mtmsr	r0		
	isync			# can not use r4-r31 until TGPR bit is clear

	# is_prolog_ppc
	mtspr	SPRG1, r15	# save the value of r15
	mfspr	r15, SPRG0	# get ppda pointer
	ba	DATA(is_flih)	# branch to flih
 	.extern	DATA(is_flih)

ifm_Gbit:
	# set bit 3 to indicate I fetch from Guarded page
	cau	r1,0,SRR_IS_GUARD>16 
	b	ifm_goto_ISI


#******************************************************************************
#
# NAME:	 tlb_flih_drm
#
# FUNCTION:  This is 603 specific interrupt handler for tlb miss on
#	     data read.
#	     This code is overlaid at 0x1100 during system initialization
#
# Execution Environment:  interrupts disabled, I/D translation off	      	
#
#   On entry, the SPR registers setting is the same as that for DSI except that 
#   (a) CRF0 is saved in SRR1,
#   (b) TGPR in msr is set,
#   (c) DMISS register contains the effective address that causes the fault,
#	DAR dees not contain the effective address that causes the fault,
#       HASH1 and HASH2 registers contain the physical addresses at which the
#       primary and secondary PTEGs (respectively) reside, and
#       DCMP register contains the first word in the PTE for which the table
#       search is looking (H bit is 0).
#
#   CRF0 needs to be restored and TGPR in msr needs to be reset before exiting
#   from this routine.
#******************************************************************************

DATA(tlb_flih_drm):
	mfspr	r2,HASH1	# r2 = address of the first entry of primary PTEG
	mfctr	r0		# r0 = saved ctr
	mfspr	r3,DCMP		# r3 = search target
drm_grp:
	lil	r1,0x8
	addi	r2,r2,-8	# subtract by 8
	mtctr	r1		# set ctr to 8
drm_next:
	lwzu	r1,8(r2)	# read first word of the entry
	cmp	cr0,r1,r3
	beq	drm_found
	bdnz+	drm_next	

	andi.	r1,r3,IDCMP_Hbit  # test H bit 
	mfspr	r2,HASH2	  # r2 = address of the first entry of 2nd PTEG
	bne-	cr0,drm_goto_DSI

	# go for the secondary PTEG
	ori	r3,r3,IDCMP_Hbit  # set H bit
	b	drm_grp		  # search 2nd PTEG

drm_found:
	lwz	r1,4(r2)	# target found, read the second word of the entry
	mtctr	r0		# restore ctr
	mfspr	r3,SRR1		
	ori	r1,r1,PTE_Rbit	# set R bit
	mfspr	r0,DMISS
	mtcrf	0x80,r3		# restore cr0
	mtspr	RPA,r1		
	tlbld	r0		# load data tlb entry selected by r0 and srr1(w) 
				# with DCMP and RPA	
	stw	r1,4(r2)	# set R bit in PTE

DATA(tlb_nop2):			# 603e 1.4 workaround sync patched at si
	.globl	DATA(tlb_nop2) 	# time
	sync
	rfi

drm_goto_DSI:
	# reconstruct SRR1, DSISR, DAR, and MSR 
        # On entry, SRR0 contains the EA of the instruction that causes the 
        # interrupt.

	mtctr	r0		# restore ctr
	mfspr	r3,SRR1		
	mfspr	r0,DMISS
	andi.	r2,r3,0xffff	# extract old msr
	mtspr   DAR,r0		# save DMISS in DAR
	cau	r1,0,DSISR_PFT>16
	mfmsr	r0	
	mtspr	SRR1,r2		# and save it in SRR1
	andi.	r0,r0,0xffff	
	mtspr	DSISR,r1	# set bit 1 of DSISR
	mtcrf	0x80,r3		# restore cr0
	mtmsr	r0		# clear bit 0 - 15 of msr, esp TGPR bit (14)
	isync

	# ds_prolog_ppc
	mtspr	SPRG1, r15	# save the value of r15
	mfspr	r15, SPRG0	# get ppda pointer
	ba	DATA(ds_flih)	# branch to flih
 	.extern	DATA(ds_flih)
		



#******************************************************************************
#
# NAME:	 tlb_flih_dwm
#
# FUNCTION:  This is 603 specific interrupt handler for tlb miss or C = 0 
#            exception on data write.
#	     This code is overlaid at 0x1200 during system initialization
#
# Execution Environment:  interrupts disabled, I/D translation off	      	
#
#   On entry, the SPR registers setting is the same as that for DSI except that 
#   (a) CRF0 is saved in SRR1,
#   (b) TGPR in msr is set,
#   (c) DMISS register contains the effective address that causes the fault,
#	DAR does not contain the effective address that causes the fault,
#       HASH1 and HASH2 registers contain the physical addresses at which the
#       primary and secondary PTEGs (respectively) reside, and
#       DCMP register contains the first word in the PTE for which the table
#       search is looking (H bit is 0).
#
#   CRF0 needs to be restored and TGPR in msr needs to be reset before exiting
#   from this routine.
#******************************************************************************
	
DATA(tlb_flih_dwm):
	mfspr	r2,HASH1	# r2 = address of the first entry of primary PTEG
	mfctr	r0		# r0 = saved ctr
	mfspr	r3,DCMP		# r3 = search target
dwm_grp:
	lil	r1,0x8
	addi	r2,r2,-8	# subtract by 8
	mtctr	r1		# set ctr to 8
dwm_next:
	lwzu	r1,8(r2)	# read first word of the entry
	cmp	cr0,r1,r3
	beq	dwm_found
	bdnz+	dwm_next	

	andi.	r1,r3,IDCMP_Hbit  # test H bit 
	mfspr	r2,HASH2	# r2 = address of the first entry of 2nd PTEG
	bne-	cr0,dwm_not_found

	# go for the secondary PTEG
	ori	r3,r3,IDCMP_Hbit  # set H bit
	b	dwm_grp		# search 2nd PTEG

	# The following logic will prevent the setting of C bit in PTE
	# in case protection is violated.
dwm_found:
	lwz	r1,4(r2)	# target found, read the second word of the entry
	mtctr	r0		# restore ctr
	rlinm.	r3,r1,0,PTE_Cbit # check C bit
	mfspr	r0,DMISS
	beq-	cr0,dwm_C0       # branch if C bit == 0     

dwm_chkok:
	mfspr	r3,SRR1		
	ori	r1,r1,(PTE_Rbit|PTE_Cbit) # set R bit and C bit
	mtcrf	0x80,r3		# restore cr0
	mtspr	RPA,r1		

	# It is NOT necessary to execute "tlbie" to synchronize instruction tlb
	# with data tlb (C bit may be different).
	# C bit in instruction tlb is dont care.

	tlbld	r0		# load data tlb entry selected by r0 and srr1(w) 
				# with DCMP and RPA	
	stw	r1,4(r2)	# set R bit and C bit in PTE

DATA(tlb_nop3):			# 603e 1.4 workaround sync patched at si
	.globl	DATA(tlb_nop3) 	# time
	sync
	rfi

dwm_C0:
	# C bit == 0
	rlinm.	r3,r1,0,30,30   # check for PP == 1x 
	beq-	cr0,dwm_chk0
	# PP == 1x
	rlinm.	r3,r1,0,31,31	# check for PP == 10
	beq+	cr0,dwm_chkok   # branch if PP == 10

	# PP == 11	
	cau	r1,0,(DSISR_PROT|DSISR_ST)>16
	b	dwm_goto_DSI    

dwm_chk0:
	# PP == 00 or 01
	mfspr	r3,SRR1		
	andi.	r3,r3,MSR_PR	# test if user mode	
	mfsrin	r3,r0           # r3 = SR of faulting eaddr
	bne-	dwm_user
	rlinm.	r3,r3,0,1,1	# check for Ks bit
	beq+	dwm_chkok	# branch if Ks bit == 0
	cau	r1,0,(DSISR_PROT|DSISR_ST)>16
	b	dwm_goto_DSI    

dwm_user:
	rlinm.	r3,r3,0,2,2	# check for Kp bit
	beq+	dwm_chkok	# branch if Kp bit == 0
	cau	r1,0,(DSISR_PROT|DSISR_ST)>16
	b	dwm_goto_DSI    

dwm_not_found:
	mtctr	r0		# restore ctr
	cau	r1,0,(DSISR_PFT|DSISR_ST)>16
	mfspr	r0,DMISS

dwm_goto_DSI:
	# reconstruct SRR1, DSISR, DAR, and MSR 
        # On entry, SRR0 contains the EA of the instruction that causes the 
        # interrupt.

	mtspr   DAR,r0		# save DMISS in DAR
	mfspr	r3,SRR1		
	andi.	r2,r3,0xffff	# extract old msr
	mfmsr	r0	
	mtspr	SRR1,r2		# and save it in SRR1
	andi.	r0,r0,0xffff	
	mtspr	DSISR,r1	
	mtcrf	0x80,r3		# restore cr0
	mtmsr	r0		# clear bit 0 - 15 of msr, esp TGPR bit (14)
	isync

	# ds_prolog_ppc
	mtspr	SPRG1, r15	# save the value of r15
	mfspr	r15, SPRG0	# get ppda pointer
	ba	DATA(ds_flih)	# branch to flih
 	.extern	DATA(ds_flih)

	.globl	DATA(tlb_flih_ifm)
	.globl	DATA(tlb_flih_drm)
	.globl	DATA(tlb_flih_dwm)
include(scrs.m4)
include(machine.m4)
',)
