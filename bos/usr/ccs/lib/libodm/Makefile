# @(#)18        1.4  src/bos/usr/ccs/lib/libodm/Makefile, libodm, bos411, 9428A410j 11/9/93 17:58:40
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



SHARED_LIBRARIES	= libodm.a
MSGHDRS         =
CATFILES        =
HFILES          = ${MSGHDRS}

EXPLIB_TARGETS  = export_libodm.a

ILIST           = libodm.a
IDIR            = /usr/lib/

LIBS		= libc.a
EXPORTS		= -bE:libodm.exp

SHARED_OFILES	= \
	invoke.o odmascii.o odmerrmsg.o odmfind.o odmi.o \
	odmlike.o odmlock.o odmpath.o odmraw.o odmtrace.o \
	odmvchar.o

.include <${RULES_MK}>
