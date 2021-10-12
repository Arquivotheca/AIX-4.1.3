# @(#)87        1.4  src/bos/usr/lbin/tty/crash/Makefile, cmdtty, bos411, 9428A410j 4/25/94 10:45:10
#
# COMPONENT_NAME: (cmdtty) System Extension for tty/crash
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# LEVEL 1, 5 Years Bull Confidential Information
#
#-------------------------------------------------------------------------
# Makefile for common tty style loadable device drivers and line disciplines
#-------------------------------------------------------------------------
#-------------------------------------------------------------------------
#       Makefile for loadable device drivers
#-------------------------------------------------------------------------

#	Make file for tty crash.
#

VPATH		= ${TARGET_MACHINE}:${MAKETOP}/bos/kernext/tty:${MAKETOP}/bos/kernext/pty:${MAKETOP}/bos/kernext/pse:

TTYDIR=/usr/lbin/tty
CRASH_PREFIX=crash-

CFLAGS	= '-DTTYDIR="$(TTYDIR)"' '-DCRASH_PREFIX="$(CRASH_PREFIX)"' \
		-DTTYDBG -DFOR_CRASH -DIN_TTYDBG

	
OBJECTS		= unpack.o
PROGRAMS	= tty crash-sptr crash-ldterm crash-nls crash-ptydd \
			crash-shtty \
			${${TARGET_MACHINE}_PROGRAMS}

tty_OFILES		= tty.o ${OBJECTS}
crash-sptr_OFILES	= crash-sptr.o sptr_db.o ${OBJECTS}
crash-ldterm_OFILES	= crash-ldterm.o ldtty_db.o ${OBJECTS}
crash-nls_OFILES	= crash-nls.o snls_db.o ${OBJECTS}
crash-ptydd_OFILES	= crash-ptydd.o spty_db.o ${OBJECTS}
crash-shtty_OFILES	= crash-shtty.o str_tty_db.o ${OBJECTS}

INCDIRS:=${INCDIRS:S;/usr/include$;/kernel/include;g}

INCFLAGS	= -I${MAKETOP}/bos/kernel -I${MAKETOP}/bos/kernext/tty \
			-I${MAKETOP}/bos/kernext/pty

LIBS		= -lIN -lc -lPW

ILIST		= ${PROGRAMS}

tty_IDIR	= /usr/lib/ras/dmprtns/
IDIR		= /usr/lbin/tty/

.include <${TARGET_MACHINE}/machdep.mk>

.include <${RULES_MK}>
