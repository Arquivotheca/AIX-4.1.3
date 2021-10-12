# @(#)45	1.15  src/bos/kernel/ml/uthread.m4, sysml, bos41J, 9513A_all 3/24/95 15:19:03
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS: none
#
#   ORIGINS: 27, 83
#
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

	.dsect	uthread
ut_save:	.space	mstend-mstsave	# mstsave area
ut_kstack:	.long	0		# own kernel stack

#	system call state

ut_scsave:	.space	4*8		# save area for system call handler
ut_msr:		.long	0		# saved user msr
ut_audsvc:	.long	0		# auditing data
ut_errnopp:	.long   0		# address of pointer to errno
ut_error:	.byte	0		# return error code
		.space	1
ut_flags:	.short	0		# uthread flags

#	signal management

ut_oldmask:	.long	0		# mask from before sigpause
		.long	0
ut_code:	.long	0		# iar of instr. causing exception
ut_sigsp:	.long	0		# special signal stack

#	miscellaneous

ut_fstid:	.long	0		# file system transaction ID
ut_ioctlrv:	.long	0		# return value for PSE ioctls
ut_selchn:	.long	0		# select control block
ut_timer:	.space	4*1		# t
ut_link:	.long	0		# uthread blocks free list
ut_loginfo:	.long	0		# loginfo pointer

		.space	5*4

ut_end:		.space	0

#	ut_flags values

	.set	UTSCSIG		, 0x10	# system call to signal path taken
