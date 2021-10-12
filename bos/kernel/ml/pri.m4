# @(#)74        1.1  src/bos/kernel/ml/pri.m4, sysml, bos411, 9428A410j 7/27/93 18:42:13
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS: none
#
#   ORIGINS: 27 83
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
#   LEVEL 1,  5 Years Bull Confidential Information
#
#*****************************************************************************
#
#   IMPORTANT NOTE:  This file should match sys/pri.h
#
#****************************************************************************
	
	.set	PMASK,		127
	.set	PCATCH,		0x100		# must not be equal to SWAKEONSIG
						# Both PCATCH and SWAKEONSIG must be
						# greater than PMASK
	.set	PSWP,		0		# "swapping"
	.set	PRI_SCHED,	16		# priority for scheduling proces
	.set	PINOD,		18		# inode locks
	.set	PZERO,		25		# see SWAKEONSIG and sleepx() for details
	.set	PPIPE,		26
	.set	PMSG,		27		# ipc messages
	.set	TTIPRI,		30		# tty input
	.set	TTOPRI,		31		# tty output
	.set	PWAIT,		33
	.set	PUSER,		40
	.set	PIDLE,		PMASK		# wait process's priority
	.set	PRI_LOW,	PIDLE-1		# least favored priority for a non realtime
						# process
