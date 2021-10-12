# @(#)83  1.12 src/bos/kernext/fddi/Makefile, sysxfddi, bos41J, 9522A_all  5/30/95  18:50:58
#
#   COMPONENT_NAME: FDDI_DD
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

EXPDIR		= /kernel/include/sys/

OTHERS          = fddi_errids.h  \
		  root/devices.mca.8ef4.rte.err.S  \
		  root/devices.mca.8ef4.rte.trc.S

KERNEL_EXT 		= fddidd

IDIR 			= /usr/lib/drivers/

ILIST                   = fddidd

fddidd_ENTRYPOINT 	= fddi_config

LIBS			= ${KERNEXT_LIBS}

${fddidd_OFILES}: fddi_errids.h

fddidd_OFILES = \
	fddi_config.o	\
	fddi_open.o	\
	fddi_close.o	\
	fddi_tx.o	\
	fddi_rx.o	\
	fddi_ctl.o	\
	fddi_intr.o	\
	fddi_cmd.o	\
	fddi_limbo.o	\
	fddi_prim.o	\
	fddi_dump.o

fddi_errids.h: devices.mca.8ef4.rte.err.S
		${CP} ${devices.mca.8ef4.rte.err.S:P} .
		${ERRTMPLTBLD} -d devices.mca.8ef4.rte.err.S -o fddi_errids.h
		${RM} -f devices.mca.8ef4.rte.err.S

root/devices.mca.8ef4.rte.err.S: devices.mca.8ef4.rte.err.S
		-${MKDIR} root
		${CP} ${devices.mca.8ef4.rte.err.S:P} root/devices.mca.8ef4.rte.err.S

root/devices.mca.8ef4.rte.trc.S: devices.mca.8ef4.rte.trc.S
		-${MKDIR} root
		${CP} ${devices.mca.8ef4.rte.trc.S:P} root/devices.mca.8ef4.rte.trc.S

.include <${RULES_MK}>

