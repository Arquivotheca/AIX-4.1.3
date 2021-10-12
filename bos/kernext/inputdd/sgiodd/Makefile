# @(#)60	1.2  src/bos/kernext/inputdd/sgiodd/Makefile, inputdd, bos41J, 9516B_all 4/18/95 15:47:50
# COMPONENT_NAME: INPUTDD
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994,1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

.if defined(GS_DEBUG)
CFLAGS += -DGS_DEBUG_TRACE -DGS_DEBUG
.endif

VPATH               += :${MAKETOP}/bos/kernext/inputdd/common
INCFLAGS             = -I${MAKETOP}/bos/kernext/inputdd/inc -I${MAKETOP}/bos/kernext/inputdd/inc/sys
KERNEXT_IMPORTS     += -bI:tty.exp
STRIP_FLAG          ?= -s
LDFLAGS_KERNEXT     += ${STRIP_FLAG}
LIBS                 = ${KERNEXT_LIBS}

KERNEL_EXT           = sgiodd
IDIR                 = /usr/lib/drivers/
ILIST                = sgiodd

sgiodd_ENTRYPOINT = sgio_config

OFILES = sgioconfig.o sikproc.o sgioutil.o sgio_pm.o

.include <${RULES_MK}>
