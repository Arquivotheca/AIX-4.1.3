# @(#)96	1.2  src/bos/usr/lib/piostw/fmtrs/Makefile, cmdpiostw, bos411, 9428A410j 3/6/93 14:59:55
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
PIOS = ${MAKETOP}bos/usr/lib/pios
PIOS_KJI = ${MAKETOP}bos/usr/lib/piosk
PIOS_TW = ${MAKETOP}bos/usr/lib/piostw

INCFLAGS = -I${PIOS_KJI}/msg -I${PIOS_KJI}/libR5 -I${PIOS} -I${PIOS}/msg
CFLAGS = -DAIX32 -D_ICONV_INTERNAL -DX11R5BOS

piof5575dcht.o_CFLAGS = ${CFLAGS} -DIBM5575 -DDRIVER=5575
piof5577dcht.o_CFLAGS = ${CFLAGS} -DIBM5577 -DDRIVER=5577

piof5575dcht_OFILES = piof5575dcht.o
piof5577dcht_OFILES = piof5577dcht.o

ENTRYPOINT = setup
IMPORTS = -bI:piof.imps
EXPORTS = -bE:${PIOS}/pioformat.imps

LIBFLAGS = -L${PIOS_KJI}/libR5
LIBS = -lc -liconv -lfontbos

PROGRAMS = piof5575dcht piof5577dcht


IDIR = /usr/lib/lpd/pio/fmtrs/
ILIST = ${PROGRAMS}

.include <${RULES_MK}>

piof5575dcht.o \
piof5577dcht.o :	${piof5xxxdcht.c:P}
	${RM} -f ${.TARGET:.o=.c}; ${CP} -p ${.ALLSRC} ${.TARGET:.o=.c}
	${_CC_} -c ${_CCFLAGS_} ${.TARGET:.o=.c}
