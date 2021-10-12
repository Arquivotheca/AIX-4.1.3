# @(#)65	1.5  src/bldenv/tr/Makefile, ade_build, bos412, GOLDA411a 4/6/94 11:19:19
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
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/usr/bin/tr:${MAKETOP}/bos/usr/ccs/lib/libc:${MAKETOP}/bos/kernel:${MAKETOP}/bos/usr/include


PROGRAMS	= tr
MSGHDRS		= tr_msg.h
CATFILES	= tr.cat
HFILES		= ${MSGHDRS} local_includes
INCFLAGS	= -I./includes

ILIST		= tr
IDIR		= /usr/bin/

local_includes	: stdlib.h sys/localedef.h sys/lc_core.h sys/lc_sys.h ctype.h patlocal.h
	${TOUCH} ${.TARGET}
	${RM} -fr includes
	${MKDIR} -p includes/sys
	${CP} ${stdlib.h:P} includes
	${CP} ${sys/localedef.h:P} includes/sys
	${CP} ${sys/lc_core.h:P} includes/sys
	${CP} ${sys/lc_sys.h:P} includes/sys
	${CP} ${ctype.h:P} includes
	${CP} ${patlocal.h:P} includes

.include <${RULES_MK}>
