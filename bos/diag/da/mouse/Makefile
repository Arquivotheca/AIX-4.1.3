# @(#)31	1.6  src/bos/diag/da/mouse/Makefile, damouse, bos411, 9428A410j 3/4/94 15:26:45
#
#   COMPONENT_NAME: DAMOUSE
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1993.
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= dmouse

MSGHDRS		= mouse_msg.h
CATFILES 	= mouse.cat
HFILES		= ${MSGHDRS}
 
CFLAGS		+= ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

INCFLAGS        = -I${MAKETOP}/bos/diag/tu/mouse

LIBS		= -ldiag -lasl -lcur -lcurses -lcfg -lm -lc -lodm -lmousetu

ILIST		= ${PROGRAMS} mouse.cat
IDIR		= /usr/lpp/diagnostics/da/
mouse.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dmouse_OFILES	= dmouse.o

.include <${RULES_MK}>
