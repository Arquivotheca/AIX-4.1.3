# @(#)51        1.3  src/bos/usr/ccs/bin/lex/Makefile, cmdlang, bos411, 9428A410j 6/10/94 12:23:55
# COMPONENT_NAME: (CMDLANG) Language Utilities
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1991, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH			= ${MAKETOP}/bos/usr/ccs/lib/libc
PROGRAMS		= lex
MSGHDRS                 = lex_msg.h
CATFILES                = lex.cat

ILIST			= lex
IDIR			= /usr/ccs/bin/

CFLAGS			= 
HFILES			= ${MSGHDRS}  hash.h ldefs.h once.h

OFILES			= header.o  main.o   parser.o sub1.o  sub2.o
YFILES			= parser.y

LIBS			= -lld
GARBAGE			=

.include <${RULES_MK}>
