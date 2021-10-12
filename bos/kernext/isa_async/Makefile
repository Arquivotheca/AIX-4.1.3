# @(#)36 1.2 src/bos/kernext/isa_async/Makefile, sysxs128, bos41J 3/22/95 16:51:00
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
		root/devices.isa.cxia.rte.err.S \
		root/devices.isa.cxia.rte.trc.S

VPATH              +=.:${TARGET_MACHINE}:${MAKETOP}bos/kernext/tty:${MAKETOP}bos/kernext/tty/${TARGET_MACHINE}:
VPATH              +=${MAKETOP}/bos/kernext/tty/ttydbg

#SUBDIRS = 

KERNEL_EXT         = cxiadd

INCFLAGS    = -I${TARGET_MACHINE}\
              -I${MAKETOP}bos/kernext/tty\
              -I${MAKETOP}bos/kernext/tty/${TARGET_MACHINE}\

IDIR    	       = /usr/lib/drivers/
ILIST    	       = ${KERNEL_EXT}

IMPORTS            = -bI:pse.exp sockets.exp -bI:pse_int.exp -bI:dbtty.exp
LIBS               = ${KERNEXT_LIBS}

.include <${TARGET_MACHINE}/machdep.mk>

cxma_errids.h:	devices.isa.cxia.rte.err.S
		${CP} ${devices.isa.cxia.rte.err.S:P} .
		${ERRTMPLTBLD} -d devices.isa.cxia.rte.err.S -o cxma_errids.h
		${RM} -f devices.isa.cxia.rte.err.S

root/devices.isa.cxia.rte.err.S:	devices.isa.cxia.rte.err.S
		-${MKDIR} root
		${CP} ${devices.isa.cxia.rte.err.S:P} root/devices.isa.cxia.rte.err.S

root/devices.isa.cxia.rte.trc.S:	devices.isa.cxia.rte.trc.S
		-${MKDIR} root
		${CP} ${devices.isa.cxia.rte.trc.S:P} root/devices.isa.cxia.rte.trc.S

.include <${RULES_MK}>
