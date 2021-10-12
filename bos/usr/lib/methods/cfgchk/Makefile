# @(#)79        1.1  src/bos/usr/lib/methods/cfgchk/Makefile, cfgmethods, bos411, 9428A410j 8/5/92 17:09:04
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

PROGRAMS	= cfgchk
MSGHDRS		= cfgchk_msg.h
HFILES		= ${MSGHDRS} cfgchk.h

OFILES		= cfgchk.o cfgtools.o cfgchk_64p.o

INCFLAGS	= -I../common

ILIST		= cfgchk
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
