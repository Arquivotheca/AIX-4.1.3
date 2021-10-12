# @(#)23        1.8  src/bldenv/lint/Makefile, ade_build, bos412, GOLDA411a 9/6/94 18:48:46
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

SUBDIRS			= pass2

VPATH			= ${MAKETOP}bos/usr/ccs/bin/lint:${MAKETOP}bos/usr/ccs/bin/common:${MAKETOP}bos/usr/ccs/bin/lint/${TARGET_MACHINE}:${MAKETOP}bos/usr/ccs/lib/libc/${TARGET_MACHINE}:${MAKETOP}bos/usr/ccs/lib:${MAKETOP}bos/usr/ccs/lib/libcurses

PROGRAMS		= lint1
SCRIPTS			= lint
MSGHDRS		 = ctools_msg.h
CATFILES		= ctools.cat

DATAFILES	       = llib-lansi llib-lcrses llib-lm llib-port \
			  llib-lc llib-lcrses32


ANSI_LIST	       = llib-lansi.ln
LLIB_LIST	       = llib-lc.ln llib-port.ln llib-lm.ln llib-lcrses.ln \
			  llib-lcrses32.ln

OTHERS		  = ${ANSI_LIST} ${LLIB_LIST}

ILIST			= lint1 lint ${DATAFILES} ${ANSI_LIST} ${LLIB_LIST}
IDIR			= /usr/lib/
lint_IDIR		= /usr/bin/

INCFLAGS		= -I${MAKETOP}bos/usr/ccs/bin/common
CFLAGS			= -DNEWCALL -DHOSTIEEE -DSDBDEBUG -DSINGLE_PRECISION \
			  -DTWOPASS -D_COMPILER -DLINT

OFILES			= cgram.o code.o comm1.o local.o messages.o optim.o \
			  pftn.o scan.o tftn.o trees.o xdefs.o
HFILES			= ${MSGHDRS} curses.h term.h

LIBS			= 

.include <${RULES_MK}>

TMP = ./tmp

keycaps: caps
	${RM} -rf keycaps
	${GREP} "KEY_" ${caps:P} > keycaps
	@if /usr/bin/tail -1 keycaps | ${GREP} 'KEY_SUNDO' > /dev/null;then :;\
	else ${ECHO} New keys!!! Must change setkeymap.c; exit 1;fi

curses.h: keycaps curses.ed copyright.h compat/curses3.h
	${RM} -rf ${TMP}
	${MKDIR} ${TMP}
	${RM} -f curses.h curses4.h copyright.h
	${CP} ${copyright.h:P} copyright.h
	${ED} - < ${curses.ed:P}
	${ECHO} "#ifndef _AIX32_CURSES\n" > curses.h
	${CAT} curses4.h >> curses.h
	${ECHO} "\n#else /* _AIX32_CURSES */\n" >> curses.h
	${CAT} ${compat/curses3.h:P} >> curses.h
	${ECHO} "\n#endif /* _AIX32_CURSES */" >> curses.h
	${CHMOD} 0664 curses.h
	${RM} -rf  capnames.c tifnames.c tinames.c tnames.c \
         copyright.h curses4.h ${TMP}

term3.h : libcurses/compat/caps compat/maketerm.ex
	${RM} -rf ${TMP}
	${MKDIR} ${TMP}
	${RM} -f term3.h caps.e
	${CP} ${libcurses/compat/caps:P} caps.e
	${CHMOD} u+w caps.e
	${ED} - < ${compat/maketerm.ex:P}
	${RM} -f caps.e

term.h : term3.h caps maketerm.ed copyright.h
	${RM} -rf ${TMP}
	${MKDIR} ${TMP}
	${RM} -f term.h term4.h caps copyright.h
	${CP} ${caps:P} caps
	${CP} ${copyright.h:P} copyright.h
	${ED} - < ${maketerm.ed:P}
	${ECHO} "#ifndef _AIX32_CURSES\n" > term.h
	${CAT} term4.h >> term.h
	${ECHO} "\n#else /* _AIX32_CURSES */\n" >> term.h
	${CAT} term3.h >> term.h
	${ECHO} "\n#endif /* _AIX32_CURSES */" >> term.h
	${CHMOD} 0664 term.h 
	${RM} -rf caps copyright.h term4.h tnames.c tinames.c \
         tifnames.c capnames.c ${TMP}

${ANSI_LIST} : ${.TARGET:.ln=}
	${RM} ${_RMFLAGS_} ${.TARGET}
	${CPP} -E -C -DLINT -Dlint -D_AIX -D_POWER -D_ANSI_C_SOURCE -qlanglvl=ansi ${_CC_INCDIRS_} ${${.TARGET:.ln=}:P} | ${LINT1} -wAP -MA -L${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE}; }

${LLIB_LIST} : term.h curses.h ${.TARGET:.ln=}
	${RM} ${_RMFLAGS_} ${.TARGET}
	${CPP} -E -C -DLINT -Dlint -D_AIX -D_POWER -D_LONG_LONG -I. ${_CC_INCDIRS_} ${${.TARGET:.ln=}:P} |  ${LINT1} -Nn2500 -Nd3000 -L${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE}; }
