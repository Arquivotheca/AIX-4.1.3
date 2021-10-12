# @(#)10 1.3 src/bos/usr/lbin/tty/stty/Makefile, Makefiles, bos411, 9428A410j 3/22/94 18:07:42
#
# COMPONENT_NAME: (CMDTTY)
#
# FUNCTIONS: makefile for stty
#
# ORIGINS: 27, 83
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
# LEVEL 1, 5 Years Bull Confidential Information
#

VPATH		= ${TARGET_MACHINE}

TTYDIR		=/usr/lbin/tty
STTY_PREFIX	=stty-

PROGRAMS	= ${${TARGET_MACHINE}_PROGRAMS}
MSGHDRS		= ${${TARGET_MACHINE}_MSGHDRS}
CATFILES	= ${${TARGET_MACHINE}_CATFILES}
HFILES		= ${MSGHDRS}

ILIST		= ${PROGRAMS}
IDIR		= /usr/lbin/tty/

CFLAGS		= '-DTTYDIR="${TTYDIR}"' '-DSTTY_PREFIX="${STTY_PREFIX}"' \
                  ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS       += ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

LIBS		= -lcurses

.include <${TARGET_MACHINE}/machdep.mk>

.include <${RULES_MK}>
