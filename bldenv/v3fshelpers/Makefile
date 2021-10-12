# @(#)68        1.3  src/bldenv/v3fshelpers/Makefile, ade_build, bos412, GOLDA411a 7/20/94 16:12:15
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

VPATH		= ${MAKETOP}/bos/sbin/helpers/v3fshelpers:${MAKETOP}/bos/usr/ccs/lib/libIN

EXPINC_SUBDIRS  = libfs
EXPLIB_SUBDIRS  = libfs

PROGRAMS	= v3fshelper
MSGHDRS		= op_namei_msg.h op_check_msg.h op_make_msg.h op_extend_msg.h
CATFILES	= op_namei.cat op_check.cat op_make.cat op_extend.cat
HFILES		= ${MSGHDRS}
LIBS		= -lfs

ILIST		= v3fshelper
IDIR		= /sbin/helpers/

OFILES		= fshelper.o op_statfs.o op_make.o op_namei.o \
		  op_debug.o op_extend.o op_check.o bufpool.o CSlen.o

INCFLAGS	= -I${MAKETOP}/bos/sbin/helpers/v3fshelpers

.include <${RULES_MK}>

LLSEEK_EXIST	!=/usr/bin/nm /usr/lib/libc.a | ${GREP} llseek >/dev/null; \
		[ $$? -eq 0 ] || ${ECHO} "-D_BLD"

CFLAGS		=${LLSEEK_EXIST}
