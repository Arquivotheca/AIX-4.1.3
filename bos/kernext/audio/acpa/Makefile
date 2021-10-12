# @(#)95        1.1  src/bos/kernext/audio/acpa/Makefile, sysxacpa, bos411, 9428A410j 8/5/92 18:08:35
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT	= acpadd
IDIR		= /usr/lib/drivers/
ILIST		= acpadd

acpadd_ENTRYPOINT	= acpaconfig
LIBS			= ${KERNEXT_LIBS}

LOCALCFLAGS	= -D_DISPLAYTRACEFLAG

INCFLAGS	=-I${MAKETOP}bos/kernext/inc

acpadd_OFILES	= acpadd.o acpadd2.o

.include <${RULES_MK}>

