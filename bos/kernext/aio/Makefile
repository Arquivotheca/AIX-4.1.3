# @(#)93        1.3  src/bos/kernext/aio/Makefile, sysxaio, bos412, 9445C412a 11/1/94 09:34:30
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#VPATH	=${MAKETOP}bos/kernel/exp:${MAKETOP}bos/kernel/exp/${TARGET_MACHINE}

.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT      = aio.ext aiopin

IDIR		=/usr/lib/drivers/
ILIST           = aio.ext aiopin
LIBS		= ${KERNEXT_LIBS}

aio.ext_ENTRYPOINT      = aio_init
aio.ext_EXPORTS         = -bE:aio.exp
aio.ext_IMPORTS         = -bI:aiopin.exp
aio.ext_OFILES =        aio.o aio_init.o aio_syscalls.o kproc.o \
			queue.o requests.o server.o suspend.o

aiopin_ENTRYPOINT   = arl_iodone
aiopin_EXPORTS      = -bE:aiopin.exp

.include <${RULES_MK}>
