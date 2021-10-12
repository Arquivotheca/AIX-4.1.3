# @(#)86        1.7  src/bos/usr/bin/ate/Makefile, cmdate, bos411, 9428A410j 2/21/94 08:40:28
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= ate xmodem
DATAFILES	= dir
MSGHDRS		= ate_msg.h xshell_msg.h
CATFILES	= ate.cat xshell.cat
HFILES		= ${MSGHDRS}

ILIST		= ate xmodem ${DATAFILES}
IDIR		= /usr/bin/
dir_IDIR	= /usr/lib/

CFLAGS		+= ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

ate_OFILES	= alter.o check.o cls.o command.o connect.o directory.o \
		  hangup.o help.o lex.yy.o lookup.o main.o menu.o \
		  menuinit.o message.o modify.o msghelp.o pacing.o \
		  portrw.o portvt.o setups.o shared.o signal.o vt100.o \
		  xmodem.o

xmodem_OFILES	= xshell.o

ate_LIBS	= -lcur -lcurses -lrts

.include <${RULES_MK}>

lex.yy.c:	lexer
	${LEX} ${lexer:P}
	${SED} '/# define input() /d' lex.yy.c > yyy
	${SED} '/# define yymore() /d' yyy > lex.yy.c
	${RM} -f yyy
