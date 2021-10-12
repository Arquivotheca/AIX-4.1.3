# @(#)36	1.4  src/bos/usr/lib/piostw/fmtrsTW/Makefile, cmdpiostw, bos411, 9428A410j 8/24/93 05:04:05
#
#   COMPONENT_NAME: CMDPIOSTW
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

PIOS 	= ${MAKETOP}bos/usr/lib/pios
PIOS_KJI = ${MAKETOP}bos/usr/lib/piosk

INCFLAGS = -I${PIOS_KJI}/fmtrs -I${PIOS_KJI}/msg -I${PIOS_KJI}/libR5 -I${PIOS} -I${PIOS}/msg
CFLAGS = -DAIX32 -DX11R5BOS

piof557xdcht.o_CFLAGS = ${CFLAGS} -DIBM557x
piof558xdcht.o_CFLAGS = ${CFLAGS} -DIBM558x

piof557xdcht_OFILES = piof557xdcht.o
piof558xdcht_OFILES = piof558xdcht.o

ENTRYPOINT = setup
IMPORTS = -bI:${PIOS_KJI}/fmtrs/piof.imps
EXPORTS = -bE:${PIOS}/pioformat.imps

LIBFLAGS = -L${PIOS_KJI}/libR5
LIBS = -lc -liconv -lfontbos -lpiosk

PROGRAMS = piof557xdcht piof558xdcht

IDIR = /usr/lib/lpd/pio/fmtrs/
ILIST = ${PROGRAMS} ${OTHERS}

.include <${RULES_MK}>

piof557xdcht.c \
piof558xdcht.c : ${PIOS_KJI}/fmtrs/piof5xxxd.c
	${RM} -f ${.TARGET}
	${CP} -p ${.ALLSRC} ${.TARGET}

