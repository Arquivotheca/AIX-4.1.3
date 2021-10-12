# @(#)85        1.10  src/bos/kernel/ml/mststack.s, sysml, bos411, 9428A410j 8/5/93 15:37:44
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
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
#
# LEVEL 1,  5 Years Bull Confidential Information
#
# FUNCTION:
#	declare storage for mst stacks
#
#****************************************************************************


	.file	"mststack.s"
	.machine "com"

        include(low_dsect.m4)
        include(mstsave.m4)

	# This forces the required 4K alignment (2**12)

	.comm	DATA(mststack), framesize*NUMBER_OF_FRAMES*MAXCPU, 12

