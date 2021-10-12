# @(#)33 1.2 src/bos/kernext/mca_async/Makefile, sysxs128, bos41J 3/22/95 16:52:34
#
# COMPONENT_NAME: (sysxtty) Makefile to generate all needed modules
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1, 5 Years Bull Confidential Information
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

OTHERS		= cxma_errids.h \
		root/devices.mca.ffe1.rte.err.S \
		root/devices.mca.ffe1.rte.trc.S

VPATH              +=${TARGET_MACHINE}:${MAKETOP}bos/kernext/isa_async:${MAKETOP}bos/kernext/isa_async/${TARGET_MACHINE}:${MAKETOP}bos/kernext/tty:${MAKETOP}bos/kernext/tty/${TARGET_MACHINE}:
VPATH              +=${MAKETOP}/bos/kernext/tty/ttydbg

#SUBDIRS = 

KERNEL_EXT         = cxmadd

INCFLAGS    = -I${TARGET_MACHINE}\
              -I${MAKETOP}bos/kernext/tty\
              -I${MAKETOP}bos/kernext/isa_async \
	      -I${MAKETOP}bos/kernext/isa_async/${TARGET_MACHINE}\
              -I${MAKETOP}bos/kernext/tty/${TARGET_MACHINE}\

IDIR    	       = /usr/lib/drivers/
ILIST    	       = ${KERNEL_EXT}

IMPORTS            = -bI:pse.exp sockets.exp -bI:pse_int.exp -bI:dbtty.exp
LIBS               = ${KERNEXT_LIBS}

.include <${TARGET_MACHINE}/machdep.mk>

cxma_errids.h:	devices.mca.ffe1.rte.err.S
		${CP} ${devices.mca.ffe1.rte.err.S:P} .
		${ERRTMPLTBLD} -d devices.mca.ffe1.rte.err.S -o cxma_errids.h
		${RM} -f devices.mca.ffe1.rte.err.S

root/devices.mca.ffe1.rte.err.S:	devices.mca.ffe1.rte.err.S
		-${MKDIR} root
		${CP} ${devices.mca.ffe1.rte.err.S:P} root/devices.mca.ffe1.rte.err.S

root/devices.mca.ffe1.rte.trc.S:	devices.mca.ffe1.rte.trc.S
		-${MKDIR} root
		${CP} ${devices.mca.ffe1.rte.trc.S:P} root/devices.mca.ffe1.rte.trc.S

.include <${RULES_MK}>
