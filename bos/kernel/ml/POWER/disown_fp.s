# @(#)59	1.24  src/bos/kernel/ml/POWER/disown_fp.s, sysml, bos411, 9428A410j 5/3/94 09:10:03
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS: Floating Point management
#
#   ORIGINS: 27, 83
#
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#******************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

	.file	"disown_fp.s"
	.machine "com"

include(systemcfg.m4)
include(macros.m4)

#******************************************************************************#
#                                                                              #
#       disown_fp.s -- Make sure that specified thread does NOT have           #
#                      any floating point resources.                           #
#                                                                              #
#******************************************************************************#

#  Name:  disown_fp

#  Function:  Make sure that the specified thread's floating point resources
#             (registers) have been stored into the thread's mst save area.

#       This routine is called by:
#               The FP Unavailable FLIH, to save FP regs for one thread
#               before granting FP access to another thread.

#               Thread termination, to make sure that a thread being
#               terminated does not still hold any FP resources.

#		Signal Delivery, to force-save the thread's floating
#		point state to the uthread before copying it to the 
#		sigcontext structure. This allows signal handlers to do
#		floating point instructions, etal.

#               Swap-out, to ensure that a thread being swapped to disk
#               does not still have any register-resident state.

#		The fp_cpusync() system call, when logically setting the
#		MSR(FE/IE) bits. The FE/IE bits are completely governed by the
#		FP bit. It is not valid to JUST set the FE/IE bit(s), and have
#		the FP bit off. The FE/IE bit may only be on for a thread that
#		owns the floating point unit and is using it. It is a good
#		performance win to `float' the FE bit to zero when the task
#		is just doing fixed point instructions.

#               Debugger, to ensure that all of a thread's state is stored
#               in its mst save area before the debugger attempts to display it.
#               In addition, this lets the debugger use floating point
#               instructions if needed.

#       A thread ID is passed as the only input parameter.  If this thread
#       currently "owns" the floating point registers (i.e., if its FP state
#       is currently resident in the FP registers), then the FP registers are
#       stored into the thread's mst save area and its copy of the MSR is
#       modified to show no FP access, and its FE & IE bits are reset.
#	If the thread does not own the FP registers, this routine simply
#       returns.

#       If the thread ID passed in is == -1, then if ANY thread currently
#       owns the FP registers they are stored in that thread's mst save area
#       and the thread is no longer the FP owner.  In addition, on return
#       from disown_fp(), FP Available is set to '1' in the MSR.  This allows
#       the debugger (or any other authorized, disabled interrupt routine) to
#       use floating point instructions.  Note that when the debugger returns,
#       FP Avail will still be set, but it will be turned off when the next
#       thread is dispatched because no thread will then own the FP registers.
#	In addition, the FE & IE bits are always reset in this case as well.
#	This avoids an accidental floating point trap in the debugger.

#       In order to avoid getting imprecise interrupts reported Twice
#       (among other things), this code must serialize Both the floating-
#       point & fixed point unit. This is not yet implemented.

# 	This is what this code does in terms of the objects it operates on:
#
#	[ NO_CURRENT_FP_OWNER && (-1 Input) ] ==> Set hardware MSR(FP) bit.
#	[ NO_CURRENT_FP_OWNER && Valid_TID_Input ] ==> Nothing.
#	[ FP_OWNER_EXISTS && (-1 Input) ] ==> disown fp unit & set FP bit.
#	[ FP_OWNER_EXISTS != Valid_TID_Input ] ==> Nothing.
#	[ FP_OWNER_EXISTS == Valid_TID_Input ] ==> disown fp unit.

#       Input:
#               GPR0   = Scratch, not saved
#               GPR1   = Save area (not used)
#               GPR2   = Kernel TOC address
#               GPR3 (parm no. 1) = Thread ID which is to "disown" FP regs, or -1
#               GPR4   = volatile, not saved
#               ...        "        "    "
#               GPR12  =   "        "    "
#               SR0    = R/W access to kernel segment
#               LR     = Return address

#       Returned:
#               Nothing.
#               MSR set to "FP Avail" if input parm (r3) == -1,
#                 else MSR is restored to caller's MSR on entry


##page;
#******************************************************************************#
#                                                                              #
#       ".set" values                                                          #
#                                                                              #
#******************************************************************************#

       .set     null_thrdp, 0           # Value in fp_owner which signifies
                                        #   "no owner"

#******************************************************************************#
#                                                                              #
#       disown_fp                                                              #
#                                                                              #
#******************************************************************************#

        S_PROLOG(disown_fp)

	# disable here to make this operation atomic

	mfmsr	r8			# save old msr
        cal     r0, (DISABLED_MSR | MSR_FP)(0)  # Enable FP access and
        mtmsr   r0                              #   disable interrupts,
						#   and perhaps other bits as well
DATA(isync_disownfp_1):
	.globl	DATA(isync_disownfp_1)
	isync					# make sure FP is noticed, and
						# user segment 2 quiesced
ifdef(`_POWER_MP',`
	GET_PPDA( cr0, r4 )
	l	r5, ppda_fpowner(r4)	# Owners thread table pointer or NULL
',`
        LTOC(r4,fp_owner,data)          # Get address of fp_owner pointer
        l       r5, 0(r4)               # Owners thread table pointer or NULL
')
        cmpi    cr1, r3, -1          	# See if input parm is -1
        cmpi    cr0, r5, 0              # Compare pointer to NULL
        crand   cr6*4+eq, cr0*4+eq, cr1*4+eq # Combine tests
        beq     cr6, di_20              # Branch if no owner AND r3 == -1
        beq     cr0, di_30              # Return if no owner
	l	r9, t_procp(r5)
	l	r6, t_tid(r5)		# Load thread id from thread table entry
	l	r7, p_adspace(r9)	# Load process private segmnt id for SR2
        cmp     cr0, r6, r3             # Compare thread id to input
        beq     cr1, di_10              # Save regs anyway if r3 == -1
        bne     cr0, di_30              # Return if our guy isn't the owner

#       The subject of "disown_fp" IS the owner of the FP regs.  Disable so
#       that we don't lose control while manipulating the FP status.  Establish
#       addressability to his mst save area.

di_10:
        mfsr    r9, r_user              # Save current sr2 contents
        rlinm   r7, r7, 0, ~SR_KsKp    	# Ensure r/w access to FP owner's private seg
					# NB -- no isync here; caught by one above
        mtsr    r_user, r7              # Establish addressability
DATA(isync_disownfp_2):
	.globl	DATA(isync_disownfp_2)
	isync				# make sure mtsr is noticed
	l	r7, t_uthreadp(r5)	# Get address of mst
       .using   mstsave, r7

#       Save all 32 floating registers in the mst save area.  Also save
#       the FPSCR, which is tricky because it can only be loaded into a
#       64-bit fp register but it's only 32 bits long.  We finesse this by
#       picking up the word preceding it in the mst save area (whatever
#       it may be), storing it using "store fp double", and then overlaying
#       the high-order word with its previous contents.

#       Note that "store fp single" is NOT the same as storing the lower
#       half of the register.  What we really need is a "store fp lower half"
#       instruction" or a "move fp lower half to GPR" instruction or (best)
#       a "mfspr <gp reg>, FPSCR" instruction.

        stfd    f0,  mstfpr +  0 * 8    # Store all of
        stfd    f1,  mstfpr +  1 * 8    #   the FP registers
        mffs    f0                      # Get FPSCR into low end of f0
        stfd    f2,  mstfpr +  2 * 8
        stfd    f3,  mstfpr +  3 * 8
        stfd    f4,  mstfpr +  4 * 8
        stfd    f5,  mstfpr +  5 * 8
        l       r0,   mstfpscr - 4      # Pick up word preceeding mstfpscr
        stfd    f6,  mstfpr +  6 * 8
        stfd    f7,  mstfpr +  7 * 8
        stfd    f8,  mstfpr +  8 * 8
        stfd    f9,  mstfpr +  9 * 8
        stfd    f0,  mstfpscr - 4       # Store the FPSCR, plus hi word of garbage
        st      r0,   mstfpscr - 4      # Replace the word smashed by high FPSCR
        stfd    f10, mstfpr + 10 * 8
        stfd    f11, mstfpr + 11 * 8
        stfd    f12, mstfpr + 12 * 8
        stfd    f13, mstfpr + 13 * 8
        l       r6,   mstmsr            # Pick up MSR value from mst save area
	stfd    f14, mstfpr + 14 * 8
        stfd    f15, mstfpr + 15 * 8
        stfd    f16, mstfpr + 16 * 8
        stfd    f17, mstfpr + 17 * 8
					# Take away FP unit access
	andil.	r6, r6, (~(MSR_FP | MSR_FE | MSR_IE)) & (0xFFFF) # avoid `as' err
        stfd    f18, mstfpr + 18 * 8
        stfd    f19, mstfpr + 19 * 8
        stfd    f20, mstfpr + 20 * 8
        stfd    f21, mstfpr + 21 * 8
        st      r6,   mstmsr             # Store new MSR value
        stfd    f22, mstfpr + 22 * 8
        stfd    f23, mstfpr + 23 * 8
        stfd    f24, mstfpr + 24 * 8
        stfd    f25, mstfpr + 25 * 8
        cal     r6, null_thrdp(0)        # Null out fp_owner to show that
ifdef(`_POWER_MP',`
	st	r6, ppda_fpowner(r4)	# no one owns FP regs now
        lfs     f0, ppda_fpowner(r4)    # Load genuine 0 into FP reg 0
',`
        st      r6, 0(r4)               # no one owns FP regs now
        lfs     f0, 0(r4)               # Load genuine 0 into FP reg 0
')
        stfd    f26, mstfpr + 26 * 8
        stfd    f27, mstfpr + 27 * 8
        stfd    f28, mstfpr + 28 * 8
        stfd    f29, mstfpr + 29 * 8
        lfs     f0, 0(r4)               # Load genuine 0 into FP reg 0
        stfd    f30, mstfpr + 30 * 8
        stfd    f31, mstfpr + 31 * 8
        mtfsf   0xFF, f0                # Set all bits in FPSCR to 0, to avoid
                                        #   spurious (and baffling!) interrupts

#       Restore previous user private segment register and state of MSR
#       force off FP access in MSR in case caller was target of disown_fp().

DATA(isync_disownfp_3):
	.globl	DATA(isync_disownfp_3)
	isync				# force pending storage ops to complete ****
        mtsr    r_user, r9              # Restore sr2
					# Zap the FP & FE bits for caller
	andil.	r8, r8, (~(MSR_FP | MSR_FE | MSR_IE)) & (0xFFFF) # avoid `as' err
        mtmsr   r8                      # Restore caller's MSR
DATA(isync_disownfp_4):
	.globl	DATA(isync_disownfp_4)
	isync				# make sure change is noticed  *****

#       Return to caller, unless input parm (r3) == -1

        bner    cr1

#       Input parameter is -1; on return, make sure that FP Avail is
#       turned on in MSR.  (Typically, this is a call from debugger.)

di_20:
        oril    r8, r8, MSR_FP          # Turn on "FP Avail" bit
        mtmsr   r8                      # Put new value in MSR
DATA(isync_disownfp_5):
	.globl	DATA(isync_disownfp_5)
	isync				# make sure change is noticed  ****
        br                              # Return to caller

di_30:
	mtmsr	r8			# restore old msr and return
DATA(isync_disownfp_6):
	.globl	DATA(isync_disownfp_6)
	isync				# make sure change is noticed  ****
	br


        _DF(_DF_NOFRAME)              # traceback table
##page;
#******************************************************************************#
#                                                                              #
#       included macro files                                                   #
#                                                                              #
#******************************************************************************#
include(param.m4)
include(scrs.m4)
include(low_dsect.m4)
include(mstsave.m4)
include(intr.m4)
include(proc.m4)
include(user.m4)
include(seg.m4)
include(machine.m4)
include(flihs.m4)

#******************************************************************************#
#                                                                              #
#       TOC references                                                         #
#                                                                              #
#******************************************************************************#

ifdef(`_POWER_MP',`
',`
       .toc
       TOCE(fp_owner,data)
')
