# @(#)42	1.2  src/bos/usr/bin/csh/Makefile, cmdcsh, bos411, 9428A410j 11/12/92 13:27:26
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1992
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/kernel/exp

PROGRAMS	= csh
MSGHDRS		= csh_msg.h
CATFILES	= csh.cat
HFILES		= ${MSGHDRS}

ILIST		= csh
IDIR		= /usr/bin/

CFLAGS		= -DALLOC -D_BSD -DFILEC

OFILES		= dir.o dol.o doprnt.o err.o exec.o exp.o \
		  file.o func.o glob.o hist.o init.o lex.o misc.o \
		  parse.o print.o proc.o sem.o set.o sh.o time.o


csh_IMPORTS	= -bI:syscalls.exp

.include <${RULES_MK}>
