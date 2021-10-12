# @(#)73        1.2  src/bldenv/libodm/Makefile, ade_build, bos412, GOLDA411a 12/4/93 16:07:45
#
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

VPATH		= ${MAKETOP}/bos/usr/ccs/lib/libodm:${MAKETOP}/bos/usr/include:${MAKETOP}/bos/usr/bin/odmget

LIBRARIES	= libodm.a
EXPLIB_TARGETS  = export_libodm.a

INCFLAGS	= -I${MAKETOP}bos/usr/ccs/lib/libodm -I${MAKETOP}bos/usr/bin/odmget

INCLUDES	= odmi.h odmcmd_msg.h odmroutine.h
EXPDIR		= /usr/include/
libodm.a_EXPDIR	= /usr/lib/

OFILES		= invoke.o odmascii.o odmerrmsg.o odmfind.o odmi.o \
		  odmlike.o odmlock.o odmpath.o odmraw.o odmtrace.o \
		  odmvchar.o

.include <${RULES_MK}>
