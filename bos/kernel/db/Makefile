# @(#)19        1.6  src/bos/kernel/db/Makefile, Makefiles, bos41B, 412_41B_sync 12/6/94 14:34:13
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27 83
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
# LEVEL 1,  5 Years Bull Confidential Information

.include <${MAKETOP}bos/kernel/Kernel.mk>

VPATH		=${TARGET_MACHINE}:${MAKETOP}bos/kernel/sys

#		    platform specific preprocessor flags

LOCALCFLAGS	+= -DNATIVE ${PLAT_CFLAGS}

INCFLAGS	= -I${TARGET_MACHINE}

PINNED_OFILES	= \
	vdbprf.o \
	dbbreak.o \
	dbkern.o

DBG_OFILES	= \
	dbtty.o \
	dbstream.o \
	dbdebug.o \
	dbppd.o \
	dbgetcmd.o \
	dbdriver.o \
	dbparse.o \
	dbgetadr.o \
	dbvar.o \
	dbalter.o \
	dbscreen.o \
	dbtrb.o \
	dbxlate.o \
	vdbmemop.o \
	vdbdisp.o \
	vdbsup.o \
	vdbterm.o \
	vdbperr.o \
	proc.o \
	thread.o \
	cbhelps.o \
	vdbfmtu.o \
	vdbfmtut.o \
	vdbfindm.o \
	dbvmm.o \
	dbdev.o \
	dbnet.o \
	${${TARGET_MACHINE}_DBG_OFILES}

INIT_OFILES      = \
	dbinit.o

OBJECTS	= dbg_end.o

.include <${TARGET_MACHINE}/machdep.mk>

.include <${RULES_MK}>
