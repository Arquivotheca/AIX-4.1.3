# @(#)35        1.7  src/bos/diag/tu/siosl/kbddev/Makefile, tu_siosl, bos411, 9428A410j 2/21/94 08:40:08
#
#   COMPONENT_NAME: TU_SIOSL
#
#   FUNCTIONS:
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1991,1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.


VPATH		= ${MAKETOP}/bos/diag/tu/kbd

LIBRARIES	= libkbddevtu.a

EXPLIB_TARGETS  = export_libkbddevtu.a
EXPDIR		= /usr/lib/

CFLAGS          += -DSALMON -DDIAGNOSTICS ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

LIBS            = -lcur -lcurses 

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/kbd
MSGHDRS		= dkbd_msg.h
HFILES		= ${MSGHDRS}

OFILES		= kbddev.o


.include <${RULES_MK}>
