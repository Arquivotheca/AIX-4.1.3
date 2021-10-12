# @(#)70 1.1 src/bos/usr/lib/methods/ucfgpty/Makefile, cfgmethods, bos411, 9428A410j 6/2/94 08:17:51
#
# COMPONENT_NAME: (CFGMETHODS)
#
# FUNCTIONS:
#
# ORIGINS: 83
#
# LEVEL 1, 5 Years Bull Confidential Information
#

VPATH       = ../common:../cfgpty:../cfgtty

PROGRAMS    = ucfgpty

ucfgpty_OFILES    = cfgtools.o \
					commondds.o \
					ldtermdds.o \
					nlsdds.o \
					pop_push.o \
					sptrdds.o \
					sptydds.o \
					tiocdds.o \
                    ucfgpty.o

INCFLAGS    = -I../common \
			  -I${MAKETOP}/bos/kernext/pty \
			  -I${MAKETOP}/bos/kernext/tty

ucfgpty_LINTFILES = ucfgpty.c

LINTFLAGS   = -wk -u -x

LIBS        = -ls -lodm -lcfg -lrts

ILIST       = ucfgpty
IDIR        = /usr/lib/methods/

.include <${RULES_MK}>
