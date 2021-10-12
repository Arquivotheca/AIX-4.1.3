# @(#)34	1.35  src/bos/kernel/ml/POWER/i_able.s, sysml, bos41J, 9511A_all 3/7/95 13:12:27
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNNCTIONS: Interrupt Management
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# LEVEL 1,  5 Years Bull Confidential Information
#
# NOTES:
#       These procedures are pinned and only fault if called on a
#       pageable stack. Touches the stack while @ INTMAX and disabled
#       to ensure the pages are pinned.
#
#       No vm critical section
#       No fixed stack
#       No backtrack
#       Can page fault on callers stack
#
#-----------------------------------------------------------------------#

        .file    "i_able.s"
        .machine "com"

include(systemcfg.m4)
include(macros.m4)

#------------------------------------------------------------------------------
#
# NAME: disable_ints
#
# FUNCTION: fast disable to INTMAX
#
# CALL: int disable_ints()
#
# EXECUTION ENVIORNMENT:
#       Can be called at process or interrupt level
#
# NOTES:
#       The caller must not page fault while disabled, even if at the
#       process level
#
#       The caller must not block while disabled with this service
#
# RETURNS:
#       previous interrupt priority
#
#------------------------------------------------------------------------------

        S_PROLOG(disable_ints)
        .using   low, r0
        GET_CSA(cr0, r3, r4)            # get mst pointer
        mfmsr   r8                      # get current msr
        lbz     r3, mstintpri(r4)       # get current intpri
        cmpi    cr0, r3, INTBASE        # check if we are at INTBASE
        rlinm   r8, r8, 0, ~MSR_EE      # generate disabled msr
        lil     r9, INTMAX              # get new interrup priority
        mtmsr   r8                      # disable interrupts
        bne     cr0, no_stk_touch       # only fix stack if at INTBASE

ifdef(`_POWER_MP',`
	GET_PPDA(cr0, r10)		# set stack fix flag for LRU
	lil	r5, 1
	stb	r5, ppda_stackfix(r10)
')
        st      r1, mststackfix(r4)     # fix the stack
        l       r5, -STACKTOUCH(r1)     # touch stack to make sure its there
        l       r5, 0(r1)               # if we fault the EIMs will stay
        l       r5, STACKTOUCH(r1)      # the same since software priority
                                        # has not changed

no_stk_touch:
        stb     r9, mstintpri(r4)       # set new interrupt priority
        br

#------------------------------------------------------------------------------
#
# NAME: enable_ints
#
# FUNCTION: enable interrupts after a call to disable_ints
#
# CALL: void enable_ints(ipri)
#       int ipri;                       /* previous interrupt priority */
#
# EXECUTION ENVIORNMENT:
#       Can be called at processes or interrupt level
#
# RETUNRS:
#       None
#
#------------------------------------------------------------------------------

        S_PROLOG(enable_ints)
        .using   low, r0
        cmpi    cr1, r3, INTBASE        # check if returning to INTBASE
        cmpi    cr0, r3, INTMAX         # check if returning to INTMAX
	GET_PPDA(cr7, r6)		# get ppda pointer
        mfmsr   r5                      # get current msr
	l	r4, ppda_csa(r6)	# get csa
        bne     cr1, no_un_fix          # only unfix the stack if returning
        lil     r0, 0                   #  to INTBASE
        st      r0, mststackfix(r4)
ifdef(`_POWER_MP',`
	stb	r0, ppda_stackfix(r6)
')

no_un_fix:
        oril    r5, r5, MSR_EE          # generate enabled MSR
        beqr    cr0                     # return if returning to INTMAX
        stb     r3, mstintpri(r4)       # set new interrupt priority
        mtmsr   r5                      # enable interrupts
        br

#-----------------------------------------------------------------------#
# i_dosoft -- set up to call state_save to process queued interrupts
#
# FUNCTION: save the state of the current process and then call
#	finish_interrupt to do the dirty work.
#
# NOTE: This function acts a lot like swtch().  state_save() does most
#	of the work of save the process information.  Then finish_interrupt()
#	is called to start the processing of queued interrupts.
#
# 	This function is currently implemented to only run on a PowerPC
#	box.
#
# PSEUDO CODE:
#
# i_dosoft()
# {
#	struct ppda	*ppda;
#	int		oldmsr;
#
#	if( Trconflag[0] {
#		trchook(HKWD_KERN_FLIH| SOFT_LEVEL, iar)
#	}
#	oldmsr = mfmsr();
#	mtmsr(DISABLED_REAL_MSR);
#	mtspr(SPRG1, r15);
#
#	/* get ppda pointer into r15 for this processor */
#	ppda = GET_PPDA();
#
#	/* set up saved IAR & MSR to be that of the caller */
#	mtsrr0(mflr());
#	mtsrr1(oldmsr);
#
#	/* save the state of the process */
#	state_save();
#
#	/* it does not return */
#	finish_interrupt();
#
# }
#-----------------------------------------------------------------------#

	.machine "ppc"
        S_PROLOG(i_dosoft)

	mfmsr	r3			# get callers msr
	lil	r0, DISABLED_REAL_MSR	# get disabled real MSR value
	mtmsr	r0

DATA(isync_i_dosoft):
	.globl DATA(isync_i_dosoft)
	isync                           # make sure mtmsr is noticed

	mtspr	SPRG1, r15		# save r15 for state save
	mfspr	r0, LR			# state_save stores r0 into mstlr
	mfspr	r15, SPRG0		# get PPDA pointer
	mtspr	SRR1, r3
	mtspr	SRR0, r0
	bl	ENTRY(state_save_pc)
	.extern	ENTRY(state_save_pc)

ifdef(`MLDEBUG',`
	lhz	r3,ppda_softpri(PPDA_ADDR)	# r3 - interrupt queue
	tweqi	r3, 0			# assert pending interrupts
        lil     r0, 1                   # always trace when debug is on
',`
        lbz     r0,DATA(Trconflag)      # get current trace mode
')
        cmpi    cr0, r0, 0              # check for trace on?
        bne-    i_dosoft_trc            # trace is on, so go trace

	ba	ENTRY(finish_interrupt)
	.extern ENTRY(finish_interrupt)

i_dosoft_trc:
        li      r3, SOFT_LEVEL          # interrupt "level(32)" for i_dosoft
        li	r4, -1			# don't set mstintpri
	bl	ENTRY(begin_interrupt)
	.extern	ENTRY(begin_interrupt)

	ba	ENTRY(finish_interrupt)

        FCNDES(i_dosoft)
        FCNDES(enable_ints)
        FCNDES(disable_ints)

        _DF(_DF_NOFRAME)

       .toc
        TOCE(i_data,data)

include(param.m4)
include(flihs.m4)
include(scrs.m4)
include(low_dsect.m4)
include(mstsave.m4)
include(intr.m4)
include(machine.m4)
