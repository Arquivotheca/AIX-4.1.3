# @(#)51 1.3 src/bos/kernext/tty/POWER/sfix.s, sysxs64, bos411, 9435C411a 8/31/94 09:23:20
#
# COMPONENT_NAME: (sysxtty) System Extension for tty support
#
# FUNCTIONS: new_stack, rest_stack
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

# r3 addr of new stack
# r4 size in bytes of new stack

	.globl stack_begin

new_stack:
	a	r0, r3, r4
	ai	r3, r0, -0xC
	st	r1, 0(r3)
	oril	r1, r3, 0
	stu	r1, -0x50(r1)
	br
S_EPILOG

rest_stack:
	ai	r3, r1, 0x50
	l	r1, 0(r3)
	br
S_EPILOG

#
# FUNCTION:	slion_recv
#
# PURPOSE:	Allocate a new stack area for receive data.
#
# INPUT:
#      ld:	r3:Pointer to sliondata structure.
#      tp:	r4:Pointer to str_lion structure.
#      BusMem:	r5:Pointer to bus memory.
#
# INTERNAL PROCEDURES CALLED:
#	_slion_recv
#
# EXTERNAL PROCEDURES CALLED:
#	new_stack	rest_stack
#
# DESCRIPTION:
#	Allocate the new stack area for receive data before calling the
#	_slion_recv. Then it is released before the exit.
#	After returning from Rx data handling,  the condition of read queue 
#	is checked. If Q is more than HWM, slionproc(T_BLOCK) is called. 
#	If Q is less than LWM, slionproc(T_UNBLOCK) is called.
#	So at this entry, if ld->block flg is on, read data
#	operation is skipped until these flgs become off. 
#
# CALLED BY:
#	slion_offlev
#
# Moved from slion.c
#  slion_recv(ls,tp,BusMem)
#  {
#    new_stack(intr_stack, sizeof(intr_stack));
#    _slion_recv(ld, tp);
#    rest_stack();
#  }
# 
# r3 addr of struct sliondata lp
# r4 addr od struct str_lion tp
# r5 BusMem addr 

S_PROLOG(slion_recv)
#slion_recv:
	stmw	r29,-0xC(r1)
	addi	r30,r4,0
	mflr	r0
	addi	r31,r3,0
	stw	r0,8(r1)
	LTOC(r0,stack_begin,data)
	stwu	r1,-0x50(r1)
	addic	r3,r0,0xc
	li	r4,0x2000		# new stack size
	bl	new_stack
	ori	r0,r0,0
	addi	r3,r31,0
	addi	r4,r30,0
	bl	ENTRY(_slion_recv)
	.extern ENTRY(_slion_recv)
	stw	r3,0x38(r1)
	bl	rest_stack
	ori 	r0,r0,0
	addic	r1,r1,0x50
	lwz	r0,8(r1)
	mtlr	r0
	lmw	r29,-0xc(r1)
	br
S_EPILOG
FCNDES(slion_recv)

			.align 2
DATA(stack_begin):	.space 0x2000
DATA(stack_end):	.space 4

		.toc
	TOCL(stack_begin,data)

