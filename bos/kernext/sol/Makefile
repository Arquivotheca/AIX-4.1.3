# @(#)28        1.7  src/bos/kernext/sol/Makefile, sysxsol, bos41J, 9522A_all 5/30/95 18:50:38
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

.include <${MAKETOP}/bos/kernext/Kernext.mk>

OTHERS          = sol_errids.h  \
		  root/devices.sys.slc.rte.err.S  \
		  root/devices.sys.slc.rte.trc.S

KERNEL_EXT	= soldd solddpin
IDIR		= /usr/lib/drivers/

ILIST           = soldd solddpin

LIBS		= ${KERNEXT_LIBS}

soldd_ENTRYPOINT	= sol_config
soldd_IMPORTS		= -bI:solddpin.exp
soldd_EXPORTS		= -bE:soldd.exp

solddpin_IMPORTS	= -bI:soldd.exp
solddpin_EXPORTS	= -bE:solddpin.exp

${soldd_OFILES}: sol_errids.h

soldd_OFILES =	\
	sol_ioctl.o sol_close.o	sol_config.o sol_mpx.o	\
	sol_open.o sol_read.o sol_write.o

solddpin_OFILES =	\
	imcsasm.o sol_rcv.o sol_xmit.o sol_notify.o	\
	sol_misc.o solhqm.o solintr.o solproc.o	\
	soltxrx.o solutil.o

imcsasm.o_INCFLAGS	=-I${MAKETOP}/bos/kernel/ml/${TARGET_MACHINE}

sol_errids.h: devices.sys.slc.rte.err.S
		${CP} ${devices.sys.slc.rte.err.S:P} .
		${ERRTMPLTBLD} -d devices.sys.slc.rte.err.S -o sol_errids.h
		${RM} -f devices.sys.slc.rte.err.S

root/devices.sys.slc.rte.err.S: devices.sys.slc.rte.err.S
		-${MKDIR} root
		${CP} ${devices.sys.slc.rte.err.S:P} root/devices.sys.slc.rte.err.S

root/devices.sys.slc.rte.trc.S: devices.sys.slc.rte.trc.S
		-${MKDIR} root
		${CP} ${devices.sys.slc.rte.trc.S:P} root/devices.sys.slc.rte.trc.S

STRIP_FLAG ?= -s

LDFLAGS_KERNEXT += ${STRIP_FLAG}

.include <${RULES_MK}>
