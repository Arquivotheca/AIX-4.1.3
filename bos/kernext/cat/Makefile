# @(#)73	1.5  src/bos/kernext/cat/Makefile, sysxcat, bos411, 9428A410j 3/7/94 13:56:51
# COMPONENT_NAME:
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

KERNEL_EXT	= catdd
IDIR		= /usr/lib/drivers/
ILIST		= catdd

INCLUDES	= catdd.h pscadefs.h catproto.h

LIBS		= ${KERNEXT_LIBS}

LOCALCFLAGS = -D_AIXV3_POSTWAIT

catdd_ENTRYPOINT	= catconfig

catdd_OFILES = \
	cat_config.o cat_open.o cat_close.o cat_cdt.o \
	cat_ioctl.o cat_intr.o cat_mpx.o cat_util.o \
	cat_pcaio.o cat_recv.o cat_select.o cat_xmit.o 


.include <${RULES_MK}>
