# @(#)40        1.2  src/bos/kernel/ipc/Makefile, sysipc, bos411, 9428A410j 10/27/93 16:29:01
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernel/Kernel.mk>

VPATH		=${TARGET_MACHINE}


PAGED_OFILES	=	\
	ipc.o 		\
	msg.o 		\
	sem.o		\
	shm.o		\
	shm_mem.o

PAGED_OFILES_BACK =	\
	segoff.o

.include <${RULES_MK}>
