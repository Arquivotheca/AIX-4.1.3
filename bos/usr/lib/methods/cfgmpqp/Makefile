# @(#)51        1.2  src/bos/usr/lib/methods/cfgmpqp/Makefile, cfgmethods, bos411, 9428A410j 9/8/93 14:20:08
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

PROGRAMS	= cfgmpqp

OFILES		= cfgdevice.o cfgtools.o chkslot.o cfgmpqp.o

INCFLAGS	= -I../common \
		-I${MAKETOP}/bos/kernext/mpqp

ILIST		= cfgmpqp
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
