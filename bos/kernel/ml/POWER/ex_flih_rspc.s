# @(#)70	1.11  src/bos/kernel/ml/POWER/ex_flih_rspc.s, sysml, bos41J 5/2/95 16:53:49
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: ex_flih_rspc
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

	.file "ex_flih_rspc.s"

	.using  low, r0

include(macros.m4)
	.machine "ppc"

#------------------------------------------------------------------------------
#
# Name:   ex_flih_rspc
#
# Function:
#
#   The External FLIH is entered when the machine takes an "external" or I/O
#   interrupt.  On entry, interrupts are disabled and translate is off.
#
#------------------------------------------------------------------------------

DATA(ex_flih_rspc):
	.globl  DATA(ex_flih_rspc)
ENTRY(ex_flih_rspc):
	.globl  ENTRY(ex_flih_rspc)

	mflr	r0

#	Call the state-saving routine, which will set up kernel and user
#	segment r/w addressability and stash all the state except the
#	floating point registers in the mst pointed to by csa.

	bl	ENTRY(state_save_pc)
	.extern ENTRY(state_save_pc)

#	On return from state_save, the following registers are set up:
#	 r1	Address of stack (for C)
#	 r2	Address of kernel TOC
#	r13	Address of current  mst save area (new value of csa)
#	r14	Address of previous mst save area (old value of csa)
#	r15	Address of PPDA
#	sr0	R/W access to kernel segment
#	sr2	R/W access to process segment
#	sr14	R/W access to kernel extension segment

#	Perform initial interrupt processing

	li	r3, 5			# Interrupt "level" 5 for 0x500
	li	r4, -1			# Flag to cause mstintpri not to be set
	bl	ENTRY(begin_interrupt)
	.extern ENTRY(begin_interrupt)

#
#	Non-volatile register usage:  Note above registers after state_save
#
#	cr1 - Eq - Primary = MPIC (implies Secondary = 8259)
#	      Ne - otherwise (Primary = 8259)
#	cr2 - 601 vs 603/604
#	cr3 - Interrupt source < 8
#	cr4 - How Level relates to 0x10
#	sr11 - MPIC segment address		601 only
#	sr12 - IO space segment address		601 only
#	sr15 - interrupt hardware address	601 only
#	BAT0 - interrupt hardware address	603/604 only
#	BAT1 - IO space segment address		603/604 only
#	BAT2 - MPIC segment address		603/604 only
#	r0 - 0
#	r16 - 8259 IVR contents
#	r17 - contents of ppda->i_softpri
#	r18 - base address of i_prilvl[]
#	r19 - index into i_prilvl[]
#	r20 - contents of ppda->i_prilvl[priority]
#	r21 - 0x00008000 - bit for i_softpri shift
#	r22 - 0x80000000 - bit for i_prilvl[] shift
#	r23 - priority value of interrupt - from dummy struct
#	r24 - Pointer to dummy intr structure
#	r25 - IO segment address
#	r26 - Base address of i_data structure
#	r27 - level value of i_prilvl- from dummy struct
#	r28 - Rotate priority + non-specific EOI
#	r29 - 
#	r30 - intctl_pri, i.e. type of the primary interrupt controller
#	r31 - mpic_base, later a pointer to the MPIC per-processor registers
#		for the running CPU
#
# logic:
#	- Read the interrupt vector register.  It returns the level of the
#	  highest priority interrupt, and luckly the one we are processing.
#	  Because of the way the master and slave 8259s were setup earlier
#	  this value will be in the range of [0..1, 3..15].  The missing 2
#	  is because the slave cascades into the master on that level.
#	- Find dummy intr struct off of poll array.
#	- update ppda->i_softpri and level in ppda->i_prilvl[priority]
#	- Write the non-specific EOI, with priority rotate, to master 8259
#	- If interrupt level was >8 write EOI(+rotate) to slave
#	- Call finish_interrupt
#

	lwz	r3, syscfg_impl		# if 601 then use BUID 7f
	lwz	r31, DATA(mpic_base)	# Real base of MPIC, if present
	lhz	r30, DATA(intctl_pri)	# Primary Interrupt Controller
	li	r17, rspcint_vect	# load EA of IVR register
	cmpi	cr2, r3, POWER_601	#
	cmpi	cr1, r30, INT_CTL_MPIC	# Primary an MPIC?
	LTOC(r26, i_data, data)		# Address of i_data struct
	bne-	cr2, ex_flih_bat	# set up BATs if not a 601

#	601 uses BUID 7F
	lis	r7, u.rspcint_seg	# IVR:
	lis	r25, u.rspcio_seg	# ISA IO: Setup ISA I/O segment addrs
	rlwinm	r6, r31, 4, 0xF		# MPIC: high 4 RADDR bits

	ori	r7, r7, l.rspcint_seg	# IVR:
	ori	r25, r25, l.rspcio_seg	# ISA IO:
	oris	r6, r6, u.buid7f_gen	# MPIC:

	mtsr	sr15, r7		# IVR: sr15 will allow access to IVR
	mtsr	sr12, r25		# ISA IO: sr12 allows access to IO space
	mtsr	sr11, r6		# MPIC: sr11 will allow access to MPIC

ex_flih_ivr:
#
# Determine what the primary interrupt controller is in preparation for
# acquiring the source interrupt level.
#
	bne	cr1, ex_flih_8259	# Branches for Primary != MPIC
#
# On an MPIC, every processor has its own bank of registers, size 4096 (<<12).
# The base of the per-processor registers is +0x20000, seen as just '2' below.
#
ifdef(`_POWER_MP',`
	mfspr	r29, PID		# Get Physical CPU ID (601,604 only)
	rlinm	r29, r29, 12, 0x0000F000 # times 4K (PID is 4 bits)
',` #else _POWER_MP
	li	r29, 0			# Fake Physical CPU ID, all UPs
') #endif _POWER_MP

	rlwinm	r31, r31, 0, 0x0FFFFFFF	# mpic_base Real->Virtual transform
	addis	r31, r31, MPIC_VSEG + 2	# Virt. Base, per CPU registers
	add	r29, r29, r31		# per processor registers, this CPU
	li	r30, MPIC_INTACK	#  intctl_pri leaves scope..
	lwbrx	r16, r31, r30		# get interrupt acknowledge register
	cmpi	cr0, r16, 0x00FF	# MPIC Spurious Interrupt?
	cmpi	cr4, r16, 0x0010	# from the 8259s?
	beq-	cr0, mpic_phantom	# Yes, a spurious MPIC interrupt
	bne+	cr4, gen_soft		#
#
# Ask the hardware what level interrupt we are dealing with
#
ex_flih_8259:
	lbz	r16, 0(r17)		# Read the interrupt level (IVR)

ifdef(`INTRDEBUG',`
	li	r3, 0x4953		# FLIS - flih start
	oris	r3, r3, 0x464c		#
	mr	r4, r16			# Go trace information
	li	r5, 0
	li	r6, 0
	bla	ENTRY(mltrace)
	.extern ENTRY(mltrace)
',)

#
# Find a pointer to the intr structure for this hardware level
#
	lis	r25, 0xC000		# Create base address of ISA space
	li	r28, 0xA0		# Rotate priority+non-specific EOI
	cmpi	cr3, r16, 0x0008	# See if the interrupt is 8-15(slave)
	cmpi	cr4, r16, 0x0010	# Interrupt from MPIC? (in 8259 path)
gen_soft:
	lhz	r17, ppda_softpri(PPDA_ADDR) # Load early with no dependencies
	la	r11, i_dpoll(r26)	# Lookup the intr structure for this
	sli	r4, r16, I_DATA_SH	#   level
	li	r0, 0			#
	lwzx	r24, r11, r4		# Get contents to i_data.i_poll[intlvl]
	ori	r21, r0, 0x8000		# bit for i_softpri shift
	lis	r22, 0x80000000>16	# bit for i_prilvl shift
	cmpi	cr0, r24, nullA		# See if we really have an intr struct
	beq-	cr0, what_phantom	# phantom interrupt
#
# 1. Get the priority and interrupt level from dummy intr struct.
# 2. Calculate address of i_prilvl[priority] and load it.
# 3. Set level in i_prilvl[priority]
# 4. Set priority in i_softpri
# 5. Save them back
#
	lwz	r27, intr_level(r24)	# get i_prilvl bit
	la	r18, ppda_prilvl0(PPDA_ADDR)  # base address of i_prilvl[]
	lwz	r23, intr_priority(r24)	# get priority
	srw	r22, r22, r27		# shift i_prilvl bit
	lhz	r3, intr_flags(r24)	# get the flags 
	rlwinm	r19, r23, 2, 0x3C	# priority is index into i_prilvl[]
	srw	r21, r21, r23		# shift i_softpri bit
	lwzx	r20, r18, r19		# get i_prilvl[priority]
	andi.	r3, r3, INTR_LEVEL	# cr0 - isolate level trigger flag
	or	r17, r17, r21		# stuff priority bit in i_softpri
	or	r20, r20, r22		# stuff level bit in i_prilvl[]
	sth	r17, ppda_softpri(PPDA_ADDR)  # save it back
	stwx	r20, r18, r19		# save new i_prilvl[priority]
#
# The interrupt is now queued so optionally issue an EOI
# This is where a test for Level==2, MPIC MPC (on MP) can occur.
#
	bgt	cr4, flih_exit		# If MPIC, we are finished, no EOI

#
# Issue hardware End Of Interrupt(EOI)
#   - If this is level triggered interrupt, then first mask the line
#     before issuing EOI.
#
#   - The non-specific EOI command and rotate the priority is used.
#     This means that fairness is preserved if somebody somehow figures out
#     how to service their favorite interrupt fast enough to keep the others
#     locked out.
#

	bne-	cr0, rspc_mask_lvl	# branch if level-trigger to mask IRQ

i8259_eoi:
	stb	r28, INTA00(r25)	# Ack the one we always need
	blt-	cr3, mpic_eoi		# If interrupt 0-7 don't hit slave
	eieio				# ensure ack master first
	stb	r28, INTB00(r25)	# give the slave a shot
mpic_eoi:
	bne	cr1, flih_exit		# there's no MPIC (it must be primary)
	eieio				# Serialize 8259 writes with MPIC eoi
	stw	r0, MPIC_EOI(r31)	# Issue the EOI.  not byte swabbed!
flih_exit:
	sync				# get everything out
	bne-	cr2, ex_flih_batclr	# if not 601 then tear down the BATs
	b	ENTRY(finish_interrupt)
	.extern ENTRY(finish_interrupt)

# Mask the IRQ line before issuing the EOI
rspc_mask_lvl:
	li	r3, 1			# a bit to shift
	LTOC(r5, rspc_8259_enables, data)    # get address of 8259 IMR mask
	LTOC(r6, rspc_8259_mask_sys, data)   # get address of system view
	rlwnm	r3, r3, r16, 0xffff	# align it to correct IRQ
	lhz	r7, 0(r6)		# get the system mask
	lwz	r4, 0(r5)		# get the 8259 mask
	or	r7, r7, r3		# mask IRQ line in system mask
	or	r4, r4, r3		# mask IRQ line in 8259 mask
	sth	r7, 0(r6)		# save new system mask
	stw	r4, 0(r5)		# save new 8259 mask value
	bge+	cr3, rspc_mask_slave	# mask IRQ in slave 8259
	stb	r4, INTA01(r25)		# write new IMR to master
	eieio
	b	i8259_eoi

rspc_mask_slave:
	rlwinm	r5, r4, 24, 0xFF	# move slave IMR to low byte
	stb	r5, INTB01(r25)		# write new IMR to slave
	eieio
	b	i8259_eoi

#
# Load bats 0, 1 & 2 for access to HW registers
#	BAT 0 points to IVR
#	BAT 1 points to ISA ports
#	BAT 2 points to MPIC
#
ex_flih_bat:
	lis	r8, lu0.bat_rspcint_seg		# IVR:
	lis	r7, uu0.bat_rspcint_seg		# IVR:
	lis	r10, lu1.bat_rspcio_seg		# ISA IO:
	lis	r9, uu1.bat_rspcio_seg		# ISA IO:
	andis.	r6, r31, 0xF000			# MPIC: high 4 RADDR bits
	lis	r5, uu.bat_mpic_seg		# MPIC: BATU value (eq. sr11)

	ori	r8, r8, ll0.bat_rspcint_seg	# IVR:
	ori	r7, r7, ul0.bat_rspcint_seg	# IVR:
	ori	r10, r10, ll1.bat_rspcio_seg	# ISA IO:
	ori	r9, r9, ul1.bat_rspcio_seg	# ISA IO:
	ori	r6, r6, ll.bat_gen		# MPIC: BATL bits
	ori	r5, r5, ul.bat_gen		# MPIC: BATU bits

	.machine "603"
	mtdbatl	0, r8			# VADDR 0xF... .... -> IVR
	mtdbatu	0, r7
	mtdbatl	1, r10			# VADDR 0xC... .... -> ISA
	mtdbatu	1, r9
	mtdbatl	2, r6			# VADDR 0xB... .... -> MPIC
	mtdbatu	2, r5
	isync				# Speculative execution fence
	b	ex_flih_ivr

# tear down the BATs before leaving

ex_flih_batclr:
	li	r31, 0			#
	mtdbatu	0, r31			# bit 31(valid bit) should be
	mtdbatu	1, r31			#    0 in register
	mtdbatu	2, r31			#
	.machine "ppc"
	isync
	b	ENTRY(finish_interrupt)

#
# Phantom interrupt - update the missing interrupt count and continue
#
what_phantom:				# Which interrupt controller?
	bgt	cr4, mpic_phantom	# Source > 0x10 is from MPIC
i8259_phantom:				#
	LTOC(r7, i_missing, data)	# Point to beginning of missing array
	LTOC(r5, phantom_pri, data)	#
	bne	cr1, i8259_pri		# Branch if Primary != MPIC
	addi	r5, r5, 4		# &phantom_sec=&phantom_pri+sizeof(uint)
i8259_pri:				#
	rlinm	r3, r16, 2, 0x3C	# Multiply interrupt number by size(4)
	lwzx	r4, r7, r3		# i_missing[level]++
	lwz	r30, 0(r5)		# phantom count, pri or sec
	addic	r4, r4, 1		#
	stwx	r4, r7, r3		#
	addic	r30, r30, 1		#
	stw	r30, 0(r5)		#
	b	i8259_eoi		#
#
# mpic_phantom is branched to directly if the MPIC_INTACK reads 0xFF (spurious)
# or indirectly from gen_soft/what_phantom if there is no intr structure.
#
mpic_phantom:
	lwz	r5, DATA(phantom_pri)	# MPIC is only supported as the primary
	addic	r5, r5, 1		# interrupt controller.
	stw	r5, DATA(phantom_pri)	#
	b	flih_exit		# Do not EOI the phantom to MPIC

	.toc
	TOCE(i_data, data)
	TOCE(phantom_pri, data)
	TOCE(i_missing, data)
	TOCE(rspc_8259_enables, data)
	TOCE(rspc_8259_mask_sys, data)

include(flihs.m4)
include(scrs.m4)
include(mstsave.m4)
include(seg.m4)
include(machine.m4)
include(intr.m4)
include(ioacc.m4)
include(low_dsect.m4)
include(interrupt.m4)
include(systemcfg.m4)
include(system_rspc.m4)
