# @(#)33        1.2  src/bos/usr/lib/methods/cfgfddi/Makefile, sysxfddi, bos411, 9428A410j 3/16/94 16:49:15
#
#   COMPONENT_NAME: SYSXFDDI
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH           = ../common

PROGRAMS	= cfgfddi
CATFILES	= fddi.cat

OFILES          = cfgcommo.o cfgtoolsx.o chkslot.o cfgfddi.o

INCFLAGS        = -I../common -I../cfgcie \
		  -I${MAKETOP}/bos/kernext

ILIST		= cfgfddi fddi.cat
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
