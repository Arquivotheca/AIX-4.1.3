# @(#)89        1.6  src/bos/usr/bin/msgs/Makefile, cmdcomm, bos411, 9428A410j 4/20/94 08:21:18
#
# COMPONENT_NAME: CMDCOMM
#
# FUNCTIONS:  Makefile for command "msgs".
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= msgs
MSGHDRS		= msgs_msg.h
CATFILES	= msgs.cat
HFILES		= ${MSGHDRS}

CFLAGS		+= ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURSESLIBFLAG}

ILIST		= msgs
IDIR		= /usr/bin/

LIBS		= -ls -lcurses

.include <${RULES_MK}>
