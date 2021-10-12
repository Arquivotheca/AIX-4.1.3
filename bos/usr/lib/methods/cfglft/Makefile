# @(#)01	1.3  src/bos/usr/lib/methods/cfglft/Makefile, lftdd, bos411, 9428A410j 12/1/93 09:25:45
#   COMPONENT_NAME: LFTDD
#
# FUNCTIONS:
#
#   ORIGINS: 27
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

VPATH		= ../common

PROGRAMS	= cfglft

OFILES		= cfglft.o ddstools.o cfg_lfterr.o

INCFLAGS	= -I../common -I${MAKETOP}/bos/kernext/tty \
		  -I${MAKETOP}/bos/kernext/tty/${TARGET_MACHINE}

ILIST		= cfglft
IDIR		= /usr/lib/methods/

LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
