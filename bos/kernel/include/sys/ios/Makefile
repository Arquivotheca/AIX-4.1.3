# @(#)32	1.2  src/bos/kernel/include/sys/ios/Makefile, sysios, bos411, 9428A410j 1/26/93 09:26:54
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 10,27
#
# IBM CONFIDENTIAL -- (IBM Confidential	Restricted when
# combined with	the aggregated modules for this	product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT	International Business Machines	Corp. 1988, 1993
# All Rights Reserved
#
# US Government	Users Restricted Rights	- Use, duplication or
# disclosure restricted	by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/kernel/ios:${MAKETOP}/bos/kernel/ios/${TARGET_MACHINE}

IDIR		= /kernel/include/sys/ios/

DATAFILES	=

INCLUDES	= ${DATAFILES} interrupt.h

ILIST		=

.if exists (${TARGET_MACHINE}/machdep.mk)
.include <${TARGET_MACHINE}/machdep.mk>
.endif

.include <${RULES_MK}>
