# @(#)20	1.2  src/bos/usr/lib/methods/cfgmpqd/Makefile, cfgmethods, bos411, 9428A410j 9/9/93 17:54:00
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

PROGRAMS	= cfgmpqd

OFILES		= cfgdevice.o cfgtools.o chkslot.o ddstools.o cfgmpqd.o

INCFLAGS	= -I../common

ILIST		= cfgmpqd
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
