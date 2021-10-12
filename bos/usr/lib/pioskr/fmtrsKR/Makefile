# @(#)37	1.4  src/bos/usr/lib/pioskr/fmtrsKR/Makefile, cmdpioskr, bos411, 9428A410j 8/24/93 05:04:11
#
#   COMPONENT_NAME: CMDPIOSKR
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

PIOS 	 = ${MAKETOP}bos/usr/lib/pios
PIOS_KJI = ${MAKETOP}bos/usr/lib/piosk

INCFLAGS = -I${PIOS_KJI}/fmtrs -I${PIOS_KJI}/msg -I${PIOS_KJI}/libR5 -I${PIOS} -I${PIOS}/msg
CFLAGS   = -DAIX32 -DX11R5BOS

piof557xdk.o_CFLAGS = ${CFLAGS} -DIBM557x
piof4019e5587dk.o_CFLAGS = ${CFLAGS} -DIBM5587G

piof557xdk_OFILES = piof557xdk.o
piof4019e5587dk_OFILES = piof4019e5587dk.o

ENTRYPOINT = setup
IMPORTS = -bI:${PIOS_KJI}/fmtrs/piof.imps
EXPORTS = -bE:${PIOS}/pioformat.imps

LIBFLAGS = -L${PIOS_KJI}/libR5
LIBS = -lc -liconv -lfontbos -lpiosk

PROGRAMS = \
  	piof557xdk piof4019e5587dk

IDIR = /usr/lib/lpd/pio/fmtrs/
ILIST = ${PROGRAMS} ${OTHERS}

.include <${RULES_MK}>

piof557xdk.c : ${PIOS_KJI}/fmtrs/piof5xxxd.c
	${RM} -f ${.TARGET}
	${CP} -p ${.ALLSRC} ${.TARGET}

piof4019e5587dk.c : ${PIOS_KJI}/fmtrs/piof4019e5587d.c
	${RM} -f ${.TARGET}
	${CP} -p ${.ALLSRC} ${.TARGET}

