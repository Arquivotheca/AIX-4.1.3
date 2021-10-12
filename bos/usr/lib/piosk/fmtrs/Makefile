# @(#)02	1.9  src/bos/usr/lib/piosk/fmtrs/Makefile, cmdpiosk, bos411, 9428A410j 8/24/93 05:06:36
#
#   COMPONENT_NAME: CMDPIOSK
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

SUBDIRS = trans

PIOS = ${MAKETOP}bos/usr/lib/pios
PIOS_KJI = ${MAKETOP}bos/usr/lib/piosk

INCFLAGS = -I${PIOS_KJI}/msg -I${PIOS_KJI}/libR5 -I${PIOS} -I${PIOS}/msg
CFLAGS = -DAIX32 -DX11R5BOS

piof557xd.o_CFLAGS = ${CFLAGS} -DIBM557x
piof558xd.o_CFLAGS = ${CFLAGS} -DIBM558x
piof557xs.o_CFLAGS = ${CFLAGS} -DDRIVER=5575 -DPIOTRANS
piof5587s.o_CFLAGS = ${CFLAGS} -DDRIVER=5587 -DPIOTRANS
piof4019e5587d.o_CFLAGS = ${CFLAGS} -DIBM5587G

piof557xd_OFILES = piof557xd.o
piof558xd_OFILES = piof558xd.o
piof557xs_OFILES = piof557xs.o piotrans.o
piof5587s_OFILES = piof5587s.o piotrans.o
piof4019e5587d_OFILES = piof4019e5587d.o
pioflips3_OFILES = pioflips.o

ENTRYPOINT = setup
IMPORTS = -bI:piof.imps
EXPORTS = -bE:${PIOS}/pioformat.imps

LIBS = -lc -liconv -lfontbos -lpiosk

PROGRAMS = pioflips3 piof4019e5587d \
  	piof557xd piof558xd \
  	piof557xs piof5587s

IDIR = /usr/lib/lpd/pio/fmtrs/
ILIST = ${PROGRAMS} ${OTHERS}

.include <${RULES_MK}>

piof557xd.c \
piof558xd.c : piof5xxxd.c
	${RM} -f ${.TARGET}
	${CP} -p ${.ALLSRC} ${.TARGET}

piof557xs.c \
piof5587s.c : piof5xxxs.c
	${RM} -f ${.TARGET} 
	${CP} -p ${.ALLSRC} ${.TARGET}

piotrans.c : ${PIOS}/fmtrs/piotrans.c
	${RM} -f ${.TARGET}
	${CP} -p ${.ALLSRC} ${.TARGET}

