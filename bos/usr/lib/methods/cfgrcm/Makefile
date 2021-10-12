# @(#)96	1.3  src/bos/usr/lib/methods/cfgrcm/Makefile, rcm, bos411, 9428A410j 2/28/94 13:30:38
#   COMPONENT_NAME: RCM
#
#   FUNCTIONS:
#
#   ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ../common

PROGRAMS	= cfgrcm

OFILES		= cfgrcm.o cfgtools.o ddstools.o

INCFLAGS	= -I../common 

ILIST		= cfgrcm
IDIR		= /usr/lib/methods/

LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
