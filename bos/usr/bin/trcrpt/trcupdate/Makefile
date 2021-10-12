#  @(#)53        1.7  src/bos/usr/bin/trcrpt/trcupdate/Makefile, cmdtrace, bos411, 9428A410j 4/13/94 16:20:16
#
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 83 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# LEVEL 1, 5 Years Bull Confidential Information
#

VPATH		= ../:${MAKETOP}/bos/usr/sbin/trace

PROGRAMS	= trcupdate
MSGHDRS		= cmdtrace_msg.h
HFILES		= ${MSGHDRS} parse.h

OTHERS		= trcfmt.Z
ILIST		= trcupdate ${OTHERS}
trcupdate_IDIR	= /usr/bin/
trcfmt.Z_IDIR	= /etc/


trcupdate_OFILES = updt.o data.o gettmplt.o misc.o lex.o sym.o prutil.o  \
                   parse.o parseutil.o builtins.o list.o genexit.o exec.o \
		   eval.o nm.o prevent.o opts.o getevent.o


trcupdate_LIBS	= -lras

COMPRESS_FILES	= trcfmt

NOUPDATE_LIST = /etc/trcfmt.Z

.include <${RULES_MK}>
