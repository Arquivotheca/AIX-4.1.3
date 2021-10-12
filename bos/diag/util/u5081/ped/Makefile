# @(#)11	1.5  src/bos/diag/util/u5081/ped/Makefile, dsau5081, bos411, 9428A410j 3/17/94 16:35:45
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


VPATH		= ${MAKETOP}/bos/kernext/exp:${MAKETOP}/bos/diag/tu/ped4
PROGRAMS	= uped

LIBS		= -lasl -ldiag -lodm -lcfg -lc -ltu_ped4
IMPORTS		= -bI:rcm.exp

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/ped4

ILIST		= ${PROGRAMS}
IDIR		= /usr/lpp/diagnostics/bin/

uped_OFILES	= uped.o

.include <${RULES_MK}>
