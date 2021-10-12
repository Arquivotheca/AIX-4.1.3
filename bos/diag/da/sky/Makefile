# @(#)52	1.7  src/bos/diag/da/sky/Makefile, dasky, bos411, 9428A410j 3/17/94 16:18:39
#
#   COMPONENT_NAME: DASKY
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


VPATH		= ${MAKETOP}/bos/diag/tu/sky:${MAKETOP}/bos/kernext/exp
PROGRAMS	= dsky

MSGHDRS		= dsky_msg.h
CATFILES 	= dsky.cat
HFILES		= ${MSGHDRS}

CFLAGS		+= ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

LIBS		= -lasl -ldiag -lodm -lcfg -lc -lskCtu

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/sky

ILIST		= ${PROGRAMS} dsky.cat
IDIR		= /usr/lpp/diagnostics/da/
dsky.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dsky_IMPORTS	= -bI:rcm.exp

dsky_OFILES	= dsky.o

.include <${RULES_MK}>
