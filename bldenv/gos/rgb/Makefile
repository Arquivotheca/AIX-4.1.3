# @(#)35        1.2  src/bldenv/gos/rgb/Makefile, x11, bos412, GOLDA411a 2/19/94 19:59:18
#
# COMPONENT_NAME: x11
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1987, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#

VPATH		= ${MAKETOP}/gos/2d/XTOP/rgb

PROGRAMS	= rgb

ILIST		= ${PROGRAMS}
IDIR		= /usr/bin/

LIBS		= -ldbm

CFLAGS		= -DAIXV3 -D_BSD
INCFLAGS	= -I${MAKETOP}/gos/2d/XTOP/server/include

.include <${RULES_MK}>
