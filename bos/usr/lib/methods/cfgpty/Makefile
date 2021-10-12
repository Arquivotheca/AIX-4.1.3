# @(#)54 1.4 src/bos/usr/lib/methods/cfgpty/Makefile, cfgmethods, bos411, 9428A410j 2/4/94 06:42:59
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
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1, 5 Years Bull Confidential Information
#

VPATH            = ../common:../cfgtty

PROGRAMS         = cfgpty

OFILES           = cfgpty.o\
                   cfgtools.o\
                   check_files.o\
                   commondds.o\
                   ldtermdds.o\
                   nlsdds.o\
                   pop_push.o\
                   sptrdds.o\
                   sptydds.o\
                   tiocdds.o

INCFLAGS         = -I../common\
                   -I${MAKETOP}/bos/kernext/pty\
                   -I${MAKETOP}/bos/kernext/tty

chgtty_LINTFILES = cfgpty.c\
                   check_files.c\
                   sptydds.c

LINTFLAGS        = -wk -u -x

ILIST            = cfgpty
IDIR             = /usr/lib/methods/


LIBS             = -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
