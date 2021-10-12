# @(#)62        1.1  src/bos/kernel/ml/POWER/cpage_ppc_splt.s, sysml, bos411, 9428A410j 7/14/93 13:41:53
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: v_copypage
#
# ORIGINS: 27
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
#****************************************************************************
        .file   "cpage_ppc_splt.s"
#****************************************************************************
        .machine "ppc"

#*****************************************************************************
#
# v_copypage for Power PC platform with split I/D cache
#
#*****************************************************************************

undefine(`_POWER_RS1')
undefine(`_POWER_RSC')
undefine(`_POWER_RS2')
undefine(`_POWER_RS')
undefine(`_POWER_601')

include(cpage.m4)





