# @(#)23        1.1  src/bos/kernext/psla/Makefile, sysxpsla, bos411, 9428A410j 8/5/92 18:12:18
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

KERNEL_EXT	= psladd
IDIR		= /usr/lib/drivers/
ILIST		= psladd

LIBS		= ${KERNEXT_LIBS} libIN.a libc.a libPW.a

psladd_ENTRYPOINT	= gswconfig

psladd_OFILES	=	\
		gsw13.o gsw23.o	gsw33.o	gswintr.o


.include <${RULES_MK}>
