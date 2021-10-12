# @(#)43	1.2  src/bos/kernel/ml/POWER/ioacc.m4, sysml, bos41B, 9504A 1/20/95 10:42:31
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
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#                                                                            
#       ioacc.m4
#                                                                             
#*****************************************************************************
#
#  IMPORTANT NOTE:  This file should stay in sync with sys/POWER/ioacc.h
#
#*****************************************************************************

	.set	BUS_TYPE_MSK, 0x3FF	# bus type mask
	.set	BUS_TYPE_SHF, 6		# but type shift count

# Valid bus types
	.set	IO_PCI, 3		# PCI bus
	.set	IO_ISA, 4		# ISA bus
	.set	IO_VME, 5		# VME bus
	.set	IO_FT,	6		# Fault-tolerant bus
	.set	IO_MBUS,7		# Maintenance bus
