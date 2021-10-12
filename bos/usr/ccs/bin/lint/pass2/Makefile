# @(#)53        1.1  src/bos/usr/ccs/bin/lint/pass2/Makefile, cmdprog, bos411, 9428A410j 8/5/92 16:28:10
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

VPATH		= ../../common

PROGRAMS	= lint2
MSGHDRS		= lint_msg.h ctools_msg.h
CATFILES	= lint.cat ctools.cat
HFILES		= ${MSGHDRS}

ILIST		= lint2
IDIR		= /usr/ccs/lib/

INCFLAGS	= -I../../common
CFLAGS		= -DHOSTIEEE -DLINTP2

OFILES		= \
	hasher.o lint2.o main.o output.o reader.o storage.o tftn.o \
	xdefs.o

.include <${RULES_MK}>
