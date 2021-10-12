# @(#)52	1.5  src/bos/kernel/ml/POWER/sr_flih.s, sysml, bos411, 9428A410j 3/7/94 22:20:52
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: sr_flih
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential	Restricted when
# combined with	the aggregated modules for this	product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT	International Business Machines	Corp. 1989, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

	.file	"sr_flih.s"
	.machine "com"

#******************************************************************************
#
# NAME: sr_flih
#
# FUNCTION:
#       System resets are non-recoverable.  The system reset flih is
#       architecture independent.  All the work is done by the
#       sr_slih().
#
# PSEUDO CODE:
#       int sr_flih()
#       {
#               state_save();
#               begin_interrupt(SR_LEVEL, INTMAX<<8);
#               sr_slih(0);
#       }
#
#
#******************************************************************************

	.csect	sr_flih[PR]
DATA(sr_flih):
	.globl	DATA(sr_flih)
ENTRY(sr_flih):
	.globl	ENTRY(sr_flih)

	mfspr	r0, LR				# save state
	bl	ENTRY(state_save)
	.extern	ENTRY(state_save)

	lil	r3, SR_LEVEL			# call begin interrupt
	lil	r4, INTMAX*256
	bl	ENTRY(begin_interrupt)
	.extern	ENTRY(begin_interrupt)

ifdef(`_KDB',`
	LTOC(r3, kdb_avail, data)		# get address of kdb_avail
	l    	r3,0(r3)
	cmpi	cr0,r3,0
	beq	nokdb
	bl	ENTRY(kdb_sr_flih)		# pass control to kdb
	.extern	ENTRY(kdb_sr_flih)
nokdb:
') #endif _KDB

	lil	r3, 0
	b	ENTRY(sr_slih)			# pass control to sr_slih
	.extern	ENTRY(sr_slih)			# does not return

ifdef(`_KDB',`
		.toc
	TOCE(kdb_avail, data)
') #endif _KDB

include(flihs.m4)
include(i_machine.m4)
include(scrs.m4)
include(mstsave.m4)
