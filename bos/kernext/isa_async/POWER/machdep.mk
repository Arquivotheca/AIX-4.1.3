# @(#)46 1.1 src/bos/kernext/isa_async/POWER/machdep.mk, sysxs128, bos41J, 9512A_all 3/22/95 12:10:28
# COMPONENT_NAME: (sysxtty)
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1, 5 Years Bull Confidential Information
#

KERNEL_EXT      = cxiadd
ILIST           = cxiadd 

LOCALCFLAGS           = -g -D_KERNSYS -DISA

cxiadd_ENTRYPOINT     = scxma_config


OFILES          = scxma_1.o scxma_2.o scxma_3.o scxma_4.o\
		  common_open.o swtopen.o sdtropen.o
