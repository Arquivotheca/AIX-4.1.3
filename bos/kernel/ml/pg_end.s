# @(#)17	1.1  src/bos/kernel/ml/pg_end.s, sysml, bos411, 9428A410j 11/3/92 06:44:46
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

	.file	"pg_end.s"

        .csect	pg_obj_end[RO]

DATA(pg_obj_end):
        .globl  DATA(pg_obj_end)

        .comm   DATA(pg_com_end), 0

