# @(#)26        1.6  src/bos/diag/da/kbd/Makefile, dakbd, bos411, 9428A410j 2/21/94 08:39:19
#
#   COMPONENT_NAME: DAKBD
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


VPATH		= ${MAKETOP}/bos/diag/tu/kbd
PROGRAMS	= dkbd

MSGHDRS		= dkbd_msg.h

CFLAGS		+= ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS} -DDIAGNOSTICS
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/kbd

LIBS		= -lasl -ldiag -lcur -lcurses -lodm -lcfg -lm -lc -lkbdtu

ILIST		= ${PROGRAMS}
IDIR		= /usr/lpp/diagnostics/da/

dkbd_OFILES	= dkbd.o

.include <${RULES_MK}>
