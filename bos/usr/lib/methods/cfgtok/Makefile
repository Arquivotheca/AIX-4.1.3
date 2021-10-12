# @(#)56        1.3  src/bos/usr/lib/methods/cfgtok/Makefile, sysxtok, bos411, 9428A410j 3/16/94 09:49:27
# COMPONENT_NAME: SYSXTOK
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

VPATH		= ../common

PROGRAMS	= cfgtok

OFILES          = cfgcommo.o cfgtoolsx.o chkslot.o cfgtok.o comtok.o

INCFLAGS        = -I../common -I${MAKETOP}/bos/kernext -I../cfgcie

ILIST		= cfgtok
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
