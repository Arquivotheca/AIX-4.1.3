# @(#)89	1.8  src/bos/kernel/ml/vr_end.s, sysml, bos411, 9428A410j 6/16/90 03:44:22
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
# (C) COPYRIGHT International Business Machines Corp. 1989, 1990
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

	.file	"vr_end.s"

        .csect	pin_obj_end[RO]

DATA(pin_obj_end):
        .globl  DATA(pin_obj_end)

        .comm   DATA(pin_com_end), 0

