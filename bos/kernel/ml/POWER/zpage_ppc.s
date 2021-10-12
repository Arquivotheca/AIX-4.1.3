# @(#)65        1.1  src/bos/kernel/ml/POWER/zpage_ppc.s, sysml, bos411, 9428A410j 7/14/93 13:44:49
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: v_zpage
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
        .file   "zpage_ppc.s"
#****************************************************************************
        .machine "ppc"

#*****************************************************************************
#
# Power PC version of v_zpage
#
#*****************************************************************************

undefine(`_POWER_RS1')
undefine(`_POWER_RSC')
undefine(`_POWER_RS2')
undefine(`_POWER_RS')

include(zpage.m4)

