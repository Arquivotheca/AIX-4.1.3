# @(#)99        1.4  src/bos/usr/bin/mail/Makefile, cmdmailx, bos411, 9428A410j 3/29/94 15:24:42
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

PROGRAMS	= mail
MSGHDRS		= mail_msg.h
CATFILES	= mail.cat
HFILES		= ${MSGHDRS}

ILIST		= mail Mail.rc
IDIR		= /usr/bin/
Mail.rc_IDIR	= /usr/share/lib/

CFLAGS		= -D_BSD -DAIX -DVMUNIX -DV7 -DBSD_INCLUDES -DNOCATMACRO

OFILES		= aux.o cmd2.o cmd3.o cmdtab.o collect.o config.o edit.o \
		  fio.o getname.o head.o lex.o list.o main.o names.o optim.o \
		  quit.o send.o strings.o temp.o tty.o v7.local.o vars.o \
		  cmd1.o version.o popen.o

LIBS		= -lbsd
.include <${RULES_MK}>
