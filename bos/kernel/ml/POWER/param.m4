# @(#)16	1.5  src/bos/kernel/ml/POWER/param.m4, sysml, bos411, 9428A410j 8/31/93 14:21:56
#
#   COMPONENT_NAME: SYSML
#
#   ORIGINS: 27, 83
#
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1989, 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#******************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

#-----------------------------------------------------------------------#
#	fundamental parameters						#
#-----------------------------------------------------------------------#

	.set	NULL, 0			# null pointer value
	.set    STACKTOUCH, 2048        # touch the stack size
	.set    PAGESIZE, 4096          # page size
	.set	KSTACKSIZE, 96*1024	# kernel stack size
