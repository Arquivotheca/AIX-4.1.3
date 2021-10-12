# @(#)47        1.4  src/bos/kernel/ml/Makefile, sysml, bos411, 9428A410j 3/15/93 08:03:04
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernel/Kernel.mk>

VPATH		=${TARGET_MACHINE}

INCFLAGS	=-I./${TARGET_MACHINE} -I${MAKETOP}bos/kernel/io/${TARGET_MACHINE}

PINNED_OFILES	= \
	${${TARGET_MACHINE}_PINNED_OFILES}

PAGED_OFILES	= \
	mststack.o

DBG_OFILES = \
	${${TARGET_MACHINE}_DBG_OFILES}

INIT_OFILES	= \
	start.o \
	${${TARGET_MACHINE}_INIT_OFILES}


OBJECTS	= vr_end.o last.o pg_end.o ${${TARGET_MACHINE}_OBJECTS}

.include <${TARGET_MACHINE}/machdep.mk>

.include <${RULES_MK}>
