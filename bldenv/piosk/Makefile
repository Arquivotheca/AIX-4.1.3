# @(#)74        1.2  src/bldenv/piosk/Makefile, cmdpiosk, bos412, GOLDA411a 1/10/94 15:28:18
#
#   COMPONENT_NAME: CMDPIOSK
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

OTHERS	=	s83_932 s90_932 e78_eucJP e83_eucJP \
		j78_fold7 j83_fold7 j90_fold7

ILIST		= ${OTHERS}
IDIR		= /usr/lib/lpd/pio/transJP/

VPATH		= ${MAKETOP}/bos/usr/lib/piosk/fmtrs/trans/

.include <${RULES_MK}>

compile_flags	= ${_CCFLAGS_} ${_LDFLAGS_}
s83_932_def	= -DSJIS83TOIBM932 ${compile_flags}
s90_932_def	= -DSJIS90TOIBM932 ${compile_flags}

e78_eucJP_def	= -DEUC78TOIBMEUCJP ${compile_flags}
e83_eucJP_def	= -DEUC83TOIBMEUCJP ${compile_flags}

j78_fold7_def	= -DJIS78TOFOLD7 ${compile_flags}
j83_fold7_def	= -DJIS83TOFOLD7 ${compile_flags}
j90_fold7_def	= -DJIS90TOFOLD7 ${compile_flags}

${OTHERS} : mktrans.c
	${RM} ${_RMFLAGS_} ${.TARGET}
	${CC} -o ${.TARGET} ${${.TARGET}_def} ${mktrans.c:P}
	${MD} ${_MDFLAGS_} .
