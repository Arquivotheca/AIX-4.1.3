# @(#)12        1.9  src/bos/kernext/ientdiag/Makefile, diagddient, bos41J, 9522A_all 5/30/95 18:50:55
# COMPONENT_NAME: DIAGDDIENT
#
# FUNCTIONS:
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

OTHERS          = ient_comio_errids.h  \
		  root/devices.mca.8ef2.diag.com.err.S \
		  root/devices.mca.8ef2.diag.com.trc.S

KERNEL_EXT	= ethdiag

IDIR		= /usr/lib/drivers/

ILIST           = ethdiag

LIBS			= ${KERNEXT_LIBS}

ethdiag_ENTRYPOINT      = cioconfig

${ethdiag_OFILES}: ient_comio_errids.h

ethdiag_OFILES = \
        i_ent.o         \
        i_entrw.o       \
        i_ciodd.o       \
        quickrev.o

cinval.o_INCFLAGS      = -I${MAKETOP}/bos/kernel/ml/${TARGET_MACHINE}

ient_comio_errids.h: devices.mca.8ef2.diag.com.err.S
		${CP} ${devices.mca.8ef2.diag.com.err.S:P} .
		${ERRTMPLTBLD} -d devices.mca.8ef2.diag.com.err.S -o ient_comio_errids.h
		${RM} -f devices.mca.8ef2.diag.com.err.S

root/devices.mca.8ef2.diag.com.err.S: devices.mca.8ef2.diag.com.err.S
		-${MKDIR} root
		${CP} ${devices.mca.8ef2.diag.com.err.S:P} root/devices.mca.8ef2.diag.com.err.S

root/devices.mca.8ef2.diag.com.trc.S: devices.mca.8ef2.diag.com.trc.S
		-${MKDIR} root
		${CP} ${devices.mca.8ef2.diag.com.trc.S:P} root/devices.mca.8ef2.diag.com.trc.S

.include <${RULES_MK}>
