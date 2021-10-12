# @(#)28	1.1  src/bos/usr/lib/methods/cfgcat/Makefile, sysxcat, bos411, 9428A410j 9/27/93 11:58:23
#
#   COMPONENT_NAME: 370PCA_DD
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

PROGRAMS	= cfgcat

OFILES		= cfgdevice.o cfgtoolsx.o chkslot.o cfgcat.o 

INCFLAGS	= -I${MAKETOP}/bos/usr/lib/methods/common \
		  -I${MAKETOP}/bos/kernext/cat

ILIST		= cfgcat
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
