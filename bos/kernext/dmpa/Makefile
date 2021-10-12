# @(#)78	1.1  src/bos/kernext/dmpa/Makefile, sysxdmpa, bos411, 9428A410j 10/4/93 16:20:27
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

KERNEL_EXT	= dmpadd
IDIR		= /usr/lib/drivers/
ILIST		= dmpadd

LIBS		= ${KERNEXT_LIBS}

dmpadd_ENTRYPOINT	= mpaconfig

dmpadd_OFILES = \
	dmpa_close.o     \
	dmpa_config.o    \
	dmpa_intr.o      \
	dmpa_ioctl.o     \
	dmpa_mpx.o       \
	dmpa_open.o      \
	dmpa_recv.o      \
	dmpa_select.o    \
	dmpa_util.o      \
	dmpa_xmit.o


.include <${RULES_MK}>
