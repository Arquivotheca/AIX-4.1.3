# @(#)56        1.4  src/bos/kernel/si/Makefile, syssi, bos411, 9428A410j 5/4/94 09:33:24
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

# Do not compile si code with data_in_toc
DATA_IN_TOC	=

VPATH		=${TARGET_MACHINE}
INCFLAGS      	=-I${TARGET_MACHINE}

PINNED_OFILES	=	\
        globals.o

PAGED_OFILES_BACK =	\
	aixinit.o

INIT_OFILES	=	\
	main.o		\
	${${TARGET_MACHINE}_INIT_OFILES}

.include <${TARGET_MACHINE}/machdep.mk>

.include <${RULES_MK}>
