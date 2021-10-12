# @(#)25	1.8  src/bos/kernext/tok/Makefile, sysxtok, bos41J, 9522A_all 5/30/95 18:50:44
#
#   COMPONENT_NAME: SYSXTOK
#
#   FUNCTIONS: none
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

.include <${MAKETOP}bos/kernext/Kernext.mk>

OTHERS          = tr_mon_errids.h  \
		  root/devices.mca.8fc8.rte.err.S  \
		  root/devices.mca.8fc8.rte.trc.S

KERNEL_EXT	= tokdd

IDIR		= /usr/lib/drivers/

ILIST           = tokdd

LIBS		= ${KERNEXT_LIBS}

tokdd_ENTRYPOINT	= tokconfig

${tokdd_OFILES}: tr_mon_errids.h

tokdd_OFILES	= trmon_close.o trmon_dd.o trmon_dump.o trmon_init.o \
	trmon_intr.o trmon_ioctl.o trmon_limbo.o trmon_open.o trmon_prim.o \
	trmon_recv.o trmon_rw.o trmon_ucode.o trmon_util.o trmon_xmit.o

tr_mon_errids.h: devices.mca.8fc8.rte.err.S
		${CP} ${devices.mca.8fc8.rte.err.S:P} .
		${ERRTMPLTBLD} -d devices.mca.8fc8.rte.err.S -o tr_mon_errids.h
		${RM} -f devices.mca.8fc8.rte.err.S

root/devices.mca.8fc8.rte.err.S: devices.mca.8fc8.rte.err.S
		-${MKDIR} root
		${CP} ${devices.mca.8fc8.rte.err.S:P} root/devices.mca.8fc8.rte.err.S

root/devices.mca.8fc8.rte.trc.S: devices.mca.8fc8.rte.trc.S
		-${MKDIR} root
		${CP} ${devices.mca.8fc8.rte.trc.S:P} root/devices.mca.8fc8.rte.trc.S

.include <${RULES_MK}>
