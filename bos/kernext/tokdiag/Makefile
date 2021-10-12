# @(#)31        1.9  src/bos/kernext/tokdiag/Makefile, diagddtok, bos41J, 9522A_all 5/30/95 18:50:47
# COMPONENT_NAME: DIAGDDTOK
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

OTHERS          = tok_comio_errids.h  \
		  root/devices.mca.8fc8.diag.err.S \
		  root/devices.mca.8fc8.diag.trc.S

KERNEL_EXT	= tokdiag

IDIR		= /usr/lib/drivers/

ILIST           = tokdiag

LIBS		= ${KERNEXT_LIBS}

tokdiag_ENTRYPOINT	= tokconfig

${tokdiag_OFILES}: tok_comio_errids.h

tokdiag_OFILES	= tokinit.o toklimbo.o tokintr.o \
	tokioctl.o tokdd.o tokclose.o	\
	tokopen.o tokselect.o tokmisc.o tokrw.o	\
	tokrecv.o tokxmit.o tokprim.o tokdwnld.o \
	tokutil.o	

tok_comio_errids.h: devices.mca.8fc8.diag.err.S
		${CP} ${devices.mca.8fc8.diag.err.S:P} .
		${ERRTMPLTBLD} -d devices.mca.8fc8.diag.err.S -o tok_comio_errids.h
		${RM} -f devices.mca.8fc8.diag.err.S

root/devices.mca.8fc8.diag.err.S: devices.mca.8fc8.diag.err.S
		-${MKDIR} root
		${CP} ${devices.mca.8fc8.diag.err.S:P} root/devices.mca.8fc8.diag.err.S

root/devices.mca.8fc8.diag.trc.S: devices.mca.8fc8.diag.trc.S
		-${MKDIR} root
		${CP} ${devices.mca.8fc8.diag.trc.S:P} root/devices.mca.8fc8.diag.trc.S

.include <${RULES_MK}>
