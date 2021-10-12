# @(#)18	1.3  src/bos/kernext/mpqp/Makefile, Makefiles, bos411, 9436B411a 9/6/94 00:16:47
# COMPONENT_NAME:
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

CFLAGS	       += -D_POWER_MPQP
M4FLAGS	       += -D_POWER_MPQP
OTHERS          = root/devices.mca.8f70.mpqp.trc.S
KERNEL_EXT	= mpqpdd
IDIR		= /usr/lib/drivers/
ILIST		= mpqpdd

LIBS		= ${KERNEXT_LIBS}

mpqpdd_ENTRYPOINT	= mpqconfig

mpqpdd_OFILES = \
	mpqconfig.o mpqopen.o mpqclose.o mpqread.o mpqwrite.o	\
	mpqioctl.o mpqintr.o mpqoffl.o mpqselect.o mpqmpx.o	\
	mpqutil.o mpqiocfn.o	


root/devices.mca.8f70.mpqp.trc.S: devices.mca.8f70.mpqp.trc.S
	-${MKDIR} root
	${CP} ${devices.mca.8f70.mpqp.trc.S:P} root/devices.mca.8f70.mpqp.trc.S

.include <${RULES_MK}>
