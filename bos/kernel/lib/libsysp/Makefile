# @(#)46        1.2  src/bos/kernel/lib/libsysp/Makefile, libsysp, bos411, 9428A410j 1/10/94 11:26:10
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

VPATH		= ${TARGET_MACHINE}

LIBRARIES	= libsysp.a
MSGHDRS         =
CATFILES        =
HFILES          = ${MSGHDRS}

EXPLIB_TARGETS  = export_libsysp.a

ILIST           =libsysp.a
IDIR            = /kernel/lib/

CFLAGS		= -D_KERNEL
OFILES		= \
	copystr.o insque.o insque2.o listmgr.o sgetl.o \
	${${TARGET_MACHINE}_OFILES}

.include <${TARGET_MACHINE}/machdep.mk>

.include <${RULES_MK}>
