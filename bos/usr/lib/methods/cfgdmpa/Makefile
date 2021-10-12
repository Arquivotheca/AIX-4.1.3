# @(#)81	1.2  src/bos/usr/lib/methods/cfgdmpa/Makefile, cfgmethods, bos411, 9428A410j 5/16/94 07:56:04
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

PROGRAMS	= cfgdmpa
MSGHDRS		= dmpa_msg.h
CATFILES	= dmpa.cat
HFILES		= 

OFILES		= cfgdevice.o cfgtoolsx.o chkslot.o cfgdmpa.o 

INCFLAGS	= -I${MAKETOP}/bos/usr/lib/methods/common

ILIST		= cfgdmpa dmpa.cat
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
