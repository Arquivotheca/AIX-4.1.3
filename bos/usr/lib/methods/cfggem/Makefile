# @(#)78	1.2  src/bos/usr/lib/methods/cfggem/Makefile, gemini, bos411, 9428A410j 9/29/93 14:12:38
#
#   COMPONENT_NAME: gemini
#
#   FUNCTIONS:  gto configuration method
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1988,1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#

VPATH		= ../common

PROGRAMS	= cfggem

OFILES		= cfgdevice.o cfgtools.o chkslot.o cfggem.o ddstools.o

INCFLAGS	= -I../common -I${MAKETOP}/bos/kernext/inc/${TARGET_MACHINE}  \
		  -I../common -I${MAKETOP}/bos/kernext/disp/gem/inc

ILIST		= cfggem
IDIR		= /usr/lib/methods/

LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
