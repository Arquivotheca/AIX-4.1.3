# @(#)59        1.5  src/bos/kernel/vmm/Makefile, sysvmm, bos411, 9438C411a 9/23/94 10:51:15
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

.include <${MAKETOP}bos/kernel/Kernel.mk>

VPATH		=${TARGET_MACHINE}

PINNED_OFILES	=	\
	${${TARGET_MACHINE}_PINNED_OFILES}

PAGED_OFILES_PRIM =	\
	${${TARGET_MACHINE}_PAGED_OFILES_PRIM}

PAGED_OFILES	=	\
	${${TARGET_MACHINE}_PAGED_OFILES}

INIT_OFILES	=	\
	${${TARGET_MACHINE}_INIT_OFILES}

INCFLAGS	=-I${TARGET_MACHINE}

.include <${TARGET_MACHINE}/machdep.mk>

.include <${RULES_MK}>
