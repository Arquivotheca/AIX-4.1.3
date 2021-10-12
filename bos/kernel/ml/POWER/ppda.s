# @(#)01	1.2  src/bos/kernel/ml/POWER/ppda.s, sysml, bos411, 9428A410j 7/27/93 20:54:24
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: ppda section
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1992, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

		.file	"ppda.s"

include(ppda.m4)

#	The ppda must be at least quad word aligned

	.set	LOG2_DCACHE_LSIZE,	6	# 64 bytes

		.csect	ppda_sect[RW], LOG2_DCACHE_LSIZE
ifdef(`_POWER_MP',`
DATA(ppda):
	.space	(ppdend-ppdarea)*MAXCPU

        .toc
		.globl	DATA(ppda)
') # _POWER_MP





