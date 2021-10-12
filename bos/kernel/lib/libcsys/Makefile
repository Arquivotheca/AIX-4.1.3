# @(#)44	1.8  src/bos/kernel/lib/libcsys/Makefile, Makefiles, bos41J, 9514B 4/7/95 16:21:32
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential	Restricted when
# combined with	the aggregated modules for this	product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT	International Business Machines	Corp. 1988, 1994
# All Rights Reserved
#
# US Government	Users Restricted Rights	- Use, duplication or
# disclosure restricted	by GSA ADP Schedule Contract with IBM Corp.
#
VPATH		= ${MAKETOP}bos/usr/ccs/lib/libc:${MAKETOP}bos/usr/ccs/lib/libc/${TARGET_MACHINE}

LIBRARIES	= libcsys.a
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS}

EXPLIB_TARGETS	= export_libcsys.a

EXPDIR		= /kernel/lib/

ILIST		= libcsys.a
IDIR		= /usr/lib/

libcsys.a_OFILES=		\
        a64l.o atoi_sys.o atoi_table.o l64a.o move.o ptrgl.o bcmp.o		\
	bzero.o memccpy.o memchr.o memcmp.o memset.o moveeq.o	\
	fill.o NCisshift_tmp.o strcat.o strchr.o strcmp.o	\
	strcpy.o strcspn.o strlen.o strncat.o strncmp.o	\
	strncpy.o strpbrk.o strrchr.o strspn.o strstr.o	strtok.o \
	clc.o \
	${${TARGET_MACHINE}_SYS_OFILES}

.include <${TARGET_MACHINE}/machdep.mk>

.include <${RULES_MK}>


