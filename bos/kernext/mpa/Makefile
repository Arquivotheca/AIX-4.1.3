# @(#)32        1.2  src/bos/kernext/mpa/Makefile, sysxmpa, bos411, 9428A410j 9/28/93 09:47:23
# COMPONENT_NAME: SYSXMPA
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

.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT	= mpadd
IDIR		= /usr/lib/drivers/
ILIST		= mpadd

LIBS		= ${KERNEXT_LIBS}

mpadd_ENTRYPOINT	= mpa_config

mpadd_OFILES = \
	mpa_config.o mpa_open.o mpa_close.o mpa_write.o \
	mpa_ioctl.o mpa_intr.o mpa_mpx.o mpa_util.o


.include <${RULES_MK}>
