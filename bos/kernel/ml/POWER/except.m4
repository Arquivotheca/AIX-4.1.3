# @(#)02	1.4.1.2  src/bos/kernel/ml/POWER/except.m4, sysml, bos411, 9428A410j 12/7/93 17:19:16
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) machine language routines
#
# FUNCTIONS: exception include file
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

#	Programmed I/O Error Recover Retry count
	.set	PIO_RETRY_COUNT, 3

#
#	Exceptions
#		Defines for types of exceptions
	.set	EXCEPT_ERRNO,	0x0000007f	# the highest errno
	.set	EXCEPT_FLOAT,	0x00000080	# floating point exception
	.set	EXCEPT_INV_OP,	0x00000081	# invalid operation
	.set	EXCEPT_PRIV_OP,	0x00000082	# priveleged operation
	.set	EXCEPT_TRAP,	0x00000083	# trap instruction
	.set	EXCEPT_ALIGN,	0x00000084	# alignment exception
	.set	EXCEPT_INV_ADDR, 0x00000085	# invalid address
	.set	EXCEPT_PROT,	0x00000086	# protection exception
	.set	EXCEPT_IO,	0x00000087	# synchronous I/O
	.set	EXCEPT_FLOAT_IMPRECISE, 0x8c	# imprecise floating point

	.set	EXCEPT_MACHINE,		0x00000100	# machine dependent exceptions
	.set	EXCEPT_IFETCH_IO,	0x00000100	# I-fetch from I/O segment
	.set	EXCEPT_IFETCH_SPEC,	0x00000101	# I-fetch from special segment
	.set	EXCEPT_DATA_LOCK,	0x00000102	# data lock
	.set	EXCEPT_FP_IO,		0x00000103	# floating pt in I/O segment
	.set	EXCEPT_MIXED_SEGS,	0x00000104	# data straddles different segments
	.set	EXCEPT_DSI,        	0x00000106	# data storage interrupt
	.set	EXCEPT_ISI,        	0x00000107	# instruction storage interrupt
	.set	EXCEPT_GRAPHICS_SID,	0x00000108	# special graphics sid exception
	.set	EXCEPT_INVAL_EAR,	0x00000109	# invalid ear exception 

#
#	PIO_EXCEPTIONs
#
	.set    PIO_EXCP_INV_OP,	0x10000000      # invalid i/o operation
	.set	PIO_EXCP_PROT,		0x50000000	# Bus Memory Access Authority Error


