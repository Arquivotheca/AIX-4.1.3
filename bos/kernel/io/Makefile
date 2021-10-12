# @(#)37        1.10  src/bos/kernel/io/Makefile, sysio, bos411, 9428A410j 12/8/93 14:29:34
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 2, 12 Years Bull Confidential Information
#

# trcdd.o, trcdata.o and trchkutil.o are removed in this Makefile
# New trc_ptr.o and trcgenXX.o are added

.include <${MAKETOP}/bos/kernel/Kernel.mk>

VPATH		=${TARGET_MACHINE}

SUBDIRS		= ${${TARGET_MACHINE}_SUBDIRS}

PINNED_OFILES	= \
	errdd.o dumpdd.o trc_ptr.o trcgenXX.o\
	${${TARGET_MACHINE}_PINNED_OFILES}

PAGED_OFILES	= \
	mem.o sys.o console.o auditdev.o dumpdd_pg.o

PAGED_OFILES_BACK = \
	errdd_pg.o errprobe.o

INIT_OFILES	= \
	ramdd.o rampack.o ramio.o \
	errdd_si.o

trcgenXX.o_INCFLAGS     = -I${TARGET_MACHINE} -I${MAKETOP}/bos/kernel/ml -I${MAKETOP}/bos/kernel/ml/${TARGET_MACHINE}

trc_ptr.o_INCFLAGS      = -I${MAKETOP}/bos/kernel/sys


ramdd.o_INCFLAGS        = -I${MAKETOP}/bos/kernel/vmm/${TARGET_MACHINE}\
                 	  -I${MAKETOP}/bos/kernel/sys/${TARGET_MACHINE}

.include <${TARGET_MACHINE}/machdep.mk>

.include <${RULES_MK}>

