# @(#)00        1.1  src/bos/usr/bin/xsend/Makefile, cmdmailx, bos411, 9428A410j 8/5/92 16:55:29
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

PROGRAMS	= xsend xget enroll
MSGHDRS		= xsend_msg.h
CATFILES	= xsend.cat
HFILES		= ${MSGHDRS}

ILIST		= xsend xget enroll
IDIR		= /usr/bin/

CFLAGS		= -D_BSD -DNOCATMACRO

xsend_OFILES	= xsend.o lib.o
xget_OFILES	= xget.o lib.o
enroll_OFILES	= enroll.o lib.o

LIBS		= -lbsd

.include <${RULES_MK}>
