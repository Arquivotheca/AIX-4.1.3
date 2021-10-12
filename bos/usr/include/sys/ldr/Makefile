# @(#)85	1.2  src/bos/usr/include/sys/ldr/Makefile, Makefiles, bos411, 9428A410j 2/8/93 13:48:12
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 10,27
#
# IBM CONFIDENTIAL -- (IBM Confidential	Restricted when
# combined with	the aggregated modules for this	product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT	International Business Machines	Corp. 1988, 1989
# All Rights Reserved
#
# US Government	Users Restricted Rights	- Use, duplication or
# disclosure restricted	by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/kernel/ldr:${MAKETOP}/bos/kernel/ldr/${TARGET_MACHINE}

IDIR		= /usr/include/sys/ldr/

DATAFILES	= \
	ld_rbytes.h \
	ld_data.h m_ld_data.h

INCLUDES	= ${DATAFILES}

ILIST		=

.if exists (${TARGET_MACHINE}/machdep.mk)
.include <${TARGET_MACHINE}/machdep.mk>
.endif

.include <${RULES_MK}>
