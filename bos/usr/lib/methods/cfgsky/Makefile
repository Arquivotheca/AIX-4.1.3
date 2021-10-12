# @(#)58	1.4  src/bos/usr/lib/methods/cfgsky/Makefile, sysxdispsky, bos411, 9428A410j 11/22/93 12:17:31
#
#   COMPONENT_NAME: sysxdispsky
#
#   FUNCTIONS: 
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

PROGRAMS	= cfgsky

OFILES		= cfgdevice.o cfgtools.o chkslot.o cfgsky.o ddstools.o

INCFLAGS	= -I../common

ILIST		= cfgsky
IDIR		= /usr/lib/methods/

LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
