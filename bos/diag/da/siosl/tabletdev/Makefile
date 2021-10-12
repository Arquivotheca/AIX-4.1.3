# @(#)71	1.6  src/bos/diag/da/siosl/tabletdev/Makefile, databletsl, bos411, 9428A410j 2/21/94 08:39:51
#
#   COMPONENT_NAME: DATABLETSL
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994.
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


VPATH		= ${MAKETOP}/bos/diag/tu/tablet
PROGRAMS	= dtabletsal

CFLAGS		+= -DSALMON -DDIAGNOSTICS ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

MSGHDRS		= dtablet_msg.h


INCFLAGS	= -I${MAKETOP}/bos/diag/tu/siosl/tabletdev

LIBS		= -lasl -ldiag -lcur -lcurses -lodm -lcfg -lc -ltabletdevtu

ILIST		= ${PROGRAMS}
IDIR		= /usr/lpp/diagnostics/da/

dtabletsal_OFILES	= dtabletsal.o

.include <${RULES_MK}>
