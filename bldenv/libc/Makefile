# @(#)21        1.8  src/bldenv/libc/Makefile, ade_build, bos412, GOLDA411a 4/25/94 11:19:40
#
#   COMPONENT_NAME: BOSBUILD
#
#   FUNCTIONS:
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#   This Makefile is used to build crt0.o and crt0_r.o
#
#   THREAD_SAFE is defined if building crt0_r.o, see src/bldenv/libc_r for 
#   how THREAD_SAFE gets defined

#
# common between crt0.o and crt0_r.o
#
LIBC_PATH               = ${MAKETOP}/bos/usr/ccs/lib/libc
LIBC_R_PATH             = ${MAKETOP}/bos/usr/ccs/lib/libc_r
LIBM_PATH               = ${MAKETOP}/bos/usr/ccs/lib/libm
KERN_EXP_PATH           = ${MAKETOP}/bos/kernel/exp
KERN_EXT_EXP_PATH       = ${MAKETOP}/bos/kernext/exp

#
# For thread safe crt0, add _r
#
.if defined(THREAD_SAFE)
LIBC            = libc_r.a
CRT0            = crt0_r.o
GCRT0           = gcrt0_r.o
MCRT0           = mcrt0_r.o
THREADS_INIT    = __threads_init.o
.else
LIBC            = libc.a
CRT0            = crt0.o
GCRT0           = gcrt0.o
MCRT0           = mcrt0.o
THREADS_INIT    =
.endif

#
# thread_safe specific
#
.if defined (THREAD_SAFE)
INCFLAGS        = -I${LIBC_PATH} -I${LIBC_PATH}/${TARGET_MACHINE} -Iincludes
VPATH           = ${LIBC_PATH}:${LIBC_PATH}/${TARGET_MACHINE}:${LIBC_R_PATH}:${LIBM_PATH}:${LIBM_PATH}/${TARGET_MACHINE}:${KERN_EXP_PATH}:${KERN_EXT_EXP_PATH}:${MAKETOP}bos/kernel/sys/${TARGET_MACHINE}:${MAKETOP}bos/usr/include
CFLAGS          = -D_THREAD_SAFE
M4FLAGS         = -D_THREAD_SAFE
#
# crt0.o specific
#
.else
INCFLAGS        = -I${LIBC_PATH} -I${LIBC_PATH}/${TARGET_MACHINE} -Iincludes
VPATH           = ${LIBC_PATH}:${LIBC_PATH}/${TARGET_MACHINE}:${LIBM_PATH}:${LIBM_PATH}/${TARGET_MACHINE}:${KERN_EXP_PATH}:${KERN_EXT_EXP_PATH}:${MAKETOP}bos/kernel/sys/${TARGET_MACHINE}:${MAKETOP}bos/usr/include
.endif

.if defined (THREAD_SAFE)
MSGHDRS		= libc_msg.h
INCLUDES	= libc_msg.h
OTHERS		= ${LOCAL_HFILES} target.file ${CRT0} ${GCRT0}
ILIST		= ${CRT0} ${GCRT0}
.else
MSGHDRS		= libc_msg.h
INCLUDES	= libc_msg.h
libc_msg.h_EXPDIR = /usr/include/
LOCAL_HFILES	= sys/asdef.s sys/comlink.m4
OTHERS		=${LOCAL_HFILES} target.file ${CRT0} glink.o ${GCRT0}
ILIST		= ${CRT0} glink.o ${GCRT0}
.endif

BINARIES	= yes

IDIR		= /usr/lib/

.include <${RULES_MK}>

CRT0LDFLAGS	= -r -D0 -b'loadmap:${.PREFIX}.map' -b'R:${.PREFIX}.rmap'

crt0msg.o : libc_msg.h includes/nl_types.h
__mcount.o	: libc_msg.h includes/nl_types.h
__mcount_construct.o	: libc_msg.h includes/nl_types.h

${LOCAL_HFILES} includes/nl_types.h : $${.TARGET:T}
	${RM} ${_RMFLAGS_} ${.TARGET}
	[ -d ${.TARGET:H} ] || ${MKDIR} ${.TARGET:H}
	${CP} ${${.TARGET:T}:P} ${.TARGET}

CRT0LDFLAGS = -r -bloadmap:${.PREFIX}.out -bR:${.PREFIX}.map
_CRT0LDFLAGS_ = ${CRT0LDFLAGS} ${CRT0LDENV} ${CRT0LDARGS}

${CRT0}: crt0main.o crt0msg.o ${THREADS_INIT}
	${LD} ${_CRT0LDFLAGS_} -o ${.TARGET} ${.ALLSRC}

${GCRT0}: pgcrt0.o __mcount.o __mcount_construct.o _stk.o crt0msg.o _getcall.o _getcall2.o ${THREADS_INIT}
	${LD} ${_CRT0LDFLAGS_} -o ${.TARGET} ${.ALLSRC}

target.file     : crt0main.s
		${RM} -f ${.TARGET}
		${CP} ${.ALLSRC} .
		${TOUCH} ${.TARGET}

