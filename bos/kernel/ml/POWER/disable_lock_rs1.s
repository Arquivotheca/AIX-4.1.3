# @(#)68        1.1  src/bos/kernel/ml/POWER/disable_lock_rs1.s, sysml, bos411, 9428A410j 7/27/93 18:41:59
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS:	disable_lock
#		unlock_enable
#
#   ORIGINS: 27 83
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
#   LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

#
# NOTES:
#       These procedures are pinned and only fault if called on a
#       pageable stack. Touches the stack while @ INTMAX and disabled
#       to ensure the pages are pinned.
#
#       No vm critical section
#       No fixed stack
#       No backtrack
#       Can page fault on caller's stack
#
#-----------------------------------------------------------------------#
undefine(`_POWER_RS2')
undefine(`_POWER_601')
undefine(`_POWER_PC')

	.file    "disable_lock_rs1.s"
	.machine "pwr"

include(macros.m4)
include(disable_lock.m4)
