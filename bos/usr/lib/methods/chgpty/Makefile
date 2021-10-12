# @(#)50 1.1 src/bos/usr/lib/methods/chgpty/Makefile, cfgmethods, bos411, 9428A410j 2/4/94 07:00:48
#
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 83
#
# LEVEL 1, 5 Years Bull Confidential Information
#

VPATH       = ../common:../cfgpty

PROGRAMS    = chgpty

OFILES      = cfgtools.o\
              chgdevice.o\
              chgpty.o\
              check_files.o\
              location.o\
              sptydds.o

INCFLAGS    = -I../common\
              -I${MAKETOP}/bos/kernext/pty


LINTFILES   = chgpty.c

LINTFLAGS   = -wk -u -x
ILIST       = chgpty
IDIR        = /usr/lib/methods/

LIBS        = -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
