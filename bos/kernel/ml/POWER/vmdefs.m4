# @(#)35	1.6  src/bos/kernel/ml/POWER/vmdefs.m4, sysml, bos411, 9428A410j 6/15/90 19:03:01
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

#*********************************************************************
# 
#  vmdefs.m4:  vmm definitions for use in assembly language routines
#		--constructed from vmdefs.h
#                                                                     
#*********************************************************************
#
#
#  System limits for appropriate hardware, etc -
#
	.set	PSIZE,4096		# page size in bytes
	.set 	L2PSIZE,12		# log base 2 of page size
	.set	L2SSIZE,28		# log base 2 of segment size
	.set	PGMASK,0xfff		# mask for byte offset in page
#
#
#	segment registers used by the VMM, and the system -
#
	.set	TEMPSR,13		# all purpose segreg
	.set	PTASR,12		# PTA segreg
