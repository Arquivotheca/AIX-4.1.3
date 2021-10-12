# @(#)66        1.1  src/bos/diag/util/u5081/unep/Makefile, dsaunep, bos411, 9439A411b 9/26/94 20:47:38
#
#   COMPONENT_NAME: DSAUNEP
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993,1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/diag/tu/nep:${MAKETOP}/bos/kernext/exp

PROGRAMS	= unep

LIBS		= -lasl -ldiag -lodm -lcfg -lc  -ltu_nep
LIBFLAGS	=	-L${MAKETOP}/bos/diag/tu/nep

INCFLAGS	=	-I${MAKETOP}/bos/usr/include  \
			 	-I${MAKETOP}/bos/diag/tu/nep  \
				-I${MAKETOP}/bos/usr/include/sys

IMPORTS = -bI:rcm.exp

ILIST		= ${PROGRAMS} unep.cat
IDIR		= /usr/lpp/diagnostics/bin/
unep.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/
unep_OFILES	= unep.o

OTHERS =
OTHERS += unep.cat
unep.cat: unep.cat.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
