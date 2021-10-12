# @(#)78        1.4  src/bos/usr/bin/sysline/Makefile, cmdstat, bos411, 9428A410j 3/2/94 14:57:35
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

PROGRAMS	= sysline
MSGHDRS		= sysline_msg.h
CATFILES	= sysline.cat
HFILES		= ${MSGHDRS}

ILIST		= sysline
IDIR		= /usr/bin/

CFLAGS		+= -D_BSD -DAIXV3AR ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURSESLIBFLAG}

LIBS		= -lcurses

.include <${RULES_MK}>
