# @(#)45        1.2  src/bos/kernel/lib/libsys/Makefile, libsys, bos411, 9428A410j 9/2/92 20:04:41
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

.include <${MAKETOP}/bos/kernel/Kernel.mk>

VPATH		= ${TARGET_MACHINE}

LIBRARIES	= libsys.a
MSGHDRS         =
CATFILES        =
HFILES          = ${MSGHDRS}

EXPLIB_TARGETS  = export_libsys.a

EXPDIR		= /kernel/lib/

ILIST           = libsys.a
IDIR            = /usr/lib/

CFLAGS		= -D_KERNEL

OFILES		= \
	timeout.o xdump.o \
	${${TARGET_MACHINE}_OFILES}

.include <${TARGET_MACHINE}/machdep.mk>

.include <${RULES_MK}>
