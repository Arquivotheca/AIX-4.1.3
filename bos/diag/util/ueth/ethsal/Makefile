# @(#)61	1.3  src/bos/diag/util/ueth/ethsal/Makefile, dsaueth, bos411, 9428A410j 2/22/94 19:27:17
#
#   COMPONENT_NAME: DSAUETH
#
#   FUNCTIONS: Makefile
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1989,1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/diag/util/ueth/eth

MSGHDRS		= uenet_msg.h
HFILES		= uenet_msg.h
CATFILES	= uenet.cat

PROGRAMS	= saluenet

LIBS		= -ldiag -lasl -lodm -lcfg -lc -lethsaltu

INCFLAGS	= -I${MAKETOP}/bos/diag/util/ueth/eth -I${MAKETOP}/bos/diag/tu/ethsal

ILIST		= ${PROGRAMS}
IDIR		= /usr/lpp/diagnostics/bin/

saluenet_OFILES	= saluenet.o

.include <${RULES_MK}>


