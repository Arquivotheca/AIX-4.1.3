# # @(#)39        1.2  src/bos/usr/ccs/bin/cxref/Makefile, cmdprog, bos411, 9428A410j 1/19/93 12:54:43
# COMPONENT_NAME: (CMDPROG) Programming Utilities
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1991
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH			= ../common

PROGRAMS		= cxref xpass
MSGHDRS           	= ctools_msg.h cxref_msg.h
CATFILES          	= ctools.cat   cxref.cat

ILIST			= xpass cxref
INCFLAGS		= -I../common
IDIR			= /usr/ccs/bin/
xpass_IDIR		= /usr/ccs/lib/

CFLAGS			= -DNEWCALL -DHOSTIEEE -DSDBDEBUG -DCXREF \
		          -DTWOPASS -DSINGLE_PRECISION  -D_COMPILER
HFILES			= ${MSGHDRS} 

xpass_OFILES		= cgram.o	code.o	comm1.o	local.o \
			  messages.o	optim.o	pftn.o	scan.o	\
			  tftn.o	trees.o	xdefs.o xpass.o
cxref_OFILES		= cxref.o

LIBS			= -lIN


.include <${RULES_MK}>
