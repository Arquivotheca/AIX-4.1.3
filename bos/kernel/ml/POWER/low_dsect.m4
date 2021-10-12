# @(#)05	1.11  src/bos/kernel/ml/POWER/low_dsect.m4, sysml, bos411, 9428A410j 4/30/93 10:26:36
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Defines
#
# FUNCTIONS:
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

#****************************************************************************
# NAME: low_dsect.m4
#
# FUNCTION:
#
# This dsect defines the memory map for low kernel memory.
# Assembler programs which reference low memory variables include
# this file to reference them at the right locations and without
# using a base register (via ".using low, r0").
#
#****************************************************************************

include(INCLML/ppda.m4)
include(INCLML/overlay.m4)

	.dsect	low		# this goes at address zero
	.using	low,0

include(INCLML/low.m4)
include(INCLML/ufcp.m4)
include(INCLML/fcp.m4)

