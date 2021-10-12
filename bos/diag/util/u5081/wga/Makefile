# @(#)22	1.9  src/bos/diag/util/u5081/wga/Makefile, dsau5081, bos411, 9428A410j 1/17/94 17:20:10
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
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


VPATH		= ${MAKETOP}/bos/kernext/exp:${MAKETOP}/bos/diag/tu/wga
PROGRAMS	= uwga

LIBS		= -lasl -ldiag -lodm -lcfg -lm -lc -lwgatu

IMPORTS		= -bI:rcm.exp

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/wga

ILIST		= ${PROGRAMS}
IDIR		= /usr/lpp/diagnostics/bin/

uwga_OFILES       = uwga.o 

.include <${RULES_MK}>
