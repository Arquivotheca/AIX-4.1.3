# @(#)84        1.2  src/bos/usr/lib/methods/chgaio/Makefile, Makefiles, bos411, 9428A410j 6/4/93 08:29:19
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

PROGRAMS	= chgaio

OFILES		= chgdevice.o location.o aio.o

INCFLAGS	= -I../common

ILIST		= chgaio
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
