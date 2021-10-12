# @(#)83        1.1  src/bos/usr/bin/su/Makefile, cmdsauth, bos411, 9428A410j 9/1/92 09:39:57
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

VPATH		= ${MAKETOP}/bos/usr/sbin/tsm

PROGRAMS	= su
OFILES		= su.o tsm_su_util.o
MSGHDRS		= su_msg.h tsm_msg.h
CATFILES	= su.cat
HFILES		= ${MSGHDRS}

INCFLAGS	=-I${MAKETOP}/bos/usr/sbin/tsm

ILIST		= su
IDIR		= /usr/bin/

LIBS		= -ls

.include <${RULES_MK}>
