# @(#)58	1.3  src/bos/kernel/db/POWER/dbg_end.s, sysdb, bos411, 9428A410j 11/10/93 16:43:54
#-----------------------------------------------------------------------#
#

#
# COMPONENT_NAME: (SYSDB) Kernel Debugger
#
# FUNCTIONS:
#
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# FUNCTION: This is used by initialization to determine where pinned
#           memory ends and the debugger code begins.
#
# EXECUTION ENVIRONMENT: not executable
#
#-----------------------------------------------------------------------#

        .file   "dbg_end.s"
        .csect	pin_dbg_end[RO]

DATA(pin_dbg_end):
        .globl  DATA(pin_dbg_end)

        .comm   DATA(pin_dbgcom_end), 0

