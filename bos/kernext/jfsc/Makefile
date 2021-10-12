# @(#)71	1.4  src/bos/kernext/jfsc/Makefile, sysxjfsc, bos411, 9428A410j 6/9/94 08:11:30
#
# COMPONENT_NAME: SYSXJFSC (JFS Compression)
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT	= comp.kext comp.uext
IDIR		= /sbin/
ILIST		= comp.kext comp.uext

LOCALCFLAGS	= -DKERNEL -D_KERNEL
ASFLAGS		= -many

KERNEXT_IMPORTS = sysoverlay.exp kernex.exp syscalls.exp tracecalls.exp debug.exp dead.exp

LIBS			= ${KERNEXT_LIBS} libsys.a libcsys.a
comp.kext_ENTRYPOINT	= compress_config
comp.kext_OFILES	= comp.kext.o
comp.kext_IMPORTS	= -bI:comp.uext.exp

comp.uext_ENTRYPOINT	= getcompent
comp.uext_OFILES	= lzasm.o comp.uext.o
comp.uext_EXPORTS	= -bE:comp.uext.exp

.include <${RULES_MK}>
