# @(#)52        1.1  src/bos/usr/lib/methods/cfgbadisk/Makefile, cfgmethods, bos411, 9428A410j 8/5/92 17:03:15
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

PROGRAMS	= cfgbadisk

OFILES		= cfgdisk.o cfgbadisk.o pvidtools.o cfgtools.o \
		  chkslot.o

INCFLAGS	= -I../common

ILIST		= cfgbadisk
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
