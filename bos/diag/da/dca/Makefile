# @(#)42	1.4  src/bos/diag/da/dca/Makefile, dadca, bos411, 9428A410j 1/7/94 10:19:43
#
#   COMPONENT_NAME: DADCA
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= ddca

MSGHDRS		= ddca_msg.h
CATFILES 	= ddca.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc -ldcatu

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/dca

ILIST		= ${PROGRAMS} ddca.cat
IDIR		= /usr/lpp/diagnostics/da/
ddca.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

ddca_OFILES	= ddca.o disp_men.o

.include <${RULES_MK}>
