# @(#)77        1.2  src/bos/usr/lib/methods/cfgcon/Makefile, cfgmethods, bos411, 9428A410j 10/21/93 13:18:04
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

PROGRAMS	= cfgcon
MSGHDRS		= console_msg.h
HFILES		= ${MSGHDRS}

OFILES		= cfgcon.o findcons.o

ILIST		= cfgcon
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
