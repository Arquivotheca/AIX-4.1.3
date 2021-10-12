# @(#)64        1.3  src/bos/kernel/ml/POWER/zpage_pwr.s, sysml, bos411, 9428A410j 4/4/94 17:42:50
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
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
        .file   "zpage_pwr.s"
#****************************************************************************
        .machine "pwr"
#*****************************************************************************
#
# Power version of v_zpage
#
#*****************************************************************************

undefine(`_POWER_601')
undefine(`_POWER_PC')
undefine(`_POWER_603')
undefine(`_POWER_604')

include(zpage.m4)


