# @(#)73        1.6  src/bos/kernel/ml/POWER/ex_flih_pwr.s, sysml, bos411, 9428A410j 4/15/94 17:08:42
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: ex_flih_rs1, ex_flih_rs2
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential	Restricted when
# combined with	the aggregated modules for this	product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT	International Business Machines	Corp. 1993, 1994
# All Rights Reserved
#
# US Government	Users Restricted Rights	- Use, duplication or
# disclosure restricted	by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

	.file "ex_flih_pwr.s"
	.machine "pwr"

	.using	low, r0

include(macros.m4)

#
#
# Name:	  ex_flih_rs1
#
# Function:
#
#   The	External FLIH is entered when the machine takes	an "external" or I/O
#   interrupt.	On entry, interrupts are disabled and translate	is off.
#
#

DATA(ex_flih_rs1):
	.globl	DATA(ex_flih_rs1)
ENTRY(ex_flih_rs1):
	.globl	ENTRY(ex_flih_rs1)

	mfspr	r0, LR

#	Call the state-saving routine, which will set up kernel	and user
#	segment	r/w addressability and stash all the state except the
#	floating point registers in the	mst pointed to by csa.

	bl	ENTRY(state_save_rs)
	.extern	ENTRY(state_save_rs)

#       On return from state_save, the following registers are set up:
#           r1     Address of stack (for C)
#           r2     Address of kernel TOC
#           r13    Address of current  mst save area (new value of csa)
#           r14    Address of previous mst save area (old value of csa)
#           r15    Address of PPDA
#           sr0    R/W access to kernel segment
#           sr2    R/W access to process segment
#           sr14   R/W access to kernel extension segment

#       Perform initial interrupt processing

	cal	r3, 5(0)		# Interrupt "level" 5 for 0x500
	cal	r4, -1(0)		# Flag to cause	mstintpri not to be set
	bl	ENTRY(begin_interrupt)
	.extern	ENTRY(begin_interrupt)
#
#       Non-volatile Register usage:  Note above registers after state_save
#
#           r16    Base address for BUID0/IOCC accesses uses sr15
#           r17    Mask for lvl'th bit for EIS0
#           r18    Mask for lvl'th bit for EIS1
#           r19    Base address for EIS access
#           r20    Base address for EIM access
#           r21    IOCC number isolated from intr.bid
#           r22    intr.bus_type
#           r23    Saved EIS0
#           r24    Saved EIS1
#           r25    Segreg value for BUID0/IOCC access
#
#       Load values of EIS and EIM.  Note that this resets the contents of the
#       EIS register to zero.

        cau     r25, 0, BUID0           # Load sr value for access to EICRs
        mtsr    r_segr15, r25           # Place value in segment reg 15
        cau     r16, 0, 0xf000          # Select segment reg 15

        cal     r19, EIS0(r16)          # Read EIS0 and EIS1
        lsi     r23, r19, 8
        cal     r20, EIM0(r16)          # Read EIM0 and EIM1
        lsi     r5, r20, 8
#
#       Call i_slih to compute the interrupt level and priority for this
#       interrupt The EIS and EIM values just read are passed as parameters
#
        mr      r3, r23                 # Move for call to i_slih
        mr      r4, r24
       .extern  ENTRY(i_slih[PR])
        bl      ENTRY(i_slih[PR])       # Call i_slih

#       Returned value (in r3) from i_slih is address of "intr" structure
#       which contains interrupt level and priority. Use the level value (0..63)
#       to reset a bit in the EIS.  The priority value becomes the value of
#       mstintpri in our new mst.  If value in r3 is NULL, then this was a
#       "phantom interrupt" -- record it and keep going.
#
#       Note: This code supports only 2 IOCCs at BUID 20 and 21

        cmpi    cr0, r3, nullA          # Check for NULL returned pointer
        bne     cr0,ex_05               # non-NULL, continue processing

ifdef(`MLDEBUG',`
        TRAP                            # DEBUG ONLY
',)

        l       r3,DATA(phantom_int)    # Record this phantom interrupt
        ai      r3,r3,1                 #   by bumping count
        st      r3,DATA(phantom_int)
        stsi    r23, r19, 8             # Store them back so we don't lose them
        b       ENTRY(finish_interrupt) # Skip further processing
	.extern	ENTRY(finish_interrupt)

#       RSC bug requires timer and external int to be on EIS1. 

ex_05:
        l       r3, 0(r3)		# step over dummy intr struct
       .using   intr, r3
        lhz     r22, intr_bus_type      # get bus type of handler
        l       r5, intr_bid            # get bid of interrupt handler
        cmpi    cr1, r22, BUS_MICRO_CHANNEL
        rlinm.  r21, r5, 0, 0x00F00000  # mask low nible of BUID and save
        l       r4, intr_level          # Pick up level and priority
        crand   cr0+eq, cr0+gt, 4*cr1+eq # combine tests
        cmpi    cr1, r4, 32
        l       r5, intr_priority       #   from returned intr struct

       .drop    r3
        xor     r17, r17, r17           # Clear mask for EIS0
        xor     r18, r18, r18           # Clear mask for EIS1
        cau     r0, 0, 0x8000           # Set bit 0
        bge     cr1, ex_06              # if level >= 32 do EIS1
        bne     cr0, ex_07              # branch if buid 20 or level < 32
ex_06:
        rrib    r18, r0, r4             # Reset the "lvl'th" bit of EIS 1
        oril    r3, r4, 0x20            # Make sure lvl > 32, move for call
        b       ex_09                   # to i_poll
ex_07:
        rrib    r17, r0, r4             # Rset the "lvl'th" bit of EIS 0
        mr      r3, r4                  # move for call to i_poll

#       Reset EIS bit for selected level
#       Set interrupt priority, adjust mask in EIM and mask bit in MSR

ex_09:
        andc    r11, r23, r17           # Reset the EIS bit
        andc    r12, r24, r18
        stsi    r11, r19, 8             # write the new EIS bits
        cmpi    cr0, r5, INTMAX         # Check for INTMAX
        rlinm   r5, r5, 8, 0xFF00       # Form priority || backtrack_flag
        LTOC(r12,i_data,data)           # Load addr of "i_data" table
        sth     r5, mstintpri(NEW_MST)  # Store new inter. priority in mst
        beq     ex_10                   # Branch if INTMAX; don't enable EE
                                        #   in MSR and don't set EIM

#       New interrupt priority is not INTMAX; set proper value into EIM
#       and set MSR to allow external interrupts.

        rlinm   r5, r5, 27, 0x7F8       # Multiply priority by 8 for word index
        a       r5, r5, r12             # Calculate mask address
        lsi     r11, r5, 8              # Load the new mask value
        mtsr    r_segr15, r25           # access BUID0
        mfmsr   r7                      # Get MSR value into r7
        stsi    r11, r20, 8             # Write new mask values
        oril    r7, r7, MSR_EE          # Turn on EE bit
        mtmsr   r7                      #   in MSR
ex_10:

#       Call i_poll to actually handle the interrupt, passing the level that
#       interrupted

       .extern  ENTRY(i_poll)
        bl      ENTRY(i_poll)

        mfmsr   r7                      # Turn off EE bit
        cmpi    cr1, r22, BUS_MICRO_CHANNEL # if BUS_MICRO_CHANNEL issue EOI
        rlinm   r7, r7, 0, ~MSR_EE
        mtmsr   r7
        bne     cr1, ex_15
#
#       If on micro channel reset EIS again and issue EOI
#
        mtsr    r_segr15, r25           # Access BUID0 handle
        lsi     r11, r19, 8             # read to clear EIS
        andc    r11, r11, r17           # Turn off EIS bit for level
        andc    r12, r12, r18
        stsi    r11, r19, 8             # write the new EIS bits

        cau     r5, 0, u.iocc_seg       # Load constant value of segment
        oril    r5, r5, l.iocc_seg      # register used to access IOCC
        or      r5, r5, r21             # put in BUID
        mtsr    r_segr15, r5            # load sr15 with IOCC srval
        cau     r16, r16, u.iocc_addr   # Load constant value of IOCC
        l       r4, eoidata(r16)        # issue an EOI command

#       Finish interrupt processing; resume interrupted program

ex_15:
ifdef(`_SLICER',`
#	Slicer-only modification.  This interrupt processing may be as a result
#	of a Slicer interrupt.  If so, the CPU context we were originally
#	executing in may now be different.  Slicer must reset certain
#	registers to use the new CPU context as if it were the original
#	CPU context (i.e., when this interrupt was originally taken).
#
	GET_PPDA(cr0, PPDA_ADDR)
	GET_CSA(cr0, r5, NEW_MST)
	l	INTR_MST, mstprev(NEW_MST)
')
        b       ENTRY(finish_interrupt)

#------------------------------------------------------------------------------
#
# Name:   ex_flih_rs2
#
# Function:
#
#   The External FLIH is entered when the machine takes an "external" or I/O
#   interrupt.  On entry, interrupts are disabled and translate is off.
#
#   NOTE: After executing a "mtspr   ILCR,GPRx" that changes the current
#         interrupt level, a "mtmsr" setting MSR(EE) to b'1' will enable
#         external interrupts after the new interrupt level is effective
#         under either of the following conditions:
#
#         a) five fixed point instructions of any type are placed between the
#            mtspr and the mtmsr
#
#         b) two fixed point multiplies are placed between the mtspr and
#            the mtmsr
#
#         The requirement is that two cycles separate the "mtspr" from the
#         "mtmsr".  The fixed point multiplies each take two cycles to complete
#         and are restricted to executing in the B unit.  One may overlap the
#         execution of the "mtspr" resulting in a three cycle separation or the
#         first may follow the "mtspr" causing a four cycle separation.
#
#         If there is useful work to be done, there are other alternatives
#         such as:
#
#               1) execute the "mtmsr   ILCR,GPRx"
#               2) execute a simple fixed-point op (not multiply)
#               3) execute a fixed-point multiply
#               4) execute the "mtmsr" setting MSR(EE)=b'1'
#
#         This sequence ensures a minimum of two cycles and a maximum of three
#         cycles between the critical instructions.
#
#
#------------------------------------------------------------------------------


DATA(ex_flih_rs2):
	.globl	DATA(ex_flih_rs2)
ENTRY(ex_flih_rs2):
	.globl	ENTRY(ex_flih_rs2)

	mfspr	r0, LR

#	Call the state-saving routine, which will set up kernel	and user
#	segment	r/w addressability and stash all the state except the
#	floating point registers in the	mst pointed to by csa.

	bl	ENTRY(state_save_rs)

#       On return from state_save, the following registers are set up:
#           r1     Address of stack (for C)
#           r2     Address of kernel TOC
#           r13    Address of current  mst save area (new value of csa)
#           r14    Address of previous mst save area (old value of csa)
#           r15    Address of PPDA
#           sr0    R/W access to kernel segment
#           sr2    R/W access to process segment
#           sr14   R/W access to kernel extension segment

#       Perform initial interrupt processing

	cal	r3, 5(0)		# Interrupt "level" 5 for 0x500
	cal	r4, -1(0)		# Flag to cause	mstintpri not to be set
	bl	ENTRY(begin_interrupt)
#
#       Non-volatile Register usage:  Note above registers after state_save
#
#           r16    Base address for BUID0/IOCC accesses uses sr15
#           r17    CIL/PIL (ILCR)
#           r18    level of interrupt
#           r19    Poll array address
#           r20    Level, from cil array, to disable to - adjusted to
#                    CIL byte in ILCR
#           r21    IOCC number isolated from intr.bid
#           r22    intr.bus_type
#
        mfspr   r17, ILCR               # Get the CIL/PIL value
        rlinm   r18, r17, 0, 0xFF       # Isolate PIL
#
#       Find poll array, using PIL as index get pointer to intr structure
#
        LTOC(r12,i_data,data)           # Load addr of "i_data" table
        cal     r11, i_dpoll(r12)       # point to poll array
        sli     r4, r18, I_DATA_SH      # multiply index by 4
        lx      r3, r4, r11             # get pointer intr structure

#       If value in r3 is NULL, then this was a
#       "phantom interrupt" -- record it and keep going.

        cmpi    cr0, r3, nullA          # Check for NULL pointer
        bne     cr0,ex_05_r2            # non-NULL, continue processing

#       Ignore phantom decrementer tics
#       cmpi    cr0, r18, 0x3E
#       beq     ENTRY(finish_interrupt) # Skip further processing

ifdef(`MLDEBUG',`
        TRAP                            # DEBUG ONLY
',)

        l       r3,DATA(phantom_int)    # Record this phantom interrupt
        ai      r3,r3,1                 #   by bumping count
        st      r3,DATA(phantom_int)
        b       ENTRY(finish_interrupt) # Skip further processing

ex_05_r2:
	l       r3, 0(r3)		# step over dummy intr struct
       .using   intr, r3
        lhz     r22, intr_bus_type      # get bus type of handler
        l       r5, intr_bid            # get bid of interrupt handler
        rlinm   r21, r5, 0, 0x00F00000  # mask low nible of BUID and save
        l       r4, intr_level          # Pick up level and priority
        l       r5, intr_priority       #   from intr struct

       .drop    r3
        lbzx    r20, r5, r12            # Get level to disable to
        rlinm   r20, r20, 8, 0xFF00     # Adjust it to CIL byte

#       No need to clear PEIS bit since it was cleared by the
#       side effect of reading the PIL.
#       Set interrupt priority, adjust mask in EIM and mask bit in MSR

        mtspr   ILCR, r20               # Write new level into CIL
                                        # 5 instructions between here & EE=1
        cmpi    cr0, r5, INTMAX         # Check for INTMAX
        rlinm   r5, r5, 8, 0xFF00       # Form priority || backtrack_flag
        sth     r5, mstintpri(NEW_MST)  # Store new inter. priority in mst
        beq     ex_10_r2                # Branch if INTMAX; don't enable EE
                                        #   in MSR

#       New interrupt priority is not INTMAX; set proper value into CIL
#       and set MSR to allow external interrupts.

        mfmsr   r7                      # Get MSR value into r7
        oril    r7, r7, MSR_EE          # Turn on EE bit
        mtmsr   r7                      #   in MSR

#       Call i_poll to actually handle the interrupt, passing level of the
#       interrupt

ex_10_r2:
        mr      r3, r18                 # level of the interrupt
       .extern  ENTRY(i_poll)
        bl      ENTRY(i_poll)

        mfmsr   r7                      # Turn off EE bit
        cmpi    cr1, r22, BUS_MICRO_CHANNEL # if BUS_MICRO_CHANNEL issue EOI
        rlinm   r7, r7, 0, ~MSR_EE
        mtmsr   r7
        bne     cr1, ex_15_r2

        cau     r3, r18, ICO_CLI        # Clear the PEIS bit for level
        mtspr   ILCR, r3                # Reset PEIS bit again

        cau     r5, 0, u.iocc_seg       # Load constant value of segment
        oril    r5, r5, l.iocc_seg      # register used to access IOCC
        or      r5, r5, r21             # put in BUID
        mtsr    r_segr15, r5            # load sr15 with IOCC srval
        cau     r16, 0, 0xF000          # use sr15
        cau     r16, r16, u.iocc_addr   # Load constant value of IOCC
        l       r4, eoidata(r16)        # issue an EOI command

#       Finish interrupt processing; resume interrupted program

ex_15_r2:
        b       ENTRY(finish_interrupt)

	.toc
	TOCE(i_data, data)

include(iocc.m4)
include(flihs.m4)
include(scrs.m4)
include(mstsave.m4)
include(seg.m4)
include(machine.m4)
include(intr.m4)
include(low_dsect.m4)
include(interrupt.m4)
ifdef(`_SLICER',`
include(systemcfg.m4)
')

