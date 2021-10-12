# @(#)30        1.6  src/bos/usr/games/lib/fortune/Makefile, cmdgames, bos41J, 9516A_all 4/17/95 15:26:26
#
# COMPONENT_NAME: CMDGAMES
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= fortune
OTHERS		= fortunes.dat

ILIST		= ${PROGRAMS} ${OTHERS}
IDIR		= /usr/games/lib/fortune/
fortune_IDIR	= /usr/games/

LIBS		= -lbsd

.include <${RULES_MK}>

fortunes.dat: scene
	${RM} -f ${.TARGET} fortunes
	${CAT} ${scene:P} > fortunes
	${ECHO} "%-" >> fortunes
	${ODE_TOOLS}/usr/games/lib/fortune/strfile -r fortunes ${.TARGET}
