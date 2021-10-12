# @(#)20	1.6  src/bldenv/lex/Makefile, ade_build, bos412, GOLDA411a 4/7/94 12:29:00
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

VPATH			= ${MAKETOP}bos/usr/ccs/bin/lex:${MAKETOP}bos/usr/ccs/lib/libl:${MAKETOP}bos/usr/ccs/lib/libc:${MAKETOP}/bos/kernel:${MAKETOP}/bos/usr/include

PROGRAMS		= lex
MSGHDRS                 = lex_msg.h
CATFILES                = lex.cat

ILIST			= lex ncform
IDIR			= /usr/bin/
ncform_IDIR    		= /usr/ccs/lib/lex/

INCFLAGS		= -I${MAKETOP}bos/usr/ccs/bin/lex -I./includes

HFILES			= ${MSGHDRS}  hash.h ldefs.h once.h local_includes

OFILES			= header.o main.o parser.o sub1.o sub2.o colval.o
YFILES			= parser.y

LIBS			= -lld
GARBAGE			=


local_includes : stdlib.h sys/localedef.h sys/lc_core.h sys/lc_sys.h ctype.h
	${TOUCH} ${.TARGET}
	${RM} -fr includes
	${MKDIR} -p includes/sys
	${CP} ${stdlib.h:P} includes
	${CP} ${sys/localedef.h:P} includes/sys
	${CP} ${sys/lc_core.h:P} includes/sys
	${CP} ${sys/lc_sys.h:P} includes/sys
	${CP} ${ctype.h:P} includes

.include <${RULES_MK}>
