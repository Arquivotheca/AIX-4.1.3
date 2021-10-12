# @(#)21	1.10  src/bos/kernel/ml/POWER/kgetsig.m4, sysml, bos41J, 9508A 2/3/95 10:01:58
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS: kgetsig
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

#******************************************************************************#
#
# NAME: kgetsig()
#
# FUNCTION: call signal handler
#	the saved context is on the user stack.
#	we return to user mode and resume the thread at the entry
#	point for the signal handler.
#
# EXECUTION ENVIRONMENT:
#	called on the kernel stack for the thread
#
# NOTES: This code must reside in non-privileged memory (read access for key 1)
#	 since it executes using the user-level segment register 0.
#
# RETURN VALUES:
#     NONE
#
# INPUT:
# struct	sig_save	
# {
# 	int	signo;			/* the signal number */
# 	int	code;			/* exception code */
# 	struct	sigcontext	*scp;	/* pointer to save context */
#	ulong_t msr;			/* user msr value */
#	char	*sh_fp;			/* function descrip for sig handler */
#	char	*stkp;		/* stack ptr; NULL if not to change stacks */
#	char	*sh_ret;		/* function descriptor for sigreturn */
#	vmhandle_t sreg;		/* segment register val for scp */
# };
#
#	r3 - pointer to parameters
#	r4 - pointer to thread block
#******************************************************************************#

ENTRY(kgetsig):

	mfmsr	r14
	rlinm	r13, r14, 0, ~MSR_EE	# disable interrupts
	mtmsr	r13

	lha	r13, t_suspend(r4)	# get suspend count
	l	r6, t_userp(r4)		# get user structure address
	l       r7, t_graphics(r4)

	l	r0, stkp(r3)		# load the stack pointer, or NULL
	ai	r13, r13, -1		# decrement suspend count
	sth	r13, t_suspend(r4)

        cmpli   cr0, r0, nullA  	# test for null

        # make "stkp" the stack pointer, unless it's NULL

        beq     kg_10           	# branch if NULL
        mr      r1, r0          	# make it the stack pointer
kg_10:
	l	r12, sh_fp(r3)		# load the function pointer
        l	r15, sh_ret(r3)		# save function to call
	l	r13, scp(r3)		# save context structure address
	l	r14, msr(r3)		# user msr value
	l	r4, code(r3)		# load arguments (signo, code, scp)
	l	r3, signo(r3)	
	mr	r5, r13

#	It was really SRR1 that was saved into mstsave.msr.
#	There are bits in SRR1 that are not properly part of
#	a 32-bit PowerPC MSR, and those bits can collide with
#	MSR bits, so we make this code look like an rfi --
#	preserve bits 0, 5-9, and 16-31 only.

	cau	r16, 0, u.SRR1_MSR_BITS
	oril	r16, r16, l.SRR1_MSR_BITS
	and	r14, r14, r16		# Keep bits 0, 5-9, and 16-31
	
#       Restore segment regs

	lm	r16, u_adspace_sr(r6)	# pick up seg regs 0-15

	cmpi	cr0,r7,0		# test if it is a graphics thread

        mtsr    r_segr1, r17
        mtsr    r_segr3, r19
        mtsr    r_segr4, r20
        mtsr    r_segr5, r21
        mtsr    r_segr6, r22
        mtsr    r_segr7, r23
        mtsr    r_segr8, r24
        mtsr    r_segr9, r25
        mtsr    r_segr10, r26
        mtsr    r_segr11, r27
        mtsr    r_segr12, r28
        mtsr    r_segr13, r29
        mtsr    r_segr15, r31

	# Set up access rights to displays controlled by bats/segment registers
	# for graphics threads.
	# Page fault is not allowed until back to the problem state.
	# Otherwise, segment registers may contain obsolete display access
	# rights.
	mr      r26,r7
	bnel	sr_bat_display_setup	# branch if a graphics thread

DATA(isync_kgetsig):
	.globl	DATA(isync_kgetsig)
	isync

	# restore segr0, segr2, segr14
        mtsr    r_segr0, r16
        mtsr    r_segr2, r18
        mtsr    r_segr14, r30

	# Set MSR to problem state
	mtmsr	r14			# store new msr
	isync

	# load r2 with the toc from the passed function pointer
	# this could page fault so we must be in user mode.

	l	r2, 4(r12)		# first word is the toc	
	l	r7, 0(r12)		# load entry point address into r7

	mtspr	CTR, r7			# store into count register

	# Branch-and-link to the entry point for the signal handler.
        bctrl				# branch to address

	# return here in problem state and user sregs.
	# simulate going thru pointer glue to get to sigreturn()
	# (can't use prtgl directly because its fetch protected)

	mr	r3, r13			# param is sigcontext structure
	l	r2,4(r15)		# r2 = sys call number
	b	ENTRY(svc_instr)	# go to it
	.extern	ENTRY(svc_instr)

        _DF(_DF_NOFRAME,0)
