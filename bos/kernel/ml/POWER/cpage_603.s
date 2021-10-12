# @(#)02        1.1  src/bos/kernel/ml/POWER/cpage_603.s, sysml, bos41J, 9521A_all 5/23/95 13:57:51
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
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
        .file   "cpage_603.s"
#****************************************************************************
        .machine "ppc"

#*****************************************************************************
#
# v_copypage for Power PC 603e platform with split I/D cache
#
#*****************************************************************************

undefine(`_POWER_RS1')
undefine(`_POWER_RSC')
undefine(`_POWER_RS2')
undefine(`_POWER_RS')
undefine(`_POWER_601')
define(_POWER_603_14)

include(cpage.m4)





