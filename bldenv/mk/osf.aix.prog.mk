# @(#)09	1.10  src/bldenv/mk/osf.aix.prog.mk, ade_build, bos41J 6/3/94 19:12:10
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
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#  (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
#  ALL RIGHTS RESERVED 
# 
#  OSF/1 1.1
#

.if !defined(_OSF_PROG_MK_)
_OSF_PROG_MK_=

#
#  Build rules
#
.if defined(PROGRAMS) || defined(PROGRAMS++)

_EXPORTS_	= ${${.TARGET}_EXPORTS:U${EXPORTS}}
__EXPORTS__	= ${_EXPORTS_:S/-bE://g}

_IMPORTS_	= ${${.TARGET}_IMPORTS:U${IMPORTS}}
__IMPORTS__	= ${_IMPORTS_:S/-bI://g}

__ENTRY__FLAG__	= -e${${.TARGET}_ENTRYPOINT}
__ENTRY_FLAG__	= ${__ENTRY__FLAG__:S/-e$//}

.if defined(PROGRAMS)

#
# Dependencies on libraries
#
__LIBS__	= ${_LIBS_:!${FINDFILE} ${_LDFLAGS_:M-L*} ${_LIBS_:S/-l/lib/g:=.a}!}

.endif

.if defined(PROGRAMS++)

#
# Dependencies on libraries for C++
#
__LIBS_C__	= ${_LIBS_:!${FINDFILE} ${_C++_LDFLAGS_:M-L*} ${_LDFLAGS_:M-L*} ${_LIBS_:S/-l/lib/g:=.a} libC.a libm.a!}

.endif

${PROGRAMS} : $${_OFILES_} $${__IMPORTS__} $${__EXPORTS__} $${__LIBS__} ${MAKEFILE_DEPENDENCY}
	@if [ -L ${.TARGET}.rmap ]; \
	then \
		${CP} ${.TARGET}.rmap ${.TARGET}.rmap.X; \
		${MV} -f ${.TARGET}.rmap.X ${.TARGET}.rmap; \
	fi
	@if [ -L ${.TARGET}.map ]; \
	then \
		${CP} ${.TARGET}.map ${.TARGET}.map.X; \
		${MV} -f ${.TARGET}.map.X ${.TARGET}.map; \
	fi
	@${RM} ${_RMFLAGS_} ${.TARGET}
	${_CC_} -o ${.TARGET}.X ${_LDFLAGS_} ${_OFILES_} \
	${__ENTRY_FLAG__} \
	${__IMPORTS__:@TMP@-bI:${${TMP}:P}@} \
	${_LIBS_} \
	${__EXPORTS__:@TMP@-bE:${${TMP}:P}@}
	${MV} ${.TARGET}.X ${.TARGET}

${PROGRAMS++} : $${_OFILES_} $${__IMPORTS__} $${__EXPORTS__} $${__LIBS_C__} ${MAKEFILE_DEPENDENCY}
	@if [ -L ${.TARGET}.rmap ]; \
	then \
		${CP} ${.TARGET}.rmap ${.TARGET}.rmap.X; \
		${MV} -f ${.TARGET}.rmap.X ${.TARGET}.rmap; \
	fi
	@if [ -L ${.TARGET}.map ]; \
	then \
		${CP} ${.TARGET}.map ${.TARGET}.map.X; \
		${MV} -f ${.TARGET}.map.X ${.TARGET}.map; \
	fi
	@${RM} ${_RMFLAGS_} ${.TARGET}
	${C++} -o ${.TARGET}.X ${_C++_LDFLAGS_} ${_LDFLAGS_} ${_OFILES_}  \
	${__ENTRY_FLAG__}  \
	${__IMPORTS__:@TMP@-bI:${${TMP}:P}@}  \
	${_LIBS_}  \
	${_C++LIBS_}  \
	${__EXPORTS__:@TMP@-bE:${${TMP}:P}@}
	${MV} ${.TARGET}.X ${.TARGET}
.endif

.endif
