# @(#)72        1.6  src/bos/usr/bin/adb/Makefile, cmdadb, bos411, 9428A410j 6/1/94 14:51:36
# COMPONENT_NAME:
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

VPATH		= ${TARGET_MACHINE}:${MAKETOP}/bos/usr/ccs/lib/libdbx/${TARGET_MACHINE}
PROGRAMS	= adb
MSGHDRS		= adb_msg.h
CATFILES	= adb.cat
HFILES		= ${MSGHDRS}

ILIST		= adb
IDIR		= /usr/bin/

INCFLAGS	= \
-I${TARGET_MACHINE} \
-I${MAKETOP}/bos/usr/ccs/lib/libdbx/${TARGET_MACHINE} \

OFILES		= \
	access.o command.o disassembly.o expr.o extern.o format.o input.o machdep.o \
	main.o opcode.o ops.o output.o pcs.o print.o runpcs.o setup.o steps.o \
	sym.o

LIBS		= -lld

.include <${RULES_MK}>
