# @(#)47        1.7  src/bos/kernext/ent/Makefile, sysxent, bos41J, 9522A_all 5/30/95 18:50:35
#
# COMPONENT_NAME: sysxent - High Performance Ethernet Device Driver
#
# FUNCTIONS: Makefile for Ethernet Device Driver
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

OTHERS          = en3com_errids.h                 \
		  root/devices.mca.8ef5.rte.err.S  \
		  root/devices.mca.8ef5.rte.trc.S

KERNEL_EXT	= entdd

IDIR		= /usr/lib/drivers/

ILIST           = entdd

LIBS		= ${KERNEXT_LIBS}

entdd_ENTRYPOINT	= en3com_config

${entdd_OFILES}: en3com_errids.h

entdd_OFILES = \
	en3com_open.o en3com_cfg.o en3com_init.o en3com_close.o \
	en3com_ctl.o en3com_util.o en3com_out.o en3com_intr.o \
	en3com_err.o 

en3com_errids.h: devices.mca.8ef5.rte.err.S
		${CP} ${devices.mca.8ef5.rte.err.S:P} .
		${ERRTMPLTBLD} -d devices.mca.8ef5.rte.err.S -o en3com_errids.h
		${RM} -f devices.mca.8ef5.rte.err.S

root/devices.mca.8ef5.rte.err.S: devices.mca.8ef5.rte.err.S
		-${MKDIR} root
		${CP} ${devices.mca.8ef5.rte.err.S:P} root/devices.mca.8ef5.rte.err.S

root/devices.mca.8ef5.rte.trc.S: devices.mca.8ef5.rte.trc.S
		-${MKDIR} root
		${CP} ${devices.mca.8ef5.rte.trc.S:P} root/devices.mca.8ef5.rte.trc.S

.include <${RULES_MK}>
