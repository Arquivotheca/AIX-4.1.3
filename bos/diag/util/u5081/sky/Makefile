# @(#)56	1.4  src/bos/diag/util/u5081/sky/Makefile, dsau5081, bos411, 9428A410j 3/17/94 16:25:56
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


VPATH		= ${MAKETOP}/bos/kernext/exp:${MAKETOP}/bos/diag/tu/sky
PROGRAMS	= usky

LIBS		= -lasl -ldiag -lodm -lcfg -lm -lc -lskCtu

IMPORTS		= -bI:rcm.exp

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/sky

ILIST		= ${PROGRAMS}
IDIR		= /usr/lpp/diagnostics/bin/

usky_OFILES       = usky.o 

.include <${RULES_MK}>
