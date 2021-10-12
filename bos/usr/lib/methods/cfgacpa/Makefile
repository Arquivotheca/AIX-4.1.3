# @(#)80        1.1  src/bos/usr/lib/methods/cfgacpa/Makefile, sysxacpa, bos411, 9428A410j 8/5/92 17:09:16
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

VPATH		= ../common

PROGRAMS	= cfgacpa

OFILES		= cfgdevice.o cfgtools.o ddstools.o chkslot.o cfgacpa.o

INCFLAGS	= -I../common -I${MAKETOP}/bos/kernext/audio/acpa

ILIST		= cfgacpa
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
