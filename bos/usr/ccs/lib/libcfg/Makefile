# @(#)19        1.8  src/bos/usr/ccs/lib/libcfg/Makefile, libcfg, bos41J, 9516A_all 4/17/95 12:13:41
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH	= ${TARGET_MACHINE}:${MAKETOP}/bos/objclass:${MAKETOP}/bos/kernel/exp:${MAKETOP}/bos/usr/lib/methods/common

SHARED_LIBRARIES	= libcfg.a
MSGHDRS         =
CATFILES        =
HFILES          = ${MSGHDRS}

EXPLIB_TARGETS  = export_libcfg.a

ILIST           = libcfg.a
IDIR            = /usr/lib/

LIBS		= libc.a libodm.a
IMPORTS		= -bI:syscalls.exp
EXPORTS		= -bE:libcfg.exp ${${TARGET_MACHINE}_EXPORTS}

SHARED_OFILES	= \
	concurn.o cfgodm.o getattr.o attrval.o \
	cfdefmsg.o findmcode.o cfgresid.o\
	${${TARGET_MACHINE}_SHARED_OFILES}

INCFLAGS	= -I${TARGET_MACHINE}     \
		  -I${MAKETOP}/bos/usr/lib/methods/common


.include <${TARGET_MACHINE}/machdep.mk>

.include <${RULES_MK}>

cfgodm.c: cfgodm.cre
	${RM} -f ${.TARGET} ${.TARGET:.c=.h}
	${CP} ${cfgodm.cre:P} .
	${ODMCREATE} -h -p cfgodm.cre
	${RM} -f cfgodm.cre
