# @(#)99	1.7  src/bos/diag/da/lega/Makefile, dalega, bos411, 9428A410j 3/17/94 16:14:57
#
#   COMPONENT_NAME: DALEGA
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


VPATH		= ${MAKETOP}/bos/diag/tu/lega:${MAKETOP}/bos/kernext/exp
PROGRAMS	= dlega

MSGHDRS		= lega2_msg.h
CATFILES 	= lega2.cat
HFILES		= ${MSGHDRS}

CFLAGS		+= ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

LIBS		= -lasl -ldiag -lodm -lcfg -lc -ltu_lega

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/lega

ILIST		= ${PROGRAMS} lega2.cat
IDIR		= /usr/lpp/diagnostics/da/
lega2.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dlega_IMPORTS	= -bI:rcm.exp

dlega_OFILES	= dlega.o

.include <${RULES_MK}>
