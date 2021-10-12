# @(#)52        1.3  src/bos/usr/bin/trcrpt/trcrpt/Makefile, cmdtrace, bos411, 9428A410j 9/30/93 18:46:49
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

PROGRAMS	= trcrpt handfmt
MSGHDRS		= cmdtrace_msg.h
HFILES		= ${MSGHDRS} parse.h

ILIST		=  trcrpt handfmt
trcrpt_IDIR	= /usr/bin/
handfmt_IDIR 	= /usr/lpp/bos.sysmgt/

trcrpt_OFILES	= data.o eval.o nm.o misc.o prutil.o parseutil.o \
		  gettmplt.o builtins.o prevent.o exec.o getevent.o \
		  main.o  sym.o lex.o opts.o parse.o rpt_blk.o \
		  genexit.o list.o

handfmt_OFILES  = handfmt.o

trcrpt_LIBS	= -lras
handfmt_LIBS	= -lcfg

CFLAGS		= -DTRCRPT

.include <${RULES_MK}>
