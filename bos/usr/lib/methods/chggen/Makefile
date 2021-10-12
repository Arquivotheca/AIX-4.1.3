# @(#)26        1.1  src/bos/usr/lib/methods/chggen/Makefile, cfgmethods, bos411, 9428A410j 8/5/92 16:58:44
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

PROGRAMS	= chggen

OFILES		= chgdevice.o cfgtools.o location.o chggen.o

INCFLAGS	=-I../common

ILIST		= chggen
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
