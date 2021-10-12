# @(#)28        1.2  src/bos/usr/lib/methods/cfgsio/Makefile, cfgmethods, bos411, 9428A410j 5/9/94 16:04:54
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		=../common

PROGRAMS	= cfgsio

OFILES		= cfgdevice.o cfgtools.o chkslot.o cfgsio.o cfgvpd.o

INCFLAGS	=-I../common

ILIST		= cfgsio
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
