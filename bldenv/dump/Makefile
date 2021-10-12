# @(#)44	1.2  src/bldenv/dump/Makefile, ade_build, bos41J, 9519A_all 4/27/95 10:06:59
#
#   COMPONENT_NAME: ADE_BUILD
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# storclass.h is copied locally because the version of storclass.h currently
# in export/power/bldenv/usr/include is not current with the source version in
# src/bos/usr/include.  dump requires a definition for C_INFO.  when the
# version in export/power/bldenv/usr/include is current with then version
# in src/bos/usr/include the changes made by defect 176323 can be undone.
#

VPATH		= ${MAKETOP}/bos/usr/ccs/bin/dump:${MAKETOP}/bos/usr/include

PROGRAMS	= dump
MSGHDRS		= dump_msg.h
CATFILES	= dump.cat
LOCAL_HFILES	= include/storclass.h
HFILES		= ${MSGHDRS} ${LOCAL_HFILES}

ILIST		= dump
IDIR		= /usr/bin/

OFILES		= archive.o dump.o xcoff.o

LIBS		= -lIN -lld

INCFLAGS	+= -Iinclude

.include <${RULES_MK}>

${LOCAL_HFILES} : $${.TARGET:T}
	${RM} ${_RMFLAGS_} ${.TARGET} ${.TARGET:T}
	[ -d ${.TARGET:H} ] || ${MKDIR} ${.TARGET:H}
	${CP} ${${.TARGET:T}:P} ${.TARGET}
