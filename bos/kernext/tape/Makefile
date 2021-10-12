# @(#)29        1.2  src/bos/kernext/tape/Makefile, Makefiles, bos411, 9428A410j 3/24/93 21:32:34
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

KERNEL_EXT	= tape tapepin
IDIR		= /usr/lib/drivers/
ILIST		= tape tapepin

LIBS		= ${KERNEXT_LIBS}

tape_ENTRYPOINT	= strconfig
tape_IMPORTS	= -bI:tapepin.exp
tape_EXPORTS	= -bE:tape.exp

tapepin_IMPORTS	= -bI:tape.exp
tapepin_EXPORTS	= -bE:tapepin.exp

tapepin_OFILES 	= tapeb.o

STRIP_FLAG ?= -s

LDFLAGS_KERNEXT += ${STRIP_FLAG}

.include <${RULES_MK}>
