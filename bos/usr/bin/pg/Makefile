# @(#)06        1.4  src/bos/usr/bin/pg/Makefile, cmdscan, bos411, 9428A410j 2/21/94 08:41:12
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

PROGRAMS	= pg
MSGHDRS		= pg_msg.h
CATFILES	= pg.cat
HFILES		= ${MSGHDRS}

CFLAGS		+= ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURSESLIBFLAG}

ILIST		= pg
IDIR		= /usr/bin/

LIBS		= -lcurses

.include <${RULES_MK}>
