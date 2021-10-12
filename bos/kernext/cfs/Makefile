# @(#)99        1.3  src/bos/kernext/cfs/Makefile, sysxcfs, bos411, 9428A410j 2/19/93 10:45:33
#
# COMPONENT_NAME: (SYSXCFS) CDROM File System
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

KERNEL_EXT	= cfs.ext

IDIR		= /usr/lib/drivers/
ILIST		= cfs.ext

LIBS		= ${KERNEXT_LIBS}

cfs.ext_ENTRYPOINT	= cdr_config

cfs.ext_IMPORTS		= loadfs.exp debug.exp

LOCALCFLAGS		= -D_VOPS

cfs.ext_OFILES		= cdr_vfsops.o cdr_vnops.o cdr_vmm.o cdr_diriso.o cdr_dirrrg.o cdr_subs.o

.include <${RULES_MK}>
