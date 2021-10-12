# @(#)01        1.1  src/bos/kernel/ml/POWER/zpage_603.s, sysml, bos41J, 9521A_all 5/23/95 13:57:43
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
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
        .file   "zpage_603.s"
#****************************************************************************
        .machine "ppc"

#*****************************************************************************
#
# Power PC 603e version of v_zpage
#
#*****************************************************************************

undefine(`_POWER_RS1')
undefine(`_POWER_RSC')
undefine(`_POWER_RS2')
undefine(`_POWER_RS')
define(_POWER_603_14)

include(zpage.m4)

