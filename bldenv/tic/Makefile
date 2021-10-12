# @(#)95	1.6  src/bldenv/tic/Makefile, libcurses, bos412, GOLDA411a 5/16/94 08:25:48
#
# COMPONENT_NAME: LIBCURSES
#
# FUNCTIONS:
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

LIBCURSES	= ${MAKETOP}bos/usr/ccs/lib/libcurses
VPATH		= ${MAKETOP}bos/usr/ccs/lib:${LIBCURSES}

PROGRAMS	= tic
MSGHDRS		= tic_msg.h
CATFILES	= tic.cat
HFILES		= ${MSGHDRS} curses.h

OFILES		= tic_captab.o tic_error.o tic_hash.o tic_main.o tic_parse.o \
		  tic_read.o tic_scan.o tnames.o tinames.o tifnames.o
INCFLAGS        = -I${LIBCURSES}
 
LIBS		= -ls

ILIST		= ${PROGRAMS}
IDIR		= /usr/bin/

.include <${RULES_MK}>

TMP = ./tmp

tnames.c tinames.c tifnames.c: term.h

caps: libcurses/caps
	${CP} ${libcurses/caps:P} caps

copyright.h: libcurses/copyright.h
	${CP} ${libcurses/copyright.h:P} copyright.h

keycaps: caps
	${RM} -rf ${TMP}/keycaps keycaps
	${GREP} "KEY_" caps > keycaps
	@if ${TAIL} -1 keycaps | ${GREP} 'KEY_SUNDO' > /dev/null;then :; \
	else ${ECHO} New keys!!! Must change setkeymap.c; \
	     ${RM} -f keycaps; \
	     exit 1; \
	fi

copyright.h: libcurses/copyright.h
	${CP} ${libcurses/copyright.h:P} copyright.h

curses.h: keycaps curses.ed copyright.h
	${RM} -rf ${TMP}
	${MKDIR} ${TMP}
	${RM} -f curses.h curses4.h
	${ED} - < ${curses.ed:P}
	${CAT} curses4.h > curses.h
	${CHMOD} 0664 curses.h

term.h : caps maketerm.ed copyright.h
	${RM} -rf ${TMP}
	${MKDIR} ${TMP}
	${RM} -f term.h term4.h
	${ED} - < ${maketerm.ed:P}
	${CAT} term4.h > term.h
	${CHMOD} 0664 term.h tnames.c tinames.c tifnames.c
