# @(#)27        1.6  src/bos/usr/bin/capture/Makefile, cmdsh, bos411, 9428A410j 7/7/94 14:06:59
# COMPONENT_NAME:
#
# FUNCTIONS:
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

PROGRAMS	= capture
MSGHDRS		= capture_msg.h
CATFILES	= capture.cat
HFILES		= ${MSGHDRS}

CFLAGS		+= ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

ILIST		= capture
IDIR		= /usr/bin/

OFILES		= capture.o vtif.o vtparse.o

LIBS		= -lcur -lcurses -ls

.include <${RULES_MK}>
