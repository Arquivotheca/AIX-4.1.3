# @(#)47	1.8  src/bos/diag/da/ped/Makefile, daped, bos411, 9428A410j 3/17/94 16:16:42
#
#   COMPONENT_NAME: DAPED
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


VPATH		= ${MAKETOP}/bos/diag/tu/ped4:${MAKETOP}/bos/kernext/exp
PROGRAMS	= dped

MSGHDRS		= ped_msg.h
CATFILES 	= ped.cat
HFILES		= ${MSGHDRS}

CFLAGS		+= ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

LIBS		= -lasl -ldiag -lodm -lcfg -lc -ltu_ped4

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/ped4

ILIST		= ${PROGRAMS} ped.cat
IDIR		= /usr/lpp/diagnostics/da/
ped.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dped_IMPORTS	= -bI:rcm.exp

dped_OFILES	= dped.o

.include <${RULES_MK}>
