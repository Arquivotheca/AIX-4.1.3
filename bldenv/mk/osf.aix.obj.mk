# @(#)07        1.12  src/bldenv/mk/osf.aix.obj.mk, ade_build, bos41J 5/3/94 11:44:34
#
#   COMPONENT_NAME: BLDPROCESS
#
#   FUNCTIONS: defined
#		
#
#   ORIGINS: 27,71
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#  (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
#  ALL RIGHTS RESERVED 
# 
#  OSF/1 1.1 Snapshot 2
#

.if !defined(_OSF_OBJ_MK_)
_OSF_OBJ_MK_=

#
# definitions for compilation
#
_ALL_OFILES_=${OFILES:U${PROGRAMS:@.PROG.@${${.PROG.}_OFILES:U${.PROG.}.o}@}} ${OFILES:U${PROGRAMS++:@.PROG++.@${${.PROG++.}_OFILES:U${.PROG++.}.o}@}}
_OFILES_=${OFILES:U${${.TARGET}_OFILES:U${.TARGET}.o}}

#
#  Default double suffix compilation rules
#

.cre.h .cre.c: ${MAKEFILE_DEPENDENCY}
	@if [ -L `${BASENAME} ${.IMPSRC} .cre`.c ]; \
	then \
		${RM} ${_RMFLAGS_} `${BASENAME} ${.IMPSRC} .cre`.c; \
	fi
	@if [ -L `${BASENAME} ${.IMPSRC} .cre`.h ]; \
	then \
		${RM} ${_RMFLAGS_} `${BASENAME} ${.IMPSRC} .cre`.h; \
	fi
	@${CP} ${.IMPSRC} .
	${ODMCREATE} -h -p `${BASENAME} ${.IMPSRC}`
	@${RM} ${_RMFLAGS_} `${BASENAME} ${.IMPSRC}`

.s.o: ${MAKEFILE_DEPENDENCY}
	@${RM} ${_RMFLAGS_} ${.TARGET}
	@${ECHO} "include(sys/asdef.s)" >${.TARGET:.o=.__s}
	${M4} ${_M4FLAGS_} ${.TARGET:.o=.__s} ${.IMPSRC} | ${AS} ${_ASFLAGS_} -o ${.TARGET} || { ${RM} -f ${.TARGET:.o=.__s} ${.TARGET}; ${FALSE};}
	@${ECHO} "traceon" >${.TARGET:.o=.__s}
	@${ECHO} "include(sys/asdef.s)" >>${.TARGET:.o=.__s}
	@${M4} ${_M4FLAGS_} ${.TARGET:.o=.__s} ${.IMPSRC} >/dev/null 2>${.TARGET:.o=.t}
	@${EGREP} '^Trac..*(..*): include' ${.TARGET:.o=.t} | ${SED} -e 's!)$$!!' -e 's!^Trac..*(..*): include(!!p' | ${SORT} -u >${.TARGET:.o=.x}
	@${ECHO} ${.TARGET}": " `${BASENAME} ${.IMPSRC}` >${.TARGET:.o=.u}
	@for x in `${CAT} ${.TARGET:.o=.x}` ; do \
	${ECHO} ${.TARGET}": "`${FINDFILE} ${_GENINC_} ${_CC_INCDIRS_} $$x` >>${.TARGET:.o=.u}; \
	done
	@${RM} ${_RMFLAGS_} ${.TARGET:.o=.__s} ${.TARGET:.o=.t} ${.TARGET:.o=.x}

.c.o: ${MAKEFILE_DEPENDENCY}
	@${RM} ${_RMFLAGS_} ${.TARGET}
	${_CC_} -c ${_CCFLAGS_} ${.IMPSRC}

.oB.o: ${MAKEFILE_DEPENDENCY}
	${RM} -f ${.TARGET}
	${CP} ${.IMPSRC} ${.TARGET}

.C.o: ${MAKEFILE_DEPENDENCY}
	@${RM} ${_RMFLAGS_} ${.TARGET}
	${C++} -c ${_C++FLAGS_} ${.IMPSRC}

.c.i: ${MAKEFILE_DEPENDENCY}
	@${RM} ${_RMFLAGS_} ${.TARGET}
	${_CC_} -P ${_CCFLAGS_} ${.IMPSRC}

.cN.c: ${MAKEFILE_DEPENDENCY}
	@${RM} -f ${.TARGET}
	${BRAND} ${BRANDDICT} <${.IMPSRC} >${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE}; }

.f.o: ${MAKEFILE_DEPENDENCY}
	@${RM} ${_RMFLAGS_} ${.TARGET}
	${_XLF_} ${_XLFFLAGS_} -c ${.IMPSRC}

.y.o: ${MAKEFILE_DEPENDENCY}
	@${RM} ${_RMFLAGS_} ${.TARGET}
	${YACC} ${_YFLAGS_} ${.IMPSRC}
	${_CC_} -c ${_CCFLAGS_} y.tab.c
	-${RM} -f y.tab.c
	${MV} -f y.tab.o ${.TARGET}

.y.c: ${MAKEFILE_DEPENDENCY}
	@${RM} ${_RMFLAGS_} ${.TARGET}
	${YACC} ${_YFLAGS_} ${.IMPSRC}
	${MV} -f y.tab.c ${.TARGET}
	${RM} -f y.tab.h

.if !defined(NO_Y_H_RULE)
.y.h: ${MAKEFILE_DEPENDENCY}
	@${RM} ${_RMFLAGS_} ${.TARGET}
	${YACC} -d ${_YFLAGS_} ${.IMPSRC}
	${MV} -f y.tab.h ${.TARGET}
	${RM} -f y.tab.c
.endif

.l.o: ${MAKEFILE_DEPENDENCY}
	@${RM} ${_RMFLAGS_} ${.TARGET}
	${LEX} ${_LFLAGS_} ${.IMPSRC}
	${_CC_} -c ${_CCFLAGS_} lex.yy.c
	-${RM} -f lex.yy.c
	${MV} -f lex.yy.o ${.TARGET}

.l.c: ${MAKEFILE_DEPENDENCY}
	@${RM} ${_RMFLAGS_} ${.TARGET}
	${LEX} ${_LFLAGS_} ${.IMPSRC}
	${MV} -f lex.yy.c ${.TARGET}

.c.pp: ${MAKEFILE_DEPENDENCY}
	@${RM} ${_RMFLAGS_} ${.TARGET}
	${_CC_} -E ${_CCFLAGS_} ${.IMPSRC} >${.TARGET}

.if defined(OFILES) || defined(PROGRAMS) || defined(PROGRAMS++)
${_ALL_OFILES_} : ${HFILES} ${MAKEFILE_DEPENDENCY}
.endif

.endif
