# @(#)65	1.7  src/bos/usr/bin/bsh/Makefile, cmdbsh, bos411, 9428A410j 4/22/94 18:10:44
# COMPONENT_NAME: cmdbsh
#
# FUNCTIONS:
#
# ORIGINS: 27
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

VPATH		= ${MAKETOP}/bos/kernel/exp

PROGRAMS	= bsh
MSGHDRS		= bsh_msg.h
CATFILES	= bsh.cat
HFILES		= ${MSGHDRS}

ILIST		= bsh
IDIR		= /usr/bin/

CFLAGS		= -DACCT

OFILES		= \
	args.o blok.o cmd.o ctype.o defs.o echo.o error.o expand.o \
	fault.o func.o hash.o hashserv.o io.o macro.o main.o msg.o \
	name.o nls.o print.o pwd.o service.o setbrk.o stak.o string.o \
	test.o word.o xec.o

bsh_IMPORTS	= -bI:syscalls.exp
bsh_LDFLAGS	= -bnso

.include <${RULES_MK}>
