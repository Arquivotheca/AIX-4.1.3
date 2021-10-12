# @(#)11        1.6  src/bos/usr/bin/more/Makefile, cmdscan, bos411, 9428A410j 3/15/94 17:29:26
#
# COMPONENT_NAME: (CMDSCAN) commands that scan files
#
# FUNCTIONS:
#
# ORIGINS: 85, 27
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
#
# (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 1.2
#


PROGRAMS	= more
MSGHDRS		= more_msg.h
CATFILES	= more.cat
HFILES		= ${MSGHDRS}

CFLAGS		+= ${_AIX3CURSESFLAGS} -DTERMIO
LIBFLAGS	+= ${_AIX3CURSESLIBFLAG}

ILIST		= more
IDIR		= /usr/bin/
LIB_IDIR        = /usr/share/lib/

OFILES = ch.o command.o decode.o help.o input.o line.o linenum.o main.o \
        option.o os.o output.o position.o prim.o screen.o signal.o tags.o \
        ttyin.o

LIBS		= -lcurses

.include <${RULES_MK}>
