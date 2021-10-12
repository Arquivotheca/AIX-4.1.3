# @(#)53	1.3  src/bos/kernel/ml/POWER/state_pwr.s, sysml, bos411, 9428A410j 4/4/94 17:43:01
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
# (C) COPYRIGHT	International Business Machines	Corp. 1994
# All Rights Reserved
#
# US Government	Users Restricted Rights	- Use, duplication or
# disclosure restricted	by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

	.file	"state_pwr.s"
	.machine "pwr"

undefine(`_POWER_601')
undefine(`_POWER_PC')
undefine(`_POWER_603')
undefine(`_POWER_604')


include(state.m4)
	.globl	ENTRY(state_save_rs)
	.globl	ENTRY(resume_rs)

	FCNDES(state_save_rs, label)
	FCNDES(resume_rs, label)

