# @(#)74        1.16  src/bos/kernel/ml/POWER/ex_flih_ppc.s, sysml, bos41J, 9511A_all 3/7/95 13:12:54
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: ex_flih_ppc
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

	.file "ex_flih_ppc.s"
	.machine "ppc"

	.using	low, r0

include(macros.m4)

#------------------------------------------------------------------------------
#
# Name: ex_flih_ppc
#
# Function:
#
#   The External FLIH is entered when the machine takes an "external" or I/O
#   interrupt. On entry, interrupts are disabled and translate is off.
#
#------------------------------------------------------------------------------

DATA(ex_flih_ppc):
	.globl	DATA(ex_flih_ppc)
ENTRY(ex_flih_ppc):
	.globl	ENTRY(ex_flih_ppc)

	mfspr	r0, LR

#	Call the state-saving routine, which will set up kernel and user
#	segment r/w addressability and stash all the state except the
#	floating point registers in the mst pointed to by csa.

	bl	ENTRY(state_save_pc)
	.extern	ENTRY(state_save_pc)

#	On return from state_save, the following registers are set up:
#	    r1	   Address of stack (for C)
#	    r2	   Address of kernel TOC
#	    r13	   Address of current  mst save area (new value of csa)
#	    r14	   Address of previous mst save area (old value of csa)
#	    r15	   Address of PPDA
#	    sr0	   R/W access to kernel segment
#	    sr2	   R/W access to process segment
#	    sr14   R/W access to kernel extension segment

#	Perform initial interrupt processing

	cal	r3, 5(0)		# Interrupt "level" 5 for 0x500
	cal	r4, -1(0)		# Flag to cause mstintpri not to be set
	bl	ENTRY(begin_interrupt)
	.extern	ENTRY(begin_interrupt)
#
#	Non-volatile Register usage:  Note above registers after state_save
#
#	r16	Address of interrupt control registers
#	r17	XIRR from BA+4 (side effects)
#	r18	Address of buid_map
#	r19	buid of interrupt source
#	r20	source level on the buid
#	r21	poll array group for BUID
#	r22	unused
#	r23	processor level of the interrupt
#
	l	r16, ppda_intr(PPDA_ADDR) # Address of interrupt registers
	l	r17, xirr(r16)		# get XIRR value with side effects

ifdef(`INTRDEBUG',`
	lil     r3, 0x4953              # FLIS - flih start
	oriu	r3, r3, 0x464c
	mr      r4, r17                 # XIRR read in on start
	lbz	r5, cppr(r16)		# current CPPR
	lbz	r6, mstintpri(r14)	# previous intpri
	bla	 ENTRY(mltrace)
	.extern ENTRY(mltrace)
',)

	LTOC(r18,buid_map,data)		# address of buid_map
	andil.	r22, r17, XISR_MASK	# Check for phantom interrupt
	rlinm	r19, r17, 28, 0x1FF	# Isolate BUID
	rlinm	r20, r17, 0, 0xF	# Isolate source level
	sli	r12, r19, 1		# multiply BUID by 2
	beq	ex_phantom
	lhzx	r21, r18, r12		# get poll array group number
#
#	Find poll array entry, using group number and source level to
#	index into array.
#
	LTOC(r12,i_data,data)		# Load addr of "i_data" table
	cal	r11, i_dpoll(r12)	# point to poll array
	sli	r4, r21, 4		# move poll array group over just
					# enough to add source level
	a	r23, r4, r20		# add in source level
	sli	r4, r23, I_DATA_SH	# now should have poll array index
	lx	r3, r4, r11		# get pointer intr structure

#	If value in r3 is NULL, then this was a
#	"phantom interrupt" -- record it and keep going.

	cmpi	cr0, r3, nullA		# Check for NULL pointer
	bne+	cr0, ex_05_r3		# non-NULL, continue processing

ex_phantom:
ifdef(`MLDEBUG',`
	TRAP				# DEBUG ONLY
',)

	l	r3, DATA(phantom_int)	# Record this phantom interrupt
	ai	r3, r3, 1		#   by bumping count
	st	r3, DATA(phantom_int)
	b	ENTRY(finish_interrupt) # skip further processing
	.extern ENTRY(finish_interrupt)

#
#	Begin software-managed interrupt section.  At this point, we have
#	the dummy interrupt structure from which we must extract the level and
#	priority fields.  The priority field is turned into a bit mask into
#	i_softpri and also into an array index into i_prilvl.  The
#	extracted level field is turned into a bit mask with
#	i_prilvl[priority].
#
#	Need to queue the interrupt as follows:
#	    ppda->i_softpri |= 0x8000 >> dummy->priority;
#	    ppda->i_prilvl[dummy->priority] |= 0x80000000 << dummy->level;
#
#
#	Relevant register usage:
#
#	 r3 - addr of dummy struct
#	 r4 - bit for i_softpri shift, then resultant bit-mask
#	 r5 - bit for i_prilvl shift, then resultant bit-mask
#	 r6 - dummy->level
#	 r7 - &ppda->i_prilvl
#	 r8 - dummy->priority
#	 r9 - offset of ppda->i_prilvl[priority]
#	r10 - i_prilvl[priority]
#	r11 - ppda->i_softpri
#
#
ex_05_r3:
	la   	r7,ppda_prilvl0(PPDA_ADDR) # &ppda->i_prilvl
	lhz	r11, ppda_softpri(PPDA_ADDR) # ppda->i_softpri
	lwz	r6, intr_level(r3)	# get dummy->level
	lwz	r8, intr_priority(r3)	# get dummy->priority
	addis	r5, 0, 0x8000		# 8000 0000 -- bit for i_prilvl shift
	srw	r5, r5, r6		# set level bit to OR into i_prilvl[N]
	rlwinm	r9, r8, 2, 0x3c		# shift priority to get offset to
					#  i_prilvl[priority] (array of ints)
	lwzx	r10, r7, r9		# get i_prilvl[dummy->priority]
	li	r0, 0
	ori	r4, r0, 0x8000		# 8000 -- bit for i_softpri shift
	srw	r4, r4, r8		# set priority bit to OR into i_softpri
	or	r11, r11, r4		# OR in priority bit in i_softpri
	or	r10, r10, r5		# OR in level into i_prilvl[N]

	sth	r11, ppda_softpri(PPDA_ADDR) # store new i_softpri
	stwx	r10, r7, r9		# store new i_prilvl[priority]

#
#	Interrupt now queued.  Trace it (if INTRDEBUG) and branch to
#	finish_interrupt
#

ifdef(`INTRDEBUG',`
	lil     r3, 0x4945              # FLIE - flih end
	oriu	r3, r3, 0x464c
	mr      r4, r17                 # XIRR on start
	mr	r5, r11			# ppda->i_sofpri stored
	lbz	r6, mstintpri(r14)	# previous intpri
	bla	 ENTRY(mltrace)
',)

#	Finish interrupt processing; resume interrupted program

	b       ENTRY(finish_interrupt)
	.extern	ENTRY(finish_interrupt)

	.toc
	TOCE(i_data, data)
	TOCE(buid_map, data)
	TOCE(_system_configuration,data)

include(flihs.m4)
include(scrs.m4)
include(mstsave.m4)
include(seg.m4)
include(machine.m4)
include(intr.m4)
include(interrupt.m4)
include(low_dsect.m4)
include(systemcfg.m4)

