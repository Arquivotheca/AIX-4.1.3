# @(#)24 1.7 src/bos/kernext/tty/ttydbg/Makefile, sysxcommon, bos41J, 9522A_all 5/30/95 18:49:43
#
# COMPONENT_NAME: (sysxtty) Makefile to generate ttydbg extension
#
# FUNCTIONS:
#
# ORIGINS: 83
#
# LEVEL 1, 5 Years Bull Confidential Information
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

VPATH              +=:${TARGET_MACHINE}:${MAKETOP}/bos/kernext/pty:${MAKETOP}/bos/kernext/pse:${MAKETOP}/bos/kernext/tty:${MAKETOP}/bos/kernext/tty/${TARGET_MACHINE}:
VPATH              +=:${MAKETOP}/bos/kernext/isa_async:${MAKETOP}/bos/kernext/mca_async:${MAKETOP}/bos/kernext/isa_async/${TARGET_MACHINE}:

OTHERS		   = cxma_errids.h

KERNEL_EXT         = ttydbg

INCFLAGS           = -I${TARGET_MACHINE}\
                     -I${MAKETOP}bos/kernext/tty\
                     -I${MAKETOP}bos/kernext/isa_async \
                     -I${MAKETOP}bos/kernext/isa_async/${TARGET_MACHINE}\
                     -I${MAKETOP}bos/kernext/tty/${TARGET_MACHINE}

IDIR    	   = /usr/lib/drivers/
ILIST    	   = ${KERNEL_EXT}

IMPORTS            = -bI:pse.exp sockets.exp -bI:pse_int.exp -bI:dbtty.exp
LIBS               = ${KERNEXT_LIBS}

${ttydbg_OFILES}: cxma_errids.h

ttydbg_OFILES      =	\
	ttydbg_start.o	\
	ttydbg_reg.o	\
	ttydbg_end.o	\
	ttydbg_si.o	\
	ttydbg_dbg.o	\
	str_tty_db.o	\
	ldtty_db.o	\
	srs_db.o	\
	slion_db.o	\
	scxma_print.o	\
	spty_db.o	\
	snls_db.o	\
	sptr_db.o
str_tty_db.o_CFLAGS  = ${CFLAGS} -DIN_TTYDBG
ldtty_db.o_CFLAGS    = ${CFLAGS} -DIN_TTYDBG
srs_db.o_CFLAGS      = ${CFLAGS} -DIN_TTYDBG
slion_db.o_CFLAGS    = ${CFLAGS} -DIN_TTYDBG
scxma_print.o_CFLAGS = ${CFLAGS} -DIN_TTYDBG -I ..
spty_db.o_CFLAGS     = ${CFLAGS} -DIN_TTYDBG
snls_db.o_CFLAGS     = ${CFLAGS} -DIN_TTYDBG
sptr_db.o_CFLAGS     = ${CFLAGS} -DIN_TTYDBG
ttydbg_CARGS         = -DIN_TTYDBG
ttydbg_ENTRYPOINT    = tty_db_config
ttydbg_EXPORTS       = -bE:ttydbg.exp
ttydbg_LDFLAGS_KERNEXT = -bnoobjreorder ${LDFLAGS_KERNEXT}

.include <${RULES_MK}>

cxma_errids.h:  devices.mca.ffe1.rte.err.S
		${RM} -f ${.TARGET}
		${CP} ${devices.mca.ffe1.rte.err.S:P} .
		${ERRTMPLTBLD} -d devices.mca.ffe1.rte.err.S -o ${.TARGET}
		${RM} -f devices.mca.ffe1.rte.err.S

