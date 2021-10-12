# @(#)03	1.4.1.12  src/bos/kernel/ml/POWER/misc_init.s, sysml, bos41J, 9511A_all 3/7/95 13:12:47
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: misc. functions
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1992,1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#
# NOTE:
#	Many of these functions are overlays, with addresses and sizes
# assigned in overlay.h.  Do not update this file without checking
# overlay.h
#
# 	Overlays are required to be position independent
#
#****************************************************************************

#
# Contains assembler functions that are only used during system
# initialization.  The text for these functions is discared after
# system initialization is complete
#

	.file	"misc_init.s"

include(macros.m4)

#**********************************************************************
#
# NAME: ldsr/mtsr
#
# FUNCTION: load segment register
#	mtsr(segreg,segreg_value)
#       ldsr(segreg,segreg_value)       
#
#  INPUT STATE:
#	r3 = segment register number
#	r4 = segment register value
#
# NOTES:
#	Do not change register usage without updating inline.h
#
# OUTPUT STATE:
#	The specified segment register is loaded with the input value.
#
#  EXECUTION ENVIRONMENT:
#	Overlay data for PowerRS machines and 601
#
#**********************************************************************

	.csect mtsr_sect_pwr[RO]
DATA(mtsr_pwr):
	.globl	DATA(mtsr_pwr)
	rlinm	r3, r3, 28, 0xF0000000	# generate an effective address
	mtsri	r4, 0, r3
	br

ifdef(`_POWER_RS',`
	.machine "pwr"

#**********************************************************************
#
# NAME: chgsr
#
#  FUNCTION: change segment register
#	chgsr(segreg,segreg_value)
#
# INPUT STATE:
#	r3 = segment register number
#	r4 = segment register value
#
# NOTES:
#	Do not change register usage without updating inline.h
#
# OUTPUT STATE:
#	The specified segment register is loaded with the input value.
#	The old segement register is returned
#
# EXECUTION ENVIRONMENT:
#	Overlay data for PowerRS
#
#**********************************************************************

	.csect chgsr_sect_pwr[RO]
DATA(chgsr_pwr):
	.globl	DATA(chgsr_pwr)
	rlinm	r5, r3, 28, 0xF0000000	# generate an effective address
	mfsri	r3, 0, r5
	mtsri	r4, 0, r5
	br

#******************************************************************************
#
# NAME: mfsri_pwr
#
# FUNCTION:
#	Move from segment register indirect.
#
# NOTES:
#	Do not change register usage without updating inline.h
#
# EXECUTION ENVIRONMENT:
#	Overlay data
#
# INPUT:
#	r3 - address whose bits 0-3 form the segment to save
#
# RETURNS:
#	r3 - contents of segment reg.
#
#******************************************************************************

	.csect	mfsri_pwr_overlay[RO]
DATA(mfsri_pwr):
	.globl	DATA(mfsri_pwr)
	mfsri	r3,r0,r3
	br


#******************************************************************************
#
# NAME: mfsr_pwr
#
# FUNCTION:
#	Move from segment register indirect.
#
# EXECUTION ENVIRONMENT:
#	Overlay data
#
# NOTES:
#	Do not change register usage without updating inline.h
#
# INPUT:
#	r3 - segreg number to read
#
# RETURNS:
#	r3 - contents of segment reg.
#
#******************************************************************************

	.csect	mfsr_pwr_overlay[RO]
DATA(mfsr_pwr):
	.globl	DATA(mfsr_pwr)
	rlinm	r3, r3, 28, 0xF0000000	# generate effective address
	mfsri	r3,r0,r3
	br



#******************************************************************************
#
# NAME: sc_front_pwr
#
# FUNCTION:
#	This overlay is placed at 0x1fe0 on Power machines.  It simply
# calls the sc_flih (common sc handler).
#
#*****************************************************************************

	.csect sc_flih_pwr_overlay[PR]
DATA(sc_front_pwr):
	.globl	DATA(sc_front_pwr)
	ba	ENTRY(sc_flih)
	.extern	ENTRY(sc_flih)

#******************************************************************************
#
# NAME: cs_pwr
#
# FUNCTION:
#       The 4.1 cs() implementation is an exported kernel entry
#       point.  The entry point lies in the machine-dependent
#       overlay area.  The Power version uses a fast SVC to
#       implement an atomic load/store sequence.  There is a hook in
#       the ds_flih() to handle page faults during cs().  The Power
#       PC version uses lwarx/stwcx to perform the the atomic op,
#       provided the cs() address is word aligned.  Unaligned Power
#       PC cs() operations are passed to the cmp_swap() system call.
#       This is called through the standards SC handler using a
#       system call index (r2) loaded into a global variable at SI
#       time.  The cmp_swap() system call is serialize access by use
#       of a lock.
#
#       The cs() system call is not available to kernel extensions.
#       This is because it must backup and fabricate a problem state
#       MSR when a page fault occurs.  The ds_flih must assume the
#       cs() caller was in problem state when fabricating the MSR
#       (else there is a security hole).  This problem is solvable
#       with enough work, but it is not felt to be a requirement.
#
#******************************************************************************

	.csect cs_pwr_sect[PR]
DATA(cs_pwr):
	.globl	DATA(cs_pwr)
	cror	0,0,0		# a noop because of machine bug
	svc	1,0,0		# svc 1 link-reg unchanged

#*******************************************************************************
#
# NAME: cs_svc_pwr
#
# FUNCTION:
#	This code is overlayed to 0x1020 (SVC(1) vector).
# 	if a page-fault occurs on load or store we back-track
# to .cs in dsis_flih. a wild-branch to the middle of the
# code will either take a program exception on the rfsvc
# or look like a storage exception at the beginning of .cs.
#
#*******************************************************************************

	.csect cs_sc_pwr_overlay[PR]
DATA(cs_svc_pwr):
	.globl	DATA(cs_svc_pwr)
	l	r6,0(r3)		# load the word
	cmp	0,r4,r6   		# compare the word
	bne 	0,noswap 		# branch if different
	st	r5,0(r3)		# swap word
	cal	r3,0(r0)		# r3 = equal retcode
	rfsvc				# return
noswap:
	cal	r3,1(r0)		# r3 = noswap retcode
	rfsvc				# return

#******************************************************************************
#
# NAME: al_pwr
#
# CALL: None
#
# FUNCTION: This sequence is copied to 0x600 at system initialization time
#	when running on a Power machine.
#
# NOTES:
#	Do not change this function without updating overlay.h
#
# RETURNS : None
#
#******************************************************************************

	.csect	al_ppc_overlay[PR]
DATA(al_prolog_pwr):
	.globl	DATA(al_prolog_pwr)
        st      r15, ra_save(0)         # save original contents of r15
ifdef(`_POWER_MP',`
        l       r15, proc_arr_addr(0)   # load address of ppda
',`
	cal	r15, ppda(0)		# get ppda address
')
        stm     r25, SAVE_R25(r15)      # save off 25-31
	cal	r25, SAVE_WORK0(r15)	# point r25 at start of al flih
        l       r15, ra_save(0)         # load up original contents of r15
        xor     r26,r26,r26             # zero r26
        st      r26, ra_save(0)         # zero out ra_save
        ba      ENTRY(al_flih)		# branch to common alignment code



#*****************************************************************************
#
# NAME: dc_line_size
#
# CALL:	int dc_line_size()
#
# FUNCTION: gets the L1 data cache line size
#
# RETURNS:
#	data cache line size in bytes
#
#*****************************************************************************

	S_PROLOG(dc_line_size)
	clcs	r3, 0xd			# get data cache line size
	br 

#*****************************************************************************
#
# NAME: ic_line_size
#
# CALL:	int ic_line_size()
#
# FUNCTION: gets the L1 instruction cache line size
#
# RETURNS:
#	instruction cache line size in bytes
#
#*****************************************************************************

	S_PROLOG(ic_line_size)
	clcs	r3, 0xC			# get instruction cache line size
	br 

#******************************************************************************
#
# NAME: flih_prolog_pwr
#
# CALL: None
#
# FUNCTION: contains the instruction sequence to be placed at interrupt
#	vectors on Power machines
#
# NOTES:
#	Do not update without changing mc_prolog_pwr
#
# RETURNS: None
#
#******************************************************************************

	.csect	flih_pwr[PR]
DATA(flih_prolog_pwr):			# address exported to C code
	.globl	DATA(flih_prolog_pwr)
	st	r15, ra_save(0)		# save the value of r15
ifdef(`_POWER_MP',`
	l	r15, proc_arr_addr(0)	# get ppda address
',`
	cal	r15, ppda(0)		# get ppda address
')
	st	r0, ppda_save0(r15)	# save a register
	ba	0			# branch to flih

#******************************************************************************
#
# NAME: noop_instr_pwr
#
# CALL: None
#
# FUNCTION: contains the overlay data for no-op instructions used no-op
#	instructions on Power
#
# RETURNS: None
#
#*******************************************************************************

	.csect	noop_inst_pwr[RO]
	.globl	noop_inst_pwr[RO]
	cror	30,30,30

#*******************************************************************************
#
# NAME: mc_flih_prolog
#
# FUNCTION:
#	Instruction sequence overlayed at flih to fix bug in RS1
# processor machine check handling.
#
# NOTES:
#	Do not change without updating flih_prolog_pwr
#
#	Size of routine is in overlay.h
#
# RETURNS: None
#
#*******************************************************************************

	.csect	mc_pwr[PR]
DATA(mc_prolog_pwr):
	.globl	DATA(mc_prolog_pwr)

	dcs				# invalidate icache line to prevent
	bl	mc_next_pwr		# checkstop
mc_next_pwr:
	mflr	r1
	ai	r1, r1, 12
	cli	0, r1
	ics

#	standard power prolog

	st	r15, ra_save(0)		# save the value of r15
ifdef(`_POWER_MP',`
	l	r15, proc_arr_addr(0)	# get ppda address
',`
	cal	r15, ppda(0)		# get ppda address
')
	st	r0, ppda_save0(r15)	# save a register
	ba	DATA(mc_flih)		# branch to flih
	.globl	DATA(mc_flih)


#*******************************************************************************
#
# FUNCTION: Set Current interrupt level for the processor
#
# CALL: void i_cil_set(level)
#       int level;                      /* 0-63.  level to set to */
#
# RETURNS:
#       None
#
#*******************************************************************************

        S_PROLOG(i_cil_set)
        rlinm   r3, r3, 8, 0xFF00       # move it over to CIL byte
                                        # Byte 0 implied 0 for CIL write
        mtspr   ILCR, r3                # set CIL
        br

#*******************************************************************************
#
# NAME: ds_prolog
#
# FUNCTION:
#	overlay for dsi flih prolog.  The ds/is flihs need a prolog
# that does not alter the save[0] field in the ppda
#
# NOTES:
#	Do not change without updating overlay.h
#
# RETUNRS: None
#
#*******************************************************************************

	.csect	ds_sect_pwr[PR]
DATA(ds_prolog_pwr):
	.globl	DATA(ds_prolog_pwr)
	st	r15, ra_save(0)		# save the value of r15
ifdef(`_POWER_MP',`
	l	r15, proc_arr_addr(0)	# get ppda address
',`
	cal	r15, ppda(0)		# get ppda address
')
	ba	DATA(ds_flih)		# branch to flih

#*******************************************************************************
#
# NAME: is_prolog
#
# FUNCTION:
#	overlay for dsi flih prolog.  The ds/is flihs need a prolog
# that does not alter the save[0] field in the ppda
#
# NOTES:
#	Do not change without updating overlay.h
#
# RETUNRS: None
#
#*******************************************************************************

	.csect	is_sect_pwr[PR]
DATA(is_prolog_pwr):
	.globl	DATA(is_prolog_pwr)
	st	r15, ra_save(0)		# save the value of r15
ifdef(`_POWER_MP',`
	l	r15, proc_arr_addr(0)	# get ppda address
',`
	cal	r15, ppda(0)		# get ppda address
')
	ba	DATA(is_flih)		# branch to flih

',)

	_DF(_DF_NOFRAME)

ifdef(`_POWER_PC',`
	.machine "ppc"

#**********************************************************************
#
# NAME: chgsr
#
#  FUNCTION: change segment register
#	chgsr(segreg,segreg_value)
#
# INPUT STATE:
#	r3 = segment register number
#	r4 = segment register value
#
# NOTES:
#	Do not change register usage without updating inline.h
#
# OUTPUT STATE:
#	The specified segment register is loaded with the input value.
#	The old segement register is returned
#
# EXECUTION ENVIRONMENT:
#	Overlay data for 601
#
#**********************************************************************

	.csect chgsr_sect_601[RO]
DATA(chgsr_601):
	.globl	DATA(chgsr_601)
	rlinm	r5, r3, 28, 0xF0000000	# generate an effective address
	mfsrin	r3, r5			# read segment value
	mtsri	r4, r0, r5		# write new value
	br

#**********************************************************************
#
#  NAME: ldsr/mtsr
#
#  FUNCTION: load segment register
#	mtsr(segreg,segreg_value)
#       ldsr(segreg,segreg_value)       
#
# NOTES:
#	Do not change register usage without updating inline.h
#
#  INPUT STATE:
#     r3 = segment register number
#     r4 = segment register value
#
#  OUTPUT STATE:
#     The specified segment register is loaded with the input value.
#
#  EXECUTION ENVIRONMENT:
#     Overlay data for PowerPC
#
#**********************************************************************

	.csect mtsr_sect_ppc[RO]
DATA(mtsr_ppc):
	.globl	DATA(mtsr_ppc)
	rlinm	r3, r3, 28, 0xF0000000	# generate an effective address
	isync
	mtsri	r4,r0,r3
	isync
	br


#**********************************************************************
#
# NAME: chgsr_ppc
#
#  FUNCTION: change segment register
#	chgsr(segreg,segreg_value)
#
# INPUT STATE:
#	r3 = segment register number
#	r4 = segment register value
#
# NOTES:
#	Do not change register usage without updating inline.h
#
# OUTPUT STATE:
#	The specified segment register is loaded with the input value.
#	The old segement register is returned
#
# EXECUTION ENVIRONMENT:
#	Overlay data for PowerPC
#
#**********************************************************************

	.csect chgsr_sect_ppc[RO]
DATA(chgsr_ppc):
	.globl	DATA(chgsr_ppc)
	rlinm	r5, r3, 28, 0xF0000000	# generate an effective address
	mfsrin	r3, r5			# read old value
	isync
	mtsri	r4, 0, r5		# write new value
	isync
	br


#******************************************************************************
#
# NAME: mfsri_ppc
#
# FUNCTION:
#	Move from segment register indirect.
#
# NOTES:
#	Do not change register usage without updating inline.h
#
# EXECUTION ENVIRONMENT:
#	Called though the branch table
#
# INPUT:
#	r3 - address whose bits 0-3 form the segment to save
#
# RETURNS:
#	r3 - contents of segment reg.
#
#******************************************************************************

        .csect mfsri_ppc_overlay[RO]
DATA(mfsri_ppc):
	.globl	DATA(mfsri_ppc)
	mfsrin	r3, r3
	br

#******************************************************************************
#
# NAME: mfsr_ppc
#
# FUNCTION:
#	Move from segment register indirect.
#
# EXECUTION ENVIRONMENT:
#	Called though the branch table
#
# NOTES:
#	Do not change register usage without updating inline.h
#
# INPUT:
#	r3 - seg reg number to read
#
# RETURNS:
#	r3 - contents of segment reg.
#
#******************************************************************************

        .csect mfsr_ppc_overlay[RO]
DATA(mfsr_ppc):
	.globl	DATA(mfsr_ppc)
	rlinm	r3, r3, 28, 0xF0000000	# generate effective address
	mfsrin	r3, r3
	br

#******************************************************************************
#
# NAME: noop_instr_ppc
#
# CALL: None
#
# FUNCTION: contains the overlay data for no-op instructions used no-op
#	instructions on Power PC
#
# RETURNS: None
#
#*******************************************************************************

	.csect	noop_inst_ppc[RO]
	.globl	noop_inst_ppc[RO]
	oriu	r0, r0, r0

#*******************************************************************************
#
# NAME: sc_front (PPC Version)
#
# FUNCTION:
#       There is one common SVC handler with a machine-dependent
#       front end for each architecture.  The Power SC vector is
#       simply a branch to the common handler.  The Power PC SC
#       vector must move the move SRR1 to the CTR, and turn
#       on translation before branching to the common handler.  The
#       front ends are overlays set up at system initialization
#       time.
#
#*******************************************************************************

	.csect sc_front_ppc_overlay[PR]
DATA(sc_front_ppc):
	.globl	DATA(sc_front_ppc)
	lil	r12, DISABLED_MSR	# disabled msr xlate on
	mfspr	r11, SRR1		# get old msr value
	mtmsr	r12			# xlate on
	mtctr	r11			# put it here for SC flih
	ba	ENTRY(sc_flih)		# common SC handler
	.extern	ENTRY(sc_flih)

#*******************************************************************************
#
# NAME: cs (PPC version)
#
# FUNCTION:
#       The 4.1 cs() implementation is an exported kernel entry
#       point.  The entry point lies in the machine-dependent
#       overlay area.  The Power version uses a fast SVC to
#       implement an atomic load/store sequence.  There is a hook in
#       the ds_flih() to handle page faults during cs().  The Power
#       PC version uses lwarx/stwcx to perform the the atomic op,
#       provided the cs() address is word aligned.  Unaligned Power
#       PC cs() operations are passed to the cmp_swap() system call.
#       This is called through the standards SC handler using a
#       system call index (r2) loaded into a global variable at SI
#       time.  The cmp_swap() system call is serialize access by use
#       of a lock.
#
#       The cs() system call is not available to kernel extensions.
#       This is because it must backup and fabricate a problem state
#       MSR when a page fault occurs.  The ds_flih must assume the
#       cs() caller was in problem state when fabricating the MSR
#       (else there is a security hole).  This problem is solvable
#       with enough work, but it is not felt to be a requirement.
#
# NOTES:
#	Do not change without updating overlay.h
#
#*******************************************************************************

	.csect cs_ppc_overlay[PR]
DATA(cs_ppc):
	.globl	DATA(cs_ppc)
	andil.	r6, r3, 0x3		# check for work aligned address
	bne	cr0, cs_unalign		# branch if not word aligned
cs_again:
	lwarx	r6, 0, r3		# load word
	cmp	cr1, r6, r4		# check value
	bne	cr1, cs_fail		# if wrong value then fail
	stwcx.	r5, 0, r3		# store new value
	bne	cr0, cs_again		# try again
	lil	r3, 0			# good return code
	br				# return

cs_fail:
	stwcx.	r6, 0, r3		# clear reservation
	lil	r3, 1			# bad return code
	br				# return

cs_unalign:
	l	r2, DATA(cmp_swap_index)(0)	# sc index
	ba	ENTRY(svc_instr)	# do standard system call
	.extern	ENTRY(svc_instr)

#******************************************************************************
#
# NAME: al_ppc
#
# CALL: None
#
# FUNCTION: This sequence is copied to 0x600 at system initialization time
#	when running on a PPC machine.
#
# NOTES:
#	Do not change this function without updating overlay.h
#
# RETURNS : None
#
#******************************************************************************

	.csect	al_ppc_overlay[PR]
DATA(al_prolog_ppc):
	.globl	DATA(al_prolog_ppc)	
	mtspr	SPRG1,r15		# save original contents of r15
	mfspr	r15, SPRG0		# load address of ppda
	stm	r25, SAVE_R25(r15)	# save off 25-31
	or	r25,r15,r15		# move save address to r25
	mfspr	r15, SPRG1		# load up original contents of r15
	ba	ENTRY(al_flih)
	.extern	ENTRY(al_flih)

#******************************************************************************
#
# NAME: flih_prolog_ppc
#
# CALL: None
#
# FUNCTION: contains the instruction sequence to be placed at interrupt
#	vectors on Power machines
#
# RETURNS: None
#
#******************************************************************************

	.csect	flih_ppc[PR]
DATA(flih_prolog_ppc):			# address exported to C code
	.globl	DATA(flih_prolog_ppc)

	mtspr	SPRG1, r15		# save a working version
	mfspr	r15, SPRG0		# get ppda pointer
	st	r0, ppda_save0(r15)	# save a register
	ba	0			# branch to flih

#*******************************************************************************
#
# NAME: mtsprgs()
#
# CALL:	void mtsprgs(value0, value2, value3)
#	int value0;     /* value to load into SPRG0 - ppda */
#	int value2;     /* value to load into SPRG2 - curthread */
#	int value3;     /* value to load into SPRG3 - csa  */
#
# FUNCTION: load sprg0, sprg2 and sprg3
#
# RETURNS: None
#
#*******************************************************************************

	S_PROLOG(mtsprgs)
	mtspr	SPRG0, r3
	mtspr	SPRG2, r4
	mtspr	SPRG3, r5
	br

#*******************************************************************************
#
# NAME: writedec_vec
# 
# FUNCTION:
# 	write the decrementer interrupt vector and invalidate cache
#
# 	input: 
#         r3=data 
#                                           
#	output:  the decrementer interrupt vector is changed.
#
# EXECUTION ENVIRONMENT:
#	called by tinit() to allow decrementer interrupts
#
#*******************************************************************************

 	S_PROLOG(writedec_vec)
#
					# DRBIT, 0x10
	mfmsr	r6			# get current msr
	rlinm	r7, r6, 0, ~MSR_DR	# unset data relocate bit
	mtmsr	r7			# put it back in msr
	isync

	lil	r4, 0x900		# address of vector
	st	r3,0(r4)		# store word
	dcbf	0, r4			# flush/invalidate cache
	sync
	icbi	0, r4
	mtmsr	r6			# reset msr to the way it was
	isync

	br

#*******************************************************************************
#
# NAME: ds_prolog
#
# FUNCTION:
#	overlay for dsi flih prolog.  The ds/is flihs need a prolog
# that does not alter the save[0] field in the ppda
#
# NOTES:
#	Do not change without updating overlay.h
#
# RETUNRS: None
#
#*******************************************************************************

	.csect	ds_sect_ppc[PR]
DATA(ds_prolog_ppc):
	.globl	DATA(ds_prolog_ppc)	
	mtspr	SPRG1, r15		# save the value of r15
	mfspr	r15, SPRG0		# get ppda pointer
	ba	DATA(ds_flih)		# branch to flih

#*******************************************************************************
#
# NAME: is_prolog
#
# FUNCTION:
#	overlay for dsi flih prolog.  The ds/is flihs need a prolog
# that does not alter the save[0] field in the ppda
#
# NOTES:
#	Do not change without updating overlay.h
#
# RETUNRS: None
#
#*******************************************************************************

	.csect	is_sect_ppc[PR]
DATA(is_prolog_ppc):
	.globl	DATA(is_prolog_ppc)
	mtspr	SPRG1, r15		# save the value of r15
	mfspr	r15, SPRG0
	ba	DATA(is_flih)		# branch to flih

#*******************************************************************************
#
# NAME: dse_prolog
#
# FUNCTION:
#	overlay for dsi flih prolog.  The DSE flih needs a prolog
# that does not alter the save[0] field in the ppda
#
# NOTES:
#	Do not change without updating overlay.h
#
# RETUNRS: None
#
#*******************************************************************************

	.csect	dse_sect_ppc[PR]
DATA(dse_prolog_ppc):
	.globl	DATA(dse_prolog_ppc)
	ba	DATA(dse_flih)		# branch to flih
	.extern	DATA(dse_flih)

',)
	.extern	DATA(is_flih)
	.extern DATA(ds_flih)

include(scrs.m4)
include(low_dsect.m4)
include(machine.m4)
include(systemcfg.m4)
