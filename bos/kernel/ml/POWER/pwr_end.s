# @(#)07	1.1  src/bos/kernel/ml/POWER/pwr_end.s, sysml, bos411, 9428A410j 11/2/92 17:08:41
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1992
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

	.file	"pwr_end.s"

        .csect	pwr_obj_end[RO]

DATA(pwr_obj_end):
        .globl  DATA(pwr_obj_end)

        .comm   DATA(pwr_com_end), 0

