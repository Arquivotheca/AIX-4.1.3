# @(#)34 1.1 src/bos/kernext/mca_async/POWER/machdep.mk, sysxs128, bos41J, 9512A_all 3/22/95 04:57:57
#
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

KERNEL_EXT      = cxmadd
ILIST           = cxmadd 

LOCALCFLAGS           = -g -D_KERNSYS 

cxmadd_ENTRYPOINT     = scxma_config


OFILES          = common_open.o sdtropen.o swtopen.o \
		 scxma_1.o scxma_2.o scxma_3.o scxma_4.o
