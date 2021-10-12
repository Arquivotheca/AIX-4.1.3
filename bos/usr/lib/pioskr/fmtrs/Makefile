# @(#)92	1.1  src/bos/usr/lib/pioskr/fmtrs/Makefile, cmdpioskr, bos411, 9428A410j 1/24/93 11:43:14
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
PIOS = ${MAKETOP}bos/usr/lib/pios
PIOS_KJI = ${MAKETOP}bos/usr/lib/piosk
PIOS_KR = ${MAKETOP}bos/usr/lib/pioskr

INCFLAGS = -I${PIOS_KR}/msg -I${PIOS_KJI}/libR5 -I${PIOS} -I${PIOS}/msg
CFLAGS = -DAIX32 -D_ICONV_INTERNAL -DX11R5BOS

piof5575k.o_CFLAGS = ${CFLAGS} -DIBM5575
piof5577k.o_CFLAGS = ${CFLAGS} -DIBM5577
piof4019e5587dk.o_CFLAGS = ${CFLAGS} -DIBM5587G

piof5575k_OFILES = piof5575k.o
piof5577k_OFILES = piof5577k.o
piof4019e5587dk_OFILES = piof4019e5587dk.o

ENTRYPOINT = setup
IMPORTS = -bI:piof.imps
EXPORTS = -bE:${PIOS}/pioformat.imps

LIBFLAGS = -L${PIOS_KJI}/libR5
LIBS = -lc -liconv -lfontbos

PROGRAMS = piof4019e5587dk piof5575k piof5577k


IDIR = /usr/lib/lpd/pio/fmtrs/
ILIST = ${PROGRAMS}

.include <${RULES_MK}>

