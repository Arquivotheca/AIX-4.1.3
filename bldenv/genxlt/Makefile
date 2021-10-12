# @(#)09        1.4  src/bldenv/genxlt/Makefile, ade_build, bos412, GOLDA411a 1/19/94 18:34:38
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

VPATH		= ${MAKETOP}bos/usr/bin/genxlt:${MAKETOP}bos/usr/include

PROGRAMS	= genxlt
OFILES		= genxlt.o genccsid.o
MSGHDRS		= genxlt_msg.h
CATFILES	= genxlt.cat
HFILES		= ${MSGHDRS}
LOCAL_HFILES	= includes/ccsid.h includes/iconvTable.h includes/nl_types.h

INCFLAGS	= -Iincludes

ILIST		= ${PROGRAMS}
IDIR		= /usr/bin/

genxlt : genxlt.o genccsid.o

genccsid.o : ${LOCAL_HFILES}

.include <${RULES_MK}>

${LOCAL_HFILES} : $${.TARGET:T}
	${RM} ${_RMFLAGS_} ${.TARGET}
	[ -d ${.TARGET:H} ] || ${MKDIR} ${.TARGET:H}
	${CP} ${${.TARGET:T}:P} ${.TARGET}
