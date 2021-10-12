# @(#)64	1.4  src/bos/diag/util/u5081/swmono/Makefile, dsau5081, bos411, 9428A410j 3/18/94 14:26:52
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
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


VPATH		= ${MAKETOP}/bos/kernext/exp:${MAKETOP}/bos/diag/tu/swmono
PROGRAMS	= uskym

LIBS		= -lasl -ldiag -lodm -lcfg -lm -lc -lskMtu

IMPORTS		= -bI:rcm.exp

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/swmono

ILIST		= ${PROGRAMS}
IDIR		= /usr/lpp/diagnostics/bin/

uskym_OFILES       = uskym.o 

.include <${RULES_MK}>
