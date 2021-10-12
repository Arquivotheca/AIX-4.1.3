# @(#)41        1.3  src/bos/usr/lib/methods/cfggio/Makefile, inputdd, bos411, 9428A410j 11/30/93 08:41:04
# COMPONENT_NAME: INPUTDD
#
# FUNCTIONS: N/A
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH       = ../common

PROGRAMS    = cfggio

OFILES      = cfgdevice.o cfgtools.o chkslot.o cfggio.o

INCFLAGS    = -I../common

ILIST       = cfggio
IDIR        = /usr/lib/methods/


LIBS        = -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
