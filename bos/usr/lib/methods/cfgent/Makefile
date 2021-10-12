# @(#)66        1.3  src/bos/usr/lib/methods/cfgent/Makefile, cfgmethods, bos411, 9428A410j 3/17/94 15:39:57
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

PROGRAMS	= cfgent

OFILES          = cfgcommo.o cfgtoolsx.o chkslot.o cfgent.o coment.o

INCFLAGS        = -I../common -I${MAKETOP}/bos/kernext -I../cfgcie

ILIST		= cfgent
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
