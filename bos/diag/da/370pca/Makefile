# @(#)85	1.3  src/bos/diag/da/370pca/Makefile, da370pca, bos411, 9428A410j 1/7/94 09:50:31
#
#   COMPONENT_NAME: da370pca
#
#   FUNCTIONS: 370pca makefile 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


VPATH           = ${MAKETOP}/bos/diag/tu/370pca
PROGRAMS	= d370pc
INCFLAGS        = -I${MAKETOP}/bos/diag/tu/370pca
CFLAGS          = -DAIX -DDIAGS

MSGHDRS		= d370pc_msg.h
CATFILES 	= d370pc.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc -l370pcatu

ILIST		= ${PROGRAMS} d370pc.cat
IDIR            = /usr/lpp/diagnostics/da/
d370pc.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

d370pc_OFILES	= d370pc.o d370pc_tu.o d370pc_asl.o

.include <${RULES_MK}>
