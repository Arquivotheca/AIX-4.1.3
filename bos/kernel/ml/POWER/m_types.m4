# @(#)14	1.8  src/bos/kernel/ml/POWER/m_types.m4, sysml, bos411, 9428A410j 10/26/93 13:47:55
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Defines
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1990
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************


#*****************************************************************************
#                                                                             
#       m_types.m4 -- Assembler version of (parts of) m_types.h               
#                                                                             
#*****************************************************************************


#   Definition of "address space" structure:

       .dsect   adspace_t

adsp_alloc:                             # Allocation flag set
       .long	0

adsp_srval:                             # Contents of 16 segment registers
       .space   16*4

