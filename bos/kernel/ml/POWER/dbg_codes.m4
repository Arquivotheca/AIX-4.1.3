# @(#)87	1.11  src/bos/kernel/ml/POWER/dbg_codes.m4, sysml, bos411, 9428A410j 10/19/93 09:43:43
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Defines
#
# FUNCTIONS:
#
# ORIGINS: 27 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1,  5 Years Bull Confidential Information
#****************************************************************************

#*****************************************************************************
#                                                                             
#       dbg_codes.m4 -- Machine-language version of dbg_codes.h               
#                                                                             
#*****************************************************************************

#  N.B.   This table must be kept parallel with <sys/dbg_codes.h>

        .set    DBG_FPEN,       1       # Floating point enabled exception
        .set    DBG_INVAL,      2       # Invalid operation
        .set    DBG_PRIV,       3       # Privileged operation
        .set    DBG_TRAP,       4       # Trap instruction
        .set    DGB_UNK_PR,     5       # Unknown program interrupt
        .set    DBG_MCHECK,     6       # Machine check
        .set    DBG_SYSRES,     7       # System reset
        .set    DBG_ALIGN,      8       # Alignment check
        .set    DBG_VM,         9       # Virtual memory error
	.set	DBG_KBD,       10       # Entered via keyboard ctl-alt-<key>
	.set    DBG_RECURSE,   11       # Recursion; program check in debugger
	.set	DBG_PANIC,     12	# Panic
	.set	DBG_KBD_NORMAL, 13	# Debugger entered, key in NORMAL
	.set	DBG_KBD_SECURE, 14	# Debugger entered, key in SECURE
	.set	DBG_KBD_SERVICE, 15	# Debugger entered, key in SERVICE
	.set	DBG_KBD_SERVICE1, 16	# key in SERVICE, numpad 1
	.set	DBG_KBD_SERVICE2, 17	# key in SERVICE, numpad 2
	.set	DBG_KBD_SERVICE4, 18	# key in SERVICE, numpad 4
	.set	DBG_DSI_IOCC,   19	# Data Storage Interrupt - IOCC
	.set	DBG_DSI_SLA,    20	# Data Storage Interrupt - SLA
	.set	DBG_DSI_SCU,    21	# Data Storage Interrupt - SCU
	.set	DBG_DSI_PROC,   22	# Data Storage Interrupt - PROC
	.set	DBG_EXT_DMA,    23	# External Interrupt - DMA Error
	.set	DBG_EXT_SCR,    24	# External Interrupt - Scrub Error
	.set	DBG_FPT_UN,     25	# Floating Point Unavailable Interrupt
	.set	DBG_ISI_PROC,   26	# Instruction Storage Interrupt - PROC
	.set	DBG_HTRAP,	27	# Illegal trap intruc intpt in kernel
	.set	DBG_DSI_SGA,    28	# Data Storage Interrupt - SGA
	.set	DBG_BTARGET,	32	# 601 branch target interrupt
	.set	DBG_WATCH,	33	# DABR exception
	.set	DBG_MPC_STOP,	34	# MPC_STOP
        .set    DBG_STATIC_TRAP, 35     # Static trap instruction

