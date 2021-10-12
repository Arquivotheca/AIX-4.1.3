# @(#)30	1.6  src/bos/diag/da/siosl/kbddev/Makefile, dakbdsl, bos411, 9428A410j 2/21/94 08:39:42
#
#   COMPONENT_NAME: DAKBDSL
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


VPATH		= ${MAKETOP}/bos/diag/tu/kbd:${MAKETOP}/bos/diag/tu/siosl/kbda/slih:${MAKETOP}/bos/kernext/exp
PROGRAMS	= dkbdsal

CFLAGS          += -DSALMON -DDIAGNOSTICS ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

MSGHDRS		= dkbd_msg.h

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/siosl/kbddev

LIBS		= -lasl -ldiag -lcur -lcurses -lodm -lcfg -lc -lkbddevtu

ILIST		= ${PROGRAMS}
IDIR		= /usr/lpp/diagnostics/da/

dkbdsal_OFILES	= dkbdsal.o
dkbdsal_IMPORTS	= -bI:diagex.exp -bI:kbd_slih.exp

.include <${RULES_MK}>
