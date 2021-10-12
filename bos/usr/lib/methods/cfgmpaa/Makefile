# @(#)30	1.2  src/bos/usr/lib/methods/cfgmpaa/Makefile, mpacfg, bos411, 9428A410j 5/16/94 07:54:03
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

PROGRAMS	= cfgmpaa
MSGHDRS		= mpa_msg.h
CATFILES	= mpa.cat
HFILES		= 

OFILES		= cfgdevice.o cfgtoolsx.o chkslot.o cfgmpaa.o 

INCFLAGS	= -I${MAKETOP}/bos/usr/lib/methods/common

ILIST		= cfgmpaa mpa.cat
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
