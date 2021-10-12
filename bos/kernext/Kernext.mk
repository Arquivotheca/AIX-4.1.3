# @(#)84 1.16 src/bos/kernext/Kernext.mk, Makefiles, bos411, 9436B411a 9/6/94 17:06:09
#
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
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
# LEVEL 1, 5 Years Bull Confidential Information
#

.if (${LPP} == "bos") || (${LPP}  == "rspc")
LITECFLAGS = -qproclocal=i_disable:disable_lock:i_enable:unlock_enable:simple_lock:simple_unlock:fetch_and_add
LITEIMPORTS = sysoverlay.exp
.endif

INCDIRS:=${INCDIRS:S;/usr/include$;/kernel/include;g}
LIBDIRS:=${LIBDIRS:S;/usr/ccs/lib$;/kernel/lib;g}

VPATH	=${MAKETOP}bos/kernel/exp:${MAKETOP}bos/kernel/exp/${TARGET_MACHINE}:${MAKETOP}bos/kernext/exp:

CFLAGS	= -D_KERNEL -D_POWER_MP ${LOCALCFLAGS} ${LITECFLAGS}
M4FLAGS += -D_POWER_MP

.if defined(KDB) && ${KDB} == "-D_KDB"
KDBEXTS = ${MAKETOP}bos/kernel/kdb/kdb.exp
.else
KDBEXTS =
.endif

KERNEXT_IMPORTS	= ${LITEIMPORTS} kernex.exp syscalls.exp dead.exp streams.exp ${KDBEXTS}
KERNEXT_LIBS	= libsys.a libsysp.a libcsys.a

LDFLAGS_KERNEXT	= ${STRIP_FLAG} \
		-bloadmap:${.TARGET}.out -bR:${.TARGET}.map

_LDFLAGS_KERNEXT_=${${.TARGET}_LDFLAGS_KERNEXT:U${LDFLAGS_KERNEXT}}

KERNEXT_BUILD=yes
