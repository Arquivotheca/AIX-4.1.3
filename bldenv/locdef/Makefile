# @(#)26	1.15  src/bldenv/locdef/Makefile, bldprocess, bos412, GOLDA411a 5/4/94 10:48:36
#
# COMPONENT_NAME: CMDNLS
#
# FUNCTIONS: Build the locdef tool for the locales from the localedef sources.
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/usr/bin/localedef:${MAKETOP}/bos/usr/ccs/lib/libc:${MAKETOP}/bos/kernel/sys:${MAKETOP}/bos/usr/include/sys:${MAKETOP}/bos/usr/include
PROGRAMS	= locdef
MSGHDRS		= localedef_msg.h
CATFILE		= localedef.cat
HFILES		= ${MSGHDRS} err.h list.h locdef.h method.h semstack.h symtab.h local_includes 
INCFLAGS	= -I${MAKETOP}bos/usr/bin/localedef -I./includes
CFLAGS		= -DTMPMETHPATH
OFILES		= err.o gen.o gram.o scan.o list.o localedef.o method.o \
		  sem_chr.o sem_ctype.o sem_xlat.o semstack.o symtab.o \
		  sem_coll.o init.o check.o sem_method.o copy.o __method.o \
		  fwdrefstack.o \
		  __mbtowc_sb.o __mbtowc_932.o __mbtowc_eucjp.o \
		  __wcwidth_latin.o __wcwidth_932.o __wcwidth_eucjp.o 

ILIST		= locdef
IDIR		= /usr/bin/

.include <${RULES_MK}>

scan.o : gram.h

local_includes : localedef.h localedef31.h lc_core.h lc_sys.h ctype.h limits.h
	${TOUCH} ${.TARGET}
	${RM} -fr includes
	${MKDIR} -p includes/sys
	${CP} ${localedef.h:P} includes/sys
	${CP} ${localedef31.h:P} includes/sys
	${CP} ${lc_core.h:P} includes/sys
	${CP} ${lc_sys.h:P} includes/sys
	${CP} ${limits.h:P} includes/sys
	${CP} ${ctype.h:P} includes
