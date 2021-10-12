# @(#)46        1.5  src/bos/usr/bin/keycomp/Makefile, cmdimkc, bos411, 9428A410j 2/4/94 13:17:21
# COMPONENT_NAME: (cmdimkc) AIX Input Method Keymap Compiler
#
# FUNCTIONS: Makefile for keycomp
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/usr/include/X11

SUBDIRS		= X11

PROGRAMS	= keycomp
MSGHDRS		= keycomp_msg.h
CATFILES	= keycomp.cat
HFILES		= ${MSGHDRS}

ILIST		= keycomp
IDIR		= /usr/bin/

YFLAGS		= -Nm200000

INCFLAGS	= -I${MAKETOP}/bos/usr/include

OFILES		= keycomp.o CompileCompose.o Layer.o ComposeTable.o \
			Item.o Rule.o

LIBS		= -ll -ly

.include <${RULES_MK}>

Item.o : Rule.h
