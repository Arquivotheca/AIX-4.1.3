# @(#)19	1.1  src/bos/usr/lib/methods/cfgapm/Makefile, cfgmethods, bos411, 9428A410j 9/8/93 13:54:11
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

PROGRAMS	= cfgapm

OFILES		= cfgdevice.o cfgtools.o chkslot.o ddstools.o cfgapm.o

INCFLAGS	= -I../common

ILIST		= cfgapm
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
