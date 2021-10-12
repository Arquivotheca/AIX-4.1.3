# @(#)69        1.7  src/bos/diag/da/tablet/Makefile, datablet, bos411, 9428A410j 3/3/94 17:54:49
#
#   COMPONENT_NAME: DATABLET
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1994.
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


VPATH		= ${MAKETOP}/bos/diag/tu/tablet
PROGRAMS	= dtablet

MSGHDRS		= dtablet_msg.h

CFLAGS		+= ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS} -DDIAGNOSTICS

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/tablet

LIBS		= -lasl -ldiag ${_AIX3CURLIB} ${_AIX3CURSESLIB} -lodm -lcfg -lm -lc -ltabtu

ILIST		= ${PROGRAMS} 
IDIR		= /usr/lpp/diagnostics/da/

dtablet_OFILES	= dtablet.o

.include <${RULES_MK}>

