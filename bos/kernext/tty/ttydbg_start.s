# @(#)28	1.2  src/bos/kernext/tty/ttydbg_start.s, sysxtty, bos411, 9428A410j 5/24/94 08:46:21
#****************************************************************************
#
# COMPONENT_NAME: (sysxtty)	ttydbg extension for tty debugging
#
# FUNCTIONS:
#
# ORIGINS: 83
#
#****************************************************************************
#
# LEVEL 1, 5 Years Bull Confidential Information
#
#****************************************************************************

#****************************************************************************
#
# FUNCTION: This is used by initialization to determine where pinned
#           memory and the tty debugger code begin.
#
# EXECUTION ENVIRONMENT: not executable
#
#****************************************************************************

        .file   "ttydbg_start.s"
        .csect	pin_ttydbg_obj_start[RO]

DATA(pin_ttydbg_obj_start):
        .globl  DATA(pin_ttydbg_obj_start)

        .comm   DATA(pin_ttydbg_com_start), 0

