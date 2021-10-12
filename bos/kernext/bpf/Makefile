# @(#)83	1.2  src/bos/kernext/bpf/Makefile, sysxbpf, bos411, 9428A410j 6/9/94 08:09:06
# COMPONENT_NAME: SYSXBPF
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT	= bpf
IDIR		= /usr/lib/drivers/
ILIST		= bpf

LIBS		= ${KERNEXT_LIBS}

bpf_ENTRYPOINT	= bpfconfig

bpf_OFILES	= bpfdd.o bpf_filter.o bpf.o

KERNEXT_IMPORTS = sysoverlay.exp kernex.exp syscalls.exp statcmd.exp

LOCALCFLAGS	= -DCOMPAT_43 -DIP_MULTICAST

.include <${RULES_MK}>
