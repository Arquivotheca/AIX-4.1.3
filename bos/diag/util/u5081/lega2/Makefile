# @(#)14	1.4  src/bos/diag/util/u5081/lega2/Makefile, dsau5081, bos411, 9428A410j 3/17/94 16:55:42
#
#   COMPONENT_NAME: DSAU5081
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


VPATH		= ${MAKETOP}/bos/kernext/exp:${MAKETOP}/bos/diag/tu/lega
PROGRAMS	= ulega2

LIBS		= -lasl -ldiag -lodm -lcfg -lc -ltu_lega
IMPORTS		= -bI:rcm.exp

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/lega

ILIST		= ${PROGRAMS}
IDIR		= /usr/lpp/diagnostics/bin/

ulega2_OFILES	= ulega2.o

.include <${RULES_MK}>
