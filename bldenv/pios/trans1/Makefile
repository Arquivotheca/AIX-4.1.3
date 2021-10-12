# @(#)86        1.5  src/bldenv/pios/trans1/Makefile, cmdpios, bos412, GOLDA411a 4/1/94 14:52:43
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

VPATH  		= ${MAKETOP}bos/usr/lib/pios/trans1

OTHERS		= run850 runIBM-437 runIBM-850 runIBM-856 runIBM-1046 \
		  runISO8859-1 runISO8859-2 runISO8859-3 runISO8859-4 \
		  runISO8859-5 runISO8859-6 runISO8859-7 runISO8859-8 \
		  runISO8859-9 runUCS-2 runUTF-8

INCFLAGS	= -I${MAKETOP}bos/usr/lib/pios/trans1
ILIST		= ${OTHERS}
IDIR		= /usr/lib/lpd/pio/trans1/

.include <${RULES_MK}>

${OTHERS} : $${.TARGET:S|^run||}.c
	${RM} ${_RMFLAGS_} ${.TARGET}
	${CC} -o ${.TARGET} ${_CCFLAGS_} ${_LDFLAGS_} ${.ALLSRC}
	${MD} ${_MDFLAGS_} .
