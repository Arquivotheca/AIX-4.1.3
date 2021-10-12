# @(#)21	1.10  src/bos/diag/da/wga/Makefile, dawga, bos411, 9428A410j 1/14/94 09:56:13
#
#   COMPONENT_NAME: DAWGA
#
#   FUNCTIONS: Makefile
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


VPATH		= ${MAKETOP}/bos/diag/tu/wga:${MAKETOP}/bos/kernext/exp
PROGRAMS	= dwga

MSGHDRS		= dwga_msg.h
CATFILES 	= dwga.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc -lm  -lwgatu

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/wga

ILIST		= ${PROGRAMS} dwga.cat
IDIR		= /usr/lpp/diagnostics/da/
dwga.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dwga_OFILES		= dwga.o 

dwga_IMPORTS	= -bI:rcm.exp

.include <${RULES_MK}>
