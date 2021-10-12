# @(#)64        1.7  src/bos/kernext/trace/Makefile, systrace, bos411, 9430C411a 7/20/94 04:07:38
# COMPONENT_NAME:
# 
# FUNCTIONS:
# 
# ORIGINS: 27 83
# 
#  This module contains IBM CONFIDENTIAL code. -- (IBM
#  Confidential Restricted when combined with the aggregated
#  modules for this product)
#                   SOURCE MATERIALS
#  (C) COPYRIGHT International Business Machines Corp. 1985, 1994
#  All Rights Reserved
# 
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#  @BULL_COPYRIGHT@
# HISTORY
# $Log: Makefile,v $
# $EndLog$
# 
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

VPATH		+= :${TARGET_MACHINE}
SUBDIRS		= ${${TARGET_MACHINE}_SUBDIRS}

KERNEL_EXT	= trcdd
IDIR		= /usr/lib/drivers/
ILIST		= trcdd

LOCALCFLAGS	= -DKERNEL -D_KERNEL
ASFLAGS		= -many

KERNEXT_IMPORTS	= sysoverlay.exp kernex.exp syscalls.exp tracecalls.exp debug.exp dead.exp


LIBS		= ${KERNEXT_LIBS} #libsys.a

trcdd_ENTRYPOINT	= trcconfig

# trcdd_OFILES	= trcdata.o trcdd.o trchkutil.o \
#		  ${${TARGET_MACHINE}_OFILES}
trcdd_OFILES	= trcdata.o trcdd.o trchkutil.o \
		  trcgenasm.o trcgenasm_on.o

trcgenasm.o_INCFLAGS    = -I${MAKETOP}/bos/kernel/io/${TARGET_MACHINE} -I${MAKETOP}/bos/kernel/ml -I${MAKETOP}/bos/kernel/ml/${TARGET_MACHINE}
trcgenasm_on.o_INCFLAGS    = -I${MAKETOP}/bos/kernel/io/${TARGET_MACHINE} -I${MAKETOP}/bos/kernel/ml -I${MAKETOP}/bos/kernel/ml/${TARGET_MACHINE}

# .if exists (${TARGET_MACHINE}/machdep.mk)
# .include <${TARGET_MACHINE}/machdep.mk>
# .endif

.include <${RULES_MK}>
