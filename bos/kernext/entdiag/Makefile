# @(#)04        1.7  src/bos/kernext/entdiag/Makefile, diagddent, bos41J, 9522A_all 5/30/95 18:50:49
# COMPONENT_NAME: diagddent
#
# FUNCTIONS: diagnostic device driver for the High Performance Ethernet
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

OTHERS          = ent_comio_errids.h  \
		  root/devices.mca.8ef5.diag.err.S \
		  root/devices.mca.8ef5.diag.trc.S

KERNEL_EXT	= entdiag
IDIR		= /usr/lib/drivers/

ILIST           = entdiag

entdiag_ENTRYPOINT	= cioconfig

LIBS			= ${KERNEXT_LIBS}

${entdiag_OFILES}: ent_comio_errids.h

entdiag_OFILES = \
	entds.o	entrecv.o ciodd.o \
	entxmit.o entrw.o \
	testset.o

testset.o_INCFLAGS	= -I${MAKETOP}bos/kernel/ml/${TARGET_MACHINE}

ent_comio_errids.h: devices.mca.8ef5.diag.err.S
		${CP} ${devices.mca.8ef5.diag.err.S:P} .
		${ERRTMPLTBLD} -d devices.mca.8ef5.diag.err.S -o ent_comio_errids.h
		${RM} -f devices.mca.8ef5.diag.err.S

root/devices.mca.8ef5.diag.err.S: devices.mca.8ef5.diag.err.S
		-${MKDIR} root
		${CP} ${devices.mca.8ef5.diag.err.S:P} root/devices.mca.8ef5.diag.err.S

root/devices.mca.8ef5.diag.trc.S: devices.mca.8ef5.diag.trc.S
		-${MKDIR} root
		${CP} ${devices.mca.8ef5.diag.trc.S:P} root/devices.mca.8ef5.diag.trc.S

.include <${RULES_MK}>
