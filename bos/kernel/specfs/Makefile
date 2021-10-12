# @(#)57        1.2  src/bos/kernel/specfs/Makefile, sysspecfs, bos411, 9428A410j 10/27/93 16:50:20
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

PINNED_OFILES	=	\
	pdevsubs.o

PAGED_OFILES_PRIM =	\
	devsubs.o

PAGED_OFILES	=	\
	fifo_vnops.o	\
	gno_fops.o	\
	spec_vnops.o	\
	specsubs.o	\
	syscalls.o

PAGED_OFILES_BACK =	\
	bdev_subr.o	\
	cdev_subr.o	\
	fifo_init.o	\
	mpx_subr.o	\
	spec_init.o

.include <${RULES_MK}>
