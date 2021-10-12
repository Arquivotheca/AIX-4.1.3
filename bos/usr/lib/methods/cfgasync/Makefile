# @(#)31 1.2 src/bos/usr/lib/methods/cfgasync/Makefile, cfgmethods, bos411, 9428A410j 10/18/93 10:35:15
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

PROGRAMS    = cfgasync

cfgasync_OFILES    = cfgasync.o\
                     cfgchk_64p.o\
                     cfgtools.o\
                     chkslot.o\
                     commondds.o\
                     lionadapdds.o\
                     rsadapdds.o\
                     utility.o


INCFLAGS    = -I../common -I${MAKETOP}/bos/kernext/tty/${TARGET_MACHINE}

cfgasync_LINTFILES = cfgasync.c\
                     lionadapdds.c\
                     rsadapdds.c\
                     utility.c

LINTFLAGS   = -wk -u -x

LIBS        = -ls -lodm -lcfg -lrts

ILIST       = cfgasync
IDIR        = /usr/lib/methods/

.include <${RULES_MK}>

