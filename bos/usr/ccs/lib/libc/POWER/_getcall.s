# @(#)64	1.4  src/bos/usr/ccs/lib/libc/POWER/_getcall.s, libcgen, bos411, 9428A410j 2/14/94 11:22:03
#
#   COMPONENT_NAME: libcgen
#
#   FUNCTIONS: _getcall
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1989,1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# NAME: _getcall
#
# FUNCTION: Get the return address of the caller of this function (mcount).
#      This is an address in the routine which called mcount.
#
# EXECUTION ENVIRONMENT:
#
#      This is a special service routine for mcount to get information
#      from the stack.  Not easily done in C. The stack is not altered.
#      The stack pointer is the value set by the calling routine.  Only
#      non-saved registers are used.  Standard linkage/register usage.
#
# (NOTES:) Uses standard stack frame.
#
# (RECOVERY OPERATION:) none
#
# (DATA STRUCTURES:) Does not alter any data structure.
#
# RETURNS: Link register field contents from previous stack frame.
#

        FCNDES(_getcall)

ENTRY(_getcall):

        S_PROLOG(_getcall)
        include(sys/comlink.m4)
	.file	"_getcall.s"
        l       r3,stkback(stk) # Backchain address
        l       r3,stklink(r3)  # Caller's return address
        br                      # Return
	_DF(_DF_NOFRAME)
