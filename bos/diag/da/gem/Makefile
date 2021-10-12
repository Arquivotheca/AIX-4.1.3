# @(#)53        1.6  src/bos/diag/da/gem/Makefile, dagem, bos411, 9428A410j 3/22/94 15:36:59
#
#   COMPONENT_NAME: DAGEM
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


VPATH		= ${MAKETOP}/bos/diag/tu/gem:${MAKETOP}/bos/kernext/exp
PROGRAMS	= dgraph
INCFLAGS        = -I${MAKETOP}/bos/diag/tu/gem

MSGHDRS		= dgraph_msg.h
CATFILES 	= dgraph.cat
HFILES		= ${MSGHDRS}

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/gem

LIBS            = -lasl -ldiag -lodm -lcfg -lc  -ltu_gem


ILIST		= ${PROGRAMS} dgraph.cat 8ffddbif.00.01 8ffddc25.00.02 \
		  8ffddgcp.00.02
IDIR		= /usr/lpp/diagnostics/da/
8ffddbif.00.01_IDIR = /usr/lib/microcode/
8ffddc25.00.02_IDIR = /usr/lib/microcode/
8ffddgcp.00.02_IDIR = /usr/lib/microcode/
dgraph.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dgraph_IMPORTS  = -bI:rcm.exp


dgraph_OFILES	= ${TUS} dgraph.o

.include <${RULES_MK}>
