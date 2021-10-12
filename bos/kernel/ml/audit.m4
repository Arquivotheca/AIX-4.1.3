# @(#)01	1.1  src/bos/kernel/ml/audit.m4, sysml, bos411, 9428A410j 10/15/93 12:50:17
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
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

	.dsect	aud

aud_svcnum:	.short	0		# name index from audit_klookup
aud_argcnt:	.short	0		# number of arguments stored
aud_args:	.space	10*4		# parameters for this call
aud_audbuf:	.long	0		# buffer for misc audit record
aud_bufsiz:	.long	0		# allocated size of pathname buffer
aud_buflen:	.long	0		# actual length of pathname(s)
aud_bufcnt:	.short	0		# number of pathnames stored
		.space	2
aud_status:	.long	0		# audit status bitmap

aud_end:	.space	0
