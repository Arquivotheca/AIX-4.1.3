# @(#)59	1.4  src/bos/diag/da/swmono/Makefile, daswmono, bos411, 9428A410j 3/11/94 10:17:11
#
#   COMPONENT_NAME: DASWMONO
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


VPATH		= ${MAKETOP}/bos/diag/tu/swmono:${MAKETOP}/bos/kernext/exp
PROGRAMS	= dskym

MSGHDRS		= dskym_msg.h
CATFILES 	= dskym.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc -lskMtu

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/swmono

ILIST		= ${PROGRAMS} dskym.cat
IDIR		= /usr/lpp/diagnostics/da/
dskym.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dskym_IMPORTS	= -bI:rcm.exp

dskym_OFILES	= dskym.o

.include <${RULES_MK}>
