# @(#)76 1.2 src/bos/usr/lib/methods/ucfgtty/Makefile, cfgmethods, bos411, 9428A410j 10/18/93 10:36:18
#
# COMPONENT_NAME: (CFGMETHODS)
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
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
# LEVEL 1, 5 Years Bull Confidential Information
#

VPATH       = ../common:../cfgtty

PROGRAMS    = ucfgtty

OFILES      = ucfgtty.o\
              cfgtools.o\
              commondds.o\
              ldtermdds.o\
              nlsdds.o\
              pop_push.o\
              sptrdds.o\
              tiocdds.o

INCFLAGS    = -I../common\
              -I${MAKETOP}/bos/kernext/tty

LINTFILES    = ucfgtty.c
LINTFLAGS    = -wk -u -x

ILIST       = ucfgtty
IDIR        = /usr/lib/methods/

LIBS        = -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
