# @(#)67	1.1  src/bos/usr/lib/methods/ucfglft/Makefile, lftdd, bos411, 9428A410j 10/25/93 16:56:35
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

VPATH           = ../common

PROGRAMS	= ucfglft
OFILES          = ucfglft.o pop_mod.o cfg_lfterr.o

INCFLAGS	= -I../common  -I${MAKETOP}/../export/power/kernel/include

ILIST		= ucfglft
IDIR		= /usr/lib/methods/

LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
