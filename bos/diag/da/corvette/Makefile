# @(#)79        1.3  src/bos/diag/da/corvette/Makefile, dascsi, bos411, 9428A410j 2/21/94 08:31:52
#
#   COMPONENT_NAME: DASCSI
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


VPATH		= ${MAKETOP}/bos/diag/tu/corv:${MAKETOP}/bos/kernext/exp
		  
PROGRAMS	= dacorv

CFLAGS		+= -D CORVETTE -D DIAGNOSTICS ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}
MSGHDRS		= dacorv_msg.h
CATFILES 	= dacorv.cat
HFILES		= ${MSGHDRS}
INCFLAGS 	= -I${MAKETOP}/bos/diag/tu/corv

LIBS		= -lasl -ldiag -lcur -lcurses -lodm -lcfg -lc -lcorvtu

ILIST		= ${PROGRAMS} dacorv.cat
IDIR		= /usr/lpp/diagnostics/da/
dacorv.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dacorv_IMPORTS  = -bI:diagex.exp

dacorv_OFILES	= common.o debug.o dacorv.o

.include <${RULES_MK}>
