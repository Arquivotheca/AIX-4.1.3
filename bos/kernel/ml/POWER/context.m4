# @(#)81	1.7  src/bos/kernel/ml/POWER/context.m4, sysml, bos411, 9438C411a 9/22/94 14:58:53
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

#-----------------------------------------------------------------------#
#
#               Signal Context Jump Buffer -- R2 version
#		this is the assembler version of <sys/context.h>
#		a full mstsave area follows sc_error and maps to
#		the structure found in mstsave.m4
#-----------------------------------------------------------------------#

		.dsect	sigcontext

sc_start:	.space	0
sc_onstack:	.long	0		# onstack indicator
sc_mask_lo:	.long	0		# low end of signal mask
sc_mask_hi:	.long	0		# high end of signal mask
sc_error:	.long	0		# saved u_error value
sc_end:		.space	0

	.dsect	sig_save
signo:		.long	0
code:		.long	0
scp:		.long	0
msr:		.long	0
sh_fp:		.long	0
stkp:		.long	0
sh_ret:		.long	0
sreg:		.long 	0

