# @(#)65        1.4  src/bldenv/libiconv/Makefile, ade_build, bos412, GOLDA411a 1/19/94 18:33:16
#
#   COMPONENT_NAME: ADE_BUILD
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1988, 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

VPATH		= ${MAKETOP}bos/usr/ccs/lib/libiconv:${MAKETOP}bos/usr/include:${MAKETOP}bos/kernel/sys

LIBRARIES	= libiconv.a
EXPLIB_TARGETS	= export_libiconv.a

LOCAL_HFILES 	= includes/iconv.h includes/iconvTable.h includes/iconvP.h \
		  includes/iconv932.h includes/ccsid.h includes/fcs.h \
		  includes/hcs.h includes/sys/lc_core.h
HFILES		= ${LOCAL_HFILES}

ILIST		= libiconv.a
IDIR		= /usr/lib/

CFLAGS		= -DAIXV3AR
INCFLAGS	= -Iincludes

OFILES		= iconv.o ascii.o fcs.o hcs.o ccsid.o

.include <${RULES_MK}>

${LOCAL_HFILES} : $${.TARGET:T}
	${RM} ${_RMFLAGS_} ${.TARGET} ${.TARGET:T}
	[ -d ${.TARGET:H} ] || ${MKDIR} ${.TARGET:H}
	${CP} ${${.TARGET:T}:P} ${.TARGET}
