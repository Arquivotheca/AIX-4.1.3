# @(#)31	1.2  src/bos/usr/lib/methods/cfgmpa/Makefile, mpacfg, bos411, 9428A410j 9/28/93 12:48:05
#
#   COMPONENT_NAME: 
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

VPATH		= ${MAKETOP}/bos/usr/lib/methods/common

PROGRAMS	= cfgmpa

OFILES		= cfgdevice.o cfgtoolsx.o chkslot.o cfgmpa.o 

INCFLAGS	= -I${MAKETOP}/bos/usr/lib/methods/common

ILIST		= cfgmpa
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
