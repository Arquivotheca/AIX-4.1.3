# @(#)27 1.1 src/bos/usr/lib/methods/ucfgasync/Makefile, cfgmethods, bos411, 9428A410j 10/18/93 10:49:58
#
# COMPONENT_NAME: (CFGMETHODS)
#
# FUNCTIONS:
#
# ORIGINS: 83
#
# LEVEL 1, 5 Years Bull Confidential Information
#

VPATH       = ../cfgasync:../cfgtty:../common

PROGRAMS    = ucfgasync

ucfgasync_OFILES    = cfgtools.o\
                      commondds.o\
                      lionadapdds.o\
                      rsadapdds.o\
                      ucfgasync.o

INCFLAGS    = -I../common -I${MAKETOP}/bos/kernext/tty/${TARGET_MACHINE}

ucfgasync_LINTFILES = ucfgasync.c

LINTFLAGS   = -wk -u -x

LIBS        = -ls -lodm -lcfg -lrts

ILIST       = ucfgasync
IDIR        = /usr/lib/methods/

.include <${RULES_MK}>
