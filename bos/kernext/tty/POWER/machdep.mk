# @(#)32 1.11.1.3 src/bos/kernext/tty/POWER/machdep.mk, sysxtty, bos41J, 9512A_all 3/21/95 16:14:10
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

POWER_KERNEL_EXT      = rsdd liondd
POWER_ILIST           = rsdd liondd

LOCALCFLAGS           = -g -D_KERNSYS 

rsdd_ENTRYPOINT       = srsconfig
liondd_ENTRYPOINT     = slionconfig

#rsdd_IMPORTS          = ${IMPORTS} -bI:dbtty.exp
#liondd_IMPORTS        = ${IMPORTS} -bI:dbtty.exp

liondd_OFILES         = slion.o slion_db.o sfix.o\
                        common_open.o sdtropen.o swtopen.o
rsdd_OFILES           = srs.o common_open.o sdtropen.o swtopen.o


