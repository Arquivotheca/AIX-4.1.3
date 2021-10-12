# @(#)25        1.2  src/bos/kernel/lib/libsysp/POWER/tlbi.s, libsysp, bos411, 9428A410j 12/2/91 14:55:37
#***************************************************************************
#
# COMPONENT_NAME: (LIBSYSP)
#
# FUNCTIONS: tlbi
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1991
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#*************************************************************************

		.file	"tlbi.s"

#**********************************************************************
#
#  NAME: tlbi
#
#  FUNCTION: invalidate tlb for specified page
#
#       tlbi(vaddr)             rc = none
#
#  INPUT STATE:
#     r3 = effective address of page
#
#  OUTPUT STATE:
#     The tlb entry for the referenced virtual page (if any) has been
#     invalidated.
#
#  EXECUTION ENVIRONMENT:
#       Supervisor state  : Yes
#
#**********************************************************************

        S_PROLOG(tlbi)
        tlbi    r0,r3           # invalidate tlb entry for page
        S_EPILOG
