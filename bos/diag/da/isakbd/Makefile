# @(#)13        1.1  src/bos/diag/da/isakbd/Makefile, daisakbd, bos41J, 9520A_all 5/6/95 14:29:49
#
#   COMPONENT_NAME: DAISAKBD
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/diag/tu/ppckbd
PROGRAMS	= disakbd

MSGHDRS		= disakbd_msg.h

CFLAGS		+= ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/ppckbd

LIBS		= -lasl -ldiag -lcur -lcurses -lodm -lcfg -lm -lc -lppckbd

ILIST		= ${PROGRAMS} 
IDIR		= /usr/lpp/diagnostics/da/

dkbd_OFILES	= disakbd.o

.include <${RULES_MK}>
