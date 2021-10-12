# @(#)47	1.1  src/bos/kernel/ml/POWER/state_ppc.s, sysml, bos411, 9428A410j 3/15/93 09:41:57
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: resume, state_save
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential	Restricted when
# combined with	the aggregated modules for this	product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT	International Business Machines	Corp. 1993
# All Rights Reserved
#
# US Government	Users Restricted Rights	- Use, duplication or
# disclosure restricted	by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

	.file	"state_ppc.s"
	.machine "ppc"

undefine(`_POWER_RS1')
undefine(`_POWER_RSC')
undefine(`_POWER_RS2')
undefine(`_POWER_RS')

include(state.m4)
	.globl	ENTRY(state_save_pc)
	.globl	ENTRY(resume_pc)

	FCNDES(state_save_pc, label)
	FCNDES(resume_pc, label)

