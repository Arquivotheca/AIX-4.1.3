# @(#)75	1.1  src/bos/kernel/ml/POWER/iocc.m4, sysml, bos411, 9428A410j 3/16/93 20:34:24
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
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

#
#
#  RS1/RS2 IOCC specific register labels
#
#

#       segment register value for IOCC access

       .set     iocc_seg, 0x820C00E0
       .set     u.iocc_seg, 0x820C
       .set     l.iocc_seg, 0x00E0

#       IOCC data starts at 0x0040000 with the config register at 0x00400010

        .set    iocc_addr, 0x00400000
        .set    u.iocc_addr, 0x0040
        .set    l.iocc_addr, 0x0000       
        .set    confdata, 0x0010
	.set	eoidata, 0x008C

