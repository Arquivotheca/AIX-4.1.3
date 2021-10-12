# @(#)90        1.4  src/bos/usr/bin/clear/Makefile, cmdtty, bos411, 9428A410j 2/21/94 08:40:40
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

PROGRAMS	= clear
MSGHDRS		=
CATFILES	=
HFILES		= ${MSGHDRS}

CFLAGS		+= ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURSESLIBFLAG}

ILIST		= clear
IDIR		= /usr/bin/

LIBS		= -lcurses 

.include <${RULES_MK}>
