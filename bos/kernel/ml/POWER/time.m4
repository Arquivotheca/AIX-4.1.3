# @(#)29	1.4  src/bos/kernel/ml/POWER/time.m4, sysml, bos411, 9428A410j 6/15/90 18:14:18
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
# (C) COPYRIGHT International Business Machines Corp. 1989, 1990
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

#
#               Timer Structure -- R2 version
#		NOTE:  This is the assembler version of <sys/time.h>.
#			Changes to this file should be reflected in
#			<sys/time.h> and vice versa.
#

		.dsect	timerstruc

tv_sec:		.long	0		# seconds
tv_nsec:	.long	0		# nanoseconds

		.dsect	elapstruc
cursec:		.long	0		# current time (seconds)
curns:		.long	0		# current time (ns)
elapsec:	.long	0		# seconds since last call
elapns:		.long	0		# ns since last call

