# @(#)29	1.2  src/bos/kernext/tty/ttydbg_end.s, sysxtty, bos411, 9428A410j 5/24/94 08:46:36
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
#           memory and the tty debugger code end.
#
# EXECUTION ENVIRONMENT: not executable
#
#****************************************************************************

        .file   "ttydbg_end.s"
        .csect	pin_ttydbg_obj_end[RO]

DATA(pin_ttydbg_obj_end):
        .globl  DATA(pin_ttydbg_obj_end)

        .comm   DATA(pin_ttydbg_com_end), 0

