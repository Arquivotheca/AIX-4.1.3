# @(#)55	1.1  src/bos/usr/ccs/lib/libc/POWER/_stk.s, libcgen, bos411, 9428A410j 2/22/94 06:23:17
#
#   COMPONENT_NAME: libcgen
#
#   FUNCTIONS: __stack_pointer
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# NAME: _stk
#
# FUNCTION: Get the stack pointer from R0
#
# EXECUTION ENVIRONMENT:
#
#      This is a special service routine for mcount to get information
#      from the stack.  Not easily done in C. The stack is not altered.
#      The stack pointer is the value set by the calling routine.  Only
#      non-saved registers are used.  Standard linkage/register usage.
#
# (NOTES:) Uses standary system stackframe.
#
# (RECOVERY OPERATION:) none
#
# (DATA STRUCTURES:) Does not alter any data structure.
#
# RETURNS: Link register field contents from second previous stack frame.
#

        FCNDES(__stack_pointer)

ENTRY(__stack_pointer):

        S_PROLOG(__stack_pointer)
        include(sys/comlink.m4)
	.file	"_stk.s"
        mr      r3,r1 #Stack pointer
        br                      # Return
        _DF(_DF_NOFRAME)
