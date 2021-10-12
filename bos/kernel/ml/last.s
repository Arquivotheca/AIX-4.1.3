# @(#)84        1.11  src/bos/kernel/ml/last.s, sysml, bos411, 9428A410j 11/19/93 11:48:03
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
# (C) COPYRIGHT International Business Machines Corp. 1989, 1992
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
# names: init_obj_end, endtoc, endcomm
#
# function:
#       last is the last object module in the kernel bind, so
#       its labels go at the end of their respective sections
#       i.e. TOC and common.
#
#**********************************************************************

	.file	"last.s"

        .csect	init_obj_end[RO]

DATA(init_obj_end):
        .globl  DATA(init_obj_end)


        CSECT(lasttocentry,TC)      # last byte plus one of TOC
        .toc
        .tc     DATA(lasttocentry)[TC],DATA(endtoc)
        .globl	DATA(lasttocentry)[TC]        
        .globl  DATA(endtoc)
        .csect  [RW]
DATA(endtoc):
        .long   DATA(lasttocentry)[TC]
        .globl  DATA(loader_sect_start)
DATA(loader_sect_start):
        .comm   DATA(endcomm), 0      # last byte plus one of entire module
        .csect  [PR]
        .long   0
