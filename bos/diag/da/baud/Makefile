# @(#)94	1.5  src/bos/diag/da/baud/Makefile, dabaud, bos410, 9428A410 3/4/94 10:39:50
#
#   COMPONENT_NAME: DABAUD 
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993,1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#  This VPATH doesn't work for getting to the /tu/baud directory????
#
VPATH		= ${MAKETOP}/bos/diag/tu/baud
VPATH		:= ${MAKETOP}/bos/kernext/exp
PROGRAMS	= dabaud

MSGHDRS		= dabaud_msg.h
CATFILES 	= dabaud.cat
HFILES		= ${MSGHDRS}

#INCFLAGS	= -I${MAKETOP}/bos/usr/include \
#                  -I${MAKETOP}../../../diag/tu/baud
INCFLAGS	= -I${MAKETOP}/bos/diag/tu/baud

LIBS		= -lasl -ldiag -lcur -lodm -lcfg -lc -ltu_baud

ILIST		= ${PROGRAMS} dabaud.cat
IDIR		= /usr/lpp/diagnostics/da/
dabaud.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dabaud_IMPORTS	= -bI:rcm.exp 

dabaud_OFILES	= dabaud.o 

NOUPDATE_LIST = /usr/lpp/diagnostics/da/dabaud \
		/usr/lpp/diagnostics/catalog/default/dabaud.cat

.include <${RULES_MK}>
