# @(#)88        1.2  src/bos/usr/ccs/lib/libc/POWER/compu64.s, libccnv, bos411, 9428A410j 2/21/94 13:59:25
#
#   COMPONENT_NAME: LIBCCNV
#
#   FUNCTIONS: __compu64
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
 
# NAME:
#       __compu64
#
# FUNCTION:
#       Comparison of two unsigned 64-bit integers.
#
# EXECUTION ENVIRONMENT:
#       Problem-state library routine.
#
# INPUT:
#  r3   -- High order part of first  64-bit unsigned integer A
#  r4   -- Low  order part of first  64-bit unsigned integer A
#  r5   -- High order part of second 64-bit unsigned integer B
#  r6   -- Low  order part of second 64-bit unsigned integer B
#
# RETURNS:
#       Condition register cr0 is modified as if a signed compare
#       operation was performed on the 64-bit integers.  I.E:
#
#       A <  B: cr0 bit 0 set
#       A >  B: cr0 bit 1 set
#       A == B: cr0 bit 2 set
#
# NOTES:
#       This routine is not permitted to do any floating point
#       operations.  This routine does not modify any fixed point
#       general purpose registers.  This routine may use only
#       instructions which are common to both the Rios-I platform
#       and the PowerPC architecture.
#
	.file "compu64.s"
 
# Get file with common equates (registers, S_PROLOG, S_EPILOG, FCNDES)
 
        S_PROLOG(__compu64)
        cmpl    cr0, r3, r5     # Check top parts using unsigned or
                                #   logical compare.
        bner    cr0             # UnEqual => Have our answer in cr0,
                                #   so return via link register.
                                # Equal => Fall thru for more compares.
 
                                # Top 32-bits are equal.
        cmpl    cr0, r4, r6     # If top parts are equal, the bottom
                                # part defines the result.
 
        S_EPILOG
        FCNDES(__compu64)
