# @(#)05	1.15  src/bldenv/mk/osf.aix.lib.mk, ade_build, bos41J 7/1/94 18:55:01
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

.if !defined(_OSF_LIB_MK_)
_OSF_LIB_MK_=

#
#  Build rules
#
.if defined(LIBRARIES)

STRIP_FLAG=

${LIBRARIES} : $${.TARGET}($${_OFILES_}) ${MAKEFILE_DEPENDENCY}
	${AR} ${DEF_ARFLAGS} ${.TARGET} ${.OODATE}
	[ ! -f ${.TARGET} ] || ${TOUCH} ${.TARGET}
.if !defined(D16S)
	${RM} -f ${.OODATE}
.endif
.endif

.if defined(SHARED_LIBRARIES)

SHARED_OFILE_NAME?=shr.o

${SHARED_LIBRARIES} : $${.TARGET}(${NOSHARED_OFILES} ${SHARED_OFILE_NAME} ${SHARED_OFILE_LIST}) ${MAKEFILE_DEPENDENCY}
	${RM} -f ${.TARGET}
	${AR} ${DEF_ARFLAGS} ${.TARGET} ${NOSHARED_OFILES} ${SHARED_OFILE_NAME} ${SHARED_OFILE_LIST} 


__LIBS__	=${LIBS:!${FINDFILE} ${LIBDIRS} ${LIBS}!}
__LIBI__	=${LIBIMPORTS:!${FINDFILE} ${LIBDIRS} ${LIBIMPORTS}!}
__LIB_S__	=${LIBS:S/^lib/-l/g:S/.a$//g}

_EXPORTS_       =${${.TARGET}_EXPORTS:U${EXPORTS}}
__EXPORTS__     =${_EXPORTS_:S/-bE://g}

_IMPORTS_       =${${.TARGET}_IMPORTS:U${IMPORTS}}
__IMPORTS__     =${_IMPORTS_:S/-bI://g}

${SHARED_OFILE_NAME} : ${SHARED_OFILES} $${__IMPORTS__} $${__EXPORTS__} ${__LIBS__} ${__LIBI__}
	${LD} ${_SHLDFLAGS_} -bM:Sre -o ${.TARGET} ${SHARED_OFILES} \
	${__LIBI__} \
	${__IMPORTS__:@TMP@-bI:${${TMP}:P}@} \
	${__LIB_S__} ${LIBDIRS} -blibpath:/usr/lib:/lib \
	${__EXPORTS__:@TMP@-bE:${${TMP}:P}@}

${SHARED_OFILES} : ${HFILES}
.endif

.endif
