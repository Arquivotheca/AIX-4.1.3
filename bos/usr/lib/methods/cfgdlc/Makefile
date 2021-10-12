# @(#)51       1.1  src/bos/usr/lib/methods/cfgdlc/Makefile, dlccfg, bos411, 9428A410j 1/24/94 11:03:00 
# COMPONENT_NAME: DLCCFG
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ../common

PROGRAMS	= cfgdlc

OFILES          = cfgdlc.o cfggendlc.o cfgtools.o leds.o

INCFLAGS        = -I../common

ILIST		= cfgdlc
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
