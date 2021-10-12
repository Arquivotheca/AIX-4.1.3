# @(#)67        1.2  src/bos/usr/lib/methods/chgent/Makefile, cfgmethods, bos411, 9428A410j 9/20/93 17:24:04
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

PROGRAMS	= chgent

OFILES          = chgdevice.o cfgtoolsx.o location.o chgent.o coment.o

INCFLAGS	= -I../common -I${MAKETOP}/bos/kernext

ILIST		= chgent
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
